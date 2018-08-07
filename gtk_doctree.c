#include "gtk_doctree.h"

/** create and initialize a new buffer instance to add to tree */
kinst_t *buf_new_inst (const gchar *fname)
{
    kinst_t *inst = g_slice_new (kinst_t);
    if (!inst)
        return NULL;
    inst->buf = gtk_source_buffer_new (NULL);
    if (fname)
        inst->filename = g_strdup_printf ("%s", fname);
    else
        inst->filename = NULL;
    inst->line = inst->col = 0;

    return inst;
}

/** free memory for allocated buffer instance */
void buf_delete_inst (kinst_t *inst)
{
    if (inst->filename)
        g_free (inst->filename);

    g_slice_free (kinst_t, inst);
}

void doctree_append (GtkWidget *view, const gchar *filename)
{
    GtkTreeStore *treestore;
    GtkTreeIter toplevel;
    const gchar *name = filename ? filename : "untitled";

    treestore = GTK_TREE_STORE(gtk_tree_view_get_model (GTK_TREE_VIEW(view)));
    gtk_tree_store_append (treestore, &toplevel, NULL);
    gtk_tree_store_set (treestore, &toplevel, COLNAME, name, -1);
    gtk_tree_store_set (treestore, &toplevel, COLINST, buf_new_inst(name), -1);
}

GtkTreeModel *doctree_add (mainwin_t *app, const gchar *filename)
{
    GtkTreeStore *treestore;
    GtkTreeIter toplevel;
    const gchar *name = filename ? filename : "untitled";

    if (app->doctreeview)
        treestore = GTK_TREE_STORE(gtk_tree_view_get_model (
                                GTK_TREE_VIEW(app->doctreeview)));
    else
        treestore = gtk_tree_store_new (NUMCOL, G_TYPE_STRING, G_TYPE_POINTER);

    gtk_tree_store_append (treestore, &toplevel, NULL);
    gtk_tree_store_set (treestore, &toplevel, COLNAME, name, -1);
    gtk_tree_store_set (treestore, &toplevel, COLINST, buf_new_inst(name), -1);

    return GTK_TREE_MODEL (treestore);
}

GtkTreeModel *create_and_fill_model (mainwin_t *app)
{
    gint nfiles = 4;
    /* tree with directory component commented out for single list
     * enable toplevel/child for directory/filename struct or omit
     * directory and simply have a tree of filenames.
     */
    GtkTreeStore *treestore;
    GtkTreeIter toplevel/*, child*/;

//     treestore = gtk_tree_store_new (NUMCOL, G_TYPE_STRING);
    treestore = gtk_tree_store_new (NUMCOL, G_TYPE_STRING, G_TYPE_POINTER);
#ifdef WITHDIR
    GtkTreeIter child;

    gtk_tree_store_append (treestore, &toplevel, NULL);
    gtk_tree_store_set (treestore, &toplevel, COLNAME, "Directory1", -1);
    gtk_tree_store_set (treestore, &toplevel, COLINST, buf_new_inst(NULL), -1);

    for (gint i = 0; i < nfiles; i++) {
        gchar *name = g_strdup_printf ("Filename_%03d", i);
        gtk_tree_store_append (treestore, &child, &toplevel);
        gtk_tree_store_set (treestore, &child, COLNAME, name, -1);
        gtk_tree_store_set (treestore, &child, COLINST, buf_new_inst(name), -1);
        g_free (name);
    }
#else
    for (gint i = 0; i < nfiles; i++) {
        gchar *name = g_strdup_printf ("Filename_%03d", i);
        gtk_tree_store_append (treestore, &toplevel, NULL);
        gtk_tree_store_set (treestore, &toplevel, COLNAME, name, -1);
        gtk_tree_store_set (treestore, &toplevel, COLINST, buf_new_inst(name), -1);
        g_free (name);
    }
#endif

    return GTK_TREE_MODEL (treestore);

    if (app) {}
}

GtkWidget *create_view_and_model (mainwin_t *app, gchar **argv)
{
    GtkTreeViewColumn *col;
    GtkCellRenderer *renderer;
    GtkWidget *view;
    GtkTreeModel *model;

    view = gtk_tree_view_new();

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

    model = create_and_fill_model(app);
    gtk_tree_view_set_model(GTK_TREE_VIEW(view), model);
    g_object_unref(model);

    doctree_append (view, "newfile");

    return view;

    if (argv) {}
}

/** doctree callbacks */

void doctree_activate (GtkWidget *widget, gpointer data)
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    mainwin_t *mwin = data;
    char *value, *title;
    kinst_t *inst;

    if (gtk_tree_selection_get_selected (GTK_TREE_SELECTION(widget),
                                            &model, &iter)) {

        // gtk_tree_model_get (model, &iter, COLNAME, &value,  -1);
        gtk_tree_model_get (model, &iter, COLNAME, &value, COLINST, &inst,  -1);
        // g_print ("filename: %s\n", value);
        title = g_strdup_printf ("%s - %s", APPNAME, value);

        gtk_window_set_title (GTK_WINDOW (mwin->window), title);

        gtk_text_view_set_buffer (GTK_TEXT_VIEW(mwin->view),
                                    GTK_TEXT_BUFFER(inst->buf));

        g_free (value); /* values with type G_TYPE_STRING or G_TYPE_BOXED */
                        /* have to be freed, G_TYPE_POINTER does not */
        g_free (title);
    }

    if (data) {}
}

