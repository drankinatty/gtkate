#ifndef _gtk_sourceviewmisc_h_
#define _gtk_sourceviewmisc_h_  1

#include <glib.h>
#include <gtk/gtk.h>
#include <gtksourceview/gtksourcestyleschememanager.h>

#include "gtk_appdata.h"

GtkWidget *highlight_build_menu (gpointer data);
GtkWidget *sourceview_syntax_styles_menu (GtkWidget *menu, gpointer data);

void sourceview_set_syntax_laststyle (gpointer data);
void sourceview_set_comment_syntax (gpointer data);
void sourceview_guess_language (gpointer data);
void sourceview_get_languange_info (gpointer data);  /* debug function */
void sourceview_get_scheme_info (gpointer data);

#endif
