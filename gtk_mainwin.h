#ifndef __gtk_mainwin_h__
#define __gtk_mainwin_h__  1

#include <glib/gprintf.h>
// #include <gdk/gdkkeysyms.h>  /* for GDK key values (moved appdata.h) */

#include "gtk_appdata.h"
#include "gtk_common_dlg.h"
#include "gtk_menu.h"
#include "gtk_textview.h"
#include "gtk_toolbar.h"
#include "gtk_treeview.h"

GtkWidget *create_window (mainwin_t *app);

#endif

