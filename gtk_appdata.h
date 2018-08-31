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
    gchar *filename,                    /* filename associated with buffer */
        *fname,                         /* filename only - without path */
        *fpath,                         /* file path */
        *fext;                          /* file extension */

    gint line, col;                     /* line, col when switching */

    guint filemode, fileuid, filegid;   /* file permissions & ownership */

    const gchar     *lang_id;           /* sourceview language ID */

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
                    *vboxedit,          /* bounding vbox for edit windows */
                    *vpsplit;

    GtkTreeModel    *treemodel;         /* document tree model */

    /* window layout/size */
    gint            winwidth,           /* main window width  */
                    winheight,          /* main window height */
                    treewidth,          /* document tree width */
                    swbordersz;         /* scrolled_window border size */

    /* settings flags/file information */
    gboolean        showtoolbar,        /* flag to show/hide toolbar */
                    showdocwin,         /* flag to show/hide treeview */
                    winrestore,         /* flag to restore win size */
                    winszsaved;         /* flag win size saved by user */

    einst_t         *einst[MAXVIEW];    /* pointers to editor instances */

    gint            nview,              /* no. of editor views shown */
                    nfiles,             /* no. of open files */
                    focused;            /* focused einst index */

    gchar           *fontname;          /* pango fontname */

    gint            tabstop;            /* number of spaces in tab */
    gint            softtab;            /* number of spaced to indent */
    gchar           *tabstring;         /* chars that make up tabstring */

    gchar           *laststyle;         /* sourceview last highlight style */

    gboolean        indentauto;         /* auto-indent on return */

    gboolean        lineno;             /* show line numbers */
    gboolean        linehghlt;          /* current line highlight */

    gboolean        showmargin;         /* show right margin */
    gint            marginwidth;        /* column width for margin */

    gint            nrecent;            /* no. of recent files in chooser */

    /* treeview display parameters */
    guint           nuntitled;          /* bitfield "Untitled(n) in tree */

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

/* filename functions */
void inst_free_filename (kinst_t *inst);
void inst_reset_state (kinst_t *inst);
void split_fname (kinst_t *inst);
gchar *uri_to_filename (const gchar *uri);
gchar *get_posix_filename (const gchar *fn);

/* boolean stack functions */
void bstack_clear (mainwin_t *app);
int bstack_push (mainwin_t *app, int v);
int bstack_pop (mainwin_t *app);
int bstack_last (mainwin_t *app);

/* Untitled(n) bitfield management */
gint bit_check (guint *bf, gint n);
gint untitled_get_next (mainwin_t *app);
void untitled_remove (mainwin_t *app, gint n);

/* date & time functions */
gchar *get_local_datetime (void);

/* temp dialog functions */
void err_dialog (const gchar *errmsg);

/* create pixbuf function */
GdkPixbuf *create_pixbuf_from_file (const gchar *filename);

#endif
