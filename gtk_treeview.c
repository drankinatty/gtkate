#include "gtk_treeview.h"

/** given inst use tree iter to set focus on current view
 *  (widget is textview associated with window clicked inside)
 */
/* TODO - check on using app->einst[app->focused]->view instead of
 * passing widget - cleaner? (no not really, getting buf from focused view or
 *                            calling gtk_text_view_get_buffer is a wash )
 *
 * GtkWidget *view = app->einst[app->focused]->view;
 * gpointer buf = gtk_text_view_get_buffer (GTK_TEXT_VIEW(view));
 */
void tree_set_selection (GtkWidget *widget, gpointer data)
{
    mainwin_t *app = data;
    GtkTreeModel *model = NULL;
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    gpointer buf = gtk_text_view_get_buffer (GTK_TEXT_VIEW(widget));
    // GtkWidget *view = app->einst[app->focused]->view;
    // gpointer buf = gtk_text_view_get_buffer (GTK_TEXT_VIEW(view));
    gboolean valid, found = FALSE;

    /* initialize selection to current */
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(app->treeview));

    /* get treemodel, validate */
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(app->treeview));
    if (!model) {
        g_print ("error: tree_view_get_model - failed.\n");
        return;
    }

    /* get first iter, iterate model to locate current inst, set selection */
    valid = gtk_tree_model_get_iter_first (model, &iter);
    while (valid) {         /* loop over each entry in model */
        kinst_t *inst = NULL;

#ifdef DEBUG
        /* add printing of COLNAME for debugging */
        gchar *str = NULL;

        gtk_tree_model_get (model, &iter,   /* get name & inst */
                            COLNAME, &str, COLINST, &inst, -1);

        /* compare pointer to sourceview with buf from textview (widget) */
        if ((gpointer)(inst->buf) == buf) {
            gtk_tree_selection_select_iter (selection, &iter);
            g_free (str);
            found = TRUE;
            break;
        }
        g_free (str);
#else
        gtk_tree_model_get (model, &iter, COLINST, &inst, -1);  /* get inst */

        /* compare pointer to sourceview with buf from textview (widget) */
        if ((gpointer)(inst->buf) == buf) {
            gtk_tree_selection_select_iter (selection, &iter);
            found = TRUE;
            break;
        }
#endif

        valid = gtk_tree_model_iter_next (model, &iter);
    }

    if (!found) /* validate buffer found or warn */
        g_warning ("tree_set_selection inst not found.");
}

/** get GtkTreeIter from app struct corresponding to focused textview
 *  caller is responsible for calling g_slice_free() on returned iter.
 */
GtkTreeIter *tree_get_iter_from_view (gpointer data)
{
    mainwin_t *app = data;
    GtkTreeModel *model = NULL;
    GtkTreeIter *iter = NULL;
    GtkWidget *view = app->einst[app->focused]->view;
    gpointer buf = gtk_text_view_get_buffer (GTK_TEXT_VIEW(view));
    gboolean valid;

    /* get treemodel, validate */
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(app->treeview));
    if (!model) {
        g_print ("error: tree_view_get_model - failed.\n");
        return NULL;
    }

    iter = g_slice_new (GtkTreeIter);
    if (!iter) {
        g_warning ("g_slice_new (GtkTreeIter) failed.");
        return NULL;
    }

    /* get first iter, return in validate */
    valid = gtk_tree_model_get_iter_first (model, iter);
    while (valid) {         /* loop over each entry in model */
        gchar *str = NULL;
        kinst_t *inst = NULL;

        /* TODO - remove COLNAME & str after debugging done */
        gtk_tree_model_get (model, iter,   /* get name & inst */
                            COLNAME, &str, COLINST, &inst, -1);

        /* compare pointer to sourceview with buf from textview (widget) */
        if ((gpointer)(inst->buf) == buf) {
            g_free (str);
            return iter;
        }
        g_free (str);

        valid = gtk_tree_model_iter_next (model, iter);
    }

    g_warning ("tree_set_selection inst not found.");

    return NULL;
}

