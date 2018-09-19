#ifndef _gtk_filebuf_h_
#define _gtk_filebuf_h_  1

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>  /* for GDK key values */
#include <glib/gstdio.h>

#include <gtksourceview/gtksourcestyleschememanager.h>

#include "gtk_appdata.h"
#include "gtk_charset.h"
#include "gtk_common_dlg.h"
#include "gtk_sourceview.h"
// #include "gtk_statusbar.h"
#include "gtk_textview.h"
#include "gtk_treeview.h"

// #include "gtk_filemon.h"

/** function prototypes */
void file_get_stats (kinst_t *file);

gboolean buffer_select_all (gpointer data);
gboolean buffer_deselect_all (gpointer data);
gboolean buffer_select_to_next_char (gpointer data);
gboolean buffer_select_to_prev_char (gpointer data);

void buffer_indent_lines (gpointer data, GtkTextIter *start, GtkTextIter *end);
void buffer_unindent_lines (gpointer data, GtkTextIter *start, GtkTextIter *end);
void buffer_indent_lines_fixed (gpointer data, GtkTextIter *start, GtkTextIter *end);
void buffer_unindent_lines_fixed (gpointer data, GtkTextIter *start, GtkTextIter *end);

void buffer_comment_lines (gpointer data, GtkTextIter *start, GtkTextIter *end);
void buffer_uncomment_lines (gpointer data, GtkTextIter *start, GtkTextIter *end);

void buffer_clear_focused (gpointer data);
void file_open_recent_dlg (gpointer data);

gboolean buffer_insert_file (gpointer data, kinst_t *inst, gchar *filename);
void buffer_file_insert_dlg (gpointer data);
void file_open (gpointer data, gchar *filename);
void file_save (gpointer data, gchar *filename);
void file_close (gpointer data);

gboolean joinlines (gchar *s);
void selection_for_each_char (GtkTextBuffer *buffer, gboolean (*fn) (gchar *));

#endif
