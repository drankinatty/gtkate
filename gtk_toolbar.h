#ifndef _gtk_toolbar_h_
#define _gtk_toolbar_h_  1

#include "gtk_menu.h"

GtkWidget *create_toolbar (GtkAccelGroup *mainaccel, gpointer data);
/*
 * see: https://bugzilla.gnome.org/show_bug.cgi?id=779605
 */
void toolbar_file_open_recent (GtkWidget *widget, gpointer data);


#endif
