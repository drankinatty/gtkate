#ifndef __gtk_doctree_h__
#define __gtk_doctree_h__  1

#include <gtk/gtk.h>

#include "gtk_appdata.h"

enum { COLNAME = 0, COLINST, NUMCOL };

typedef struct {
    GtkTextBuffer *buf;                 /* textview buffer instance */
    gchar *filename;                    /* filename associated with buffer */
    gint line, col;                     /* line, col when switching */
    // gboolean readonly;                  /* readonly flag */
} kinst_t;

GtkTreeModel *create_and_fill_model (void);
GtkWidget *create_view_and_model (void);

/** doctree callbacks */
void doctree_activate (GtkWidget *widget, gpointer data);

#endif