/** parse number 'n' from "Untitled(n)" to allows untitled bitfield update */
gint untitled_get_no (const gchar *name)
{
    gchar *p, *endptr;
    const gchar *u = "Untitled";
    gint i;
    gint64 tmp;

    for (i = 0; u[i]; i++)  /* validate name begins "Untitled" */
        if (name[i] != u[i])
            return -1;

    if (!name[i])       /* if at name end, name is "Untitled" */
        return 0;

    p = (gchar *)(name + i);
    while (*p && (*p < '0' || '9' < *p))
        p++;

    // g_print ("converting: '%s'\n", p);
    errno = 0;
    tmp = g_ascii_strtoll (p, &endptr, 0);

    if (endptr == p || errno || tmp < G_MININT || G_MAXINT < tmp)
        return -1;

    return (gint)tmp;
}

/** get treemodel COLNAME for selected entry
 *  (user responsible for calling g_free on allocated return)
 */
gchar *treeview_getname (gpointer data)
{
    mainwin_t *app = data;
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkTreeSelection *selection;
    gchar *name = NULL;

    /* get selection from treeview */
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(app->treeview));

    /* get model & iter from selection */
    if (gtk_tree_selection_get_selected (selection, &model, &iter))
        gtk_tree_model_get (model, &iter, COLNAME, &name, -1);

    return name;
}

/**  given inst, iterate through tree to find inst and set COLNAME
 *  to inst->fname
 */
void treeview_setname (gpointer data, kinst_t *inst)
{
    mainwin_t *app = data;
    GtkTreeStore *treestore = GTK_TREE_STORE(app->treemodel);
    GtkTreeIter *iter = tree_get_iter_from_view (data);

    if (!iter) {
        g_warning ("treeview_setname() error: iter not found.");
        return;
    }

    gtk_tree_store_set (treestore, iter, COLNAME, inst->fname, -1);
}

/** check COLNAME entry in tree, and if "Untitled(n)" clear
 *  bit in nuntitled bitfield. note: only used when changing
 *  display COLNAME in treeview, otherwise bitfield will be
 *  out of sync with names displayed in tree.
 * (i.e. used with file open/save as)
 */
void check_untitled_remove (gpointer data)
{
    gchar *name = treeview_getname (data);
    gint n = 0;

    /* check name "Untitled(n)", parse n */
    if ((n = untitled_get_no (name)) >= 0)
        untitled_remove (data, n);      /* clear bit n in app->nultitled */

    g_free(name);
}

/** given kinst_t instance, set treeview name for entry to Untitled(n)
 *  if inst is NULL, update app->nuntitled, or set name to inst->fname.
 */
gchar *treeview_initial_name (gpointer data, kinst_t *inst)
{
    mainwin_t *app = data;
    gchar *name;
    if (inst->filename == NULL) {
        gint n = untitled_get_next (app);
        if (n)
            name = g_strdup_printf ("Untitled(%d)", n);
        else
            name = g_strdup ("Untitled");
    }
    else
        name = g_strdup (inst->fname);

    return name;
}

