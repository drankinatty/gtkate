#ifndef _gtk_settings_h_
#define _gtk_settings_h_  1

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>  /* for GDK key values */
#include <gtksourceview/gtksourcecompletion.h>

#include "gtk_appdata.h"
#include "gtk_common_dlg.h"

/* settings functions */
GtkWidget *create_settings_dlg (gpointer data);
void settings_btncancel (GtkWidget *widget, gpointer data);
void settings_btnok (GtkWidget *widget, gpointer data);
void settings_fontbtn (GtkWidget *widget, gpointer data);
void chkdynwrap_toggled (GtkWidget *widget, gpointer data);
void chkshowdwrap_toggled (GtkWidget *widget, gpointer data);
void chksmarthe_toggled (GtkWidget *widget, gpointer data);
void chkwraptxtcsr_toggled (GtkWidget *widget, gpointer data);
void chkpgudmvscsr_toggled (GtkWidget *widget, gpointer data);
void chkcsrtarrow_toggled (GtkWidget *widget, gpointer data);
void chkwinrestore_toggled (GtkWidget *widget, gpointer data);
void btnwinsavesize (GtkWidget *widget, gpointer data);
void chkexpandtab_toggled (GtkWidget *widget, gpointer data);
void chksmartbs_toggled (GtkWidget *widget, gpointer data);
void chkshowtabs_toggled (GtkWidget *widget, gpointer data);
void spintab_changed (GtkWidget *widget, gpointer data);
void chkindentwspc_toggled (GtkWidget *widget, gpointer data);
void chkindentauto_toggled (GtkWidget *widget, gpointer data);
void chkindentmixd_toggled (GtkWidget *widget, gpointer data);
void spinindent_changed (GtkWidget *widget, gpointer data);
void chkcmtusesngl_toggled (GtkWidget *widget, gpointer data);
void entry_comment_activate (GtkWidget *widget, gpointer data);
void spinwordsize_changed (GtkWidget *widget, gpointer data);
void chkposcurend_toggled (GtkWidget *widget, gpointer data);
void cmbeoldefault_changed (GtkWidget *widget, gpointer data);
void chkunsaved_toggled (GtkWidget *widget, gpointer data);
void chktrimendws_toggled (GtkWidget *widget, gpointer data);
void chkposixeof_toggled (GtkWidget *widget, gpointer data);
void spinrecent_changed (GtkWidget *widget, gpointer data);
void chklinehghlt_toggled (GtkWidget *widget, gpointer data);
void chkshowmargin_toggled (GtkWidget *widget, gpointer data);
void spinmarginwidth_changed (GtkWidget *widget, gpointer data);
void chkenablecmplt_toggled (GtkWidget *widget, gpointer data);

#endif
