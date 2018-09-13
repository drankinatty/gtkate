#ifndef _gtk_filebuf_h_
#define _gtk_filebuf_h_  1

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>  /* for GDK key values */
#include <glib/gstdio.h>

#include <gtksourceview/gtksourcestyleschememanager.h>

#include "gtk_appdata.h"
// #include "gtk_charset.h"
#include "gtk_common_dlg.h"
#include "gtk_sourceview.h"
// #include "gtk_statusbar.h"
#include "gtk_textview.h"
#include "gtk_treeview.h"

// #include "gtk_filemon.h"

/** function prototypes */
void file_get_stats (kinst_t *file);
void buffer_clear_focused (gpointer data);
gboolean buffer_insert_file (gpointer data, kinst_t *inst, gchar *filename);
void file_open (gpointer data, gchar *filename);
void file_save (gpointer data, gchar *filename);
void file_close (gpointer data);

#endif
