#ifndef _gtk_print_h_
#define _gtk_print_h_  1

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>  /* for GDK key values */

#include "gtk_appdata.h"
#include "gtk_common_dlg.h"

void do_print (gpointer data);
void do_page_setup (gpointer data);

#endif
