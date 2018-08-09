#ifndef __gtk_doctree_h__
#define __gtk_doctree_h__  1

#include <gtk/gtk.h>

#include "gtk_appdata.h"

enum { COLNAME = 0, COLINST, NUMCOL };

/** struct for unique buffer info with proposed additions shown commented */
typedef struct {
    GtkSourceBuffer *buf;               /* textview buffer instance */
    gchar *filename;                    /* filename associated with buffer */
        // *lang_id;                       /* sourceview language ID */
    gint line, col;                     /* line, col when switching */
    // gboolean readonly;                  /* readonly flag */
} kinst_t;

/** function prototypes */
void doctree_append (GtkWidget *view, const gchar *filename);
GtkTreeModel *treemodel_init (gchar **argv);
GtkTreeModel *create_and_fill_model (mainwin_t *app);
GtkWidget *create_view_and_model (mainwin_t *app, gchar **argv);
kinst_t *inst_get_selected (gpointer data);

/** doctree callbacks */
void doctree_activate (GtkWidget *widget, gpointer data);

#endif

