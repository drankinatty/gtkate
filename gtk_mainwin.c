#include "gtk_mainwin.h"

#define NTESTFN 16

/* move init to a gtk_appdata.c as keyfile will change */
void mainwin_init (mainwin_t *app, char **argv)
{
    app->window         = NULL;         /* main window pointer */
    app->toolbar        = NULL;         /* toolbar widget */
    app->vboxtree       = NULL;         /* vbox for treeview show/hide */
    app->view           = NULL;         /* sourceview widget */

    app->winwidth       = 840;          /* default window width   */
    app->winheight      = 800;          /* default window height  */
    app->winrestore     = FALSE;        /* restore window size */
    app->winszsaved     = FALSE;        /* win size saved */
    app->treewidth      = 180;          /* initial treeiew width */
    app->swbordersz     = 0;            /* scrolled_window border */
    app->nsplit         = 0;            /* no. of split editor panes shown */

    app->showtoolbar    = TRUE;         /* toolbar is visible */
    app->showdocwin     = TRUE;         /* document tree is visible */

    app->fontname       = g_strdup ("DejaVu Sans Mono 8");

    app->tabstop        = 8;            /* number of spaces per tab */
    app->softtab        = 4;            /* soft tab stop size */
    app->tabstring      = NULL;         /* tabstring for indent */

    app->indentauto     = TRUE;         /* auto-indent on return */

    app->lineno         = FALSE;        /* show line numbers (sourceview) */
    app->linehghlt      = TRUE;         /* highlight current line */

    app->showmargin     = TRUE;         /* show margin at specific column */
    app->marginwidth    = 80;           /* initial right margin to display */

    app->nrecent        = 40;           /* no. of recent chooser files */
    app->nuntitled      = 0;            /* next "Untitled(n) in tree */

    if (argv) {}
}

void mainwin_destroy (mainwin_t *app)
{
    if (app) {};
}

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
    GtkWidget *vpaned;              /* vpaned widget to split document win */
    GtkWidget *vboxibscroll;        /* vbox for infobar & scrolled_window */
    GtkWidget *ibarvbox;            /* infobar vbox container   */
    GtkWidget *scrolled_textview;   /* scrolled win w/text_view */
    GtkWidget *statusbar;           /* statusbar */
    // GtkWidget *label;               /* misc label on top of tree */

    GtkWidget *sbalign;             /* alignment for statusbar  */
    guint ptop;                     /* padding, top, bot, l, r  */
    guint pbot;
    guint pleft;
    guint pright;

    /* temp vars */
    gint bordersz = 0;

    // gchar *iconfile;            /* filename to loading icon */

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
    /*
    if ((iconfile = g_strdup_printf ("%s/%s", app->imgdir, ICON))) {
        gtk_window_set_icon(GTK_WINDOW(window), create_pixbuf_from_file (iconfile));
        g_free (iconfile);
    }
    */

    /* create & attach accelerator group */
    mainaccel = gtk_accel_group_new ();
    gtk_window_add_accel_group (GTK_WINDOW (app->window), mainaccel);

    /* create vbox to hold menu, toolbar, scrolled_windows, textview & statusbar
     * and add contaier to main window
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

    /* vpaned to split document window. pack original sourceview into 1,
     * leaveing 2 empty until split, the split adding new textview to 2.
     * pack2 hpaned with vpaned with TRUE, TRUE to expand/fill whole window.
     */
    vpaned = gtk_vpaned_new();
    gtk_paned_pack2 (GTK_PANED(hpaned), vpaned, TRUE, TRUE);

    /* create vbox for infobar and scrolled_window */
    vboxibscroll = gtk_vbox_new (FALSE, 0);
    /* right-pane resize, shrink set TRUE, TRUE to allow right-pane to resize
     * and shrink as the main window is resized.
     */
    gtk_paned_pack1 (GTK_PANED(vpaned), vboxibscroll, TRUE, TRUE);
//     gtk_paned_pack2 (GTK_PANED(hpaned), vboxibscroll, TRUE, TRUE);
    gtk_widget_show (vboxibscroll);
    gtk_widget_show (vpaned);
    app->vpsplit = vpaned;

    /* create vbox to display infobar */
    ibarvbox = gtk_vbox_new (FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vboxibscroll), ibarvbox, FALSE, FALSE, 0);
    gtk_widget_show (ibarvbox);

    /* create scrolled_window and textview */
    scrolled_textview = create_textview_scrolledwindow (app);
    gtk_box_pack_start (GTK_BOX (vboxibscroll), scrolled_textview, TRUE, TRUE, 0);
    gtk_widget_show (scrolled_textview);

    gtk_text_buffer_insert_at_cursor (gtk_text_view_get_buffer(
                                        GTK_TEXT_VIEW(app->view)),
                                        "buffer_1\n", -1);
//     /* == pack2 - split test == */
//     /* vpaned works perfect to split editor window horizontally */
//     /* create vbox for infobar and scrolled_window */
//     vboxibscroll = gtk_vbox_new (FALSE, 0);
//     /* right-pane resize, shrink set TRUE, TRUE to allow right-pane to resize
//      * and shrink as the main window is resized.
//      */
//     gtk_paned_pack2 (GTK_PANED(vpaned), vboxibscroll, TRUE, TRUE);
//     gtk_widget_show (vboxibscroll);
//     gtk_widget_show (vpaned);
//     // app->vpsplit = vpaned;
//
//     /* create vbox to display infobar */
//     ibarvbox = gtk_vbox_new (FALSE, 0);
//     gtk_box_pack_start(GTK_BOX(vboxibscroll), ibarvbox, FALSE, FALSE, 0);
//     gtk_widget_show (ibarvbox);
//
//     /* create scrolled_window and textview */
//     scrolled_textview = create_textview_scrolledwindow (app);
//     gtk_box_pack_start (GTK_BOX (vboxibscroll), scrolled_textview, TRUE, TRUE, 0);
//     gtk_widget_show (scrolled_textview);

/* simply adding the scrolled window as second pane works as well. */
#ifdef SPLIT
    /* TODO coordinate currently selected inst and split using that buffer in
     * new scrolled_textview creation. Perhaps array of pointers for textview?
     */
    /* create scrolled_window and textview */
    scrolled_textview = create_textview_scrolledwindow (app);
    // gtk_box_pack_start (GTK_BOX (vboxibscroll), scrolled_textview, TRUE, TRUE, 0);
    gtk_widget_show (scrolled_textview);
    gtk_paned_pack2 (GTK_PANED(vpaned), scrolled_textview, TRUE, TRUE);
#endif
    /* create/pack statusbar at end within gtk_alignment */
    sbalign = gtk_alignment_new (0, .5, 1, 1);
    gtk_alignment_get_padding (GTK_ALIGNMENT (sbalign), &ptop, &pbot, &pleft, &pright);
    gtk_alignment_set_padding (GTK_ALIGNMENT (sbalign), ptop, pbot + 2, pleft + 5, pright);

    statusbar = gtk_statusbar_new ();
    // status_set_default (app);

    gtk_container_add (GTK_CONTAINER (sbalign), statusbar);
    gtk_box_pack_end (GTK_BOX (vbox), sbalign, FALSE, FALSE, 0);

    gtk_widget_show (statusbar);
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

    /* set window title */
    gtk_window_set_title (GTK_WINDOW (app->window), "New GtkWrite Layout");
    // gtkwrite_window_set_title (NULL, app);
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

    /* showall widgets */
    gtk_widget_show_all (app->window);

    return app->window;
}