kinst_t *treeview_append (mainwin_t *app, const gchar *filename)
{
    GtkTreeStore *treestore;
    GtkTreeIter toplevel;
    GtkTreeSelection *selection;
    kinst_t *inst = buf_new_inst (filename);  /* new instance split filename */
    gchar *name = NULL;

    if (!inst) {    /* validate instance allocated */
        /* handle error */
        g_print ("treeview_append() error : !inst\n");
        return NULL;
    }

    name = treeview_initial_name (app, inst);    /* get treeview name */
    if (!name) {
        /* handle error */
        g_print ("treeview_append() error: !name\n");
        return NULL;
    }
    /* TEMP - initial name in buffer & set modified FALSE */
//     gtk_text_buffer_insert_at_cursor (GTK_TEXT_BUFFER(inst->buf),
//                                         name, -1);
//     gtk_text_buffer_set_modified (GTK_TEXT_BUFFER(inst->buf), FALSE);

    treestore = GTK_TREE_STORE(gtk_tree_view_get_model (
                                GTK_TREE_VIEW(app->treeview)));
    if (!treestore) {
        /* handle no model error */
        g_print ("treeview_append() error: !treestore\n");
        g_free (name);
        return NULL;
    }

    /* appeand name and pointer to inst as entry in treeview */
    gtk_tree_store_append (treestore, &toplevel, NULL);
    gtk_tree_store_set (treestore, &toplevel, COLNAME, name, -1);
    gtk_tree_store_set (treestore, &toplevel, COLINST, inst, -1);

    /* initialize selection to current, set focus on new tree element */
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(app->treeview));
    gtk_tree_selection_select_iter (selection, &toplevel);

    app->nfiles++;  /* update file count */

    g_free (name);  /* free name */

    return inst;
    /* temp test */
    // g_print ("model contains %d rows (nfiles: %d)\n",
    //        doctree_get_count (app->treeview), app->nfiles);
}

/** create tree model and initialize treestore with filenames provided in
 *  argv or Untitled, set app->treemodel. argv must end with sentinel NULL.
 */
GtkTreeModel *treemodel_init (mainwin_t *app, gchar **argv)
{
    GtkTreeStore *treestore;

    /* create model with string and pointer to inst.
     * TODO - consider just using pointer to inst and change render to
     *        render name from inst->fname (but fname is NULL when Untitled)
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
                        G_CALLBACK (treeview_changed), app);

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

gint doctree_get_count (GtkWidget *treeview)
{
    GtkTreeModel *model = NULL;
    GtkTreeIter iter;
    gboolean valid;
    gint nrows = 0;

    model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
    if (!model) {
        g_print ("error: tree_view_get_model - failed.\n");
        return 0;
    }

    valid = gtk_tree_model_get_iter_first (model, &iter);
    while (valid) {
        nrows++;
        valid = gtk_tree_model_iter_next (model, &iter);
    }

    return nrows;
}

/** returns buffer instance of selected document, which includes sourceview
 *  buffer, associated filename (or NULL if "Untitled"), and current line and
 *  column for insert mark in buffer so view can be restored.
 */
kinst_t *inst_get_selected (gpointer data)
{
    GtkTreeSelection *selection;        /* tree selection for buffer inst */
    GtkTreeIter iter;
    GtkTreeModel *model;
    mainwin_t *app = data;
    kinst_t *inst = NULL;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(app->treeview));
    if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
        gtk_tree_model_get (model, &iter, COLINST, &inst, -1);
    }

    return inst;
}

/** remove file identified by tree iter, clear buff if only file */
gboolean doctree_remove_iter (gpointer data, GtkTreeIter *iter)
{
    mainwin_t *app = data;
    gchar *name = NULL;
    gint n;

    /* get COLNAME from tree */
    gtk_tree_model_get (app->treemodel, iter, COLNAME, &name, -1);

    if (!name) {
        g_warning ("doctree_remove_iter() - gtk_tree_model_get failed.");
        return FALSE;
    }

    n = untitled_get_no (name); /* parse "Untitled(n)" for n */
    g_free (name);

    if (n >= 0)
        untitled_remove (app, n);   /* clear bit in app->nuntitled */

    if (app->nfiles > 1) {  /* only remove entry if > 1 file */
        gtk_tree_store_remove (GTK_TREE_STORE(app->treemodel), iter);
        app->nfiles--;  /* decrement file count */
        return TRUE;
    }
    else {  /* no selection change, handle single file, clear buffer */
        gchar *uname, *title;
        kinst_t *inst;

        /* get kinst_t inst from tree */
        /* TODO set to app->einst[app->focused]->inst (shifted) */
        gtk_tree_model_get (app->treemodel, iter, COLINST, &inst, -1);
        /* if (newinst->fname) set COLNAME to newinst->fname and
         * COLINST to newinst ELSE set to uname and clear!
         */
        inst_reset_state (inst);                    /* set values 0/NULL */
        uname = treeview_initial_name (app, inst);       /* get Untitled(n) name */
        gtk_tree_store_set (GTK_TREE_STORE(app->treemodel), iter,
                            COLNAME, uname, -1);    /* set COLNAME in tree */

        /* TODO move to buffer_clear() funciton */
        gtk_text_buffer_set_text (GTK_TEXT_BUFFER(inst->buf), "", -1);
        gtk_text_buffer_set_modified (GTK_TEXT_BUFFER(inst->buf), FALSE);

        title = g_strdup_printf ("%s - %s", APPNAME, uname);
        gtk_window_set_title (GTK_WINDOW (app->window), title);

        g_free (title);
        g_free (uname);
    }

    return FALSE;
}

