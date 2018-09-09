#ifndef _gtk_textview_h_
#define _gtk_textview_h_  1

#include "gtk_appdata.h"
// #include "gtk_completionsv.h"
#include "gtk_treeview.h"

GtkWidget *create_scrolled_view (mainwin_t *app);
einst_t *ewin_create_split (gpointer data);
gboolean ewin_remove_split (gpointer data);
gboolean ewin_remove_view (gpointer data, einst_t *einst);

#endif
