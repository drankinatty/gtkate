#ifndef __gtk_temp_h__
#define __gtk_temp_h__  1

#include <gdk/gdkkeysyms.h>  /* for GDK key values */

#include "gtk_appdata.h"
#include "gtk_doctree.h"
#include "gtk_textview.h"

void menu_file_new_activate (GtkMenuItem *menuitem, gpointer data);
void menu_file_close_activate (GtkMenuItem *menuitem, gpointer data);
void menu_file_quit_activate (GtkMenuItem *menuitem, gpointer data);
void menu_showtb_activate (GtkMenuItem *menuitem, gpointer data);
void menu_showdoc_activate (GtkMenuItem *menuitem, gpointer data);
// void set_tab_size (PangoFontDescription *font_desc, GtkWidget *view, gint sz);
GtkWidget *create_temp_menu (gpointer data, GtkAccelGroup *mainaccel);
GtkWidget *create_temp_toolbar (mainwin_t *app, GtkAccelGroup *mainaccel);

#endif

