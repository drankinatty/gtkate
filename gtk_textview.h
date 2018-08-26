#ifndef _gtk_textview_h_
#define _gtk_textview_h_  1

#include "gtk_appdata.h"
#include "gtk_doctree.h"
// #include "gtk_completionsv.h"

GtkWidget *create_scrolled_view (mainwin_t *app);
einst_t *ewin_create_split (gpointer data);
void ewin_remove_split (gpointer data);

#endif
