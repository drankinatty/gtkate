#ifndef __gtk_appdata_h__
#define __gtk_appdata_h__  1

#include <glib.h>           /* g_fprintf */
#include <gtk/gtk.h>

#if GLIB_MAJOR_VERSION >= 2
#if GLIB_MINOR_VERSION >= 46
 #define GLIB246 1
#endif
#if GLIB_MINOR_VERSION >= 40
 #define GLIB240 1
 #define HAVEKEYFILE 1
#endif
#if GLIB_MINOR_VERSION >= 36
 #define GLIB236 1
#endif
#if GLIB_MINOR_VERSION >= 30
 #define GLIB230 1
#endif
#endif

#ifndef GLIB236
 #include <glib-object.h>
#endif

#ifndef CHAR_BIT
 #define CHAR_BIT 8
#endif

/* build with GTKSOURCEVIEW2 by default */
#if defined (WGTKSOURCEVIEW3) || defined (WGTKSOURCEVIEW4)
 #include <gtksourceview/gtksource.h>
#else   /* WGTKSOURCEVIEW2 */
 #include <gtksourceview/gtksourceview.h>
 #include <gtksourceview/gtksourcelanguagemanager.h>
#endif

#if defined (_WIN32) || defined (_WIN64) || defined (_WINNT)
 #define HAVEMSWIN 1
#endif

#define APPNAME "GtKate"
#define APPSTR "GtKate Text Editor"

#define VER         "0.0.1"
#define SITE        "https://www.rankinlawfirm.com"
#define LICENSE     "LICENSE"
#define CFGDIR      "gtkate"
#define CFGFILE     "gtkate.cfg"
#define HISTFILE    "gtkate_history"
#define IMGDIR      "img"
#define LOGO        "gtkate.png"
#define ICON        "gtkate.ico"
#define RCFILE      "gtkrc-2.0_gtkate"

/* TODO: dynamically strip 'Program Files' from from app->exename
 * and replace with progra~1 or progra~2 below to preserve correct
 * logo display for 'portable' installs.
 */
#define WINPRG      "c:/progra~1/gtkate"
#define WINPRG86    "c:/progra~2/gtkate"
#define NIXSHARE    "/usr/share/gtkate"

#define EOL_LF          "\n"
#define EOL_CR          "\r"
#define EOL_CRLF        "\r\n"
#define EOL_NO          3
#define EOLNM_LF        "LF"
#define EOLNM_CR        "CR"
#define EOLNM_CRLF      "CRLF"
/* added for settings dialog */
#define EOLTXT_LF       "Linux / Unix / OSX"
#define EOLTXT_CRLF     "DOS / Windows"
#define EOLTXT_CR       "Macintosh (pre-OSX)"
#define EOLTXT_FILE     "Use EOL from File"
#define EOLTXT_OS       "Use OS Default"
#define EOLTXT_NO       5

#define MAXVIEW         4
#define MAXLE          32

enum eolorder { LF, CRLF, CR, FILE_EOL, OS_EOL };
enum {  IBAR_VISIBLE = 0x1,
        IBAR_LABEL_SELECT = 0x2,
        IBAR_VIEW_SENSITIVE = 0x4 };    /* infobar flags */
enum { LEFT, RIGHT, STKMAX  = 0x4 };    /* boolean stack constants */

enum { COLNAME = 0, COLINST, NUMCOL };  /* GtkTreeModel constants */

enum { TOP = 0, BOT };                  /* vpaned top/bottom scrolled_window */

/** struct for unique buffer info with proposed additions shown commented */
typedef struct {
    GtkSourceBuffer *buf;               /* textview buffer instance */
    GFileMonitor *mfpmon;               /* pointer to file monitor instance */
    gchar *filename,                    /* filename associated with buffer */
        *fname,                         /* filename only - without path */
        *fpath,                         /* file path */
        *fext,                          /* file extension */
        *mfpfn;                         /* filename monitored (for save-as) */

    gulong mfphandler;                  /* holds file monitor handler ID */

    gint line, col;                     /* line, col when switching */

    guint filemode, fileuid, filegid;   /* file permissions & ownership */

    GtkSourceLanguage *language;        /* sourceview language struct */
    /* don't store lang_id, write to cfg/key file */
    // const gchar     *lang_id;           /* sourceview language ID (for cfg) */

    const gchar     *comment_single;    /* single line comment */
    const gchar     *comment_blk_beg;   /* blobk comment begin */
    const gchar     *comment_blk_end;   /* block comment end */

    // gboolean readonly;                  /* readonly flag */
} kinst_t;

