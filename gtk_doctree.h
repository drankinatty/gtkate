#ifndef __gtk_doctree_h__
#define __gtk_doctree_h__  1

#include <gtk/gtk.h>

#include "gtk_appdata.h"

/** function prototypes */
void doctree_append (GtkWidget *view, const gchar *filename);
GtkTreeModel *treemodel_init (mainwin_t *app, gchar **argv);
GtkTreeModel *create_and_fill_model (mainwin_t *app);
GtkWidget *create_view_and_model (mainwin_t *app, gchar **argv);
kinst_t *inst_get_selected (gpointer data);
void doctree_for_each (GtkWidget *widget, mainwin_t *app);
void doctree_newfile (mainwin_t *app, const gchar *filename);

/** doctree callbacks */
void doctree_activate (GtkWidget *widget, gpointer data);

#endif

