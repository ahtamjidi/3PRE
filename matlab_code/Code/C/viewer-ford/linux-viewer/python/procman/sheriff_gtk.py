import re
import sys
import sets
import time
import traceback

import gobject
import gtk
import pango

from lcm import LCM
from lcmtypes.pmsd_deputy_cmd_t import pmsd_deputy_cmd_t
from lcmtypes.pmsd_info_t import pmsd_info_t
from lcmtypes.pmsd_orders_t import pmsd_orders_t
from lcmtypes.pmsd_printf_t import pmsd_printf_t
from lcmtypes.pmsd_sheriff_cmd_t import pmsd_sheriff_cmd_t
import sheriff
import sheriff_config

PRINTF_RATE_LIMIT = 10000
UPDATE_CMDS_MIN_INTERVAL_USEC = 300

def timestamp_now (): return int (time.time () * 1000000)

try:
    getattr(__builtins__, "all")
except AttributeError:
    def all (list): return reduce (lambda x, y: x and y, list, True)

def now_str (): return time.strftime ("[%H:%M:%S] ")

class AddModifyCommandDialog (gtk.Dialog):
    def __init__ (self, parent, deputies, groups,
            initial_cmd="", initial_deputy=None, initial_group=""):
        # add command dialog
        gtk.Dialog.__init__ (self, "Add/Modify Command", parent,
                gtk.DIALOG_MODAL | gtk.DIALOG_DESTROY_WITH_PARENT,
                (gtk.STOCK_OK, gtk.RESPONSE_ACCEPT,
                 gtk.STOCK_CANCEL, gtk.RESPONSE_REJECT))
        table = gtk.Table (3, 2)

        # deputy
        table.attach (gtk.Label ("Host"), 0, 1, 0, 1, 0, 0)
        self.deputy_ls = gtk.ListStore (gobject.TYPE_PYOBJECT, 
                gobject.TYPE_STRING)
        self.host_cb = gtk.ComboBox (self.deputy_ls)

        dep_ind = 0
        pairs = [ (deputy.name, deputy) for deputy in deputies ]
        pairs.sort ()
        for name, deputy in pairs:
            self.deputy_ls.append ((deputy, deputy.name))
            if deputy == initial_deputy: 
                self.host_cb.set_active (dep_ind)
            dep_ind += 1
        if self.host_cb.get_active () < 0 and len(deputies) > 0:
            self.host_cb.set_active (0)

        deputy_tr = gtk.CellRendererText ()
        self.host_cb.pack_start (deputy_tr, True)
        self.host_cb.add_attribute (deputy_tr, "text", 1)
        table.attach (self.host_cb, 1, 2, 0, 1)
        self.deputies = deputies

        # command name
        table.attach (gtk.Label ("Name"), 0, 1, 1, 2, 0, 0)
        self.name_te = gtk.Entry ()
        self.name_te.set_text (initial_cmd)
        self.name_te.set_width_chars (40)
        table.attach (self.name_te, 1, 2, 1, 2)
        self.name_te.connect ("activate", 
                lambda e: self.response (gtk.RESPONSE_ACCEPT))
        self.name_te.grab_focus ()

        # group
        table.attach (gtk.Label ("Group"), 0, 1, 2, 3, 0, 0)
        self.group_cbe = gtk.combo_box_entry_new_text ()
        groups = groups[:]
        groups.sort ()
        for group_name in groups:
            self.group_cbe.append_text (group_name)
        table.attach (self.group_cbe, 1, 2, 2, 3)
        self.group_cbe.child.set_text (initial_group)
        self.group_cbe.child.connect ("activate",
                lambda e: self.response (gtk.RESPONSE_ACCEPT))

        self.vbox.pack_start (table, False, False, 0)
        table.show_all ()

    def get_deputy (self):
        iter = self.host_cb.get_active_iter ()
        if iter is None: return None
        return self.deputy_ls.get_value (iter, 0)
    
    def get_command (self): return self.name_te.get_text ()
    def get_group (self): return self.group_cbe.child.get_text ()

class CommandExtraData:
    def __init__ (self):
        self.tb = gtk.TextBuffer ()
        self.printf_keep_count = [ 0, 0, 0, 0, 0, 0 ]
        self.printf_drop_count = 0
        self.summary = ""