/** struct for each sourceview instance allowing split by showing/hiding
 *  containing pointers for the scrolled_window, sourceview and statusbar.
 */
typedef struct {
    GtkWidget *ebox;    /* bounding edit window vbox container */
    GtkWidget *ibox;    /* ibar vbox container */
    GtkWidget *swin;    /* scrolled window holding sourceview */
    GtkWidget *view;    /* sourceview */
    GtkWidget *sbar;    /* statusbar */
    kinst_t *inst;      /* pointer to current buffer inst in split */
} einst_t;

/** structure for application main window and settings */
typedef struct mainwin {
    /* user, path and config settings */
    const gchar     *user;              /* current username */
    gchar           *usrdatadir;        /* user data dir    */
    gchar           *sysdatadir;        /* system data dir  */
    gchar           *imgdir;            /* image directory  */

    /* config data */
    gchar           *cfgdir;            /* user config dir */
    gchar           *cfgfile;           /* user config file */
    GKeyFile        *keyfile;           /* key_file for config */

    /* window widgets */
    GtkWidget       *window,            /* application main windows */
                    *toolbar,           /* applicate toolbar */
                    *vboxtree,          /* expandable document tree */
                    *treeview,          /* document tree view */
                    *vboxedit;          /* bounding vbox for edit windows */
                    // *vpsplit;

    GtkTreeModel    *treemodel;         /* document tree model */

    einst_t         *einst[MAXVIEW];    /* pointers to editor instances */

    gint            nview,              /* no. of editor views shown */
                    nfiles,             /* no. of open files */
                    focused;            /* focused einst index */

    /* window layout/size */
    gint            winwidth,           /* main window width  */
                    winheight,          /* main window height */
                    treewidth,          /* document tree width */
                    swbordersz;         /* scrolled_window border size */

    /* sourceview variables, menus & settings */
    GtkSourceLanguageManager    *langmgr;
    GtkWidget       *stylelist;         /* sourceview styles menu list */
    GtkWidget       *hghltmenu;         /* sourceview syntax highlight menu */
    gchar           *laststyle;         /* sourceview last highlight style */
    gboolean        highlight;          /* syntax highlight enable/disable */

    /* source completion variables & settings */
    GtkSourceCompletion         *completion;
    gboolean        enablecmplt;        /* enable word-completion */
    guint           cmplwordsz;         /* completion minimum-word-size */

    /* find replace dailog data */
    gchar           **findtext;         /* storage of search terms */
    gchar           **reptext;          /* storage of replace terms */
    guint           nfentries;
    guint           nrentries;
    guint           fmax;
    guint           rmax;
    gboolean        optregex;
    gboolean        optplace;
    gboolean        optcase;
    gboolean        optwhole;
    gboolean        optfrom;
    gboolean        optback;
    gboolean        optselect;
    gboolean        optprompt;
    /* find replace placeholders
     * (TODO move to kinst_t struct) */
    GtkTextMark     *markfrom,          /* operation from mark */
                    *selstart,          /* selection start/end */
                    *selend;
    /* find replace results
     * (TODO move to kinst_t struct) */
    gboolean        txtfound;           /* prev search found text */
    GtkTextMark     *last_pos;          /* position of last match in buf */

    /* print data */
    GtkPrintSettings  *printsettings;   /* print settings object */
    GtkPageSetup    *printpgsetup;      /* page setup object */
    gdouble         margintop;          /* top    print margin */
    gdouble         marginbottom;       /* bottom print margin */
    gdouble         marginleft;         /* left   print margin */
    gdouble         marginright;        /* right  print margin */

    /* eol and character set handling */
    GtkTextMark     *cursor;
    GtkWidget       *eolLFMi;           /* radio button references */
    GtkWidget       *eolCRLFMi;         /*   for EOL tools-menu    */
    GtkWidget       *eolCRMi;
    gint            eol;                /* end-of-line */
    gint            oeol;               /* original eol */
    gint            eolos;              /* operating system default eol */
    gint            eoldefault;         /* default to override OS or file eol */
    gboolean        eolchg;             /* flag to prevent firing during UI init */
    gchar           *eolnm[EOL_NO];     /* ptrs to eol names */
    gchar           *eolstr[EOL_NO];    /* ptrs to eol strings */
    gchar           *eoltxt[EOLTXT_NO]; /* ptrs to eol descriptions */

    gint            bom;                /* Byte Order Mark */
        /* UTF-8  bom:  0xEF 0xBB 0xBF     (24-bit)
         * UTF-16 bom:  U+FEFF  FE FF      (16-bit Big Endian)
         *                      FF FE      (14-bit Little Endian)
         * (see definitions in gtk_appdata.c)
         */
    /* settings flags/file information */
    gboolean        showtoolbar,        /* flag to show/hide toolbar */
                    showdocwin,         /* flag to show/hide treeview */
                    winrestore,         /* flag to restore win size */
                    winszsaved;         /* flag win size saved by user */

    /* custom key handler flags */
    gboolean        ctrl_shift_right_fix;   /* select only whitespace or char */

    gchar           *fontname;          /* pango fontname */

    gint            tabstop;            /* number of spaces in tab */
    gint            softtab;            /* number of spaced to indent */
    gchar           *tabstring;         /* chars that make up tabstring */
    gboolean        expandtab;          /* insert spaces for tabs */
    gboolean        showtabs;           /* show tab markers */

    gboolean        smartbs;            /* use smart backspace */
    gboolean        smarthe;            /* use smart home & end */

    gboolean        indentauto;         /* auto-indent on return */
    gboolean        indentwspc;         /* indent with spaces not tabs */
    gboolean        indentmixd;         /* EMACS mixed spaces/tabs */

    gboolean        dynwrap;
    gboolean        showdwrap;
    gboolean        wraptxtcsr;
    gboolean        pgudmvscsr;

    gboolean        lineno;             /* show line numbers */
    gboolean        linehghlt;          /* current line highlight */

    gboolean        showmargin;         /* show right margin */
    gint            marginwidth;        /* column width for margin */

    gchar           *comment;           /* single-line comment string */
    GtkWidget       *cmtentry;          /* single-line comment entry */
    gboolean        cmtusesingle;       /* use single-line instead of block
                                         * comment */
    gboolean        overwrite;          /* ins/overwrite mode flag */
    gboolean        poscurend;          /* scroll to end of opened file */

    gboolean        posixeof;           /* enforce POSIX eof */
    gboolean        trimendws;          /* trim ending whitespace on save */

    gint            nrecent;            /* no. of recent files in chooser */

    /* treeview display parameters */
    guint           nuntitled;          /* bitfield "Untitled(n) in tree */

    /* GFileMonitor flag */
    gboolean        savecmd;            /* set on save to disable monitoring */

    /* boolean stack implementation
     * to provide keypress history.
     */
    guint           bstack[STKMAX];     /* sizeof guint * CHAR_BIT * STKMAX bits */
    guint           bindex;             /* bit index */

} mainwin_t;