/** gboolean treeview_remove_selected(), remove file from views and treeview.
 *  remove selected file from all views, closing views, if multiple files
 *  remain, set focus on previous file in tree, otherwise, reset current inst
 *  and clear inst->buf, highlight file in treeview, free all memory
 *  associated with removed inst filename, inst slice and allocated iter
 *  returned from tree_get_iter_from_view(). returns true on success, false
 *  otherwise.
 */
gboolean treeview_remove_selected (gpointer data)
{
    mainwin_t *app = data;
    kinst_t *inst = app->einst[app->focused]->inst,
            *colinst = NULL,
            *lastinst = NULL;;
    GtkTreeModel *model = app->treemodel;
    GtkTreeIter *victim = NULL,
                iter,
                *last = NULL;
    GtkWidget *view = NULL;
    gchar *name = NULL;
    gint n = app->nview;
    gboolean valid = FALSE, found = FALSE;

    /* save iter-to-selected here, then loop closing all instances */
    victim = tree_get_iter_from_view (data);

    /* get COLNAME from tree */
    gtk_tree_model_get (app->treemodel, victim, COLINST, &colinst,
                        COLNAME, &name, -1);

    if (inst != colinst) {  /* validate inst matches colinst */
        g_warning ("treeview_remove_current() - inst != colinst");
        return FALSE;
    }

    if (!name) {    /* validate name filled */
        g_warning ("treeview_remove_current() - gtk_tree_model_get failed.");
        return FALSE;
    }

    /* Remove all view (einst[x]) here, that will simplify the removal from
     * the tree. Use the logic while (app->nview > 1 & n--) to handle the
     * VIEWs removal, and then simply remove the files. VIEW removal is the
     * same in both cases! Git rid of view, then remove 1-file.
     */
    while (app->nview > 1 && n--) {
        einst_t *einst = app->einst[n];

        if (einst->inst && einst->inst == inst) {

            GtkWidget *ewin = einst->ebox;  /* edit window bounding vbox */
            gint focused = app->focused;    /* currently focused textview */

            gtk_widget_destroy (ewin);      /* destroy widget */

            /* shift existing higher einst down */
            for (gint i = focused; (i + 1) < app->nview; i++)
                einst_move (app->einst[i], app->einst[i+1]);

            /* if focused and focused > 0 */
            if (n == focused && focused > 0) {
                /* if focus in highest number view */
                if (focused < app->nview - 1) {
                    app->focused--; /* move focus to previous view */
                }   /* note: set focus on textview after loop complete */
            }

            app->nview--;       /* decrement number of views shown */
        }
    }

    /* update nuntitled bitfield */
    n = untitled_get_no (name);         /* check/parse "Untitled(n)" for n */
    if (n >= 0)
        untitled_remove (app, n);       /* clear bit in app->nuntitled */
    g_free (name);

    /* free allocated memory associated with inst (except buf)
     * set freed values 0/NULL, in prep for reuse or deletion
     */
    inst_reset_state (inst);

    if (app->nfiles == 1) {             /* only single file, change buffer */
        gchar *uname, *title;           /* untitled name, window title */

        uname = treeview_initial_name (app, inst);  /* get Untitled(n) */
        gtk_tree_store_set (GTK_TREE_STORE(app->treemodel), victim,
                            COLNAME, uname, -1);    /* set COLNAME in tree */

        /* TODO move to buffer_clear() funciton */
        gtk_text_buffer_set_text (GTK_TEXT_BUFFER(inst->buf), "", -1);
        gtk_text_buffer_set_modified (GTK_TEXT_BUFFER(inst->buf), FALSE);

        /* set window title */
        title = g_strdup_printf ("%s - %s", APPNAME, uname);
        gtk_window_set_title (GTK_WINDOW (app->window), title);

        found = TRUE;   /* set found flag indicating successful removal */

        g_free (title);
        g_free (uname);
    }
    else {  /* additional files remain open, just remove victim */
        view = app->einst[app->focused]->view;  /* get current view widget */
        inst = app->einst[app->focused]->inst;  /* update w/new app->focused */

        /* get first iter, iterate model to locate current inst, set selection */
        valid = gtk_tree_model_get_iter_first (model, &iter);

        while (valid) {         /* loop over each entry in model */
            /* get colinst from iter */
            gtk_tree_model_get (model, &iter, COLINST, &colinst, -1);

            /* compare pointer to sourceview with buf from textview (widget) */
            if (inst == colinst) {
                if (last) {
                    /* if last iter set, current to last */
                    app->einst[app->focused]->inst = lastinst;
                    /* set buffer in active textview */
                    gtk_text_view_set_buffer (GTK_TEXT_VIEW(view),
                                                GTK_TEXT_BUFFER(lastinst->buf));
                    /* set treeview selection on current buf COLNAME */
                    tree_set_selection (view, data);
                }
                else    /* found colinst in tree, but last iter invalid */
                    g_warning ("treeview_remove_selected() invalid last.\n");

                found = TRUE;   /* set found colinst flag */
                break;
            }
            lastinst = colinst; /* set last instance to current */
            last = &iter;       /* set last iter to current */

            /* get next tree iter */
            valid = gtk_tree_model_iter_next (model, &iter);
        }

        /* remove victim from tree and free allocated slice */
        gtk_tree_store_remove (GTK_TREE_STORE(app->treemodel), victim);
        g_slice_free (kinst_t, inst);   /* free inst slice after removal */
        app->nfiles--;                  /* decrement file count */
    }

    if (!found) /* validate inst found or warn */
        g_warning ("tree_set_selection inst not found.");

    g_slice_free (GtkTreeIter, victim);  /* free allocated iter */

    return found;
}

