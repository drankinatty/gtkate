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
            tree_set_selection (widget, app);

            /* test with tree_get_iter_from_view () - passed
            GtkTreeIter *iter = tree_get_iter_from_view (app);
            if (iter)
            g_slice_free (GtkTreeIter, iter);

            FIXME - need to get iter and gtk_tree_model_get inst and assign
                    app->einst[i]->inst = inst;
                    to coordinate correct buffer inst with focused textview.
                    we do that the other way, e.g. clicking selection changes
                    buffer, but not here in reverse. that is messing up file
                    open.
                    NOTE: done - added to treeview_changed callback!
            */
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

/** callback functions for textview */

/** Insert/Overwrite handler. */
void on_insmode (GtkWidget *widget, gpointer data)
{
    mainwin_t *app = data;

    app->overwrite = app->overwrite ? FALSE : TRUE;

    status_set_default (app);   /* update the status bar */

    if (widget) {}
}

/** on_buffer_changed fires before on_mark_set with all changes.
 *  (even undo) and fires before gtk_text_buffer_get_modified()
 *  reflects change.
 */
void on_buffer_changed (GtkTextBuffer *buffer, gpointer data)
{
    mainwin_t *app = data;
    gboolean modified = gtk_text_buffer_get_modified (buffer);

    /* set app->eolchg if buffer changed so conversion runs on eol change */
    if (modified && app->eol != app->oeol)
        app->eolchg = TRUE;

}

/** on cursor position change (insert mark_set), update line, lines, col.
 *  Note: on_mark_set fires after on_buffer_changed, so it will more
 *  accurately capture buffer modification state.
 */
void on_mark_set (GtkTextBuffer *buffer, GtkTextIter *iter,
                  GtkTextMark *mark, gpointer data)
{
    // mainwin_t *app = data;
    // gboolean modified = gtk_text_buffer_get_modified (buffer);

    /* update window title */
    // if (!modified)
    //     gtkwrite_window_set_title (NULL, app);

    /* update status bar */
    status_set_default (data);

    if (buffer || iter || mark) {}
}

gboolean on_keypress (GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    mainwin_t *app = data;
    einst_t *einst = app->einst[app->focused];

    if (gtk_text_view_im_context_filter_keypress (GTK_TEXT_VIEW (einst->view),
                                                    event)) {
        return TRUE;
    }

    /* handle control + shift key combinations */
    if (event->type == GDK_KEY_PRESS &&
        event->state & GDK_CONTROL_MASK &&
        event->state & GDK_SHIFT_MASK) {
        switch (event->keyval) {
            case GDK_KEY_Left:
                if (app->ctrl_shift_right_fix) {
                    gboolean rtn = buffer_select_to_prev_char (app);
                    /* if no prev ctrl+shift keypress or last was to left
                     * push another left keypress on stack.
                     */
                    if (!app->bindex || bstack_last (app) == LEFT)
                        bstack_push (app, LEFT);    /* record LEFT selection */
                    else
                        bstack_pop (app);           /* pop left from stack */
                    return rtn;
                }
                break;
            case GDK_KEY_Right:
                if (app->ctrl_shift_right_fix) {
                    gboolean rtn = buffer_select_to_next_char (app);
                    /* if no prev ctrl+shift keypress or last was to right
                     * push another right keypress on stack.
                     */
                    if (!app->bindex || bstack_last (app) == RIGHT)
                        bstack_push (app, RIGHT);   /* record RIGHT selection */
                    else
                        bstack_pop (app);           /* pop right from stack   */
                    return rtn;
                }
                break;
        }
        return FALSE;   /* return - only process ctrl + shift events */
    }

    /* handle control key combinations */
    /*
    if (event->type == GDK_KEY_PRESS &&
        event->state & GDK_CONTROL_MASK) {
        switch (event->keyval) {
            case GDK_KEY_Right:
                g_print ("key pressed: %s\n", "ctrl + Right->Arrow");
                break;
        }
        return FALSE;
    }
    */

    /* handle shift key combinations */
    if (event->type == GDK_KEY_PRESS &&
        event->state & GDK_SHIFT_MASK) {
        switch (event->keyval) {
            case GDK_KEY_Left:
                /* with ctrl_shift_right_fix, reduce selection by 1 at end
                 * if selection exists and started to right and user releases
                 * ctrl key and arrows backwards.
                 */
                if (app->ctrl_shift_right_fix) {
                    if (buffer_reduce_selection (app))
                        return TRUE;
                }
                break;
            // case GDK_KEY_Right:
            //     g_print ("key pressed: %s\n", "shift + Right->Arrow");
            //     break;
        }
        return FALSE;
    }

    /* handle normal keypress events */
    if (event->type == GDK_KEY_PRESS) {
        switch (event->keyval) {
            case GDK_KEY_BackSpace:
                if (app->smartbs)       /* smart_backspace in filebuf.c */
                    return smart_backspace (app);
                break;                  /* or just return FALSE; */
            case GDK_KEY_Tab:;          /* catch tab, replace with softtab spaces */
                    return smart_tab (app);
            case GDK_KEY_Return:
                if (app->indentauto)
                    return buffer_indent_auto (app);
                else
                    return buffer_insert_eol (app);
                break;
            case GDK_KEY_KP_Enter:
                if (app->indentauto)
                    return buffer_indent_auto (app);
                else
                    return buffer_insert_eol (app);
                break;
//             case GDK_KEY_Home:
// #ifndef HAVESOURCEVIEW
//                 if (app->smarthe)
//                     return ((app->kphome = smart_home (app)));
// #endif
//                 break;
        }
    }

    app->kphome = FALSE;    /* reset kphome - return above protects needed TRUE */

    return FALSE;

    if (widget) {}
    if (app) {}
}

