TokIdentifier = "Identifier"
TokOpenStruct = "OpenStruct"
TokCloseStruct = "CloseStruct"
TokAssign = "Assign"
TokEndStatement = "EndStatement"
TokString = "String"
TokEOF = "EOF"
TokComment = "Comment"

class Token:
    def __init__ (self, type, val):
        self.type = type
        self.val = val

class Tokenizer:
    def __init__ (self, f):
        self.f = f
        self.unget_char = None
        self.line_pos = 0
        self.line_len = 0
        self.line_buf = ""
        self.line_num = 1

    def _next_char (self):
        if self.unget_char is not None:
            c = self.unget_char
            self.unget_char = None
            return c
        else:
            if self.line_pos == self.line_len:
                self.line_buf = self.f.readline ()
                if not len (self.line_buf):
                    return ''
                self.line_len = len (self.line_buf)
                self.line_pos = 0

            c = self.line_buf[self.line_pos]
            self.line_pos += 1

        if c == '\n':
            self.line_num += 1
        return c
    
    def _ungetc (self, c):
        if not c: return
        self.unget_char = c
    
    def _unescape (self, c):
        d = { "n": "\n",
              "r": "\r",
              "t": "\t" }
        if c in d: return d[c]
        return c

    def next_token (self):
        c = self._next_char ()

        while c and c.isspace (): 
            c = self._next_char ()
        if not c: return Token (TokEOF, "")

        simple_tokens = { \
                "=" : TokAssign,
                ";" : TokEndStatement,
                "{" : TokOpenStruct,
                "}" : TokCloseStruct
                }
        if c in simple_tokens:
            return Token (simple_tokens[c], c)
        
        tok_chars = [ c ]

        if c == "#":
            while True:
                c = self._next_char ()
                if not c or c == "\n": 
                    return Token (TokComment, "".join (tok_chars))
                tok_chars.append (c)

        if c == "\"":
            tok_chars = []
            while True:
                c = self._next_char ()
                if c == "\n":
                    se = SyntaxError ("Unterminated string constant")
                    se.filename = ""
                    se.lineno = self.line_num
                    se.offset = self.line_pos
                    se.text = self.line_buf
                    raise se
                if c == "\\":   c = self._unescape (self._next_char ())
                elif not c or c == "\"": 
                    return Token (TokString, "".join (tok_chars))
                tok_chars.append (c)

        if c.isalpha () or c == "_":
            while True:
                c = self._next_char ()
                if not c.isalnum () and c not in "_-":
                    self._ungetc (c)
                    return Token (TokIdentifier, "".join (tok_chars))
                tok_chars.append (c)

        se = SyntaxError ()
        se.filename = ""
        se.lineno = self.line_num
        se.offset = self.line_pos
        se.text = self.line_buf
        raise se

class CommandNode:
    def __init__ (self):
        self.name = None
        self.host = None

    def _get_str (self, indent = 0):
        s = "    " * indent
        lines = []
        lines.append (s + "cmd {")
        lines.append (s + "    exec = \"%s\";" % self.name)
        if self.host is not None:
            lines.append (s + "    host = \"%s\";" % self.host)
        lines.append (s + "}")
        return ("\n".join (lines))

    def __str__ (self):
        return self._get_str ()

class GroupNode:
    def __init__ (self, name):
        self.name = name
        self.commands = []

    def add_command (self, command):
        self.commands.append (command)

    def __str__ (self):
        val = "group \"%s\" {" % self.name
        for cmd in self.commands: val = val + "\n" + cmd._get_str (1)
        val = val + "\n}"
        return val

class ConfigNode:
    def __init__ (self):
        self.groups = {}
        self.add_group (GroupNode (""))

    def has_group (self, group_name):
        return group_name in self.groups

    def get_group (self, group_name):
        return self.groups[group_name]

    def add_group (self, group):
        assert group.name not in self.groups
        self.groups[group.name] = group

    def add_command (self, command):
        none_group = self.groups[""]
        none_group.add_command (command)

    def __str__ (self):
        val = ""
        none_group = self.groups[""]
        for cmd in none_group.commands: 
            val = val + "\n" + cmd._get_str (0)

        return val + "\n" + \
                "\n".join ([str (group) for group in self.groups.values () \
                if group.name is not "" ])

