#include "gtk_doctree.h"

/** given kinst_t instance, set treeview name for entry to Untitled(n)
 *  if inst is NULL, update app->nuntitled, or set name to inst->fname.
 */
gchar *treeview_setname (mainwin_t *app, kinst_t *inst)
{
    gchar *name;
    if (inst->filename == NULL) {
        if (app->nuntitled)
            name = g_strdup_printf ("Untitled(%d)", app->nuntitled);
        else
            name = g_strdup ("Untitled");
        app->nuntitled++;
    }
    else
        name = g_strdup (inst->fname);

    return name;
}

void treeview_append (mainwin_t *app, const gchar *filename)
{
    GtkTreeStore *treestore;
    GtkTreeIter toplevel;
    kinst_t *inst = buf_new_inst (filename);  /* new instance split filename */
    gchar *name = NULL;

    if (!inst) {    /* validate instance allocated */
        /* handle error */
        g_print ("treeview_append() error : !inst\n");
        return;
    }

    name = treeview_setname (app, inst);    /* get treeview name */
    if (!name) {
        /* handle error */
        g_print ("treeview_append() error: !name\n");
        return;
    }
    gtk_text_buffer_insert_at_cursor (GTK_TEXT_BUFFER(inst->buf),
                                        name, -1);

    treestore = GTK_TREE_STORE(gtk_tree_view_get_model (
                                GTK_TREE_VIEW(app->treeview)));
    if (!treestore) {
        /* handle no model error */
        g_print ("treeview_append() error: !treestore\n");
        g_free (name);
        return;
    }

    /* appeand name and pointer to inst as entry in treeview */
    gtk_tree_store_append (treestore, &toplevel, NULL);
    gtk_tree_store_set (treestore, &toplevel, COLNAME, name, -1);
    gtk_tree_store_set (treestore, &toplevel, COLINST, inst, -1);

    g_free (name);  /* free name */
}

/** create tree model and initialize treestore with filenames provided in
 *  argv or Untitled, set app->treemodel. argv must end with sentinel NULL.
 */
GtkTreeModel *treemodel_init (mainwin_t *app, gchar **argv)
{
    GtkTreeStore *treestore;

    /* create model with string and pointer to inst.
     * TODO - consider just using pointer to inst and change render to
     *        render name from inst->fname
     */
    treestore = gtk_tree_store_new (NUMCOL, G_TYPE_STRING, G_TYPE_POINTER);
    if (!treestore) {
        g_print ("treemodel_init() error: !treestore\n");
    }
    app->treemodel = GTK_TREE_MODEL(treestore);
    gtk_tree_view_set_model(GTK_TREE_VIEW(app->treeview), app->treemodel);

    if (argv) {
        while (*argv) {
            treeview_append (app, *argv);
            argv++;
        }
    }
    else
        treeview_append (app, NULL);

    g_object_unref(app->treemodel);

    return GTK_TREE_MODEL (treestore);
}

/* initial test create/fill with up to 100 files to test scrolling window
 * for document tree. also showns example of adding directory the filenames.
 */
// GtkTreeModel *create_and_fill_model (mainwin_t *app)
// {
//     gint nfiles = 4;
//     /* tree with directory component commented out for single list
//      * enable toplevel/child for directory/filename struct or omit
//      * directory and simply have a tree of filenames.
//      */
//     GtkTreeStore *treestore;
//     GtkTreeIter toplevel/*, child*/;
//
//     treestore = gtk_tree_store_new (NUMCOL, G_TYPE_STRING, G_TYPE_POINTER);
// #ifdef WITHDIR
//     GtkTreeIter child;
//
//     gtk_tree_store_append (treestore, &toplevel, NULL);
//     gtk_tree_store_set (treestore, &toplevel, COLNAME, "Directory1", -1);
//     gtk_tree_store_set (treestore, &toplevel, COLINST, buf_new_inst(NULL), -1);
//
//     for (gint i = 0; i < nfiles; i++) {
//         gchar *name = g_strdup_printf ("Filename_%03d", i);
//         gtk_tree_store_append (treestore, &child, &toplevel);
//         gtk_tree_store_set (treestore, &child, COLNAME, name, -1);
//         gtk_tree_store_set (treestore, &child, COLINST, buf_new_inst(name), -1);
//         g_free (name);
//     }
// #else
//     for (gint i = 0; i < nfiles; i++) {
//         gchar *name = g_strdup_printf ("Filename_%03d", i);
//         gtk_tree_store_append (treestore, &toplevel, NULL);
//         gtk_tree_store_set (treestore, &toplevel, COLNAME, name, -1);
//         gtk_tree_store_set (treestore, &toplevel, COLINST, buf_new_inst(name), -1);
//         g_free (name);
//     }
// #endif
//
//     return GTK_TREE_MODEL (treestore);
//
//     if (app) {}
// }

