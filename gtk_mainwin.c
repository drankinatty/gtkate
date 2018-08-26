#include "gtk_mainwin.h"

#define NTESTFN 16

/*
 * window callbacks
 */
/** on_window_delete_event called first when WM_CLOSE button clicked.
 *  if return is FALSE, on_window_destroy is called, if TRUE, control
 *  is returned to gtk_main and you are responsible for gtk_main_quit.
 */
/** on_window_destroy called after on_window_delete_event processed
 *  if the return from on_window_delete_event is FALSE.
 */
void on_window_destroy (GtkWidget *widget, gpointer data)
{
    gtk_main_quit();
    if (widget) {}
    if (data) {}
}

gboolean on_window_delete_event (GtkWidget *widget, GdkEvent *event,
                                 gpointer data)
{
    on_window_destroy (widget, data);
    return FALSE;

    if (widget) {}
    if (event) {}
}

/** creates a new pixbuf from filename.
 *  you are responsible for calling g_object_unref() on
 *  the pixbuf when done.
 */
GdkPixbuf *create_pixbuf_from_file (const gchar *filename)
{
    GdkPixbuf *pixbuf;
    GError *error = NULL;
    pixbuf = gdk_pixbuf_new_from_file (filename, &error);

    if (!pixbuf) {
        g_warning (error->message); /* log to terminal window */
        g_error_free (error);
    }

    return pixbuf;
}

/** create application window & initialize values
 *  and connect callback functions. 'app' contains
 *  widgets for window, text_view and statusbar.
 */
GtkWidget *create_window (mainwin_t *app)
{
    GtkAccelGroup *mainaccel;
    GtkWidget *vbox;                /* vbox container   */
    GtkWidget *menubar;             /* menu container   */
    GtkWidget *hpaned;              /* hpaned widget treeview/document win */
    // GtkWidget *evbox;               /* vbox for editor instances (splits) */
    GtkWidget *ewin;                /* edit scrolled window instance */

    /* temp vars */
    gint bordersz = 0;

    gchar *iconfile;            /* filename to loading icon */

    /* create toplevel window */
    if (!(app->window = gtk_window_new (GTK_WINDOW_TOPLEVEL))) {
        g_fprintf (stderr, "create_window() gtk_window_new failure.\n");
        return NULL;
    }
    // gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
    gtk_window_set_position (GTK_WINDOW (app->window), GTK_WIN_POS_NONE);
    gtk_window_set_default_size (GTK_WINDOW (app->window), app->winwidth,
                                    app->winheight);
    // gtk_window_move (GTK_WINDOW (app->window), app->winrootx, app->winrooty);

    /* create icon filename and set icon */
    if ((iconfile = g_strdup_printf ("%s/%s", app->imgdir, ICON))) {
        GdkPixbuf *pixbuf = create_pixbuf_from_file (iconfile);
        if (pixbuf) {
            gtk_window_set_icon(GTK_WINDOW(app->window), pixbuf);
            g_object_unref (pixbuf);
        }
        g_free (iconfile);
    }

    /* create & attach accelerator group */
    mainaccel = gtk_accel_group_new ();
    gtk_window_add_accel_group (GTK_WINDOW (app->window), mainaccel);

    /* create vbox to hold menu, toolbar, treeview and vbox to hold any
     * number of ibarvbox, scrolled_textview and statusbar splits of
     * documents.
     */
    vbox = gtk_vbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (app->window), vbox);
    // gtk_box_set_spacing (GTK_BOX (vbox), 0);

    /* create menubar and menus to add */
    // menubar = create_menubar (app, mainaccel);
    // app->menubar = menubar;
    menubar = create_temp_menu (app, mainaccel);
    gtk_box_pack_start (GTK_BOX (vbox), menubar, FALSE, FALSE, 0);
    gtk_widget_show (menubar);

    /* create toolbar
     * GTK_TOOLBAR_ICONS, GTK_TOOLBAR_TEXT, GTK_TOOLBAR_BOTH, GTK_TOOLBAR_BOTH_HORIZ
     */
    // app->toolbar = create_toolbar (mainaccel, app);
    create_temp_toolbar (app, mainaccel);
    gtk_box_pack_start(GTK_BOX(vbox), app->toolbar, FALSE, FALSE, 0);
    gtk_widget_show (app->toolbar);

    /* create hpaned to display dirtree and ibar/scrolled_window
     * allowing the divider between the dirtree and scrolled_window
     * to be resized to accommodate longer filenames.
     */
    hpaned = gtk_hpaned_new();
    gtk_paned_set_position (GTK_PANED(hpaned), app->treewidth);
    gtk_container_set_border_width(GTK_CONTAINER(hpaned), 2);
    gtk_box_pack_start (GTK_BOX(vbox), hpaned, TRUE, TRUE, 0);
    gtk_widget_show (hpaned);

    /* create vbox for directory tree */
    app->vboxtree = gtk_vbox_new (FALSE, 0);
    gtk_widget_set_size_request (app->vboxtree, app->treewidth, -1);
    /* pack into left-pane with resize, shrink set FALSE, FALSE to prevent
     * left pane from expanding on window resize and to prevent it from being
     * made smaller that initial size. (consider leaving 40px or so at bottom
     * of pane to allow for TreeView search box).
     */
    /* TODO: move vboxtree creation to doctree.[ch] */
    gtk_paned_pack1 (GTK_PANED(hpaned), app->vboxtree, FALSE, FALSE);
    gtk_widget_show (app->vboxtree);
    // label = gtk_label_new ("\nvboxtree region");
    // gtk_box_pack_start(GTK_BOX(app->vboxtree), label, FALSE, FALSE, 0);
    // gtk_widget_show (label);

    /* create scrolled_window for tree */
    GtkWidget *treescroll = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (treescroll),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);
    gtk_widget_show (treescroll);

    /* create TreeView and model */
    app->treeview = create_view_and_model(app, NULL);
    gtk_container_set_border_width(GTK_CONTAINER(app->vboxtree), bordersz);
    gtk_container_add (GTK_CONTAINER (treescroll), app->treeview);
    gtk_box_pack_start(GTK_BOX(app->vboxtree), treescroll, TRUE, TRUE, 0);
    gtk_widget_show (app->treeview);
