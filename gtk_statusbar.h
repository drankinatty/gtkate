#ifndef _gtk_statusbar_h_
#define _gtk_statusbar_h_  1

#include <gtk/gtk.h>

#include "gtk_appdata.h"
#include "gtk_charset.h"

/* statusbar functions */
void status_menuitem_label (GtkMenuItem *menuitem, gpointer data);
void status_pop (GtkWidget *widget, gpointer data);
void status_set_default (gpointer data);

#endif
