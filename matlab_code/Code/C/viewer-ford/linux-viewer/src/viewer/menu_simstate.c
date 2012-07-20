#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <glib.h>

#include <common/timestamp.h>
#include <common/eventlog.h>
#include <common/glib_util.h>
#include <common/eventlog.h>
#include <dgc/globals.h>

#include <libviewer/viewer.h>


typedef struct _SimState {
    Viewer         *viewer;
    lcm_t           *lc;

    // simstate
    GHashTable  *msgs;
    char *fname;
} SimState;



// callback functions for sim state
void message_handler (const lcm_recv_buf_t *rbuf, const char *channel, void *u)
{
    SimState *self = (SimState *) u;
    
    // delete prev msg
    char *key=NULL;
    eventlog_event_t *msg=NULL;
    if (g_hash_table_lookup_extended(self->msgs, channel,
                (gpointer)key, (gpointer)msg)) {
        g_hash_table_remove(self->msgs, channel);
        if (key)
            free(key);
        if (msg) {
            eventlog_free_event(msg);
        }
    }
    // store current msg
    msg=calloc(1,sizeof(eventlog_event_t));
    msg->datalen=rbuf->data_size;
    msg->data=calloc(1,rbuf->data_size);
    msg->timestamp = timestamp_now();
    memcpy(msg->data,rbuf->data,rbuf->data_size);
    g_hash_table_insert(self->msgs, strdup(channel), msg);
}                             


int
write_simlog(SimState *self, const char *fname)
{
    fprintf(stderr,"SimState: INFO writing:%s\n",fname);
    eventlog_t *log = eventlog_create(fname, "w");

    GPtrArray *keys = gu_hash_table_get_keys(self->msgs);
    for (int i = 0; i < g_ptr_array_size(keys); i++) {
        const char *key = g_ptr_array_index(keys, i);
        eventlog_event_t *msg = g_hash_table_lookup(self->msgs, key);

        if (msg) {
            if(!msg->channel) {
                // log_write_event will handle this.
                if (!strcmp(key,"GOALS")) 
                    msg->channel = strdup("SIM_GOALS_NEW");
                else if (!strcmp(key,"SIM_LANES")) 
                    msg->channel = strdup("SIM_LANES_NEW");
                else if (!strcmp(key,"SIM_RECTS")) 
                    msg->channel = strdup("SIM_RECTS_NEW");
                else if (!strcmp(key,"POSE")) 
                    msg->channel = strdup("SIM_TELEPORT");
                else
                    msg->channel = strdup(key);
                msg->channellen = strlen(msg->channel);
            }
            eventlog_write_event(log, msg);
        }
    }
    eventlog_destroy( log );
    return 0;
}

int 
read_simlog(SimState *self, const char *fname)
{
    fprintf(stderr,"simutil: INFO reading:%s\n",fname);
    eventlog_t *log = eventlog_create(fname, "r");

    eventlog_event_t *msg = eventlog_read_next_event(log);    
    if (!msg)
        fprintf(stderr,"simutil: ERROR no messages found.\n");
    while (msg) {
        fprintf(stderr,"simutil: INFO transmitting:%s\n",msg->channel);
        lcm_publish (self->lc, msg->channel, msg->data, msg->datalen);
        msg = eventlog_read_next_event(log);
    }
    eventlog_destroy( log );
    return 0;
}



void
append_path(char *out, const int out_len, const char *path, const char *fname) {
    if (strlen (path))
        snprintf (out, out_len, "%s/%s", path, fname);
    else
        strcpy (out, fname);
}


char * 
generate_filename() {
    time_t t = time (NULL);
    struct tm ti;
    localtime_r (&t, &ti);
    char fname[512];
    char basename[512];

    snprintf (basename, sizeof (basename), "%d-%02d-%02d-sim",
              ti.tm_year+1900, ti.tm_mon+1, ti.tm_mday);
    
 
    /* Loop through possible file names until we find one that doesn't already
     * exist.  This way, we never overwrite an existing file. */
    int res;
    int filenum = 0;
    do {
        struct stat statbuf;
        snprintf (fname, sizeof(fname), "%s.%02d", basename, filenum);
        res = stat (fname, &statbuf);
        filenum++;
    } while (res == 0);

    if (errno != ENOENT) {
        perror ("Error: checking for previous sim states");
        return NULL;
    }

    return strdup(fname);
}