class ParseError (Exception):
    def __init__ (self, tokenizer, token, msg):
        self.lineno = tokenizer.line_num
        self.offset = tokenizer.line_pos
        self.text = tokenizer.line_buf
        self.token = token
        self.msg = msg

    def __str__ (self):
        ntabs = self.text.count ("\t")
        s = """%s

line %d col %s token %s
%s
""" % (self.msg, self.lineno, self.offset, self.token.val, self.text)
        s += " " * (self.offset-1 - \
                len (self.token.val) - ntabs) + "\t" * ntabs + "^"
        return s

class Parser:
    def __init__ (self):
        self.tokenizer = None
        self._cur_tok = None
        self._next_tok = None

    def _get_token (self):
        self._cur_tok = self._next_tok
        self._next_tok = self.tokenizer.next_token ()
        while self._next_tok.type == TokComment: 
            self._next_tok = self.tokenizer.next_token ()
        return self._cur_tok
    
    def _eat_token (self, tok_type):
        if self._next_tok and self._next_tok.type == tok_type:
            self._get_token ()
            return True
        return False

    def _fail (self, msg):
        raise ParseError (self.tokenizer, self._cur_tok, msg)

    def _parse_command_param_list (self, cmd):
        if not self._eat_token (TokIdentifier): return
        if self._cur_tok.val == "exec":
            if not self._eat_token (TokAssign): self._fail ("Expected '='")
            if not self._eat_token (TokString):
                self._fail ("Expected string literal for command exec")
            cmd.name = self._cur_tok.val
            if not self._eat_token (TokEndStatement):
                self._fail ("Expected ';'")
        elif self._cur_tok.val == "host":
            if not self._eat_token (TokAssign): sel.f_fail ("Expected '='")
            if not self._eat_token (TokString):
                self._fail ("Expected string literal for command host")
            cmd.host = self._cur_tok.val
            if not self._eat_token (TokEndStatement):
                self._fail ("Expected ';'")
        return self._parse_command_param_list (cmd)

    def _parse_command (self):
        cmd = CommandNode ()
        if not self._eat_token (TokOpenStruct): self._fail ("Expected '{'")
        self._parse_command_param_list (cmd)
        if not self._eat_token (TokCloseStruct): self._fail ("Expected '}'")
        if not cmd.name:
            self._fail ("Invalid command defined -- no executable specified")
        return cmd

    def _parse_command_list (self):
        cmds = []
        while self._eat_token (TokIdentifier) and self._cur_tok.val == "cmd":
            cmds.append (self._parse_command ())
        return cmds

    def _parse_group (self):
        if not self._eat_token (TokString):
            self._fail ("Expected group name string")

        name = self._cur_tok.val
        group = GroupNode (name)

        if not self._eat_token (TokOpenStruct): self._fail ("Expected '{'")

        for cmd in self._parse_command_list ():
            group.add_command (cmd)

        if not self._eat_token (TokCloseStruct): self._fail ("Expected '}'")
        return group

    def _parse_listdecl (self, node):
        if self._eat_token (TokEOF):
            return node

        if not self._eat_token (TokIdentifier) or \
                self._cur_tok.val not in [ "cmd", "group" ]:
            self._fail ("Expected command or group declaration")

        if self._cur_tok.val == "cmd":
            node.add_command (self._parse_command ())

        if self._cur_tok.val == "group":
            node.add_group (self._parse_group ())

        return self._parse_listdecl (node)

    def parse (self, f):
        self.tokenizer = Tokenizer (f)
        self._cur_tok = None
        self._next_tok = None
        self._get_token ()
        return self._parse_listdecl (ConfigNode ())

def config_from_filename (fname):
    return Parser ().parse (file (fname))

if __name__ == "__main__":
    import sys
    try:
        fname = sys.argv[1]
    except IndexError:
        print "usage: sheriff_config.py <fname>"
        sys.exit (1)

    print config_from_filename (fname)
