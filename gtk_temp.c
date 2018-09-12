#include "gtk_temp.h"

/*
 * menu callback functions
 *
 *  _File menu
 */
void menu_file_new_activate (GtkMenuItem *menuitem, gpointer data)
{
    mainwin_t *app = data;

    treeview_append (app, NULL);

    if (menuitem) {}
}

void menu_file_open_activate (GtkMenuItem *menuitem, gpointer data)
{
    /* get new filename */
    gchar *newfile = get_open_filename (data);

    /* open newfile in current or new editor instance */
    file_open (data, newfile);

    /* newfile copied in get_posix_filename(), so free newfile */
    g_free (newfile);

    if (menuitem) {}
}

// void menu_file_save_activate (GtkMenuItem *menuitem, gpointer data)
// {
//     // buffer_save_file (app, NULL);
//     // gtk_widget_grab_focus (app->view);
//
//     gchar *savefile = get_save_filename (data);
//     g_print ("save filename: %s\n", savefile);
//
//     /* temporary */
//     g_free (savefile);
//
//     if (menuitem) {}
// }

void menu_file_save_activate (GtkMenuItem *menuitem, gpointer data)
{
    mainwin_t *app = data;
    file_save (data, NULL);
    gtk_widget_grab_focus (app->einst[app->focused]->view);

    if (menuitem) {}
}

void menu_file_saveas_activate (GtkMenuItem *menuitem, gpointer data)
{
    mainwin_t *app = data;
    gchar *filename = NULL;

    filename = get_save_filename (data);
    if (filename)
        file_save (data, filename);
//     else
//         show_info_bar_ok ("Warning: Save of File Canceled!",
//                             GTK_MESSAGE_INFO, app);

    gtk_widget_grab_focus (app->einst[app->focused]->view);

    if (menuitem) {}
}

void menu_file_close_activate (GtkMenuItem *menuitem, gpointer data)
{
    mainwin_t *app = data;
    file_close (app);

    if (menuitem) {}
}

/* simple quit menu function */
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

void help_about (gpointer data)
{
    mainwin_t *app = data;
    gchar *buf, *logofn, *license;
    gsize fsize;
    GdkPixbuf *logo = NULL;

    static const gchar *const authors[] = {
            "David C. Rankin, J.D,P.E. <drankinatty@gmail.com>",
            NULL
    };

    static const gchar copyright[] = \
            "Copyright \xc2\xa9 2018 David C. Rankin";

    static const gchar comments[] = APPSTR;

    /* create pixbuf from logofn to pass to show_about_dialog */
//     if ((logofn = get_logo_filename (app))) {
    if ((logofn = g_strdup_printf ("%s/%s", app->imgdir, LOGO))) {
        // g_print ("logofn : '%s'\n", logofn);
        logo = create_pixbuf_from_file (logofn);
        g_free (logofn);
    }

#ifndef HAVEMSWIN
    /* Archlinux */
    license = g_strdup_printf ("%s/%s/%s", "/usr/share/licenses", CFGDIR, LICENSE);
    if (g_file_test (license, G_FILE_TEST_EXISTS))
        goto gotnixlic;

    /* openSuSE */
    g_free (license);
    license = NULL;
    license = g_strdup_printf ("%s/%s/%s", "/usr/share/doc/packages", CFGDIR, LICENSE);
    if (g_file_test (license, G_FILE_TEST_EXISTS))
        goto gotnixlic;

    /* generic */
    g_free (license);
    license = NULL;
    license = g_strdup_printf ("%s/%s", app->sysdatadir, LICENSE);

    gotnixlic:;
#else
    /* win32/win64 */
    license = g_strdup_printf ("%s\\%s", app->sysdatadir, LICENSE);
#endif

    /* check if license read into buf */
    if (g_file_get_contents (license, &buf, &fsize, NULL)) {

        if (logo)   /* show logo */
            gtk_show_about_dialog (GTK_WINDOW (app->window),
                                "authors", authors,
                                "comments", comments,
                                "copyright", copyright,
                                "version", VER,
                                "website", SITE,
                                "program-name", APPSTR,
                                "logo", logo,
                                "license", buf,
                                NULL);
        else    /* show stock GTK_STOCK_EDIT */
            gtk_show_about_dialog (GTK_WINDOW (app->window),
                                "authors", authors,
                                "comments", comments,
                                "copyright", copyright,
                                "version", VER,
                                "website", SITE,
                                "program-name", APPSTR,
                                "logo-icon-name", GTK_STOCK_EDIT,
                                "license", buf,
                                NULL);
        g_free (buf);
    }
    else {
#ifdef DEBUG
        g_print ("error: load of file %s failed.\n", LICENSE);
#endif
        if (logo)   /* show logo */
            gtk_show_about_dialog (GTK_WINDOW (app->window),
                                "authors", authors,
                                "comments", comments,
                                "copyright", copyright,
                                "version", VER,
                                "website", SITE,
                                "program-name", APPSTR,
                                "logo", logo,
                                NULL);
        else    /* show stock GTK_STOCK_EDIT */
            gtk_show_about_dialog (GTK_WINDOW (app->window),
                                "authors", authors,
                                "comments", comments,
                                "copyright", copyright,
                                "version", VER,
                                "website", SITE,
                                "program-name", APPSTR,
                                "logo-icon-name", GTK_STOCK_EDIT,
                                NULL);
    }

    if (logo)
        g_object_unref (logo);
    if (license)
        g_free (license);
}

