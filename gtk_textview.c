#include "gtk_textview.h"

/** function to set the tab width to sz spaces based on font description */
void set_tab_size (PangoFontDescription *font_desc, mainwin_t *app, gint sz)
{
    PangoTabArray *tab_array;
    PangoLayout *layout;
    GtkWidget *view = app->einst[app->nview]->view;
    gint width, i;

    if (app->tabstring)
        g_free (app->tabstring);
    app->tabstring = g_strdup_printf ("%*s", sz, " ");

    layout = gtk_widget_create_pango_layout (view, app->tabstring);
    pango_layout_set_font_description (layout, font_desc);
    pango_layout_get_pixel_size (layout, &width, NULL);
    if (width) {
        tab_array = pango_tab_array_new (app->winwidth/width, TRUE);
        for (i = 0; i * width < app->winwidth; i++)
            pango_tab_array_set_tab (tab_array, i, PANGO_TAB_LEFT, i * width);

        gtk_text_view_set_tabs (GTK_TEXT_VIEW(view), tab_array);
        pango_tab_array_free (tab_array);
    }
}

gboolean text_view_focus_in (GtkWidget *widget, GdkEvent *event, gpointer data)
{
    mainwin_t *app = data;
    gboolean found = FALSE;

    /* iterate over sourceview widgets to determine which is focused */
    for (gint i = 0; i < app->nview && app->einst[i]->view; i++) {
        if (widget == app->einst[i]->view) {
            app->focused = i;
#ifdef DEBUG
            g_print ("focus-in-event: app->einst[%d]->view\n", i);
#endif
            found = TRUE;
            /* TODO - use app->einst[i]->inst->buf to set highlight in tree */
            if (app->nview > 1)
                tree_get_inst_iter (widget, app);
            break;
        }
    }

    if (!found) /* handle error if focused widget not found */
        g_print ("error: focus-in-event: app->einst[n]->view not found.\n");

    return TRUE;    /* TODO - validate return in caller */

    if (widget || event || data) {}
}

/** create statusbar instance, returns GtkWidget* */
GtkWidget *create_statusbar (GtkWidget *vbox)
{
    GtkWidget *sbalign;
    GtkWidget *statusbar;
    guint ptop;                     /* padding, top, bot, l, r  */
    guint pbot;
    guint pleft;
    guint pright;

    sbalign = gtk_alignment_new (0, .5, 1, 1);

    gtk_alignment_get_padding (GTK_ALIGNMENT (sbalign), &ptop, &pbot,
                                &pleft, &pright);
    gtk_alignment_set_padding (GTK_ALIGNMENT (sbalign), ptop, pbot + 2,
                                pleft + 5, pright);

    statusbar = gtk_statusbar_new ();

    gtk_container_add (GTK_CONTAINER (sbalign), statusbar);
    gtk_box_pack_end (GTK_BOX (vbox), sbalign, FALSE, FALSE, 0);

    gtk_widget_show (statusbar);
    gtk_widget_show (sbalign);

    return statusbar;
}

