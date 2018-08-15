#ifndef __gtk_mainwin_h__
#define __gtk_mainwin_h__  1

#include <glib/gprintf.h>
#include <gdk/gdkkeysyms.h>  /* for GDK key values */

#include "gtk_appdata.h"
#include "gtk_doctree.h"
#include "gtk_textview.h"
#include "gtk_temp.h"

GtkWidget *create_window (mainwin_t *app);

#endif

