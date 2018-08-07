#include "gtk_temp.h"

/*
 * window callbacks
 */
/* simple qute menu function */
void menu_file_quit_activate (GtkMenuItem *menuitem, gpointer data)
{
    gtk_main_quit ();

    if (data || menuitem) {}
}
void menu_showtb_activate (GtkMenuItem *menuitem, gpointer data)
{
    mainwin_t *mwin = data;
    mwin->showtoolbar = gtk_widget_get_visible (mwin->toolbar) ? FALSE : TRUE;
    gtk_widget_set_visible (mwin->toolbar, mwin->showtoolbar);

    if (menuitem) {}
}

void menu_showdoc_activate (GtkMenuItem *menuitem, gpointer data)
{
    mainwin_t *mwin = data;
    mwin->showdocwin = gtk_widget_get_visible (mwin->vboxtree) ? FALSE : TRUE;
    gtk_widget_set_visible (mwin->vboxtree, mwin->showdocwin);

    if (menuitem) {}
}

/** function to set the tab width to sz spaces based on font description */
void set_tab_size (PangoFontDescription *font_desc, GtkWidget *view, gint sz)
{
    GtkWidget *window;
    PangoTabArray *tab_array;
    PangoLayout *layout;
    gchar *tabstring;
    gint width, height, pixelwidth, i;

    window = gtk_widget_get_toplevel (view);

    tabstring = g_strdup_printf ("%*s", sz, " ");
    gtk_window_get_size (GTK_WINDOW(window), &width, &height);
    // g_print ("width: %d\nheight: %d\n", width, height);

    layout = gtk_widget_create_pango_layout (view, tabstring);
    pango_layout_set_font_description (layout, font_desc);
    pango_layout_get_pixel_size (layout, &pixelwidth, NULL);
    if (width) {
        tab_array = pango_tab_array_new (width/pixelwidth, TRUE);
        for (i = 0; i * pixelwidth < width; i++)
            pango_tab_array_set_tab (tab_array, i, PANGO_TAB_LEFT, i * pixelwidth);

        gtk_text_view_set_tabs (GTK_TEXT_VIEW(view), tab_array);
        pango_tab_array_free (tab_array);
    }
}