/** create source_view, set font and tab size, set needed callbacks */
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

    /* create statusbar for textview */
    ewin->sbar = create_statusbar (ewin->ebox);
    status_set_default (app);
    gtk_widget_show (ewin->sbar);


    /* causes focus to hide textview cursor until moved with key or mouse
     * in textview.
     */
    g_signal_connect (view, "focus-in-event",
                      G_CALLBACK (text_view_focus_in), app);

    /* general event handler, modified, cursor change and insert/overwrite */
    g_signal_connect (G_OBJECT (view), "key_press_event",
                      G_CALLBACK (on_keypress), app);

    g_signal_connect (inst->buf, "changed",
                      G_CALLBACK (on_buffer_changed), app);

    g_signal_connect (inst->buf, "mark_set",
                      G_CALLBACK (on_mark_set), app);

    g_signal_connect (G_OBJECT (view), "toggle-overwrite",
                      G_CALLBACK (on_insmode), app);

    gtk_widget_show (ewin->ebox);   /* show final widget */
    gtk_widget_grab_focus (view);   /* grab focus in new view */

    return ewin->ebox;
}

void focus_prev_view (gpointer data) {

    mainwin_t *app = data;                      /* application data stuct */

    if (app->focused)  /* if nothing shifted, move focus to next view */
        app->focused--;

    /* TODO need to select remaining entry in tree view */

    gtk_widget_grab_focus (app->einst[app->focused]->view); /* grab focus */
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

    /* TODO - decide whether to place focus in new instance or leave on
     * current (present behavior). set convenience pointer kinst. decide
     * whether to scroll new instance to match current insert.
     * NOTE confirm done in doctree.
     */
    // app->focused++;
    // gtk_widget_grab_focus (app->einst[app->focused]->view);

    return app->einst[current];
}

/** remove current editor split */
gboolean ewin_remove_split (gpointer data)
{
    mainwin_t *app = data;                      /* application data stuct */
    einst_t *einst = app->einst[app->focused];  /* focused editor struct */
    GtkWidget *ewin = einst->ebox;              /* bounding vbox of view */
    gint i;

    if (app->nview == 1)                        /* if single view, return */
        return FALSE;

    gtk_widget_destroy (ewin);      /* destory bounding vbox removing view, */
    einst_reset (einst);            /* set all struct members NULL, preserving
                                     * stuct for buffer inst. */

    for (i = app->focused; (i + 1) < MAXVIEW; i++)  /* shift einst down */
        einst_move (app->einst[i], app->einst[i+1]);

    focus_prev_view (data);

    app->nview--;       /* decrement number of views shown */

    return TRUE;
}

/* remove specific instance - allows interating and removing multiple */
gboolean ewin_remove_view (gpointer data, einst_t *einst)
{
    mainwin_t *app = data;                      /* application data stuct */
    GtkWidget *ewin = einst->ebox;              /* bounding vbox of view */
    gint i;

    if (app->nview == 1)                        /* if single view, return */
        return FALSE;

    gtk_widget_destroy (ewin);      /* destory bounding vbox removing view, */
    einst_reset (einst);            /* set all struct members NULL, preserving
                                     * stuct for buffer inst. */

//     for (i = app->focused; (i + 1) < MAXVIEW; i++)  /* shift einst down */
    for (i = app->focused; (i + 1) < app->nview; i++)   /* shift einst down */
        einst_move (app->einst[i], app->einst[i+1]);

//     focus_prev_view (data);  /* handle focus in caller - multipe views */

    app->nview--;       /* decrement number of views shown */

    return TRUE;
}