class SheriffGtk:
    COL_CMDS_TV_OBJ, \
    COL_CMDS_TV_CMD, \
    COL_CMDS_TV_HOST, \
    COL_CMDS_TV_STATUS_ACTUAL, \
    COL_CMDS_TV_CPU_USAGE, \
    COL_CMDS_TV_MEM_VSIZE, \
    COL_CMDS_TV_SUMMARY = range(7)

    COL_HOSTS_TV_OBJ, \
    COL_HOSTS_TV_NAME, \
    COL_HOSTS_TV_LAST_UPDATE, \
    COL_HOSTS_TV_LOAD, \
    COL_HOSTS_TV_SKEW, \
    COL_HOSTS_TV_JITTER = range(6)

    def __init__ (self, lc):
        self.lc = lc
        self.stdout_maxlines = 2000
        self.config_filename = None
        self.next_cmds_update_time = 0

        # create sheriff and subscribe to events
        self.sheriff = sheriff.Sheriff (self.lc)
        self.sheriff.connect ("command-added", self._on_sheriff_command_added)
        self.sheriff.connect ("command-removed", 
                self._on_sheriff_command_removed)
        self.sheriff.connect ("command-status-changed",
                self._on_sheriff_command_status_changed)
        self.sheriff.connect ("command-group-changed",
                self._on_sheriff_command_group_changed)
        gobject.timeout_add (1000, self._maybe_send_orders)

        self.group_row_references = {}

        self.lc.subscribe ("PMD_PRINTF", self.on_pmsd_printf)
        self.lc.subscribe ("PMD_ORDERS", self.on_pmsd_orders)

        # regexes
        self.warn_regex = re.compile ("warning", re.IGNORECASE)
        self.summary_regex = re.compile ("summary", re.IGNORECASE)

        # setup GUI
        self.window = gtk.Window (gtk.WINDOW_TOPLEVEL)
        self.window.set_default_size (800, 600)
        self.window.connect ("delete-event", gtk.main_quit)
        self.window.connect ("destroy-event", gtk.main_quit)

        vbox = gtk.VBox ()
        self.window.add (vbox)

        # keyboard accelerators.  This probably isn't the right way to do it...
        self.accel_group = gtk.AccelGroup ()
        self.accel_group.connect_group (ord("n"), gtk.gdk.CONTROL_MASK,
                gtk.ACCEL_VISIBLE, lambda *a: None)
        self.accel_group.connect_group (ord("s"), gtk.gdk.CONTROL_MASK,
                gtk.ACCEL_VISIBLE, lambda *a: None)
        self.accel_group.connect_group (ord("t"), gtk.gdk.CONTROL_MASK,
                gtk.ACCEL_VISIBLE, lambda *a: None)
        self.accel_group.connect_group (ord("e"), gtk.gdk.CONTROL_MASK,
                gtk.ACCEL_VISIBLE, lambda *a: None)
        self.accel_group.connect_group (ord("q"), gtk.gdk.CONTROL_MASK,
                gtk.ACCEL_VISIBLE, gtk.main_quit)
        self.accel_group.connect_group (ord("o"), gtk.gdk.CONTROL_MASK,
                gtk.ACCEL_VISIBLE, lambda *a: None)
        self.accel_group.connect_group (ord("a"), gtk.gdk.CONTROL_MASK,
                gtk.ACCEL_VISIBLE, 
                lambda *a: self.cmds_tv.get_selection ().select_all ())
        self.accel_group.connect_group (ord("d"), gtk.gdk.CONTROL_MASK,
                gtk.ACCEL_VISIBLE, 
                lambda *a: self.cmds_tv.get_selection ().unselect_all ())