/*
 *  _Help menu
 */
void menu_help_about_activate (GtkMenuItem *menuitem, gpointer data)
{
    help_about (data);
    if (menuitem) {}
}

GtkWidget *create_temp_menu (gpointer data, GtkAccelGroup *mainaccel)
{
    mainwin_t *app = data;
    GtkWidget *menubar;             /* menu container */

    GtkWidget *fileMenu;            /* file menu */
    GtkWidget *fileMi;
    GtkWidget *newMi;
    GtkWidget *openMi;
    GtkWidget *saveMi;
    GtkWidget *saveasMi;
    GtkWidget *closeMi;
    GtkWidget *quitMi;

    GtkWidget *helpMenu;            /* help menu */
    GtkWidget *helpMi;
    GtkWidget *aboutMi;

    GtkWidget *tempMenu;            /* temp menu */
    GtkWidget *tempMi;
    GtkWidget *showtbMi;
    GtkWidget *showdocMi;
    GtkWidget *addsplitMi;
    GtkWidget *rmsplitMi;

    GtkWidget *sep;

    menubar             = gtk_menu_bar_new ();
        fileMenu        = gtk_menu_new ();
        tempMenu        = gtk_menu_new ();
        helpMenu        = gtk_menu_new ();
    // gtk_widget_show (menubar);

    fileMi = gtk_menu_item_new_with_mnemonic ("_File");
    sep = gtk_separator_menu_item_new ();
    newMi    = gtk_image_menu_item_new_from_stock (GTK_STOCK_NEW,
                                                   NULL);
    openMi   = gtk_image_menu_item_new_from_stock (GTK_STOCK_OPEN,
                                                   NULL);
    saveMi   = gtk_image_menu_item_new_from_stock (GTK_STOCK_SAVE,
                                                   NULL);
    saveasMi = gtk_image_menu_item_new_from_stock (GTK_STOCK_SAVE_AS,
                                                   NULL);
    closeMi  = gtk_image_menu_item_new_from_stock (GTK_STOCK_CLOSE,
                                                   NULL);
    quitMi   = gtk_image_menu_item_new_from_stock (GTK_STOCK_QUIT,
                                                   NULL);

    /* create entries under 'File' then add to menubar */
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (fileMi), fileMenu);
    gtk_menu_shell_append (GTK_MENU_SHELL (fileMenu), sep);
    gtk_menu_shell_append (GTK_MENU_SHELL (fileMenu), newMi);
    gtk_menu_shell_append (GTK_MENU_SHELL (fileMenu), openMi);
    gtk_menu_shell_append (GTK_MENU_SHELL (fileMenu), saveMi);
    gtk_menu_shell_append (GTK_MENU_SHELL (fileMenu), saveasMi);
    gtk_menu_shell_append (GTK_MENU_SHELL (fileMenu),
                           gtk_separator_menu_item_new());
    gtk_menu_shell_append (GTK_MENU_SHELL (fileMenu), closeMi);
    /* temp disabled until logic completed */
    // gtk_widget_set_sensitive (closeMi, FALSE);
    gtk_menu_shell_append (GTK_MENU_SHELL (fileMenu), quitMi);
    gtk_menu_shell_append (GTK_MENU_SHELL (menubar), fileMi);

    gtk_widget_add_accelerator (newMi, "activate", mainaccel,
                                GDK_KEY_n, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator (openMi, "activate", mainaccel,
                                GDK_KEY_o, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator (saveMi, "activate", mainaccel,
                                GDK_KEY_s, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator (saveasMi, "activate", mainaccel,
                                GDK_KEY_s, GDK_CONTROL_MASK | GDK_SHIFT_MASK,
                                GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator (closeMi, "activate", mainaccel,
                                GDK_KEY_w, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator (quitMi, "activate", mainaccel,
                                GDK_KEY_q, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    /* entries under Temp working menu */
    tempMi = gtk_menu_item_new_with_mnemonic ("_Temp");
    sep = gtk_separator_menu_item_new ();
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

    gtk_menu_item_set_submenu (GTK_MENU_ITEM (tempMi), tempMenu);
    gtk_menu_shell_append (GTK_MENU_SHELL (tempMenu), sep);
    gtk_menu_shell_append (GTK_MENU_SHELL (tempMenu), showtbMi);
    gtk_menu_shell_append (GTK_MENU_SHELL (tempMenu), showdocMi);
    gtk_menu_shell_append (GTK_MENU_SHELL (tempMenu), addsplitMi);
    gtk_menu_shell_append (GTK_MENU_SHELL (tempMenu), rmsplitMi);
    gtk_menu_shell_append (GTK_MENU_SHELL (menubar), tempMi);

    gtk_widget_add_accelerator (showtbMi, "activate", mainaccel,
                                GDK_KEY_t, GDK_CONTROL_MASK,
                                GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator (showdocMi, "activate", mainaccel,
                                GDK_KEY_d, GDK_CONTROL_MASK,
                                GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator (addsplitMi, "activate", mainaccel,
                                GDK_KEY_s, GDK_CONTROL_MASK | GDK_SHIFT_MASK,
                                GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator (rmsplitMi, "activate", mainaccel,
                                GDK_KEY_r, GDK_CONTROL_MASK | GDK_SHIFT_MASK,
                                GTK_ACCEL_VISIBLE);

    /*define help menu */
    helpMi = gtk_menu_item_new_with_mnemonic ("_Help");
    gtk_menu_item_set_right_justified ((GtkMenuItem *)helpMi, TRUE);
    sep = gtk_separator_menu_item_new ();
    aboutMi = gtk_image_menu_item_new_from_stock (GTK_STOCK_ABOUT,
                                                  NULL);

    /* create entries under 'Help' then add to menubar */
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (helpMi), helpMenu);
    gtk_menu_shell_append (GTK_MENU_SHELL (helpMenu), sep);
    gtk_menu_shell_append (GTK_MENU_SHELL (helpMenu), aboutMi);
    gtk_menu_shell_append (GTK_MENU_SHELL (menubar), helpMi);

    gtk_widget_add_accelerator (aboutMi, "activate", mainaccel,
                                GDK_KEY_a, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);

    /* File Menu */
    g_signal_connect (G_OBJECT (newMi), "activate",         /* file New     */
                      G_CALLBACK (menu_file_new_activate), app);

    g_signal_connect (G_OBJECT (openMi), "activate",        /* file Open    */
                      G_CALLBACK (menu_file_open_activate), app);

    g_signal_connect (G_OBJECT (saveMi), "activate",        /* file Save    */
                      G_CALLBACK (menu_file_save_activate), app);

    g_signal_connect (G_OBJECT (saveasMi), "activate",      /* file SaveAs  */
                      G_CALLBACK (menu_file_saveas_activate), app);

    g_signal_connect (G_OBJECT (closeMi), "activate",        /* file Quit    */
                      G_CALLBACK (menu_file_close_activate), app);

    g_signal_connect (G_OBJECT (quitMi), "activate",        /* file Quit    */
                      G_CALLBACK (menu_file_quit_activate), NULL);

    /* Temp Menu */
    g_signal_connect (G_OBJECT (showtbMi), "activate",      /* show toolbar */
                      G_CALLBACK (menu_showtb_activate), app);

    g_signal_connect (G_OBJECT (showdocMi), "activate",      /* show toolbar */
                      G_CALLBACK (menu_showdoc_activate), app);

    g_signal_connect (G_OBJECT (addsplitMi), "activate",      /* show toolbar */
                      G_CALLBACK (menu_createview_activate), app);

    g_signal_connect (G_OBJECT (rmsplitMi), "activate",      /* show toolbar */
                      G_CALLBACK (menu_removeview_activate), app);

    /* Help Menu */
    g_signal_connect (G_OBJECT (aboutMi), "activate",       /* help About   */
                      G_CALLBACK (menu_help_about_activate), app);

    gtk_widget_show_all (menubar);

    return menubar;
}

GtkWidget *create_temp_toolbar (mainwin_t *app, GtkAccelGroup *mainaccel)
{
    GtkToolItem *sep;               /* toolbar */
    GtkToolItem *new;
    GtkToolItem *exit;
    GtkToolItem *info;
    GtkToolItem *split;
    GtkToolItem *rmsplit;

    app->toolbar = gtk_toolbar_new ();
    // gtk_widget_show (app->toolbar);

    gtk_container_set_border_width(GTK_CONTAINER(app->toolbar), 2);
    gtk_toolbar_set_show_arrow (GTK_TOOLBAR(app->toolbar), TRUE);
    gtk_toolbar_set_style(GTK_TOOLBAR(app->toolbar), GTK_TOOLBAR_ICONS);

    new = gtk_tool_button_new_from_stock(GTK_STOCK_NEW);
    gtk_tool_item_set_homogeneous (new, FALSE);
    gtk_toolbar_insert(GTK_TOOLBAR(app->toolbar), new, -1);
    gtk_widget_set_tooltip_text (GTK_WIDGET(new), "New file ");

    exit = gtk_tool_button_new_from_stock(GTK_STOCK_QUIT);
    gtk_tool_item_set_homogeneous (exit, FALSE);
    gtk_toolbar_insert(GTK_TOOLBAR(app->toolbar), exit, -1);
    gtk_widget_set_tooltip_text (GTK_WIDGET(exit), "Quit ");

    sep = gtk_separator_tool_item_new();
    gtk_toolbar_insert(GTK_TOOLBAR(app->toolbar), sep, -1);

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
    g_signal_connect (G_OBJECT (new), "clicked",            /* file New     */
                      G_CALLBACK (menu_file_new_activate), app);

    g_signal_connect (G_OBJECT (exit), "clicked",           /* file Quit    */
                      G_CALLBACK (menu_file_quit_activate), NULL);

    g_signal_connect (G_OBJECT (info), "clicked",           /* info btn     */
                      G_CALLBACK (doctree_for_each), app);

    g_signal_connect (G_OBJECT (split), "clicked",          /* split btn    */
                      G_CALLBACK (menu_createview_activate), app);

    g_signal_connect (G_OBJECT (rmsplit), "clicked",        /* rmsplit btn  */
                      G_CALLBACK (menu_removeview_activate), app);

    gtk_widget_show_all (app->toolbar);

    return app->toolbar;

    if (mainaccel) {}
}