/** create treeview and initialize treemodel */
GtkWidget *create_view_and_model (mainwin_t *app, gchar **argv)
{
    GtkTreeViewColumn *col;
    GtkCellRenderer *renderer;
    GtkWidget *view;
    GtkTreeSelection *selection;        /* tree selection for buffer inst */

    view = gtk_tree_view_new();
    app->treeview = view;               /* set pointer to treeview */
    /* TODO - consider (app->treeview)[n] to allow multiple views for
     * showing files grouped by directory, or sorted based on opening or
     * ascending/descending alphabetical sort.
     */

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Documents");
    /* set column header off (or on) FALSE/TRUE */
    gtk_tree_view_set_headers_visible (GTK_TREE_VIEW(view), FALSE);
    /* if header is shown, below makes header clickable to sort ascending
     * or descending -- but I don't like the header shown, so we will have
     * to do sort in context menu.
     */
    gtk_tree_view_column_set_sort_column_id (col, COLNAME);
    gtk_tree_view_column_set_sort_indicator (col, TRUE);

    gtk_tree_view_append_column(GTK_TREE_VIEW(view), col);

    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(col, renderer, TRUE);
    gtk_tree_view_column_add_attribute(col, renderer,
        "text", COLNAME);

    treemodel_init (app, argv); /* initialize treemodel, set app->treemodel */

    /* selection to set callback */
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(view));

    /* TreeView/model callbacks */
    g_signal_connect (selection, "changed",
                        G_CALLBACK (doctree_activate), app);

    return view;
}

/* simple test for_each to dump kinst_t information */
void doctree_for_each (GtkWidget *widget, mainwin_t *app)
{
    GtkTreeModel *model = NULL;
    GtkTreeIter iter;
    gboolean valid;
    gint nrows = 0;

    model = gtk_tree_view_get_model(GTK_TREE_VIEW(app->treeview));
    if (!model) {
        g_print ("error: tree_view_get_model - failed.\n");
        return;
    }

    valid = gtk_tree_model_get_iter_first (model, &iter);
    while (valid) {
        gchar *str = NULL;
        kinst_t *inst = NULL;

        gtk_tree_model_get (model, &iter,
                            COLNAME, &str, COLINST, &inst, -1);
        // g_print ("name: %-12s  filename: %s\n", str, inst->filename);
        g_print ("name: %-16s  filename: %-22s  fname: %-16s  path: %-10s  ext: %s\n",
                str, inst->filename, inst->fname, inst->fpath, inst->fext);
        g_free (str);

        nrows++;

        valid = gtk_tree_model_iter_next (model, &iter);
    }
    g_print ("total: %d rows\n", nrows);

    if (widget) {}
}

/** returns buffer instance of selected document, which includes sourceview
 *  buffer, associate filename (or NULL if "Untitled"), and current line and
 *  column for insert mark in buffer so view can be restored.
 */
kinst_t *inst_get_selected (gpointer data)
{
    GtkTreeSelection *selection;        /* tree selection for buffer inst */
    GtkTreeIter iter;
    GtkTreeModel *model;
    mainwin_t *app = data;
    char *value;
    kinst_t *inst = NULL;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(app->treeview));
    if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
        gtk_tree_model_get (model, &iter, COLNAME, &value, COLINST, &inst, -1);
    }

    return inst;
}

/* TODO
 * void inst_set_selected (kinst_t inst)
 */

/** doctree callbacks */

void doctree_activate (GtkWidget *widget, gpointer data)
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    mainwin_t *app = data;
    GtkWidget *view = app->einst[app->focused]->view;
    char *value, *title;
    kinst_t *inst;

    if (gtk_tree_selection_get_selected (GTK_TREE_SELECTION(widget),
                                            &model, &iter)) {

        gtk_tree_model_get (model, &iter, COLNAME, &value, COLINST, &inst,  -1);

        title = g_strdup_printf ("%s - %s", APPNAME, value);

        gtk_window_set_title (GTK_WINDOW (app->window), title);
/* the reason textview2 never focused - hardcoded - needed to set var */
        gtk_text_view_set_buffer (GTK_TEXT_VIEW(view),
//         gtk_text_view_set_buffer (GTK_TEXT_VIEW(app->view[0]),
                                    GTK_TEXT_BUFFER(inst->buf));

        g_free (value); /* values with type G_TYPE_STRING or G_TYPE_BOXED */
                        /* have to be freed, G_TYPE_POINTER does not */
        g_free (title);
    }

    if (data) {}
}