#        self.accel_group.connect_group (ord("a"), gtk.gdk.CONTROL_MASK,
#                gtk.ACCEL_VISIBLE, self._do_save_config_dialog)
        self.accel_group.connect_group (gtk.gdk.keyval_from_name ("Delete"), 0,
                gtk.ACCEL_VISIBLE, self._remove_selected_commands)
        self.window.add_accel_group (self.accel_group)

        # setup the menu bar
        menu_bar = gtk.MenuBar ()
        vbox.pack_start (menu_bar, False, False, 0)

        file_mi = gtk.MenuItem ("_File")
        options_mi = gtk.MenuItem ("_Options")
        commands_mi = gtk.MenuItem ("_Commands")
        view_mi = gtk.MenuItem ("_View")
        
        # file menu
        file_menu = gtk.Menu ()
        file_mi.set_submenu (file_menu)

        self.load_cfg_mi = gtk.ImageMenuItem (gtk.STOCK_OPEN)
        self.load_cfg_mi.add_accelerator ("activate", self.accel_group, 
                ord("o"), gtk.gdk.CONTROL_MASK, gtk.ACCEL_VISIBLE)
        self.save_cfg_mi = gtk.ImageMenuItem (gtk.STOCK_SAVE_AS)
        quit_mi = gtk.ImageMenuItem (gtk.STOCK_QUIT)
        quit_mi.add_accelerator ("activate", self.accel_group, ord("q"),
                gtk.gdk.CONTROL_MASK, gtk.ACCEL_VISIBLE)
        file_menu.append (self.load_cfg_mi)
        file_menu.append (self.save_cfg_mi)
        file_menu.append (quit_mi)
        self.load_cfg_mi.connect ("activate", self._do_load_config_dialog)
        self.save_cfg_mi.connect ("activate", self._do_save_config_dialog)
        quit_mi.connect ("activate", gtk.main_quit)

        # commands menu
        commands_menu = gtk.Menu ()
        commands_mi.set_submenu (commands_menu)
        self.start_cmd_mi = gtk.MenuItem ("_Start")
        self.start_cmd_mi.add_accelerator ("activate", 
                self.accel_group, ord("s"),
                gtk.gdk.CONTROL_MASK, gtk.ACCEL_VISIBLE)
        self.start_cmd_mi.connect ("activate", self._start_selected_commands)
        self.start_cmd_mi.set_sensitive (False)
        commands_menu.append (self.start_cmd_mi)

        self.stop_cmd_mi = gtk.MenuItem ("S_top")
        self.stop_cmd_mi.add_accelerator ("activate", 
                self.accel_group, ord("t"),
                gtk.gdk.CONTROL_MASK, gtk.ACCEL_VISIBLE)
        self.stop_cmd_mi.connect ("activate", self._stop_selected_commands)
        self.stop_cmd_mi.set_sensitive (False)
        commands_menu.append (self.stop_cmd_mi)

        self.restart_cmd_mi = gtk.MenuItem ("R_estart")
        self.restart_cmd_mi.add_accelerator ("activate",
                self.accel_group, ord ("e"),
                gtk.gdk.CONTROL_MASK, gtk.ACCEL_VISIBLE)
        self.restart_cmd_mi.connect ("activate", 
                self._restart_selected_commands)
        self.restart_cmd_mi.set_sensitive (False)
        commands_menu.append (self.restart_cmd_mi)

        self.remove_cmd_mi = gtk.MenuItem ("_Remove")
        self.remove_cmd_mi.add_accelerator ("activate", self.accel_group, 
                gtk.gdk.keyval_from_name ("Delete"), 0, gtk.ACCEL_VISIBLE)
        self.remove_cmd_mi.connect ("activate", self._remove_selected_commands)
        self.remove_cmd_mi.set_sensitive (False)
        commands_menu.append (self.remove_cmd_mi)

        commands_menu.append (gtk.SeparatorMenuItem ())

        self.new_cmd_mi = gtk.MenuItem ("_New command")
        self.new_cmd_mi.add_accelerator ("activate", self.accel_group, ord("n"),
                gtk.gdk.CONTROL_MASK, gtk.ACCEL_VISIBLE)
        self.new_cmd_mi.connect ("activate", self._do_add_command_dialog)
        commands_menu.append (self.new_cmd_mi)

        # options menu
        options_menu = gtk.Menu ()
        options_mi.set_submenu (options_menu)

        self.is_observer_cmi = gtk.CheckMenuItem ("_Observer")
        self.is_observer_cmi.connect ("activate", self.on_observer_mi_activate)
        options_menu.append (self.is_observer_cmi)

        # view menu
        view_menu = gtk.Menu ()
        view_mi.set_submenu (view_menu)

        menu_bar.append (file_mi)
        menu_bar.append (options_mi)
        menu_bar.append (commands_mi)
        menu_bar.append (view_mi)

        vpane = gtk.VPaned ()
        vbox.pack_start (vpane, True, True, 0)

        # setup the command treeview
        hpane = gtk.HPaned ()
        vpane.add1 (hpane)
        
        self.cmds_ts = gtk.TreeStore (gobject.TYPE_PYOBJECT,
                gobject.TYPE_STRING, # command name
                gobject.TYPE_STRING, # host name
                gobject.TYPE_STRING, # status actual
                gobject.TYPE_STRING, # CPU usage
                gobject.TYPE_INT,    # memory vsize
                gobject.TYPE_STRING  # summary
                )

        self.cmds_tv = gtk.TreeView (self.cmds_ts)
        sw = gtk.ScrolledWindow ()
        sw.set_policy (gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
        hpane.pack1 (sw, resize = True)
        sw.add (self.cmds_tv)

        cmds_tr = gtk.CellRendererText ()
        cmds_tr.set_property ("ellipsize", pango.ELLIPSIZE_END)
        plain_tr = gtk.CellRendererText ()
        status_tr = gtk.CellRendererText ()

        cols = []
        col = gtk.TreeViewColumn ("Command", cmds_tr, text=1)
        col.set_sort_column_id (1)
        cols.append (col)

        col = gtk.TreeViewColumn ("Host", plain_tr, text=2)
        col.set_sort_column_id (2)
        cols.append (col)

        col = gtk.TreeViewColumn ("Status", status_tr, text=3)
        col.set_sort_column_id (3)
        col.set_cell_data_func (status_tr, self._status_cell_data_func)

        cols.append (col)
        col = gtk.TreeViewColumn ("CPU %", plain_tr, text=4)
        col.set_sort_column_id (4)
        cols.append (col)

        col = gtk.TreeViewColumn ("Mem (kB)", plain_tr, text=5)
        col.set_sort_column_id (5)
        cols.append (col)

        col = gtk.TreeViewColumn ("Summary", plain_tr, text=6)
        col.set_sort_column_id (6)
        col.set_expand (True)
        cols.append (col)

        for col in cols:
            col.set_resizable (True)
            self.cmds_tv.append_column (col)

            name = col.get_title ()
            col_cmi = gtk.CheckMenuItem (name)
            col_cmi.set_active (True)
            col_cmi.connect ("activate", 
                    lambda cmi, col: col.set_visible (cmi.get_active()), col)
            view_menu.append (col_cmi)

        cmds_sel = self.cmds_tv.get_selection ()
        cmds_sel.set_mode (gtk.SELECTION_MULTIPLE)
        cmds_sel.connect ("changed", self.on_cmds_selection_changed)

        gobject.timeout_add (1000, 
                lambda *s: self._repopulate_cmds_tv () or True)
        self.cmds_tv.add_events (gtk.gdk.KEY_PRESS_MASK | \
                gtk.gdk.BUTTON_PRESS | gtk.gdk._2BUTTON_PRESS)
        self.cmds_tv.connect ("key-press-event", 
                self._on_cmds_tv_key_press_event)
        self.cmds_tv.connect ("button-press-event", 
                self._on_cmds_tv_button_press_event)
        self.cmds_tv.connect ("row-activated",
                self._on_cmds_tv_row_activated)

        # commands treeview context menu
        self.cmd_ctxt_menu = gtk.Menu ()

        self.start_cmd_ctxt_mi = gtk.MenuItem ("_Start")
        self.cmd_ctxt_menu.append (self.start_cmd_ctxt_mi)
        self.start_cmd_ctxt_mi.connect ("activate", 
                self._start_selected_commands)

        self.stop_cmd_ctxt_mi = gtk.MenuItem ("_Stop")
        self.cmd_ctxt_menu.append (self.stop_cmd_ctxt_mi)
        self.stop_cmd_ctxt_mi.connect ("activate", self._stop_selected_commands)

        self.restart_cmd_ctxt_mi = gtk.MenuItem ("R_estart")
        self.cmd_ctxt_menu.append (self.restart_cmd_ctxt_mi)
        self.restart_cmd_ctxt_mi.connect ("activate", 
                self._restart_selected_commands)

        self.remove_cmd_ctxt_mi = gtk.MenuItem ("_Remove")
        self.cmd_ctxt_menu.append (self.remove_cmd_ctxt_mi)
        self.remove_cmd_ctxt_mi.connect ("activate", 
                self._remove_selected_commands)

        self.change_deputy_ctxt_mi = gtk.MenuItem ("_Change Host")
        self.cmd_ctxt_menu.append (self.change_deputy_ctxt_mi)
        self.change_deputy_ctxt_mi.show ()

        self.cmd_ctxt_menu.append (gtk.SeparatorMenuItem ())

        self.new_cmd_ctxt_mi = gtk.MenuItem ("_New Command")
        self.cmd_ctxt_menu.append (self.new_cmd_ctxt_mi)
        self.new_cmd_ctxt_mi.connect ("activate", self._do_add_command_dialog)

        self.cmd_ctxt_menu.show_all ()

#        # drag and drop command rows for grouping
#        dnd_targets = [ ('PROCMAN_CMD_ROW', 
#            gtk.TARGET_SAME_APP | gtk.TARGET_SAME_WIDGET, 0) ]
#        self.cmds_tv.enable_model_drag_source (gtk.gdk.BUTTON1_MASK, 
#                dnd_targets, gtk.gdk.ACTION_MOVE)
#        self.cmds_tv.enable_model_drag_dest (dnd_targets, 
#                gtk.gdk.ACTION_MOVE)

        # hosts treeview
        self.hosts_ts = gtk.ListStore (gobject.TYPE_PYOBJECT,
                gobject.TYPE_STRING, # deputy name
                gobject.TYPE_STRING, # last update time
                gobject.TYPE_STRING, # load
                gobject.TYPE_STRING, # jitter
                gobject.TYPE_STRING, # skew
                )

        self.hosts_tv = gtk.TreeView (self.hosts_ts)
        sw = gtk.ScrolledWindow ()
        sw.set_policy (gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
        hpane.pack2 (sw, resize = False)
        sw.add (self.hosts_tv)

        col = gtk.TreeViewColumn ("Host", plain_tr, text=1)
        col.set_sort_column_id (1)
        col.set_resizable (True)
        self.hosts_tv.append_column (col)

        col = gtk.TreeViewColumn ("Last update", plain_tr, text=2)
#        col.set_sort_column_id (2) # XXX this triggers really weird bugs...
        col.set_resizable (True)
        self.hosts_tv.append_column (col)

        col = gtk.TreeViewColumn ("Load", plain_tr, text=3)
        col.set_resizable (True)
        self.hosts_tv.append_column (col)

        col = gtk.TreeViewColumn ("Clock Skew (ms)", plain_tr, text=4)
        col.set_resizable (True)
        self.hosts_tv.append_column (col)

        col = gtk.TreeViewColumn ("Jitter (ms)", plain_tr, text=5)
        col.set_resizable (True)
        self.hosts_tv.append_column (col)

        gobject.timeout_add (1000, 
                lambda *s: self._repopulate_hosts_tv() or True)

        hpane.set_position (500)

        # stdout textview
        self.stdout_textview = gtk.TextView ()
        self.stdout_textview.set_property ("editable", False)
        self.sheriff_tb = self.stdout_textview.get_buffer ()
        sw = gtk.ScrolledWindow ()
        sw.add (self.stdout_textview)
        vpane.add2 (sw)
        vpane.set_position (300)
        
        stdout_adj = sw.get_vadjustment ()
        stdout_adj.set_data ("scrolled-to-end", 1)
        stdout_adj.connect ("changed", self.on_adj_changed)
        stdout_adj.connect ("value-changed", self.on_adj_value_changed)

        font_desc = pango.FontDescription ("Monospace")
        self.stdout_textview.modify_font (font_desc)

        # stdout rate limit maintenance events
        gobject.timeout_add (500, self._stdout_rate_limit_upkeep)

        # status bar
        self.statusbar = gtk.Statusbar ()
        vbox.pack_start (self.statusbar, False, False, 0)

        vbox.show_all ()
        self.window.show ()

    def _get_selected_commands (self):
        selection = self.cmds_tv.get_selection ()
        if selection is None: return []
        model, rows = selection.get_selected_rows ()
        col = SheriffGtk.COL_CMDS_TV_OBJ
        selected = []
        for path in rows:
            iter = model.get_iter (path)
            cmd = model.get_value (iter, col)
            if not cmd:
                child_iter = model.iter_children (iter)
                while child_iter:
                    selected.append (model.get_value (child_iter, col))
                    child_iter = model.iter_next (child_iter)
            else:
                selected.append (cmd)
        return selected
    
    def _get_selected_hosts (self):
        model, rows = self.hosts_tv.get_selection ().get_selected_rows ()
        return [ model.get_value (model.get_iter(path), 0) \
                for path in rows ]

    def _find_or_make_group_row_reference (self, group_name):
        if not group_name:
            return None
        if group_name in self.group_row_references:
            return self.group_row_references[group_name]
        else:
            iter = self.cmds_ts.append (None, 
                    ((None, group_name, "", "", "", 0, "")))
            trr = gtk.TreeRowReference (self.cmds_ts, 
                    self.cmds_ts.get_path (iter))
            self.group_row_references[group_name] = trr
            return trr

    def _get_known_group_names (self):
        return self.group_row_references.keys ()

    def _delete_group_row_reference (self, group_name):
        del self.group_row_references[group_name]

    def _repopulate_hosts_tv (self):
        to_update = sets.Set (self.sheriff.get_deputies ())
        to_remove = []

        def _deputy_last_update_str (dep):
            if dep.last_update_utime:
                now = timestamp_now ()
                return "%.3f seconds ago" % ((now-dep.last_update_utime)*1e-6)
            else:
                return "<never>"

        def _update_row (model, path, iter, user_data):
            deputy = model.get_value (iter, SheriffGtk.COL_HOSTS_TV_OBJ)
            if deputy in to_update:
                model.set (iter, 
                        SheriffGtk.COL_HOSTS_TV_LAST_UPDATE, 
                        _deputy_last_update_str (deputy),
                        SheriffGtk.COL_HOSTS_TV_LOAD, 
                        "%f" % deputy.cpu_load,
                        SheriffGtk.COL_HOSTS_TV_SKEW, 
                        "%f" % (deputy.clock_skew_usec * 1e-3),
                        SheriffGtk.COL_HOSTS_TV_JITTER, 
                        "%f" % (deputy.last_orders_jitter_usec * 1e-3),
                        )
                to_update.remove (deputy)
            else:
                to_remove.append (gtk.TreeRowReference (model, path))

        self.hosts_ts.foreach (_update_row, None)

        for trr in to_remove:
            self.hosts_ts.remove (self.hosts_ts.get_iter (trr.get_path()))

        for deputy in to_update:
            print "adding %s to treeview" % deputy.name
            new_row = (deputy, deputy.name, _deputy_last_update_str (deputy),
                    "%f" % deputy.cpu_load,
                    "%f" % (deputy.clock_skew_usec * 1e-3),
                    "%f" % (deputy.last_orders_jitter_usec * 1e-3),
                    )
            self.hosts_ts.append (new_row)

    def _repopulate_cmds_tv (self):
        now = timestamp_now ()
        if now < self.next_cmds_update_time:
            return

        selected_cmds = self._get_selected_commands ()

        cmds = sets.Set ()
        cmd_deps = {}
        deputies = self.sheriff.get_deputies ()
        for deputy in deputies:
            for cmd in deputy.get_commands ():
                cmd_deps [cmd] = deputy
                cmds.add (cmd)
        to_remove = []
        to_reparent = []

        def _update_row (model, path, iter, user_data):
            obj_col = SheriffGtk.COL_CMDS_TV_OBJ
            cmd = model.get_value (iter, obj_col)
            if not cmd: 
                # row represents a procman group

                # get a list of all the row's children
                child_iter = model.iter_children (iter)
                children = []
                while child_iter:
                    children.append (model.get_value (child_iter, obj_col))
                    child_iter = model.iter_next (child_iter)

                if not children: 
                    to_remove.append (gtk.TreeRowReference (model, path))
                    return
                statuses = [ cmd.status () for cmd in children ]
                if all ([s == statuses[0] for s in statuses]):
                    status_str = statuses[0]
                else:
                    status_str = "Mixed"
                cpu_total = sum ([cmd.cpu_usage for cmd in children])
                mem_total = sum ([cmd.mem_vsize_bytes / 1024 \
                        for cmd in children])
                cpu_str = "%.2f" % (cpu_total * 100)
                model.set (iter, 
                        SheriffGtk.COL_CMDS_TV_STATUS_ACTUAL, status_str,
                        SheriffGtk.COL_CMDS_TV_CPU_USAGE, cpu_str,
                        SheriffGtk.COL_CMDS_TV_MEM_VSIZE, mem_total)
                return
            if cmd in cmds:
                extradata = cmd.get_data ("extradata")
                cpu_str = "%.2f" % (cmd.cpu_usage * 100)
                mem_usage = int (cmd.mem_vsize_bytes / 1024)
                model.set (iter, 
                        SheriffGtk.COL_CMDS_TV_CMD, cmd.name,
                        SheriffGtk.COL_CMDS_TV_STATUS_ACTUAL, cmd.status (),
                        SheriffGtk.COL_CMDS_TV_HOST, cmd_deps[cmd].name,
                        SheriffGtk.COL_CMDS_TV_CPU_USAGE, cpu_str,
                        SheriffGtk.COL_CMDS_TV_MEM_VSIZE, mem_usage)
                if extradata:
                    model.set (iter,
                            SheriffGtk.COL_CMDS_TV_SUMMARY, extradata.summary)

                # check that the command is in the correct group in the
                # treemodel
                correct_grr = self._find_or_make_group_row_reference (cmd.group)
                correct_parent_iter = None
                if correct_grr:
                    correct_parent_iter = \
                            model.get_iter (correct_grr.get_path())
                actual_parent_iter = model.iter_parent (iter)
                if correct_parent_iter is not None and \
                        actual_parent_iter is not None:
                    correct_parent_path = model.get_path (correct_parent_iter)
                    actual_parent_path = model.get_path (actual_parent_iter)
                    if correct_parent_path != actual_parent_path:
                        print "moving %s (%s) (%s)" % (cmd.name,
                                correct_parent_path, actual_parent_path)
                        # schedule the command to be moved
                        to_reparent.append ((gtk.TreeRowReference (model, path),
                            correct_grr))

                cmds.remove (cmd)
            else:
                to_remove.append (gtk.TreeRowReference (model, path))

        self.cmds_ts.foreach (_update_row, None)

        # reparent rows that are in the wrong group
        for trr, newparent_rr in to_reparent:
            orig_iter = self.cmds_ts.get_iter (trr.get_path ())
            rowdata = self.cmds_ts.get (orig_iter, *range(7))
            self.cmds_ts.remove (orig_iter)
            newparent_iter = self.cmds_ts.get_iter (newparent_rr.get_path ())
            self.cmds_ts.append (newparent_iter, rowdata)

        # remove rows that have been marked for deletion
        for trr in to_remove:
            iter = self.cmds_ts.get_iter (trr.get_path())
            if not self.cmds_ts.get_value (iter, SheriffGtk.COL_CMDS_TV_OBJ):
                self._delete_group_row_reference (self.cmds_ts.get_value (iter,
                    SheriffGtk.COL_CMDS_TV_CMD))
            self.cmds_ts.remove (iter)

        # remove group rows with no children
        groups_to_remove = []
        def _check_for_lonely_groups (model, path, iter, user_data):
            cmd = model.get_value (iter, SheriffGtk.COL_CMDS_TV_OBJ)
            if not cmd and not model.iter_has_child (iter): 
                groups_to_remove.append (gtk.TreeRowReference (model, path))
        self.cmds_ts.foreach (_check_for_lonely_groups, None)
        for trr in groups_to_remove:
            iter = self.cmds_ts.get_iter (trr.get_path())
            self._delete_group_row_reference (self.cmds_ts.get_value (iter,
                SheriffGtk.COL_CMDS_TV_CMD))
            self.cmds_ts.remove (iter)

        # create new rows for new commands
        for cmd in cmds:
            deputy = cmd_deps[cmd]
            parent = self._find_or_make_group_row_reference (cmd.group)
            new_row = (cmd, cmd.name, deputy.name, cmd.status (), "0", 0, "")
            if parent:
                parent_iter = self.cmds_ts.get_iter (parent.get_path ())
            else:
                parent_iter = None
            iter = self.cmds_ts.append (parent_iter, new_row)

        self.next_cmds_update_time = \
                timestamp_now () + UPDATE_CMDS_MIN_INTERVAL_USEC

    def _set_observer (self, is_observer):
        self.sheriff.set_observer (is_observer)

        self._update_menu_item_sensitivities ()

        if is_observer: self.window.set_title ("Procman Observer")
        else: self.window.set_title ("Procman Sheriff")

        if self.is_observer_cmi != is_observer:
            self.is_observer_cmi.set_active (is_observer)

    # GTK signal handlers
    def _do_load_config_dialog (self, *args):
        dlg = gtk.FileChooserDialog ("Load Config", self.window, 
                buttons = (gtk.STOCK_OPEN, gtk.RESPONSE_ACCEPT,
                    gtk.STOCK_CANCEL, gtk.RESPONSE_REJECT))
#        dlg.set_current_folder_uri (CONFIG_DIR)
        if gtk.RESPONSE_ACCEPT == dlg.run ():
            self.config_filename = dlg.get_filename ()
            dlg.destroy ()
            try:
                cfg = sheriff_config.config_from_filename (self.config_filename)
            except Exception, e:
                msgdlg = gtk.MessageDialog (self.window,
                        gtk.DIALOG_MODAL|gtk.DIALOG_DESTROY_WITH_PARENT,
                        gtk.MESSAGE_ERROR, gtk.BUTTONS_CLOSE, 
                        traceback.format_exc ())
                msgdlg.run ()
                msgdlg.destroy ()
            else:
                self.sheriff.load_config (cfg)
        else:
            dlg.destroy ()

    def _do_save_config_dialog (self, *args):
        dlg = gtk.FileChooserDialog ("Save Config", self.window,
                action = gtk.FILE_CHOOSER_ACTION_SAVE,
                buttons = (gtk.STOCK_SAVE, gtk.RESPONSE_ACCEPT,
                    gtk.STOCK_CANCEL, gtk.RESPONSE_REJECT))
        if self.config_filename is not None:
            dlg.set_filename (self.config_filename)
        if gtk.RESPONSE_ACCEPT == dlg.run ():
            self.config_filename = dlg.get_filename ()
            dlg.destroy ()
            try:
                self.sheriff.save_config (file (self.config_filename, "w"))
            except IOError, e:
                msgdlg = gtk.MessageDialog (self.window,
                        gtk.DIALOG_MODAL|gtk.DIALOG_DESTROY_WITH_PARENT,
                        gtk.MESSAGE_ERROR, gtk.BUTTONS_CLOSE, str (e))
                msgdlg.run ()
                msgdlg.destroy ()
        else:
            dlg.destroy ()

    def on_observer_mi_activate (self, menu_item):
        self._set_observer (menu_item.get_active ())

    def _do_add_command_dialog (self, *args):
        deputies = self.sheriff.get_deputies ()
        if not deputies:
            msgdlg = gtk.MessageDialog (self.window, 
                    gtk.DIALOG_MODAL|gtk.DIALOG_DESTROY_WITH_PARENT,
                    gtk.MESSAGE_ERROR, gtk.BUTTONS_CLOSE,
                    "Can't add a command without an active deputy")
            msgdlg.run ()
            msgdlg.destroy ()
            return
        dlg = AddModifyCommandDialog (self.window, deputies,
                self._get_known_group_names ())
        while dlg.run () == gtk.RESPONSE_ACCEPT:
            cmd = dlg.get_command ()
            deputy = dlg.get_deputy ()
            group = dlg.get_group ().strip ()
            if not cmd.strip ():
                msgdlg = gtk.MessageDialog (self.window, 
                        gtk.DIALOG_MODAL|gtk.DIALOG_DESTROY_WITH_PARENT,
                        gtk.MESSAGE_ERROR, gtk.BUTTONS_CLOSE, "Invalid command")
                msgdlg.run ()
                msgdlg.destroy ()
            elif not deputy:
                msgdlg = gtk.MessageDialog (self.window, 
                        gtk.DIALOG_MODAL|gtk.DIALOG_DESTROY_WITH_PARENT,
                        gtk.MESSAGE_ERROR, gtk.BUTTONS_CLOSE, "Invalid deputy")
                msgdlg.run ()
                msgdlg.destroy ()
            else:
                self.sheriff.add_command (deputy.name, cmd, group)
                break
        dlg.destroy ()

    def _start_selected_commands (self, *args):
        for cmd in self._get_selected_commands ():
            self.sheriff.start_command (cmd)

    def _stop_selected_commands (self, *args):
        for cmd in self._get_selected_commands ():
            self.sheriff.stop_command (cmd)

    def _restart_selected_commands (self, *args):
        for cmd in self._get_selected_commands ():
            self.sheriff.restart_command (cmd)

    def _remove_selected_commands (self, *args):
        toremove = self._get_selected_commands ()
        for cmd in toremove:
            self.sheriff.schedule_command_for_removal (cmd)

    def _update_menu_item_sensitivities (self):
        # enable/disable menu options based on sheriff state and user selection
        selected_cmds = self._get_selected_commands ()
        can_modify = len(selected_cmds) > 0 and not self.sheriff.is_observer ()
        can_add_load = not self.sheriff.is_observer ()

        self.start_cmd_mi.set_sensitive (can_modify)
        self.stop_cmd_mi.set_sensitive (can_modify)
        self.restart_cmd_mi.set_sensitive (can_modify)
        self.remove_cmd_mi.set_sensitive (can_modify)

        self.new_cmd_mi.set_sensitive (can_add_load)
        self.load_cfg_mi.set_sensitive (can_add_load)

    def on_cmds_selection_changed (self, selection):
        selected_cmds = self._get_selected_commands ()
        if len (selected_cmds) == 1:
            cmd = selected_cmds[0]
            extradata = cmd.get_data ("extradata")
            self.stdout_textview.set_buffer (extradata.tb)
        elif len (selected_cmds) == 0:
            self.stdout_textview.set_buffer (self.sheriff_tb)
        self._update_menu_item_sensitivities ()

    def on_adj_changed (self, adj):
        if adj.get_data ("scrolled-to-end"):
            adj.set_value (adj.upper - adj.page_size)

    def on_adj_value_changed (self, adj):
        adj.set_data ("scrolled-to-end", adj.value == adj.upper-adj.page_size)

    def _stdout_rate_limit_upkeep (self):
        for cmd in self.sheriff.get_all_commands ():
            extradata = cmd.get_data ("extradata")
            if not extradata: continue
            if extradata.printf_drop_count:
                deputy = self.sheriff.get_command_deputy (cmd)
                self._add_text_to_buffer (extradata.tb, now_str() + 
                        "\nSHERIFF RATE LIMIT: Ignored %d bytes of output\n" %
                        (extradata.printf_drop_count))
                self._add_text_to_buffer (self.sheriff_tb, now_str() + 
                        "Ignored %d bytes of output from [%s] [%s]\n" % \
                        (extradata.printf_drop_count, deputy.name, cmd.name))

            extradata.printf_keep_count.pop (0)
            extradata.printf_keep_count.append (0)
            extradata.printf_drop_count = 0
        return True

    def _status_cell_data_func (self, column, cell, model, iter):
        color_map = {
                sheriff.TRYING_TO_START : "Orange",
                sheriff.RESTARTING : "Orange",
                sheriff.RUNNING : "Green",
                sheriff.TRYING_TO_STOP : "Yellow",
                sheriff.REMOVING : "Yellow",
                sheriff.STOPPED_OK : "White",
                sheriff.STOPPED_ERROR : "Red",
                sheriff.UNKNOWN : "Red"
                }

        col = SheriffGtk.COL_CMDS_TV_OBJ
        cmd = model.get_value (iter, col)
        if not cmd:
            # group node
            child_iter = model.iter_children (iter)
            children = []
            while child_iter:
                children.append (model.get_value (child_iter, col))
                child_iter = model.iter_next (child_iter)

            if not children:
                cell.set_property ("cell-background-set", False)
            else:
                cell.set_property ("cell-background-set", True)

                statuses = [ cmd.status () for cmd in children ]
                
                if all ([s == statuses[0] for s in statuses]):
                    # if all the commands in a group have the same status, then
                    # color them by that status
                    cell.set_property ("cell-background", 
                            color_map[statuses[0]])
                else:
                    # otherwise, color them yellow
                    cell.set_property ("cell-background", "Yellow")

            return

        cell.set_property ("cell-background-set", True)
        cell.set_property ("cell-background", color_map[cmd.status ()])

    def _maybe_send_orders (self):
        if not self.sheriff.is_observer (): self.sheriff.send_orders ()
        return True

    def _on_cmds_tv_key_press_event (self, widget, event):
        if event.keyval == gtk.gdk.keyval_from_name ("Right"):
            # expand a group row when user presses right arrow key
            model, rows = self.cmds_tv.get_selection ().get_selected_rows ()
            if len (rows) == 1:
                col = SheriffGtk.COL_CMDS_TV_OBJ
                iter = model.get_iter (rows[0])
                if model.iter_has_child (iter):
                    self.cmds_tv.expand_row (rows[0], True)
                return True
        elif event.keyval == gtk.gdk.keyval_from_name ("Left"):
            # collapse a group row when user presses left arrow key
            model, rows = self.cmds_tv.get_selection ().get_selected_rows ()
            if len (rows) == 1:
                col = SheriffGtk.COL_CMDS_TV_OBJ
                iter = model.get_iter (rows[0])
                if model.iter_has_child (iter):
                    self.cmds_tv.collapse_row (rows[0])
                else:
                    parent = model.iter_parent (iter)
                    if parent:
                        parent_path = self.cmds_ts.get_path (parent)
                        self.cmds_tv.set_cursor (parent_path)
                return True
        return False

    def _on_cmds_tv_button_press_event (self, treeview, event):
        if event.type == gtk.gdk.BUTTON_PRESS and event.button == 3:
            time = event.time
            treeview.grab_focus ()
            sel = self.cmds_tv.get_selection ()
            model, rows = sel.get_selected_rows ()
            pathinfo = treeview.get_path_at_pos (int (event.x), int (event.y))

            if pathinfo is not None:
                if pathinfo[0] not in rows:
                    # if user right-clicked on a previously unselected row,
                    # then unselect all other rows and select only the row
                    # under the mouse cursor
                    path, col, cellx, celly = pathinfo
                    treeview.grab_focus ()
                    treeview.set_cursor (path, col, 0)

                # build a submenu of all deputies
                selected_cmds = self._get_selected_commands ()
                can_start_stop_remove = len(selected_cmds) > 0 and \
                        not self.sheriff.is_observer ()

                deputy_submenu = gtk.Menu ()
                deps = [ (d.name, d) for d in self.sheriff.get_deputies () ]
                deps.sort ()
                for name, deputy in deps:
                    dmi = gtk.MenuItem (name)
                    deputy_submenu.append (dmi)
                    dmi.show ()

                    def _onclick (mi, newdeputy):
                        for cmd in self._get_selected_commands ():
                            old_dep = self.sheriff.get_command_deputy (cmd)

                            if old_dep == newdeputy: continue

                            self.sheriff.schedule_command_for_removal (cmd)
                            self.sheriff.add_command (newdeputy.name, cmd.name, 
                                    cmd.group)

                    dmi.connect ("activate", _onclick, deputy)

                self.change_deputy_ctxt_mi.set_submenu (deputy_submenu)
            else:
                sel.unselect_all ()

            # enable/disable menu options based on sheriff state and user
            # selection
            can_add_load = not self.sheriff.is_observer ()
            can_modify = pathinfo is not None and not self.sheriff.is_observer()

            self.start_cmd_ctxt_mi.set_sensitive (can_modify)
            self.stop_cmd_ctxt_mi.set_sensitive (can_modify)
            self.restart_cmd_ctxt_mi.set_sensitive (can_modify)
            self.remove_cmd_ctxt_mi.set_sensitive (can_modify)
            self.change_deputy_ctxt_mi.set_sensitive (can_modify)
            self.new_cmd_ctxt_mi.set_sensitive (can_add_load)

            self.cmd_ctxt_menu.popup (None, None, None, event.button, time)
            return 1
        elif event.type == gtk.gdk._2BUTTON_PRESS and event.button == 1:
            # expand or collapse groups when double clicked
            sel = self.cmds_tv.get_selection ()
            model, rows = sel.get_selected_rows ()
            if len (rows) == 1:
                if model.iter_has_child (model.get_iter (rows[0])):
                    if self.cmds_tv.row_expanded (rows[0]):
                        self.cmds_tv.collapse_row (rows[0])
                    else:
                        self.cmds_tv.expand_row (rows[0], True)
        elif event.type == gtk.gdk.BUTTON_PRESS and event.button == 1:
            # unselect all rows when the user clicks on empty space in the
            # commands treeview
            time = event.time
            x = int (event.x)
            y = int (event.y)
            pathinfo = treeview.get_path_at_pos (x, y)
            if pathinfo is None:
                self.cmds_tv.get_selection ().unselect_all ()
                

    def _on_cmds_tv_row_activated (self, treeview, path, column):
        iter = self.cmds_ts.get_iter (path)
        cmd = self.cmds_ts.get_value (iter, SheriffGtk.COL_CMDS_TV_OBJ)
        if not cmd:
            return

        old_deputy = self.sheriff.get_command_deputy (cmd)
        dlg = AddModifyCommandDialog (self.window, 
                self.sheriff.get_deputies (),
                self._get_known_group_names (),
                cmd.name, old_deputy, cmd.group)
        if dlg.run () == gtk.RESPONSE_ACCEPT:
            newname = dlg.get_command ()
            newdeputy = dlg.get_deputy ()
            newgroup = dlg.get_group ().strip ()

            if newname != cmd.name:
                self.sheriff.set_command_name (cmd, newname)

            if newdeputy != old_deputy:
                self.sheriff.schedule_command_for_removal (cmd)
                self.sheriff.add_command (newdeputy.name, cmd.name, cmd.group)

            if newgroup != cmd.group:
                self.sheriff.set_command_group (cmd, newgroup)
        dlg.destroy ()

    # Sheriff event handlers
    def _on_sheriff_command_added (self, sheriff, deputy, command):
        extradata = CommandExtraData ()
        command.set_data ("extradata", extradata)
        self._add_text_to_buffer (self.sheriff_tb, now_str() + 
                "Added [%s] [%s]\n" % (deputy.name, command.name))
        self._repopulate_cmds_tv ()

    def _on_sheriff_command_removed (self, sheriff, deputy, command):
        self._add_text_to_buffer (self.sheriff_tb, now_str() + 
                "[%d] removed (%s:%s)\n" % (command.sheriff_id,
                deputy.name, command.name))
        self._repopulate_cmds_tv ()

    def _on_sheriff_command_status_changed (self, sheriff, cmd,
            old_status, new_status):
        self._add_text_to_buffer (self.sheriff_tb,now_str() + 
                "[%s] new status: %s\n" % (cmd.name, new_status))
        self._repopulate_cmds_tv ()

    def _on_sheriff_command_group_changed (self, sheriff, cmd):
        self._repopulate_cmds_tv ()

    # LC handlers
    def _add_text_to_buffer (self, tb, text):
        end_iter = tb.get_end_iter ()
        tb.insert (end_iter, text)

        # toss out old text if the muffer is getting too big
        num_lines = tb.get_line_count ()
        if num_lines > self.stdout_maxlines:
            start_iter = tb.get_start_iter ()
            chop_iter = tb.get_iter_at_line (num_lines - self.stdout_maxlines)
            tb.delete (start_iter, chop_iter)

    def on_pmsd_orders (self, channel, data):
        msg = pmsd_orders_t.decode (data)
        if not self.sheriff.is_observer () and \
                self.sheriff.name != msg.sheriff_name:
            # detected the presence of another sheriff that is not this one.
            # self-demote to prevent command thrashing
            self._set_observer (True)

            self.statusbar.push (self.statusbar.get_context_id ("main"),
                    "WARNING: multiple sheriffs detected!  Switching to observer mode");
            gobject.timeout_add (6000, 
                    lambda *s: self.statusbar.pop (self.statusbar.get_context_id ("main")))

    def on_pmsd_printf (self, channel, data):
        msg = pmsd_printf_t.decode (data)
        if msg.sheriff_id:
            try:
                cmd = self.sheriff.get_command_by_id (msg.sheriff_id)
            except KeyError:
                # TODO
                return

            extradata = cmd.get_data ("extradata")
            if not extradata: return

            # rate limit
            msg_count = sum (extradata.printf_keep_count)
            if msg_count >= PRINTF_RATE_LIMIT:
                extradata.printf_drop_count += len (msg.text)
                return

            tokeep = min (PRINTF_RATE_LIMIT - msg_count, len (msg.text))
            extradata.printf_keep_count[-1] += tokeep

            if len (msg.text) > tokeep:
                toadd = msg.text[:tokeep]
            else:
                toadd = msg.text

            self._add_text_to_buffer (extradata.tb, toadd)

            for line in toadd.split ("\n"):
                if self.warn_regex.match (line):
                    self._add_text_to_buffer (self.sheriff_tb, now_str() + 
                            "[%s] %s\n" % (cmd.name, line))
                    extradata.summary = line[8:]
                elif self.summary_regex.match (line):
                    extradata.summary = line[8:]


def run ():
    try:
        import sheriff_config
        cfg_fname = sys.argv[1]
    except IndexError:
        cfg = None
    else:
        cfg = sheriff_config.config_from_filename (cfg_fname)

    lc = LCM ()
    def handle (*a):
        try:
            lc.handle ()
        except Exception, e:
            import traceback
            traceback.print_exc ()
        return True
    gobject.io_add_watch (lc, gobject.IO_IN, handle)
    gui = SheriffGtk (lc)
    def delayed_load ():
        gui.sheriff.load_config (cfg)
        return False
    if cfg is not None:
        gobject.timeout_add (2000, delayed_load)
    gtk.main ()

if __name__ == "__main__":
    run ()
