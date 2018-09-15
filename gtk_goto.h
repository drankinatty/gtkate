#ifndef _gtk_goto_h_
#define _gtk_goto_h_  1

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>  /* for GDK key values */

#include "gtk_appdata.h"
#include "gtk_common_dlg.h"

GtkWidget *create_goto_dlg (gpointer data);
// void goto_spinbtn_preedit (GtkWidget *widget, gchar *txt, kwinst *app);
void goto_spinbtn_changed (GtkWidget *widget, gpointer data);
void goto_btnfind (GtkWidget *widget, gpointer data);
void goto_btnclose (GtkWidget *widget, gpointer data);

#endif
