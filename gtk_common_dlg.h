#ifndef _gtk_common_dlg_h_
#define _gtk_common_dlg_h_  1

#include <gtk/gtk.h>

#include "gtk_appdata.h"

typedef struct {    /* for info_bar btntext & response_id pairs */
    const gchar *btntext;
    gint response_id;
} ibbtndef;

// void err_dialog (const gchar *errmsg);
void err_dialog_win (gpointer data, const gchar *errmsg);

void font_select_dialog (GtkWidget *widget, gpointer data);

void dlg_info (const gchar *msg, const gchar *title);
void dlg_info_win (gpointer data, const gchar *msg, const gchar *title);
gboolean dlg_yes_no_msg (gpointer data, const gchar *msg, const gchar *title,
                            gboolean default_return);

void show_info_bar_choice (const gchar *msg, gint msgtype,
                            ibbtndef *btndef,
                            void (*fn_response)(GtkInfoBar *bar,
                                    gint response_id,
                                    gpointer data),
                            gpointer data);
void show_info_bar_ok (const gchar *msg, gint msgtype, gpointer data);

GSList *get_open_filename (gpointer data);
gchar *get_save_filename (gpointer data);

#endif