/** function prototypes */

void mainwin_init (mainwin_t *app, char **argv);
void mainwin_destroy (mainwin_t *app);
kinst_t *buf_new_inst (const gchar *fn);
void buf_delete_inst (kinst_t *inst);
void einst_reset (einst_t *einst);
void einst_move (einst_t *tgt, einst_t *src);

/* filename functions */
void inst_free_filename (kinst_t *inst);
void inst_reset_state (kinst_t *inst);
void split_fname (kinst_t *inst);
gchar *uri_to_filename (const gchar *uri);
gchar *get_posix_filename (const gchar *fn);
gsize g_strlen (const gchar *s);
gboolean str2lower (gchar *str);
gboolean str2upper (gchar *str);
gboolean str2title (gchar *str);
void delete_mark_last_pos (gpointer data);

/* boolean stack functions */
void bstack_clear (mainwin_t *app);
int bstack_push (mainwin_t *app, int v);
int bstack_pop (mainwin_t *app);
int bstack_last (mainwin_t *app);

/* Untitled(n) bitfield management */
gint bit_check (guint *bf, gint n);
gint untitled_get_next (gpointer data);
void untitled_remove (gpointer data, gint n);

/* date & time functions */
gchar *get_local_datetime (void);

/* temp dialog functions */
void err_dialog (const gchar *errmsg);

/* create pixbuf function */
GdkPixbuf *create_pixbuf_from_file (const gchar *filename);

#endif