char *
choose_file(const char *caption, SimState *self, int load_flag)
{
    GtkWidget *dialog;

    char *fname=NULL;
    dialog = gtk_file_chooser_dialog_new (caption,
                                          GTK_WINDOW(self->viewer->window),
                                          load_flag?GTK_FILE_CHOOSER_ACTION_OPEN:GTK_FILE_CHOOSER_ACTION_SAVE,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          load_flag?GTK_STOCK_OPEN:GTK_STOCK_SAVE, 
                                          GTK_RESPONSE_ACCEPT,
                                          NULL);
    if (self->fname) {
        gtk_file_chooser_set_filename(GTK_FILE_CHOOSER (dialog), self->fname);
    }

    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *filename;
        
        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        if (filename)
            fname = strdup(filename);
        g_free (filename);
    }
    gtk_widget_destroy (dialog);
    return fname;
}      


static void
on_load_mi_activate (GtkMenuItem *mi, void *user_data)
{
    SimState *self = (SimState*) user_data;

    fprintf(stderr,"Load...\n");
    char *fname=choose_file("Load Sim",self,1);
    if (fname) {
        read_simlog(self,fname);
        if (self->fname) {
            free(self->fname);
            self->fname=fname;
        }
    }
    
}

static void
on_save_mi_activate (GtkMenuItem *mi, void *user_data)
{
    SimState *self = (SimState*) user_data;
    fprintf(stderr,"Save\n");
    char *fname = generate_filename();
    if (fname) {
        write_simlog(self,fname);
        if (self->fname) {
            free(self->fname);
            self->fname=fname;
        }
    }
}

static void
on_saveas_mi_activate (GtkMenuItem *mi, void *user_data)
{
    SimState *self = (SimState*) user_data;
    fprintf(stderr,"Save as...\n");
    char *fname=choose_file("Save as",self,0);
    if (fname) {
        write_simlog(self,fname);
        if (self->fname) {
            free(self->fname);
            self->fname=fname;
        }
    }
}


void setup_menu_simstate(Viewer *viewer, int priority)
{
    if (!viewer->simulation_flag)
        return;

    SimState *self = (SimState*) calloc(1, sizeof(SimState));

    self->viewer = viewer;
    self->lc = globals_get_lcm();

    // tearoff
    GtkWidget *tearoff = gtk_tearoff_menu_item_new();
    gtk_menu_append (GTK_MENU(viewer->file_menu), tearoff);
    gtk_widget_show (tearoff);

    // Load
    GtkWidget *load_mi = gtk_menu_item_new_with_mnemonic ("_Load sim...");
    gtk_menu_append (GTK_MENU(viewer->file_menu), load_mi);
    gtk_widget_show (load_mi);
    g_signal_connect (G_OBJECT (load_mi), "activate",
                      G_CALLBACK (on_load_mi_activate), self);

    // Save
    GtkWidget *save_mi = gtk_menu_item_new_with_mnemonic ("_Save sim");
    gtk_menu_append (GTK_MENU(viewer->file_menu), save_mi);
    gtk_widget_show (save_mi);
    g_signal_connect (G_OBJECT (save_mi), "activate",
                      G_CALLBACK (on_save_mi_activate), self);

    // Saveas
    GtkWidget *saveas_mi = gtk_menu_item_new_with_mnemonic ("Save sim _as...");
    gtk_menu_append (GTK_MENU(viewer->file_menu), saveas_mi);
    gtk_widget_show (saveas_mi);
    g_signal_connect (G_OBJECT (saveas_mi), "activate",
                      G_CALLBACK (on_saveas_mi_activate), self);


    
    self->msgs = g_hash_table_new(g_str_hash, g_str_equal);
    lcm_subscribe(self->lc, "SIM_RECTS", message_handler, self);
    lcm_subscribe(self->lc, "SIM_LANES", message_handler, self);
    lcm_subscribe(self->lc, "GOALS", message_handler, self);
    lcm_subscribe(self->lc, "POSE", message_handler, self);
}