/** doctree callbacks */

/* GtkTreeSelection "changed" signal callback */
void treeview_changed (GtkWidget *widget, gpointer data)
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    mainwin_t *app = data;
    GtkWidget *view = app->einst[app->focused]->view;
    gchar *value, *title;
    kinst_t *inst;

    if (gtk_tree_selection_get_selected (GTK_TREE_SELECTION(widget),
                                            &model, &iter)) {

        gtk_tree_model_get (model, &iter, COLNAME, &value, COLINST, &inst,  -1);

        /* check/set modified state in window title text on selection change */
        if (gtk_text_buffer_get_modified (GTK_TEXT_BUFFER(inst->buf)))
            title = g_strdup_printf ("%s - %s [modified]", APPNAME, value);
        else
            title = g_strdup_printf ("%s - %s", APPNAME, value);

        /* set window title */
        gtk_window_set_title (GTK_WINDOW (app->window), title);

        /* set buffer in active textview */
        gtk_text_view_set_buffer (GTK_TEXT_VIEW(view),
                                    GTK_TEXT_BUFFER(inst->buf));

        /* set app->einst[app->focused]->inst to point to buffer inst */
        app->einst[app->focused]->inst = inst;

        g_free (value); /* values with type G_TYPE_STRING or G_TYPE_BOXED */
                        /* have to be freed, G_TYPE_POINTER does not */
        g_free (title);
    }

    if (data) {}
}

