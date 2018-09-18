#ifndef _gtk_common_dlg_h_
#define _gtk_common_dlg_h_  1

#include <gtk/gtk.h>

#include "gtk_appdata.h"

// void err_dialog (const gchar *errmsg);
void err_dialog_win (gpointer data, const gchar *errmsg);
void font_select_dialog (GtkWidget *widget, gpointer data);
void dlg_info (const gchar *msg, const gchar *title);
void dlg_info_win (gpointer data, const gchar *msg, const gchar *title);
gboolean dlg_yes_no_msg (gpointer data, const gchar *msg, const gchar *title,
                            gboolean default_return);
gchar *get_open_filename (gpointer data);
gchar *get_save_filename (gpointer data);

#endif