#ifndef _gtk_textview_h_
#define _gtk_textview_h_  1

#include "gtk_appdata.h"
#include "gtk_doctree.h"
// #include "gtk_completionsv.h"

GtkWidget *create_textview_scrolledwindow (mainwin_t *app);
void set_tab_size (PangoFontDescription *font_desc, mainwin_t *app, gint sz);

#endif
