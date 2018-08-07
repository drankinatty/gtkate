#ifndef __gtk_appdata_h__
#define __gtk_appdata_h__  1

#include <glib.h>           /* g_fprintf */
#include <glib/gprintf.h>
#ifndef GLIB236
 #include <glib-object.h>
#endif
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>  /* for GDK key values */

#if GLIB_MINOR_VERSION >= 36
 #define GLIB236 1
#endif

#define APPNAME "GtKate"

typedef struct mainwin {
    GtkWidget   *window,                /* application main windows */
                *toolbar,               /* applicate toolbar */
                *vboxtree,              /* expandable document tree */
                *doctreeview,           /* document tree view */
                *doctreemodel,          /* document tree model */
                *view;                  /* textview widget */

    guint       winwidth,
                winheight;

    gboolean    showtoolbar,
                showdocwin;

} mainwin_t;

#endif
