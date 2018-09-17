#ifndef _gtk_findreplace_h_
#define _gtk_findreplace_h_  1

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>  /* for GDK key values */

#if ! defined (WGTKSOURCEVIEW3) && ! defined (WGTKSOURCEVIEW4)
 #include <gtksourceview/gtksourceiter.h>
#endif

#include "gtk_appdata.h"
#include "gtk_common_dlg.h"

#define DLGFIND 1
#define DLGREPL 2

/* general functions */
GtkWidget *create_find_dlg (gpointer data);
GtkWidget *create_replace_dlg (gpointer data);

void dlg_replace_replace (GtkWidget *widget, gpointer *data);
void chk_realloc_ent (gpointer data);
void dumpopts (gpointer data);
void chk_existing_selection (gpointer data);
void find (gpointer data, const gchar *text);
void buffer_replace_selection (gpointer data, const gchar *replacetext);
// void find (gpointer data, const gchar *text, GtkTextIter *iter);

/* dialog callbacks */
void on_window_show             (GtkWidget *widget, gpointer data);
void entry_set_find_sensitive   (GtkWidget *widget, gpointer data);
void entry_set_repl_sensitive   (GtkWidget *widget, gpointer data);
void entry_find_activate        (GtkWidget *widget, gpointer data);
void entry_replace_activate     (GtkWidget *widget, gpointer data);
void chkregex_toggled           (GtkWidget *widget, gpointer data);
void chkplace_toggled           (GtkWidget *widget, gpointer data);
void chkcase_toggled            (GtkWidget *widget, gpointer data);
void chkwhole_toggled           (GtkWidget *widget, gpointer data);
void chkfrom_toggled            (GtkWidget *widget, gpointer data);
void chkback_toggled            (GtkWidget *widget, gpointer data);
void chkselect_toggled          (GtkWidget *widget, gpointer data);
void chkprompt_toggled          (GtkWidget *widget, gpointer data);
void btnregex_activate          (GtkWidget *widget, gpointer data);
void btnplace_activate          (GtkWidget *widget, gpointer data);
void btnfind_activate           (GtkWidget *widget, gpointer data);
void btnreplace_activate        (GtkWidget *widget, gpointer data);
void btnclose_activate          (GtkWidget *widget, gpointer data);

#endif
