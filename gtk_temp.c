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
    mainwin_t *app = data;
    app->showtoolbar = gtk_widget_get_visible (app->toolbar) ? FALSE : TRUE;
    gtk_widget_set_visible (app->toolbar, app->showtoolbar);

    if (menuitem) {}
}

void menu_showdoc_activate (GtkMenuItem *menuitem, gpointer data)
{
    mainwin_t *app = data;
    app->showdocwin = gtk_widget_get_visible (app->vboxtree) ? FALSE : TRUE;
    gtk_widget_set_visible (app->vboxtree, app->showdocwin);

    if (menuitem) {}
}

void menu_createview_activate (GtkMenuItem *menuitem, gpointer data)
{
    mainwin_t *app = data;
#ifdef DEBUG
    g_print ("menu_createview_activate: %d(%d) -> ", app->nview, app->focused);
#endif
    ewin_create_split (app);
#ifdef DEBUG
    g_print ("%d(%d)\n", app->nview, app->focused);
#endif

    if (menuitem) {}
}

void menu_removeview_activate (GtkMenuItem *menuitem, gpointer data)
{
    mainwin_t *app = data;

#ifdef DEBUG
    g_print ("menu_removeview_activate: %d(%d) -> ", app->nview, app->focused);
#endif
    ewin_remove_split (app);
#ifdef DEBUG
    g_print ("%d(%d)\n", app->nview, app->focused);
#endif

    if (menuitem) {}
}

