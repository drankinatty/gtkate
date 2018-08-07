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

// #ifdef WGTKSOURCEVIEW2
 #include <gtksourceview/gtksourceview.h>
 #include <gtksourceview/gtksourcelanguagemanager.h>
// #elif defined WGTKSOURCEVIEW3
//  #include <gtksourceview/gtksource.h>
// #elif defined WGTKSOURCEVIEW4
//  #include <gtksourceview/gtksource.h>
// #endif

#if defined (_WIN32) || defined (_WIN64) || defined (_WINNT)
 #define HAVEMSWIN 1
#endif

#define APPNAME "GtKate"

typedef struct mainwin {
    GtkWidget       *window,            /* application main windows */
                    *toolbar,           /* applicate toolbar */
                    *vboxtree,          /* expandable document tree */
                    *doctreeview,       /* document tree view */
                    *view;              /* textview widget */

    GtkTreeModel    *doctreemodel;      /* document tree model */

    guint           winwidth,
                    winheight;

    gboolean        showtoolbar,
                    showdocwin;

    gchar           *tabstring;

    gchar           *laststyle;         /* sourceview last highlight style */

    gboolean        lineno;             /* show line numbers */
    gboolean        linehghlt;          /* current line highlight */

    gboolean        showmargin;         /* show right margin */
    gint            marginwidth;        /* column width for margin */

    const gchar     *comment_single;    /* single line comment */
    const gchar     *comment_blk_beg;   /* blobk comment begin */
    const gchar     *comment_blk_end;   /* block comment end */

} mainwin_t;

#endif