//     gtk_widget_show_all (app->treeview);

    /* vpaned to split document window. pack original sourceview into 1,
     * leaveing 2 empty until split, the split adding new textview to 2.
     * pack2 hpaned with vpaned with TRUE, TRUE to expand/fill whole window.
     */
    // vpaned = gtk_vpaned_new();
    app->vboxedit = gtk_vbox_new (FALSE, 0);
    gtk_paned_pack2 (GTK_PANED(hpaned), app->vboxedit, TRUE, TRUE);
    // gtk_paned_pack2 (GTK_PANED(hpaned), vpaned, TRUE, TRUE);

    /* create vbox for infobar, scrolled_window, sourceview & statusbar.
     * this is the first edit window packed in hpaned vbox. calls to
     * ewin_create_split() add a split edit window, up to MAXVIEW, and calls
     * to ewin_remove_split() will remove splits until 1 edit window remains.
     * packing with TRUE, TRUE causes even distribution of space between edit
     * windows when added and removed.
     */
    ewin = create_scrolled_view (app);
    gtk_box_pack_start(GTK_BOX(app->vboxedit), ewin, TRUE, TRUE, 0);
    app->nview++;

    gtk_widget_show (app->vboxedit);

    gtk_widget_show (vbox);

    /* connect all signals */
    g_signal_connect (G_OBJECT (app->window), "delete-event", /* window del */
                      G_CALLBACK (on_window_delete_event), NULL);

    g_signal_connect (G_OBJECT (app->window), "destroy",    /* window dest  */
                      G_CALLBACK (on_window_destroy), NULL);

    /* general */
    /*
    g_signal_connect (G_OBJECT (app->view), "key_press_event",
                      G_CALLBACK (on_keypress), app);

    g_signal_connect (app->buffer, "changed",
                      G_CALLBACK (on_buffer_changed), app);

    g_signal_connect (app->buffer, "mark_set",
                      G_CALLBACK (on_mark_set), app);

    g_signal_connect (G_OBJECT (app->view), "toggle-overwrite",
                      G_CALLBACK (on_insmode), app);
    */

    gtk_widget_show (app->window);

    /* append NTESTFN files to tree - temp */
    for (gint i = 1; i < NTESTFN; i++) {
        gchar *name = NULL;
        if (i & 1)  /* test odd/even set extension/path */
            name = g_strdup_printf ("/a-path/%s_%02d.txt", "NewFile", i);
        else
            name = g_strdup_printf ("../path/%s_%02d.cpp", "NewFile", i);
        treeview_append (app, name);
        g_free (name);
    }
    treeview_append (app, NULL);
    treeview_append (app, NULL);

    /* return focus to edit window */
    gtk_widget_grab_focus (app->treeview);
    gtk_widget_grab_focus (app->einst[0]->view);

    /* showall widgets */
    // gtk_widget_show_all (app->window);

    return app->window;
}