/** create source_view, set font and tab size */
GtkWidget *create_scrolled_view (mainwin_t *app)
{
    GtkWidget *view;
    PangoFontDescription *font_desc;            /* font description */
    einst_t *ewin = app->einst[app->nview];     /* editor window instance */
    kinst_t *inst = inst_get_selected (app);    /* inst w/buf from treeview */

    /* create vbox for infobar, scrolled_window, and statusbar */
    ewin->ebox = gtk_vbox_new (FALSE, 0);

    /* create vbox to display infobar */
    ewin->ibox = gtk_vbox_new (FALSE, 0);
    gtk_box_pack_start(GTK_BOX(ewin->ebox),
                        ewin->ibox, FALSE, FALSE, 0);
    gtk_widget_show (ewin->ibox);

    /* create scrolled_window for view */
    ewin->swin = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (ewin->swin),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start (GTK_BOX (ewin->ebox),
                        ewin->swin, TRUE, TRUE, 0);
    gtk_widget_show (ewin->swin);

    /* create text_view */
    view = ewin->view = gtk_source_view_new ();

    /* check if treeview points to file instance */
    if (inst) { /* if so, set sourceview buffer to inst->buf */
        gtk_text_view_set_buffer (GTK_TEXT_VIEW(view),
                                    GTK_TEXT_BUFFER(inst->buf));
    }
    else {  /* otherwise get instance from first file in treemodel */
        gchar *str;
        GtkTreeIter iter;
        gboolean valid = gtk_tree_model_get_iter_first (app->treemodel, &iter);

        if (valid) {
            gtk_tree_model_get (app->treemodel, &iter,
                                COLNAME, &str, COLINST, &inst, -1);
            gtk_text_view_set_buffer (GTK_TEXT_VIEW(view),
                                        GTK_TEXT_BUFFER(inst->buf));
            g_free (str);
        }
        else
            g_print ("  no valid tree model iter first\n");
    }
    ewin->inst = inst;  /* set pointer to current file instance */

    /*      app set show line numbers */
    gtk_source_view_set_show_line_numbers (GTK_SOURCE_VIEW(view), app->lineno);
    /*      app set highlight current line */
    gtk_source_view_set_highlight_current_line (GTK_SOURCE_VIEW(view), app->linehghlt);
    /*      app set auto indent */
    gtk_source_view_set_auto_indent (GTK_SOURCE_VIEW(view), app->indentauto);
    /*      app set/show right-margin */
    gtk_source_view_set_right_margin_position (GTK_SOURCE_VIEW (view),
                                                app->marginwidth);
    gtk_source_view_set_show_right_margin (GTK_SOURCE_VIEW (view),
                                            app->showmargin);

    /* set_smart_backspace available in sourceview 3 */
    // gtk_source_view_set_smart_backspace (GTK_SOURCE_VIEW(app->view), TRUE);
    gtk_source_view_set_smart_home_end (GTK_SOURCE_VIEW(view),
                                        GTK_SOURCE_SMART_HOME_END_BEFORE);

    /* create the sourceview completion object */
    // if (app->enablecmplt)
    //     create_completion (app);

    /* set wrap mode and show text_view */
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (view), GTK_WRAP_WORD);
    gtk_text_view_set_left_margin (GTK_TEXT_VIEW (view), 5);
    gtk_widget_show (view);

    /* Change default font throughout the widget */
    font_desc = pango_font_description_from_string (app->fontname);
    gtk_widget_modify_font (view, font_desc);

    /* set tab to lesser of softtab and tabstop if softtab set */
    set_tab_size (font_desc, app, (app->softtab && (app->softtab < app->tabstop) ?
                                    app->softtab : app->tabstop));
    pango_font_description_free (font_desc);

    gtk_container_add (GTK_CONTAINER (ewin->swin), view);
    gtk_container_set_border_width (GTK_CONTAINER (ewin->swin),
                                    app->swbordersz);

    ewin->sbar = create_statusbar (ewin->ebox);

    gtk_widget_show (ewin->ebox);

    /* causes focus to hide textview cursor until moved with key or mouse
     * in textview.
     */
    g_signal_connect (view, "focus-in-event",
                      G_CALLBACK (text_view_focus_in), app);

    return ewin->ebox;
}

/** create new editor split */
einst_t *ewin_create_split (gpointer data)
{
    mainwin_t *app = data;
    GtkWidget *ewin;
    gint current = app->nview;

    if (app->nview >= MAXVIEW) {    /* validate max views not reached */
        char *msg = g_strdup_printf ("MAXVIEW '%d' reached.", MAXVIEW);

        err_dialog (msg);
        g_free (msg);

        return NULL;
    }

    ewin = create_scrolled_view (app);  /* create new view/split */
    if (!ewin) {    /* validate */
        char *msg = g_strdup ("Failed to create split.");

        err_dialog (msg);
        g_free (msg);

        return NULL;
    }

    /* pack new view into vbox within right-hpaned */
    gtk_box_pack_start(GTK_BOX(app->vboxedit), ewin, TRUE, TRUE, 0);

    app->nview++;   /* increment number of views */

    /* TODO - decide whether to place focus in new instance of leave on
     * current (present behavior). set convenience pointer kinst. decide
     * whether to scroll new instance to match current insert.
     */
    // app->focused++;
    // gtk_widget_grab_focus (app->einst[app->focused]->view);

    return app->einst[current];
}

/** remove current editor split */
void ewin_remove_split (gpointer data)
{
    mainwin_t *app = data;

    if (app->nview == 1)
        return;

    gtk_widget_destroy (app->einst[app->focused]->ebox);

    app->einst[app->focused]->ebox = NULL;  /* set removed edit window */
    app->einst[app->focused]->ibox = NULL;  /* pointers to NULL */
    app->einst[app->focused]->swin = NULL;
    app->einst[app->focused]->view = NULL;
    app->einst[app->focused]->sbar = NULL;
    app->einst[app->focused]->inst = NULL;

    /* shift elements down to replace removed instance */
    for (gint i = app->focused, j = i + 1; j < app->nview; i++, j++) {
        einst_t *tgt = app->einst[i],   /* target for shift */
                *src = app->einst[j];   /* source for shift */

        tgt->ebox = src->ebox;  /* shift pointers src -> tgt */
        tgt->ibox = src->ibox;
        tgt->swin = src->swin;
        tgt->view = src->view;
        tgt->sbar = src->sbar;
        tgt->inst = src->inst;

        src->ebox = NULL;       /* set src pointers null */
        src->ibox = NULL;
        src->swin = NULL;
        src->view = NULL;
        src->sbar = NULL;
        src->inst = NULL;
    }

    if (app->focused) { /* decrement and set focus on next-lower edit window */
        app->focused--;
        gtk_widget_grab_focus (app->einst[app->focused]->view);
    }

    app->nview--;       /* decrement number of views shown */
}