GtkWidget *create_temp_menu (mainwin_t *app, GtkAccelGroup *mainaccel)
{
    GtkWidget *menubar;             /* menu container   */

    GtkWidget *fileMenu;            /* file menu        */
    GtkWidget *fileMi;
    GtkWidget *sep;
    GtkWidget *quitMi;
    GtkWidget *showtbMi;
    GtkWidget *showdocMi;
    GtkWidget *addsplitMi;
    GtkWidget *rmsplitMi;

    menubar = gtk_menu_bar_new ();
    // gtk_widget_show (menubar);

    fileMenu = gtk_menu_new ();
    fileMi = gtk_menu_item_new_with_mnemonic ("_File");
    sep = gtk_separator_menu_item_new ();
    quitMi   = gtk_image_menu_item_new_from_stock (GTK_STOCK_QUIT,
                                                   NULL);
    showtbMi = gtk_image_menu_item_new_from_stock (GTK_STOCK_CONVERT,
                                                   NULL);
    gtk_menu_item_set_label (GTK_MENU_ITEM (showtbMi), "_Show/Hide Toolbar");
    showdocMi = gtk_image_menu_item_new_from_stock (GTK_STOCK_DIRECTORY,
                                                   NULL);
    gtk_menu_item_set_label (GTK_MENU_ITEM (showdocMi), "Show/Hide _Documents");
    addsplitMi = gtk_image_menu_item_new_from_stock (GTK_STOCK_ADD,
                                                   NULL);
    gtk_menu_item_set_label (GTK_MENU_ITEM (addsplitMi), "Split Editor _Window");
    rmsplitMi = gtk_image_menu_item_new_from_stock (GTK_STOCK_REMOVE,
                                                   NULL);
    gtk_menu_item_set_label (GTK_MENU_ITEM (rmsplitMi), "Re_move Current Split");

    /* create entries under 'File' then add to menubar */
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (fileMi), fileMenu);
    gtk_menu_shell_append (GTK_MENU_SHELL (fileMenu), sep);
    gtk_menu_shell_append (GTK_MENU_SHELL (fileMenu), quitMi);
    sep = gtk_separator_menu_item_new ();
    gtk_menu_shell_append (GTK_MENU_SHELL (fileMenu), sep);
    gtk_menu_shell_append (GTK_MENU_SHELL (fileMenu), showtbMi);
    gtk_menu_shell_append (GTK_MENU_SHELL (fileMenu), showdocMi);
    gtk_menu_shell_append (GTK_MENU_SHELL (fileMenu), addsplitMi);
    gtk_menu_shell_append (GTK_MENU_SHELL (fileMenu), rmsplitMi);
    gtk_menu_shell_append (GTK_MENU_SHELL (menubar), fileMi);
    gtk_widget_add_accelerator (quitMi, "activate", mainaccel,
                                GDK_KEY_q, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator (showtbMi, "activate", mainaccel,
                                GDK_KEY_t, GDK_CONTROL_MASK,
                                GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator (showdocMi, "activate", mainaccel,
                                GDK_KEY_d, GDK_CONTROL_MASK,
                                GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator (addsplitMi, "activate", mainaccel,
                                GDK_KEY_s, GDK_CONTROL_MASK,
                                GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator (rmsplitMi, "activate", mainaccel,
                                GDK_KEY_r, GDK_CONTROL_MASK,
                                GTK_ACCEL_VISIBLE);

    /* File Menu */
    g_signal_connect (G_OBJECT (quitMi), "activate",        /* file Quit    */
                      G_CALLBACK (menu_file_quit_activate), NULL);

    g_signal_connect (G_OBJECT (showtbMi), "activate",      /* show toolbar */
                      G_CALLBACK (menu_showtb_activate), app);

    g_signal_connect (G_OBJECT (showdocMi), "activate",      /* show toolbar */
                      G_CALLBACK (menu_showdoc_activate), app);

    g_signal_connect (G_OBJECT (addsplitMi), "activate",      /* show toolbar */
                      G_CALLBACK (menu_createview_activate), app);

    g_signal_connect (G_OBJECT (rmsplitMi), "activate",      /* show toolbar */
                      G_CALLBACK (menu_removeview_activate), app);

    gtk_widget_show_all (menubar);

    return menubar;
}

GtkWidget *create_temp_toolbar (mainwin_t *app, GtkAccelGroup *mainaccel)
{
    GtkToolItem *tbexit;            /* toolbar */
    GtkToolItem *info;
    GtkToolItem *split;
    GtkToolItem *rmsplit;

    app->toolbar = gtk_toolbar_new ();
    // gtk_widget_show (app->toolbar);

    gtk_container_set_border_width(GTK_CONTAINER(app->toolbar), 2);
    gtk_toolbar_set_show_arrow (GTK_TOOLBAR(app->toolbar), TRUE);
    gtk_toolbar_set_style(GTK_TOOLBAR(app->toolbar), GTK_TOOLBAR_ICONS);

    tbexit = gtk_tool_button_new_from_stock(GTK_STOCK_QUIT);
    gtk_tool_item_set_homogeneous (tbexit, FALSE);
    gtk_toolbar_insert(GTK_TOOLBAR(app->toolbar), tbexit, -1);
    gtk_widget_set_tooltip_text (GTK_WIDGET(tbexit), "Quit ");

    info = gtk_tool_button_new_from_stock(GTK_STOCK_DIALOG_INFO);
    gtk_tool_item_set_homogeneous (info, FALSE);
    gtk_toolbar_insert(GTK_TOOLBAR(app->toolbar), info, -1);
    gtk_widget_set_tooltip_text (GTK_WIDGET(info), "Tree Output to term ");

    split = gtk_tool_button_new_from_stock(GTK_STOCK_ADD);
    gtk_tool_item_set_homogeneous (split, FALSE);
    gtk_toolbar_insert(GTK_TOOLBAR(app->toolbar), split, -1);
    gtk_widget_set_tooltip_text (GTK_WIDGET(split), "Split Window ");

    rmsplit = gtk_tool_button_new_from_stock(GTK_STOCK_REMOVE);
    gtk_tool_item_set_homogeneous (rmsplit, FALSE);
    gtk_toolbar_insert(GTK_TOOLBAR(app->toolbar), rmsplit, -1);
    gtk_widget_set_tooltip_text (GTK_WIDGET(rmsplit), "Remove Current Split ");

    /* Toolbar uses same menu callback */
    g_signal_connect (G_OBJECT (tbexit), "clicked",         /* file Quit    */
                      G_CALLBACK (menu_file_quit_activate), NULL);

    g_signal_connect (G_OBJECT (info), "clicked",          /* info btn      */
                      G_CALLBACK (doctree_for_each), app);

    g_signal_connect (G_OBJECT (split), "clicked",         /* split btn     */
                      G_CALLBACK (menu_createview_activate), app);

    g_signal_connect (G_OBJECT (rmsplit), "clicked",         /* split btn     */
                      G_CALLBACK (menu_removeview_activate), app);

    gtk_widget_show_all (app->toolbar);

    return app->toolbar;

    if (mainaccel) {}
}