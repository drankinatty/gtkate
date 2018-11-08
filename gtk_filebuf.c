#include "gtk_filebuf.h"

/** file to set file stat information for filename */
void file_get_stats (kinst_t *inst)
{
    const gchar *filename = inst->filename;
    struct stat sb;

    inst->filemode = 0;
    inst->fileuid = 0;
    inst->filegid = 0;

     /* validate inst and filename */
    if (!inst || !filename)
        return;

    /* validate file exists */
    if (!g_file_test (filename, G_FILE_TEST_EXISTS)) {
        char *str;
        str = g_strdup_printf ("error: file not found,  file_get_stats() '%s'",
                                    filename);
        g_warning (str);
        g_free (str);

        return;
    }

    if (g_stat (filename, &sb)) {   /* get file stats */
        g_print ("error: stat of file failed '%s'\n", filename);
        return;
    }

    inst->filemode = sb.st_mode;
    inst->fileuid  = sb.st_uid;
    inst->filegid  = sb.st_gid;
}

gboolean buffer_select_all (gpointer data)
{
    mainwin_t *app = data;
    einst_t *einst = app->einst[app->focused];
    kinst_t *inst = einst->inst;

    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(inst->buf);
    GtkTextIter start, end;

    if (!buffer)
        return FALSE;

    /* get iter at current insert position */
    gtk_text_buffer_get_iter_at_mark (buffer, &start,
                    gtk_text_buffer_get_insert (buffer));

    if (app->last_pos == NULL) /* set last_pos to restore on cancel */
        app->last_pos = gtk_text_buffer_create_mark (buffer, "last_pos",
                                                    &start, FALSE);
    else
        gtk_text_buffer_move_mark (buffer, app->last_pos, &start);


    /* get start and end iters, verify not the same */
    gtk_text_buffer_get_bounds (buffer, &start, &end);
    if (gtk_text_iter_equal (&start, &end))
        return FALSE;

    /* select range */
    gtk_text_buffer_select_range (buffer, &start, &end);

    return TRUE;
}

gboolean buffer_deselect_all (gpointer data)
{
    mainwin_t *app = data;
    einst_t *einst = app->einst[app->focused];
    kinst_t *inst = einst->inst;

    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(inst->buf);
    GtkTextIter start, end;

    if (!buffer)
        return FALSE;

    /* validate existing selection */
    if (!gtk_text_buffer_get_selection_bounds (buffer, &start, &end))
        return FALSE;

    /* get iter at saved insert position, or start if not saved */
    if (app->last_pos) {
        gtk_text_buffer_get_iter_at_mark (buffer, &start, app->last_pos);
        gtk_text_buffer_delete_mark (buffer, app->last_pos);
        app->last_pos = NULL;
    }
    else
        gtk_text_buffer_get_start_iter (buffer, &start);

    /* place cursor at iter */
    gtk_text_buffer_place_cursor (buffer, &start);

    return TRUE;
}

/** buffer_select_to_next_char selects from cursor to next non-ws char.
 *  this corrects the gtk_text_view default selection of whitespace
 *  plus the next word on the ctrl+shift+right-arrow combination.
 *
 *  the keypress handler must keep track of the number of left or right
 *  ctrl+shift select keypress sequences so using the arrow-key in the
 *  opposite direction will undo in the correct direction. the current
 *  bstack will only keep track of 128 simultaneous ctrl+shift keypress
 *  events in one direction at a time (or 256 if undoing 128 in the
 *  opposite direction. increase STKMAX if you need more.
 */
gboolean buffer_select_to_next_char (gpointer data)
{
    mainwin_t *app = data;
    einst_t *einst = app->einst[app->focused];
    kinst_t *inst = einst->inst;

    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(inst->buf);
    GtkTextIter start, end;
    gunichar c;

    /* check existing selection, set start = end, otherwise
     * get iter at cursor position ("Insert" mark), set end = start
     */
    if (!gtk_text_buffer_get_selection_bounds (buffer, &start, &end)) {
        gtk_text_buffer_get_iter_at_mark (buffer, &start,
                                gtk_text_buffer_get_insert (buffer));
        end = start;
        bstack_clear (app);
    }

    /* check if prev ctrl+shift keypress & if it was RIGHT */
    if (!app->bindex || bstack_last (app) == RIGHT) {
        /* get char and check if whitespace or non-whitespace */
        c = gtk_text_iter_get_char (&end);
        if (c == ' ' || c == '\t') {
            /* read contiguous whitespace to next word */
            while (c == ' ' || c == '\t') {
                if (!gtk_text_iter_forward_char (&end))
                    break;
                c = gtk_text_iter_get_char (&end);
            }
        }
        else {
            /* read contiguous non-whitespace */
            while (c != ' ' && c != '\t') {
                if (!gtk_text_iter_forward_char (&end))
                    break;
                c = gtk_text_iter_get_char (&end);
            }
            /* read contiguous whitespace to next word */
            while (c == ' ' || c == '\t') {
                if (!gtk_text_iter_forward_char (&end))
                    break;
                c = gtk_text_iter_get_char (&end);
            }
        }
    }
    else {
        c = gtk_text_iter_get_char (&start);
        if (c == ' ' || c == '\t') {
            /* read contiguous whitespace to next word */
            while (c == ' ' || c == '\t') {
                if (!gtk_text_iter_forward_char (&start) ||
                    gtk_text_iter_equal (&start, &end))
                    goto nextrdlt;
                c = gtk_text_iter_get_char (&start);
            }
            /* read contiguous non-whitespace */
            while (c != ' ' && c != '\t') {
                if (!gtk_text_iter_forward_char (&start) ||
                    gtk_text_iter_equal (&start, &end))
                    break;
                c = gtk_text_iter_get_char (&start);
            }
            nextrdlt:;
        }
        else {
            /* read contiguous non-whitespace */
            while (c != ' ' && c != '\t') {
                if (!gtk_text_iter_forward_char (&start) ||
                    gtk_text_iter_equal (&start, &end))
                    break;
                c = gtk_text_iter_get_char (&start);
            }
        }
        if (gtk_text_iter_equal (&start, &end)) {
            gtk_text_buffer_place_cursor (buffer, &start);
            bstack_clear (app);
            return TRUE;
        }
    }

    /* select range */
    gtk_text_buffer_select_range (buffer, &start, &end);

    return TRUE;
}

/** buffer_select_to_next_char selects from cursor to next non-ws char.
 *  this corrects the gtk_text_view default selection of whitespace
 *  plus the next word on the ctrl+shift+right-arrow combination.
 *
 *  (see extended note on function above for bstack tracking of
 *   ctrl+shift keypress events.
 */
gboolean buffer_select_to_prev_char (gpointer data)
{
    mainwin_t *app = data;
    einst_t *einst = app->einst[app->focused];
    kinst_t *inst = einst->inst;

    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(inst->buf);
    GtkTextIter start, end;
    gunichar c;

    /* check existing selection, set start = end, otherwise
     * get iter at cursor position ("Insert" mark), set end = start
     */
    if (!gtk_text_buffer_get_selection_bounds (buffer, &start, &end)) {
        gtk_text_buffer_get_iter_at_mark (buffer, &start,
                                gtk_text_buffer_get_insert (buffer));
        end = start;
        bstack_clear (app);
    }

    /* check if prev ctrl+shift keypress & if it was LEFT */
    if (!app->bindex || bstack_last (app) == LEFT) {

        gtk_text_iter_backward_char (&start);

        /* get char and check if whitespace or non-whitespace */
        c = gtk_text_iter_get_char (&start);
        if (c == ' ' || c == '\t') {
            /* read contiguous whitespace to next word */
            while (c == ' ' || c == '\t') {
                if (!gtk_text_iter_backward_char (&start))
                    break;
                c = gtk_text_iter_get_char (&start);
            }
        }
        else {
            /* read contiguous non-whitespace */
            while (c != ' ' && c != '\t') {
                if (!gtk_text_iter_backward_char (&start))
                    goto prevrdlt;
                c = gtk_text_iter_get_char (&start);
            }
            /* read contiguous whitespace to next word */
            while (c == ' ' || c == '\t') {
                if (!gtk_text_iter_backward_char (&start))
                    break;
                c = gtk_text_iter_get_char (&start);
            }
            prevrdlt:;
        }
        if (gtk_text_iter_equal (&start, &end)) {
            gtk_text_buffer_place_cursor (buffer, &start);
            bstack_clear (app);
            return TRUE;
        }
        else
            gtk_text_iter_forward_char (&start);
    }
    else {

        gtk_text_iter_backward_char (&end);

        /* get char and check if whitespace or non-whitespace */
        c = gtk_text_iter_get_char (&end);
        if (c == ' ' || c == '\t') {
            /* read contiguous whitespace to next word */
            while (c == ' ' || c == '\t') {
                if (!gtk_text_iter_backward_char (&end) ||
                    gtk_text_iter_equal (&start, &end))
                    goto prevrdrt;
                c = gtk_text_iter_get_char (&end);
            }
            /* read contiguous non-whitespace */
            while (c != ' ' && c != '\t') {
                if (!gtk_text_iter_backward_char (&end) ||
                    gtk_text_iter_equal (&start, &end))
                    break;
                c = gtk_text_iter_get_char (&end);
            }
            prevrdrt:;
        }
        else {
            /* read contiguous whitespace to next word */
            while (c != ' ' || c != '\t') {
                if (!gtk_text_iter_backward_char (&end) ||
                    gtk_text_iter_equal (&start, &end))
                    break;
                c = gtk_text_iter_get_char (&end);
            }
        }
        if (gtk_text_iter_equal (&start, &end)) {
            gtk_text_buffer_place_cursor (buffer, &start);
            bstack_clear (app);
            return TRUE;
        }
        else
            gtk_text_iter_forward_char (&end);
    }

    /* select range */
    gtk_text_buffer_select_range (buffer, &start, &end);

    return TRUE;
}

/** buffer reduce selection bounds by 1 at end */
gboolean buffer_reduce_selection (gpointer data)
{
    mainwin_t *app = data;
    einst_t *einst = app->einst[app->focused];
    kinst_t *inst = einst->inst;

    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(inst->buf);
    GtkTextIter start, end;
    gboolean selection = gtk_text_buffer_get_selection_bounds (buffer,
                                                                &start, &end);

    /** check existing selection, bstack active and first movement RIGHT */
    if (!selection || !app->bindex || !app->bstack[0]) {
        bstack_clear (app);
        return FALSE;
    }

    g_print ("selection: %s  app->bindex: %u  app->bstack[0]: %u\n",
            selection ? "TRUE" : "FALSE", app->bindex, app->bstack[0]);
    gtk_text_iter_backward_char (&end);

    /* select range */
    gtk_text_buffer_select_range (buffer, &start, &end);

    return TRUE;
}

/** indent current/selected lines to the next softtab stop.
 *  text will be aligned to the next softtab on indent
 *  regardless of the number of spaces before the next
 *  softtab.
 */
void buffer_indent_lines (gpointer data, GtkTextIter *start, GtkTextIter *end)
{
    mainwin_t *app = data;
    einst_t *einst = app->einst[app->focused];
    kinst_t *inst = einst->inst;

    GtkTextView *view = GTK_TEXT_VIEW(einst->view);
    GtkTextBuffer *buf = GTK_TEXT_BUFFER(inst->buf);
    GtkTextMark *start_mark, *end_mark;

    gint i, start_line, end_line, nspaces;
    gchar *ind_buffer = NULL;
    gboolean bracket_hl;

    bracket_hl = gtk_source_buffer_get_highlight_matching_brackets (GTK_SOURCE_BUFFER (buf));
    gtk_source_buffer_set_highlight_matching_brackets (GTK_SOURCE_BUFFER (buf), FALSE);

    start_mark = gtk_text_buffer_create_mark (buf, NULL, start, FALSE);
    end_mark = gtk_text_buffer_create_mark (buf, NULL, end, FALSE);

    start_line = gtk_text_iter_get_line (start);
    end_line = gtk_text_iter_get_line (end);

    if ((gtk_text_iter_get_visible_line_offset (end) == 0) &&
        (end_line > start_line)) {
            end_line--;
    }

    gtk_text_buffer_begin_user_action (buf);

    for (i = start_line; i <= end_line; i++) {

        GtkTextIter iter;
        gint offset = 0;

        gtk_text_buffer_get_iter_at_line (buf, &iter, i);

        /* iterate past tabs computing character equivalent */
        for (;;) {
            gunichar c;
            c = gtk_text_iter_get_char (&iter);

            if (c == '\t' || c == ' ')
                offset += (c == '\t') ? app->softtab : 1;
            else
                break;

            gtk_text_iter_forward_char (&iter);
        }

        nspaces = app->softtab - offset % app->softtab;

        ind_buffer = g_strdup_printf ("%*s", nspaces, " ");

        gtk_text_buffer_insert (buf, &iter, ind_buffer, -1);

        g_free (ind_buffer);

        /* reset start iter and start mark to beginning of line
         * so that selection continues to encompass entire first line.
         */
        if (start_line != end_line && i == start_line &&
            !gtk_text_iter_starts_line (&iter)) {
            gtk_text_iter_set_line_offset (&iter, 0);
            gtk_text_buffer_move_mark (buf, start_mark, &iter);
        }
    }

    gtk_text_buffer_end_user_action (buf);

    gtk_source_buffer_set_highlight_matching_brackets (GTK_SOURCE_BUFFER (buf),
                                                        bracket_hl);

    gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW (view),
                                        gtk_text_buffer_get_insert (buf));

    /* revalidate iters */
    gtk_text_buffer_get_iter_at_mark (buf, start, start_mark);
    gtk_text_buffer_get_iter_at_mark (buf, end, end_mark);

    /* adjust selection to cover entire first line */
    gtk_text_buffer_select_range (buf, end, start);

    /* delete GtkTextMark's */
    gtk_text_buffer_delete_mark (buf, start_mark);
    gtk_text_buffer_delete_mark (buf, end_mark);
}

/** unindent current/selected lines to the preceding softtab.
 *  text will be aligned to the prior softtab on unindent
 *  regardless of the number of spaces beyond the previous
 *  softtab text actually begins.
 */
void buffer_unindent_lines (gpointer data, GtkTextIter *start, GtkTextIter *end)
{
    mainwin_t *app = data;
    einst_t *einst = app->einst[app->focused];
    kinst_t *inst = einst->inst;

    GtkTextView *view = GTK_TEXT_VIEW(einst->view);
    GtkTextBuffer *buf = GTK_TEXT_BUFFER(inst->buf);
    GtkTextMark *start_mark, *end_mark;

    gint i, indent_width, start_line, end_line;
    gboolean bracket_hl;

    bracket_hl = gtk_source_buffer_get_highlight_matching_brackets (GTK_SOURCE_BUFFER (buf));
    gtk_source_buffer_set_highlight_matching_brackets (GTK_SOURCE_BUFFER (buf), FALSE);

    start_mark = gtk_text_buffer_create_mark (buf, NULL, start, FALSE);
    end_mark = gtk_text_buffer_create_mark (buf, NULL, end, FALSE);

    start_line = gtk_text_iter_get_line (start);
    end_line = gtk_text_iter_get_line (end);

    if ((gtk_text_iter_get_visible_line_offset (end) == 0) &&
        (end_line > start_line)) {
            end_line--;
    }

    indent_width = app->softtab;

    gtk_text_buffer_begin_user_action (buf);

    for (i = start_line; i <= end_line; i++) {

        GtkTextIter iter_start, iter;
        gint offset = 0, ndelete = 0;
        gboolean hastabs = FALSE;       /* special handling for tab chars */

        gtk_text_buffer_get_iter_at_line (buf, &iter_start, i);

        iter = iter_start;

        while (!gtk_text_iter_ends_line (&iter))
        {
            gunichar c;
            c = gtk_text_iter_get_char (&iter);

            if (c == '\t') {
                offset += indent_width;
                hastabs = TRUE;
            }
            else if (c == ' ')
                offset += 1;
            else
                break;

            gtk_text_iter_forward_char (&iter);
        }

        ndelete = offset % indent_width;

        /* tabs - del leading ws & insert correct spaces for unindent */
        if (hastabs) {
            char *ws = g_strdup_printf ("%*s", offset - ndelete, " ");
            if (!ws) {      /* handle error */
                g_print ("buffer_unindent_lines() error: g_strdup_printf.\n");
            }

            gtk_text_buffer_delete (buf, &iter_start, &iter);   /* del whitespace/tabs */
            gtk_text_buffer_get_iter_at_line (buf, &iter, i);   /* revalidate at start */
            gtk_text_buffer_insert (buf, &iter, ws, -1);        /* ins correct indent  */

            if (ws)
                g_free (ws);
        }
        else {  /* no tabs - all spaces */
            if (offset <= indent_width) {   /* within first softtab */
                gtk_text_buffer_delete (buf, &iter_start, &iter);
            }
            else if (!ndelete) {    /* backup full softtab width */
                gtk_text_iter_set_line_offset (&iter_start, offset - indent_width);
                gtk_text_buffer_delete (buf, &iter_start, &iter);
            }
            else {  /* bring indention to nex prior softtab */
                gtk_text_iter_set_line_offset (&iter_start, offset - ndelete);
                gtk_text_buffer_delete (buf, &iter_start, &iter);
            }
        }
    }

    gtk_text_buffer_end_user_action (buf);

    gtk_source_buffer_set_highlight_matching_brackets (GTK_SOURCE_BUFFER (buf),
                                                        bracket_hl);

    gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW (view),
                                        gtk_text_buffer_get_insert (buf));

    /* revalidate iters */
    gtk_text_buffer_get_iter_at_mark (buf, start, start_mark);
    gtk_text_buffer_get_iter_at_mark (buf, end, end_mark);

    gtk_text_buffer_delete_mark (buf, start_mark);
    gtk_text_buffer_delete_mark (buf, end_mark);
}

/** indent current/selected lines by softtab spaces.
 *  text will be indented by softtab spaces on indent
 *  preserving the existing number of space in line.
 */
void buffer_indent_lines_fixed (gpointer data, GtkTextIter *start, GtkTextIter *end)
{
    mainwin_t *app = data;
    einst_t *einst = app->einst[app->focused];
    kinst_t *inst = einst->inst;

    GtkTextView *view = GTK_TEXT_VIEW(einst->view);
    GtkTextBuffer *buf = GTK_TEXT_BUFFER(inst->buf);
    GtkTextMark *start_mark, *end_mark;

    gint i, start_line, end_line;
    gchar *ind_buffer = NULL;

    gboolean bracket_hl;
    bracket_hl = gtk_source_buffer_get_highlight_matching_brackets (GTK_SOURCE_BUFFER (buf));
    gtk_source_buffer_set_highlight_matching_brackets (GTK_SOURCE_BUFFER (buf), FALSE);

    start_mark = gtk_text_buffer_create_mark (buf, NULL, start, FALSE);
    end_mark = gtk_text_buffer_create_mark (buf, NULL, end, FALSE);

    start_line = gtk_text_iter_get_line (start);
    end_line = gtk_text_iter_get_line (end);

    if ((gtk_text_iter_get_visible_line_offset (end) == 0) &&
        (end_line > start_line)) {
            end_line--;
    }

    gtk_text_buffer_begin_user_action (buf);

    for (i = start_line; i <= end_line; i++) {

        GtkTextIter iter;

        gtk_text_buffer_get_iter_at_line (buf, &iter, i);

        ind_buffer = g_strdup_printf ("%*s", app->softtab, " ");

        gtk_text_buffer_insert (buf, &iter, ind_buffer, -1);

        g_free (ind_buffer);

        /* reset start iter and start mark to beginning of line
         * so that selection continues to encompass entire first line.
         */
        if (start_line != end_line && i == start_line &&
            !gtk_text_iter_starts_line (&iter)) {
            gtk_text_iter_set_line_offset (&iter, 0);
            gtk_text_buffer_move_mark (buf, start_mark, &iter);
        }
    }

    gtk_text_buffer_end_user_action (buf);

    gtk_source_buffer_set_highlight_matching_brackets (GTK_SOURCE_BUFFER (buf),
                                                        bracket_hl);

    gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW (view),
                                        gtk_text_buffer_get_insert (buf));

    /* revalidate iters */
    gtk_text_buffer_get_iter_at_mark (buf, start, start_mark);
    gtk_text_buffer_get_iter_at_mark (buf, end, end_mark);

    /* adjust selection to cover entire first line */
    gtk_text_buffer_select_range (buf, end, start);

    gtk_text_buffer_delete_mark (buf, start_mark);
    gtk_text_buffer_delete_mark (buf, end_mark);
}

/** Unindent current/selected lines by softtab spaces.
 *  text will be unindented by softtab spaces on unindent
 *  preserving the existing number of space in line.
 */
void buffer_unindent_lines_fixed (gpointer data, GtkTextIter *start, GtkTextIter *end)
{
    mainwin_t *app = data;
    einst_t *einst = app->einst[app->focused];
    kinst_t *inst = einst->inst;

    GtkTextView *view = GTK_TEXT_VIEW(einst->view);
    GtkTextBuffer *buf = GTK_TEXT_BUFFER(inst->buf);
    GtkTextMark *start_mark, *end_mark;

    gint i, start_line, end_line, nspaces;
    gboolean bracket_hl;

    bracket_hl = gtk_source_buffer_get_highlight_matching_brackets (GTK_SOURCE_BUFFER (buf));
    gtk_source_buffer_set_highlight_matching_brackets (GTK_SOURCE_BUFFER (buf), FALSE);

    start_mark = gtk_text_buffer_create_mark (buf, NULL, start, FALSE);
    end_mark = gtk_text_buffer_create_mark (buf, NULL, end, FALSE);

    start_line = gtk_text_iter_get_line (start);
    end_line = gtk_text_iter_get_line (end);

    if ((gtk_text_iter_get_visible_line_offset (end) == 0) &&
        (end_line > start_line)) {
            end_line--;
    }

    gtk_text_buffer_begin_user_action (buf);

    for (i = start_line; i <= end_line; i++) {

        GtkTextIter iter_start, iter;
        gint offset = 0;

        gtk_text_buffer_get_iter_at_line (buf, &iter, i);
        iter_start = iter;

        /* iterate past tabs computing character equivalent */
        for (;;) {
            gunichar c;
            c = gtk_text_iter_get_char (&iter);

            if (c == '\t' || c == ' ')
                offset += (c == '\t') ? app->softtab : 1;
            else
                break;

            gtk_text_iter_forward_char (&iter);
        }

        nspaces = offset >= app->softtab ? app->softtab : offset;

        /* set iter at nspaces and delete between iter_start - iter */
        gtk_text_iter_set_line_offset (&iter, nspaces);
        gtk_text_buffer_delete (buf, &iter_start, &iter);
    }

    gtk_text_buffer_end_user_action (buf);

    gtk_source_buffer_set_highlight_matching_brackets (GTK_SOURCE_BUFFER (buf),
                                                        bracket_hl);

    gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW (view),
                                        gtk_text_buffer_get_insert (buf));

    /* revalidate iters */
    gtk_text_buffer_get_iter_at_mark (buf, start, start_mark);
    gtk_text_buffer_get_iter_at_mark (buf, end, end_mark);

    gtk_text_buffer_delete_mark (buf, start_mark);
    gtk_text_buffer_delete_mark (buf, end_mark);
}

/** comment line based on comment syntax determined from SourceView language.
 *  start and end iterators indicating the start and end of the block (or single
 *  line) to be commented. Block comments are used if text is selected,
 *  otherwise single-line comments are used to comment entire line.
 *  TODO:
 *    provide setting to prefer single/multi-line comemnt, and update settings
 *    dialog to display current document comment syntax strings.
 */
void buffer_comment_lines (gpointer data, GtkTextIter *start, GtkTextIter *end)
{
    mainwin_t *app = data;
    einst_t *einst = app->einst[app->focused];
    kinst_t *inst = einst->inst;

    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(inst->buf);
    GtkTextMark *start_mark, *end_mark;

    gint i, start_line, end_line;
    const gchar *commentstr = app->comment;  /* set comment from settings */
    gboolean bracket_hl;
    gint start_pos, end_pos;

    if (inst->comment_single)               /* if comment based on lang_id */
        commentstr = inst->comment_single;  /* update commentstr from lang_id */

    bracket_hl = gtk_source_buffer_get_highlight_matching_brackets (GTK_SOURCE_BUFFER (buffer));
    gtk_source_buffer_set_highlight_matching_brackets (GTK_SOURCE_BUFFER (buffer), FALSE);

    /* preserve marks for start/end to revalidate iterators before return
     * using left gravity on start to mark stays left of inserted text.
     */
    start_mark = gtk_text_buffer_create_mark (buffer, NULL, start, TRUE);
    end_mark   = gtk_text_buffer_create_mark (buffer, NULL, end, FALSE);

    /* get lines indicated by start/end iters passed based on selection */
    start_line = gtk_text_iter_get_line (start);
    end_line   = gtk_text_iter_get_line (end);

    gtk_text_buffer_begin_user_action (buffer); /* begin user action */

    /* get position within line for start/end iters */
    start_pos = gtk_text_iter_get_line_offset (start);
    end_pos   = gtk_text_iter_get_line_offset (end);

    /* insert multi-line block comments */
    if (inst->comment_blk_beg && inst->comment_blk_end &&
        !app->cmtusesingle &&
        (end_line > start_line || end_pos > start_pos)) {

        /* insert beginning comment for block */
        gtk_text_buffer_insert (buffer, start, inst->comment_blk_beg, -1);

        /* revalidate end iter from saved end_mark */
        gtk_text_buffer_get_iter_at_mark (buffer, end, end_mark);
        /* insert ending comment for block */
        gtk_text_buffer_insert (buffer, end, inst->comment_blk_end, -1);
    }
    else {  /* insert single-line comments */
        GtkTextIter iter;
        /* check if end is at beginning of multi-line - subtract 1-line */
        if ((gtk_text_iter_get_visible_line_offset (end) == 0) &&
            (end_line > start_line)) {
                end_line--;
        }
        /* loop over each line inserting comment at beginning */
        for (i = start_line; i <= end_line; i++) {
            gtk_text_buffer_get_iter_at_line (buffer, &iter, i);
            gtk_text_buffer_insert (buffer, &iter, commentstr, -1);
        }
    }

    gtk_source_buffer_set_highlight_matching_brackets (GTK_SOURCE_BUFFER (buffer),
                                                        bracket_hl);

    gtk_text_buffer_end_user_action (buffer);   /* end user action */

    /* revalidate iters */
    gtk_text_buffer_get_iter_at_mark (buffer, start, start_mark);
    gtk_text_buffer_get_iter_at_mark (buffer, end, end_mark);

    /* extend selection to cover new comment syntax (for uncomment) */
    gtk_text_buffer_select_range (buffer, end, start);

    gtk_text_buffer_delete_mark (buffer, start_mark);
    gtk_text_buffer_delete_mark (buffer, end_mark);
}

/** TODO add to selection bounds at end of block at the end so block comments
 *  can be enabled and removed without having to reselect the additional end
 *  comment in range.
 */
void buffer_uncomment_lines (gpointer data, GtkTextIter *start, GtkTextIter *end)
{
    mainwin_t *app = data;
    einst_t *einst = app->einst[app->focused];
    kinst_t *inst = einst->inst;

    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(inst->buf);
    GtkTextMark *start_mark, *end_mark;
    GtkTextIter iter = *start;

    gint i, start_line, end_line;
    const gchar *commentstr = app->comment;  /* set comment from settings */

    gboolean blockcmt = FALSE;
    gboolean bracket_hl;

    if (inst->comment_single)               /* if comment based on lang_id */
        commentstr = inst->comment_single;  /* update commentstr from lang_id */
    bracket_hl = gtk_source_buffer_get_highlight_matching_brackets (GTK_SOURCE_BUFFER (buffer));
    gtk_source_buffer_set_highlight_matching_brackets (GTK_SOURCE_BUFFER (buffer), FALSE);

    /* preserve marks for start/end to revalidate iterators before return */
    start_mark = gtk_text_buffer_create_mark (buffer, NULL, start, FALSE);
    end_mark = gtk_text_buffer_create_mark (buffer, NULL, end, FALSE);

    /* get lines indicated by start/end iters passed based on selection */
    start_line = gtk_text_iter_get_line (start);
    end_line = gtk_text_iter_get_line (end);

    /* check if end is at beginning of multi-line - subtract 1-line */
    if ((gtk_text_iter_get_visible_line_offset (end) == 0) &&
        (end_line > start_line)) {
            end_line--;
    }

    gtk_text_buffer_begin_user_action (buffer); /* begin user action */

    /* if block comment strings are set */
    if (inst->comment_blk_beg && inst->comment_blk_end) {

        const gchar *p = inst->comment_blk_beg;
        gunichar c = gtk_text_iter_get_char (&iter);

        while (*p && c == (gunichar)*p) {   /* compare with chars in buffer */
            if (!gtk_text_iter_forward_char (&iter))
                break;

            c = gtk_text_iter_get_char (&iter);
            p++;
        }
        if (*p == '\0') /* all chars matched to nul-character, block comment */
            blockcmt = TRUE;
    }

    if (blockcmt) { /* handle removal of block comments, which can be sin same line */

        const gchar *p = inst->comment_blk_end; /* pointer to opening string */

        gunichar c;

        /* iter and start properly set from above block, just delete */
        gtk_text_buffer_delete (buffer, start, &iter);
        /* revalidate end iterator */
        gtk_text_buffer_get_iter_at_mark (buffer, end, end_mark);
        iter = *end;    /* set iter to end */

        while (*p)  /* move to end char of comment_blk_end */
            p++;
        p--;

        /* backup to last char before end iter */
        if (!gtk_text_iter_backward_char (&iter)) {
            /* handle error */
            goto unmatchedblk;
        }
        c = gtk_text_iter_get_char (&iter); /* get last char before end */

        /* iterate backwards until the beginning char in comment found */
        while (p > inst->comment_blk_end && (gunichar)*p == c) {
            if (!gtk_text_iter_backward_char (&iter)) {
                /* handle error */
                goto unmatchedblk;
            }
            c = gtk_text_iter_get_char (&iter);
            p--;
        }
        if (p == inst->comment_blk_end)  /* all chars matched as comment */
            gtk_text_buffer_delete (buffer, &iter, end);    /* delete comment */
        else {
            /* handle error */
            goto unmatchedblk;
        }
    }
    else {  /* remove single-line comment from all lines in selection */
        for (i = start_line; i <= end_line; i++) {

            GtkTextIter iterstart;
            gunichar c;
            gchar *p = (gchar *)commentstr;

            gtk_text_buffer_get_iter_at_line (buffer, &iterstart, i);
            iter = iterstart;

            c = gtk_text_iter_get_char (&iter);

            /* move to start of comment string (displaced comment) */
            while (c != (gunichar)*p && (c == ' ' || c == '\t')) {

                if (!gtk_text_iter_forward_char (&iter))
                    goto nxtuncomment;

                c = gtk_text_iter_get_char (&iter);
            }
            iterstart = iter;   /* set start iter */

            for (; *p; p++) {   /* set characters to delete */

                if ((gunichar)*p != gtk_text_iter_get_char (&iter) ||
                        !gtk_text_iter_forward_char (&iter))
                    break;
            }

            /* delete comment */
            gtk_text_buffer_delete (buffer, &iterstart, &iter);

            nxtuncomment:;
        }
    }
    unmatchedblk:;

    gtk_source_buffer_set_highlight_matching_brackets (GTK_SOURCE_BUFFER (buffer),
                                                        bracket_hl);

    gtk_text_buffer_end_user_action (buffer);   /* end user action */

    /* revalidate iters */
    gtk_text_buffer_get_iter_at_mark (buffer, start, start_mark);
    gtk_text_buffer_get_iter_at_mark (buffer, end, end_mark);

    gtk_text_buffer_delete_mark (buffer, start_mark);
    gtk_text_buffer_delete_mark (buffer, end_mark);
}

/** helper function for insert_eol and indent_auto actions after "mark-set".
 *  needed because "mark-set" fires before indent and eol inserted causing
 *  app->{line,col,lines} not to update statusbar on [Enter]
 */
void buffer_update_pos (gpointer data)
{
    /* Fri Jul 13 2018 18:52:36 CDT
     * moved line/col update to status_set_default to avoid
     * mark_set firing when gtk_text_view_scroll_mark_onscreen
     * is called, causing line number to reset to 1 or jump by
     * the number of lines scrolled in forward direction.
     */
    mainwin_t *app = data;
    einst_t *einst = app->einst[app->focused];
    kinst_t *inst = einst->inst;

    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(inst->buf);
    GtkTextMark *ins = gtk_text_buffer_get_insert (buffer);

    /* scroll text-view to keep cursor position in window */
    gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW(einst->view), ins);
}

/** auto-indent on return */
gboolean buffer_indent_auto (gpointer data)
{
    mainwin_t *app = data;
    einst_t *einst = app->einst[app->focused];
    kinst_t *inst = einst->inst;

    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(inst->buf);
    GtkTextMark *ins;
    GtkTextIter end, iter;
    gchar *indstr = NULL;
    gint line, nspaces = 0;

    ins = gtk_text_buffer_get_insert (buffer);
    gtk_text_buffer_get_iter_at_mark (buffer, &end, ins);
    line = gtk_text_iter_get_line (&end);
    gtk_text_buffer_get_iter_at_line (buffer, &iter, line);

    for (;;) {
        gunichar c;
        c = gtk_text_iter_get_char (&iter);

        if (c == '\t' || c == ' ')
            nspaces += (c == '\t') ? app->softtab : 1;
        else
            break;

        gtk_text_iter_forward_char (&iter);
    }

    // app->indentpl = nspaces;    /* set previous line indent */

    if (nspaces) {
        // indstr = g_strdup_printf ("\n%*s", nspaces, " ");
        indstr = g_strdup_printf ("%s%*s", app->eolstr[app->eol], nspaces, " ");
        gtk_text_buffer_insert_at_cursor (buffer, indstr, -1);
        g_free (indstr);

        /* update 'line/lines col' on statusbar */
        buffer_update_pos (data);

        return TRUE;
    }
    else
        return buffer_insert_eol (app);

    return FALSE;
}

/** remove all whitespace to prior softtab stop on backspace.
 *  this function will only remove 'spaces', all other backspace
 *  is handled by the default keyboard handler.
 */
gboolean smart_backspace (gpointer data)
{
    mainwin_t *app = data;
    einst_t *einst = app->einst[app->focused];
    kinst_t *inst = einst->inst;

    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(inst->buf);
    GtkTextMark *cur;
    GtkTextIter beg, end, iter, iter2;

    gunichar c;
    gint line = 0, col = 0, cheq = 0, ndel = 0;

    /* validate no selection exists */
    if (gtk_text_buffer_get_selection_bounds (buffer, &beg, &end))
        return FALSE;

    /* get "insert" mark, then current line/column, set end iter */
    cur = gtk_text_buffer_get_insert (buffer);
    gtk_text_buffer_get_iter_at_mark (buffer, &end, cur);
    line = gtk_text_iter_get_line (&end);
    col = gtk_text_iter_get_visible_line_offset (&end);

    if (!col)   /* already at first char */
        return FALSE;

    /* initialize iterators, set 'iter' 1-char before insert */
    gtk_text_buffer_get_iter_at_line (buffer, &beg, line);
    iter = iter2 = beg;
    gtk_text_iter_set_visible_line_offset (&iter, col - 1);

    /* if last char not ' ', return FALSE for default handling. */
    if ((c = gtk_text_iter_get_char (&iter)) != ' ')
        return FALSE;

    /* iter forward from beg to end and determine char equivalent
     * number of chars in line then set number of chars to delete
     * to next softtab stop. 'c' and 'ndel' are in *addition to*
     * the ' ' above. (they will always be total-1 chars)
     */
    while (gtk_text_iter_forward_char (&iter2) &&
            !gtk_text_iter_equal (&iter2, &end)) {
        c = gtk_text_iter_get_char (&iter2);
        if (c != ' ' && c != '\t')
            return FALSE;
        cheq += (c == '\t') ? app->softtab : 1;
    }
    ndel = cheq % app->softtab; /* chars from current 'iter' pos to del */

    /* backup iter at most ndel spaces, setting col flag */
    while (ndel-- && gtk_text_iter_backward_char (&iter) &&
            (c = gtk_text_iter_get_char (&iter)) == ' ') {
        col++;
    }

    if (col) {  /* if col, user_action to delete chars */

        gtk_text_buffer_begin_user_action (buffer);

        /* delete characters to prior tab stop from buffer */
        gtk_text_buffer_delete (buffer, &iter, &end);

        /* update line/col with current "insert" mark */
        gtk_text_buffer_get_iter_at_mark (buffer, &iter, cur);
        inst->line = gtk_text_iter_get_line (&iter);
        inst->col = gtk_text_iter_get_visible_line_offset (&iter);

        gtk_text_buffer_end_user_action (GTK_TEXT_BUFFER(buffer));

        status_set_default (data);

        return TRUE;    /* return without further handling */
    }

    return FALSE;   /* return FALSE for default handling */
}

gboolean smart_tab (gpointer data)
{
    mainwin_t *app = data;
    einst_t *einst = app->einst[app->focused];
    kinst_t *inst = einst->inst;

    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(inst->buf);
    GtkTextMark *ins;
    GtkTextIter beg, end, iter;

    gchar *tabstr = NULL;
    gint nspaces = 0, col;  //, line;

    if (!app->expandtab)    /* default handler */
        return FALSE;

    /* validate no selection exists */
    if (gtk_text_buffer_get_selection_bounds (buffer, &beg, &end))
        return FALSE;

    /* get "insert" mark, then current line/column, set end iter */
    ins = gtk_text_buffer_get_insert (buffer);
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, ins);
    col = gtk_text_iter_get_visible_line_offset (&iter);

    /* TODO: iterate to beginning to insert to check if all leading
     * whitespace, if so, if app->indontab, then call indent function
     * for line and return TRUE
     */

    gtk_text_buffer_begin_user_action (buffer);

    nspaces = app->softtab - col % app->softtab;
    tabstr = g_strdup_printf ("%*s", nspaces, " ");
    gtk_text_buffer_insert (buffer, &iter, tabstr, -1);

    gtk_text_buffer_end_user_action (buffer);

    gtk_text_buffer_get_iter_at_mark (buffer, &iter, ins);
//     app->line = gtk_text_iter_get_line (&iter);
//     app->col = gtk_text_iter_get_visible_line_offset (&iter);

    g_free (tabstr);

    return TRUE;
}

/** move cursor to beginning text on first keypress, line start on next.
 *  sequential GDK_KEY_Home keypress events are stored in app->kphome.
 *  if app->kphome is not set, move to text start, else move to start
 *  of line.
 */
gboolean smart_home (gpointer data)
{
#ifdef SMARTHEDEBUG
    g_print ("on smart_home entry, app->kphome: %s, app->smarthe: %s\n",
            app->kphome ? "TRUE" : "FALSE", app->smarthe ? "TRUE" : "FALSE");
#endif
    mainwin_t *app = data;
    einst_t *einst = app->einst[app->focused];
    kinst_t *inst = einst->inst;

    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(inst->buf);
    GtkTextMark *ins;
    GtkTextIter start, insiter, iter;
    gunichar c = 0;

    if (app->kphome)
        return ((app->kphome = FALSE));

    /* get "insert" mark, then iter at beginning of line */
    ins = gtk_text_buffer_get_insert (buffer);
    gtk_text_buffer_get_iter_at_mark (buffer, &insiter, ins);
    iter = insiter;
    gtk_text_iter_set_line_offset (&iter, 0);
    start = iter;

    /* iter forward to first non-whitespace or end */
    while (!gtk_text_iter_ends_line (&iter))
    {
        c = gtk_text_iter_get_char (&iter);

        if (c != '\t' && c != ' ' && c != 0xFFFC)
            break;

        gtk_text_iter_forward_char (&iter);
    }

    /* place cursor */
    if (c == ' ' || c == '\t')
        gtk_text_buffer_place_cursor (buffer, &start);
    else
        gtk_text_buffer_place_cursor (buffer, &iter);

    return ((app->kphome = TRUE));
}

/** remove all trailing whitespace from buffer */
void buffer_remove_trailing_ws (gpointer data)
{
    mainwin_t *app = data;
    einst_t *einst = app->einst[app->focused];
    kinst_t *inst = einst->inst;

    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(inst->buf);
    GtkTextIter iter, iter_from, iter_end;
    gunichar c;
    gint line_end;

    if (!buffer) {
        err_dialog ("Error: Invalid 'buffer' passed to function\n"
                    "buffer_remove_trailing_ws (GtkTextBuffer *buffer)");
        return;
    }

    /* get iter at start of buffer */
    gtk_text_buffer_get_start_iter (buffer, &iter);
    gtk_text_buffer_get_end_iter (buffer, &iter_end);

    line_end =  gtk_text_iter_get_line (&iter_end);

    while (gtk_text_iter_forward_to_line_end (&iter)) {

        gint line;
        iter_from = iter_end = iter;

        /* iterate over all trailing whitespace */
        while (gtk_text_iter_backward_char (&iter)) {

            c = gtk_text_iter_get_char (&iter);

            if ((c == ' ' || c == '\t') && c != 0xFFFC)
                iter_from = iter;
            else
                break;
        }

        /* save line to re-validate iter after delete */
        line = gtk_text_iter_get_line (&iter);

        /* remove trailing whitespace up to newline or end */
        if (!gtk_text_iter_equal (&iter_from, &iter_end))
            gtk_text_buffer_delete (buffer, &iter_from, &iter_end);

        /* re-validate iter */
        if (line == line_end) {
            gtk_text_buffer_get_iter_at_line (buffer, &iter, line);
            gtk_text_iter_forward_to_line_end (&iter);
            break;
        }
        gtk_text_buffer_get_iter_at_line (buffer, &iter, line + 1);
    }

    /* handle last line with trailing whitespace */
    if (!(c = gtk_text_iter_get_char (&iter))) {

        iter_from = iter_end = iter;

        while (gtk_text_iter_backward_char (&iter)) {

            c = gtk_text_iter_get_char (&iter);

            if ((c == ' ' || c == '\t') && c != 0xFFFC)
                iter_from = iter;
            else
                break;
        }

        /* remove trailing whitespace up to newline or end */
        if (!gtk_text_iter_equal (&iter_from, &iter_end))
            gtk_text_buffer_delete (buffer, &iter_from, &iter_end);
    }
}

void buffer_require_posix_eof (gpointer data)
{
    mainwin_t *app = data;
    einst_t *einst = app->einst[app->focused];
    kinst_t *inst = einst->inst;

    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(inst->buf);
    GtkTextIter end;

    gtk_text_buffer_get_end_iter (buffer, &end);

    if (gtk_text_iter_backward_char (&end)) {
        gunichar c = gtk_text_iter_get_char (&end);
        if (c != '\n' && c != '\r') {
            gtk_text_iter_forward_char (&end);
            gtk_text_buffer_insert (buffer, &end, app->eolstr[app->eol], -1);
        }
    }
}

/** insert configured EOL at cursor position on Return/Enter. */
gboolean buffer_insert_eol (gpointer data)
{
    mainwin_t *app = data;
    einst_t *einst = app->einst[app->focused];
    kinst_t *inst = einst->inst;

    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(inst->buf);

    /* validate eolstr[x] not NULL and not empty */
    if (!app->eolstr[app->eol] || !*app->eolstr[app->eol])
        return FALSE;   /* fallback to default keystroke handler */

    /* insert EOL at cursor */
    gtk_text_buffer_insert_at_cursor (buffer, app->eolstr[app->eol], -1);

    /* update 'line/lines col' on statusbar */
    buffer_update_pos (data);

    return TRUE;    /* keypress handled */
}

/** callback handling eol_chk_default infobar */
void ib_eol_chk_default (GtkInfoBar *bar, gint response_id, gpointer data)
{
    mainwin_t *app = data;
    einst_t *einst = app->einst[app->focused];

    switch (response_id) {
        case GTK_RESPONSE_NO:       /* take no action */
            gtk_widget_hide (GTK_WIDGET(bar));
            break;
        case GTK_RESPONSE_YES:      /* convert eol */
            gtk_widget_hide (GTK_WIDGET(bar));
            if (app->eoldefault < FILE_EOL) {   /* default is LF, CRLF, or CR */
                if (app->eoldefault == LF)
                    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (app->eolLFMi),
                                                    TRUE);
                else if (app->eoldefault == CRLF)
                    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (app->eolCRLFMi),
                                                    TRUE);
                else if (app->eoldefault == CR)
                    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (app->eolCRMi),
                                                    TRUE);
                app->eol = app->eoldefault;
                app->oeol = app->eol;
            }
            else {  /* default is OS_EOL */
                if (app->eolos == LF)
                    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (app->eolLFMi),
                                                    TRUE);
                else if (app->eolos == CRLF)
                    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (app->eolCRLFMi),
                                                    TRUE);
                else
                    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (app->eolCRMi),
                                                    TRUE);
                app->eol = app->eolos;
                app->oeol = app->eol;
            }
            break;
    }

    /* set text_view sensitive TRUE */
    if (!gtk_widget_get_sensitive (einst->view))
        gtk_widget_set_sensitive (einst->view, TRUE);

    /* grab focus for textview */
    gtk_widget_grab_focus (einst->view);

    /* reset flags */
    einst->ibflags = 0;
}

/** EOL check against default setting on file open to warn of mismatch (infobar) */
void ibar_eol_chk_default (gpointer data)
{
    mainwin_t *app = data;
    einst_t *einst = app->einst[app->focused];

    ibbtndef btndef[] = { { .btntext = "_Yes",    .response_id = GTK_RESPONSE_YES },
                          { .btntext = "_No",     .response_id = GTK_RESPONSE_NO },
                          { .btntext = "",        .response_id = 0 } };

    /* make text_view insensitive on display */
    einst->ibflags = IBAR_VIEW_SENSITIVE;

    /* check against eoldefault, and if eoldefault != FILE_EOL or if
     * it differes from OS_EOL, warn of mismatch.
     */
    if (app->eoldefault != FILE_EOL && app->eol != app->eoldefault) {
        if (app->eoldefault != OS_EOL || app->eol != app->eolos) {
            gchar *msg = g_markup_printf_escaped ("File contains <span font_weight=\"bold\">"
                "'%s'</span> line ends.\n"
                "Selected default line end is: <span font_weight=\"bold\">'%s'</span>\n"
                "Operating-System default is: <span font_weight=\"bold\">'%s'</span>\n\n"
                "<span font_weight=\"bold\">Tools->End-of-Line Selection</span> -- "
                "to convert between line ends at any time.\n"
                "<span font_weight=\"bold\">Settings->File Load/Save->End-of-Line "
                "Handling</span> -- to change default setting.\n\n"
                "<span font_weight=\"bold\">Convert File to '%s' line ends?</span>",
                app->eolnm[app->eol], app->eoltxt[app->eoldefault],
                app->eolnm[app->eolos], app->eolnm[app->eolos]);
//             app->ibflags |= IBAR_VISIBLE;
            show_info_bar_choice (msg, GTK_MESSAGE_WARNING, btndef,
                                    ib_eol_chk_default, app);
            g_free (msg);
        }
    }
}

/** determine current EOL by scanning buffer content */
void buffer_get_eol (gpointer data)
{
    mainwin_t *app = data;
    einst_t *einst = app->einst[app->focused];
    kinst_t *inst = einst->inst;

    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(inst->buf);
    gint cnt = 0;

    /* check buffer character count */
    if (gtk_text_buffer_get_char_count (buffer) < 1)
        return;

    /* save current position */
    GtkTextMark *ins = gtk_text_buffer_get_insert (buffer);
    GtkTextIter iter;

    /* get start iterator in buffer */
    gtk_text_buffer_get_start_iter (buffer, &iter);

    /* disable text view and during EOL detection */
    gtk_widget_set_sensitive (einst->view, FALSE);

    while (gtk_text_iter_forward_to_line_end (&iter)) {
        gunichar c = gtk_text_iter_get_char (&iter);

        if (c == '\n') {                /* do we have a linefeed */
            if (cnt && app->eol == LF)  /* if second - confirmed */
                break;
            else {
                app->eol = LF;          /* otherwise, set, check again */
                cnt = 1;
            }
        }
        else if (c == '\r') {           /* do we have a carriage-return */
            if (!gtk_text_iter_forward_char (&iter)) {  /* advance */
                app->eol = CR;
                break;
            }
            c = gtk_text_iter_get_char (&iter); /* get next char */
            if (c == '\n') {
                if (cnt && app->eol == CRLF)    /* we have CRLF */
                    break;
                else {
                    app->eol = CRLF;    /* set CRLF, check again */
                    cnt = 1;
                }
            }
            else if (cnt && app->eol == CR) {   /* we have a CR */
                break;
            }
            else {
                app->eol = CR;          /* set CR, check again */
                cnt = 1;
            }
        }
    }
#ifdef DEBUGEOL
    g_print ("buffer_get_eol() - before menu_item app->eol: '%s' (orig: '%s')\n",
            app->eolnm[app->eol], app->eolnm[app->oeol]);
#endif

    /* update tools menu active EOL radio button
     * (do not set app->eolchg before to prevent
     *  firing on gtk_check_menu_item_set_active)
     */
    if (app->eol == LF)
        gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (app->eolLFMi), TRUE);
    else if (app->eol == CRLF)
        gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (app->eolCRLFMi), TRUE);
    else
        gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (app->eolCRMi), TRUE);

    if (app->eol != app->oeol) {
        app->oeol = app->eol;   /* set orignal to current */
    }
    app->eolchg = TRUE;         /* eol changed */

    /* restore original insert position */
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, ins);
    gtk_text_buffer_place_cursor (buffer, &iter);

    /* enable text view and after EOL detection */
    gtk_widget_set_sensitive (einst->view, TRUE);

}

/** convert all end-of-line in file to selected app->eol.
 *  traverse buffer from start to end converting all end-of-line
 *  terminating characters to the user selected app->eol when
 *  a change in the end-of-line setting occurs. (or on save)
 */
void buffer_convert_eol (gpointer data)
{
    mainwin_t *app = data;
    einst_t *einst = app->einst[app->focused];
    kinst_t *inst = einst->inst;

    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(inst->buf);
    GtkTextIter iter;
    gboolean modified;

    modified = gtk_text_buffer_get_modified (GTK_TEXT_BUFFER(inst->buf));

    /* if no change -- return */
    if (app->eol == app->oeol || (!app->eolchg && !modified))
        return;

    if (!buffer) {  /* validate buffer */
        err_dialog ("Error: Invalid 'buffer' passed to function\n"
                    "buffer_remove_trailing_ws (GtkTextBuffer *buffer)");
        return;
    }

    /* get iter at start of buffer */
    gtk_text_buffer_get_start_iter (buffer, &iter);

    if (app->last_pos) {    /* delete app->last_pos mark, if set */
        gtk_text_buffer_delete_mark (buffer, app->last_pos);
        app->last_pos = NULL;
    }

    /* set app->last_pos Mark to start, and move on each iteration */
    app->last_pos = gtk_text_buffer_create_mark (buffer, "last_pos",
                                                &iter, FALSE);

    /* loop, moving to the end of each line, before the EOL chars */
    while (gtk_text_iter_forward_to_line_end (&iter)) {

        gunichar c = gtk_text_iter_get_char (&iter);
        gtk_text_buffer_move_mark (buffer, app->last_pos, &iter);

        if (c == '\n') {        /* if end-of-line begins with LF */
            gtk_text_iter_forward_char (&iter);
            gtk_text_buffer_backspace (buffer, &iter, FALSE, TRUE);
            gtk_text_buffer_get_iter_at_mark (buffer, &iter, app->last_pos);

            if (app->eol == CRLF)       /* handle change to CRLF */
                gtk_text_buffer_insert (buffer, &iter, app->eolstr[CRLF], -1);
            else if (app->eol == CR)    /* handle change to CR */
                gtk_text_buffer_insert (buffer, &iter, app->eolstr[CR], -1);
        }
        else if (c == '\r') {   /* if end-of-line begins with CR */
            if (app->eol == LF) {   /* handle change to LF */
                gtk_text_iter_forward_char (&iter);
                if (gtk_text_buffer_backspace (buffer, &iter, FALSE, TRUE)) {
                    gtk_text_buffer_get_iter_at_mark (buffer, &iter, app->last_pos);
                    if (gtk_text_iter_get_char (&iter) != '\n') {
                        gtk_text_buffer_insert (buffer, &iter, app->eolstr[LF], -1);
                    }
                }
            }
            else if (app->eol == CRLF) {    /* handle change to CRLF */
                gtk_text_iter_forward_char (&iter);
                if (gtk_text_iter_get_char (&iter) != '\n') {
                    if (gtk_text_buffer_backspace (buffer, &iter, FALSE, TRUE)) {
                        gtk_text_buffer_get_iter_at_mark (buffer, &iter, app->last_pos);
                        gtk_text_buffer_insert (buffer, &iter, app->eolstr[CRLF], -1);
                    }
                    /* CRLF a single char and can't be created? */
                    // gtk_text_buffer_insert (buffer, &iter, app->eolstr[LF], -1);
                }
            }
            else {  /* handle change to CR */
                if (gtk_text_iter_forward_char (&iter)) {
                    if (gtk_text_iter_get_char (&iter) == '\n') {
                        gtk_text_iter_forward_char (&iter);
                        gtk_text_buffer_backspace (buffer, &iter, FALSE, TRUE);
                    }
                }
            }
        }
        else {
            /* handle no-eol error */
        }
        /* revalidate interator with last_pos mark */
        gtk_text_buffer_get_iter_at_mark (buffer, &iter, app->last_pos);
    }

    if (app->last_pos) {    /* delete app->last_pos mark, if set */
        gtk_text_buffer_delete_mark (buffer, app->last_pos);
        app->last_pos = NULL;
    }

    app->oeol = app->eol;   /* update original eol to current */
}

/** gather buffer character, word and line statistics.
 *  traverse the buffer, gathering buffer statistics, including the
 *  number of whitespace and non-whitespace characters, the total,
 *  the number of words and the number of lines. present in a simple
 *  dialog.
 *  TODO: present in a formatted dialog with option to save.
 */
// void buffer_content_stats (GtkTextBuffer *buffer)
void buffer_content_stats (gpointer data)
{
    mainwin_t *app = data;
    einst_t *einst = app->einst[app->focused];
    kinst_t *inst = einst->inst;

    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(inst->buf);
    GtkTextIter iter;

    gint ws = 0, wsc = 0, nws = 0, nwrd = 0, other = 0, lines = 1;
    gboolean ldws = FALSE, lws = FALSE, havechars = FALSE;
    gunichar c;

    if (!buffer) {
        err_dialog ("Error: Invalid 'buffer' passed to function\n"
                    "buffer_remove_trailing_ws (GtkTextBuffer *buffer)");
        return;
    }

    /* get iter at start of buffer */
    gtk_text_buffer_get_start_iter (buffer, &iter);

    do {
        c = gtk_text_iter_get_char (&iter);

        /* test if iter at end of line */
        if (gtk_text_iter_ends_line (&iter)) {
            if (c == '\r' || c == '\n') { /* loop over all */
                while (c == '\r' || c == '\n') {
                    gchar current = c;
                    wsc++;              /* increment whitespace */
                    if (!gtk_text_iter_forward_char (&iter)) {
                        lines++;        /* end, add line */
                        goto wsdone;    /* goto done */
                    }
                    if (c == '\n')      /* if newline found */
                        lines++;        /* increment lines */
                    c = gtk_text_iter_get_char (&iter); /* get next char */
                    if (current == '\r' && c != '\n')   /* if CR alone   */
                        lines++;        /* increment lines Max (pre-OSX) */
                }
                /* not line-end, backup for next iteration */
                gtk_text_iter_backward_char (&iter);
            }
            if (havechars)              /* if have chars in line */
                nwrd += ws + 1;         /* number of words */
            ws = 0;                     /* word sep per line */
            lws = FALSE;                /* reset last was ws */
            havechars = FALSE;          /* reset havechars */
        }
        else {  /* checking chars in line */

            if (c == ' ' || c == '\t') {
                wsc++;                  /* add whitespace char */
                lws = TRUE;             /* set last ws TRUE */
                if (!havechars)         /* if no chars */
                    ldws = TRUE;        /* set leading whitespace */
            }
            else if (c == 0xFFFC)       /* other/tag char */
                other++;
            else {
                havechars = TRUE;       /* chars in line */
                nws++;                  /* add to non-whitespace */
                if (lws) {              /* if last flag set */
                    lws = FALSE;        /* unset */
                    if (!ldws)          /* if not leading whitespace */
                        ws++;           /* increment word-sep */
                }
                ldws = FALSE;           /* reset leading whitespace */
            }
        }

    } while (gtk_text_iter_forward_char (&iter));
    wsdone:;

    if (havechars)
        nwrd += ws + 1;

    if (!gtk_text_iter_is_end (&iter))
        g_print ("error: not end iter after exiting loop.\n");

    gchar *stats;

    stats = g_strdup_printf ("whitespace characters: %d\n"
                            "non-whitespace chars: %d\n"
                            "other characters : %d\n"
                            "total characters : %d\n"
                            "\nnumber of words: %d\n"
                            "number of lines: %d\n",
                            wsc, nws, other,
                            wsc + nws + other, nwrd, lines);

    // dlg_info (stats, "Buffer Content Statistics");
    einst->ibflags = IBAR_LABEL_SELECT;
    show_info_bar_ok (stats, GTK_MESSAGE_INFO, app);

    g_free (stats);

#ifdef DEBUG
    g_printf ("\nws  : %d\nnws : %d\noth : %d\nchr : %d\nwrd : %d\nlns : %d\n\n",
            wsc, nws, other, wsc + nws + other, nwrd, lines);
    g_printf ("line count: %d\n", gtk_text_buffer_get_line_count (buffer));
    g_printf ("char count: %d\n", gtk_text_buffer_get_char_count (buffer));
#endif
}

/** clears focused textview buffer, sets modified FALSE
 *  gpointer expects mainwin_t type.
 */
void buffer_clear_focused (gpointer data)
{
    mainwin_t *app = data;
    kinst_t *inst = app->einst[app->focused]->inst;
    // GtkTextBuffer *buf = GTK_TEXT_BUFFER(inst->buf);

    gtk_text_buffer_set_text (GTK_TEXT_BUFFER(inst->buf), "", -1);
    gtk_text_buffer_set_modified (GTK_TEXT_BUFFER(inst->buf), FALSE);
}

/* Removed until upstream bug fixed
 * see: https://bugzilla.gnome.org/show_bug.cgi?id=779605
 */
void file_open_recent_dlg (gpointer data)
{
    mainwin_t *app = data;

    GtkWidget *dialog;
    GtkRecentManager *manager;
    GtkRecentFilter *filter;

    manager = gtk_recent_manager_get_default ();
    dialog = gtk_recent_chooser_dialog_new_for_manager ("Recent Documents",
                                            GTK_WINDOW (app->window),
                                            manager,
                                            GTK_STOCK_CANCEL,
                                            GTK_RESPONSE_CANCEL,
                                            GTK_STOCK_OPEN,
                                            GTK_RESPONSE_ACCEPT,
                                            NULL);

    /* Add a filter that will only display plain text files.
     * note: the first filter defined is displayed by default.
     */
#ifndef HAVEMSWIN
    filter = gtk_recent_filter_new ();
    gtk_recent_filter_set_name (filter, "Plain Text");
    gtk_recent_filter_add_mime_type (filter, "text/plain");
    gtk_recent_chooser_add_filter (GTK_RECENT_CHOOSER (dialog), filter);
#endif
    /* Add a filter that will display all of the files in the dialog. */
    filter = gtk_recent_filter_new ();
    gtk_recent_filter_set_name (filter, "All Files");
    gtk_recent_filter_add_pattern (filter, "*");
    gtk_recent_chooser_add_filter (GTK_RECENT_CHOOSER (dialog), filter);

    /* set to choose most recently used files */
    gtk_recent_chooser_set_show_not_found (GTK_RECENT_CHOOSER (dialog),
                                           FALSE);
    gtk_recent_chooser_set_sort_type (GTK_RECENT_CHOOSER (dialog),
                                      GTK_RECENT_SORT_MRU);
    gtk_recent_chooser_set_limit (GTK_RECENT_CHOOSER (dialog), app->nrecent);
    gtk_recent_chooser_set_show_tips (GTK_RECENT_CHOOSER(dialog), TRUE);
    gtk_recent_chooser_set_local_only (GTK_RECENT_CHOOSER (dialog), TRUE);

    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
        GtkRecentInfo *info;
        const gchar *uri;
        gchar *filename;

        info = gtk_recent_chooser_get_current_item (GTK_RECENT_CHOOSER (dialog));
        /* compare with gtk_recent_chooser_get_current_uri
         * (you can use uri_to_filename here, but g_filename_from_uri is fine )
         */
        uri = gtk_recent_info_get_uri (info);
        // filename = uri_to_filename (gtk_recent_info_get_uri (info));
        if (uri) {
            filename = g_filename_from_uri (uri, NULL, NULL);
            file_open (app, filename);
            g_free (filename);
        }
        else
            err_dialog_win ((gpointer *)(app), "uri_to_filename () returned NULL");
        gtk_recent_info_unref (info);
    }
    gtk_widget_destroy (dialog);
}

/* filename for passing name of file to insert in existing document at
 * cursor position, otherwise inst->filename is used to open new file
 * TODO - pass inst? or rely on focus? (view has to be focused for open)
 */
gboolean buffer_insert_file (gpointer data, kinst_t *inst, gchar *filename)
{
    mainwin_t *app = data;
    einst_t *einst = app->einst[app->focused];
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(inst->buf);
    gchar *filebuf = NULL;
    gchar *fnameok = filename;  /* will save NULL distinguish open/insert */
    gsize size = 0;

    if (!filename)  /* if filename NULL, insert file in existing doc */
        filename = inst->filename;

    if (g_file_get_contents (filename, &filebuf, &size, NULL)) {
        if (buffer_file_get_bom (filebuf, app) > 1) {
            extern const gchar *bomstr[];
            gchar *name = treeview_getname (data);
            gchar *errstr = g_strdup_printf ("'%s' contains unsupported %s.",
                            name , bomstr[app->bom]);
            g_free (name);
            status_set_default (data);
            err_dialog_win (data, errstr);
            g_free (errstr);
            if (filebuf) g_free (filebuf);
            return FALSE;
        }
        gtk_text_buffer_insert_at_cursor (buffer, filebuf, -1);
        if (filebuf)
            g_free (filebuf);

        if (app->poscurend) {
            /* scroll to insert mark */
            gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW(einst->view),
                                        gtk_text_buffer_get_insert (buffer),
                                        0.0, TRUE, 0.0, 1.0);
        }
        else {
            /* place cursor "insert_mark" at start of file */
            GtkTextIter iter;
            gtk_text_buffer_get_start_iter (buffer, &iter);
            gtk_text_buffer_place_cursor (buffer, &iter);
        }

        if (fnameok)
            gtk_text_buffer_set_modified (buffer, TRUE);    /* inserted */
        else {
            /* get file uid/gid and mode */
            file_get_stats (inst);
            gtk_text_buffer_set_modified (buffer, FALSE);   /* opened */
            /* set editable based on readonly flag for 1st display of buf */
            gtk_text_view_set_editable (GTK_TEXT_VIEW(einst->view),
                                        inst->readonly ? FALSE : TRUE);
            gtk_widget_set_sensitive (app->rdonlyMi, TRUE);
            buffer_get_eol (data);          /* detect EOL, LF, CRLF, CR */

//             /* add GFileMonitor watch on file - or it buf_new_inst? */
//             /* TODO check passing app or inst below (likely app/data) */
//             if (!inst->filemonfn) {
//                 file_monitor_add (inst);
//             }
//             else if (!g_strcmp0 (inst->filemonfn, inst->filename) == 0) {
//                 if (file_monitor_cancel (inst))
//                     file_monitor_add (inst);
//             }

            /* set syntax highlighting language */
            if (app->highlight)
                sourceview_guess_language (data);
        }
        status_set_default (data);

        return TRUE;
    }
    return FALSE;
}

void buffer_file_insert_dlg (gpointer data)
{
    mainwin_t *app = data;
    kinst_t *inst = app->einst[app->focused]->inst;

    GtkWidget *dialog;
    gchar *filename = NULL;

    /* Create a new file chooser widget */
    dialog = gtk_file_chooser_dialog_new ("Select a file to insert",
					  // parent_window,
					  GTK_WINDOW (app->window),
					  GTK_FILE_CHOOSER_ACTION_OPEN,
					  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					  GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					  NULL);

    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        buffer_insert_file (app, inst, filename);
        g_free (filename);
    }

    gtk_widget_destroy (dialog);
}

/** open file and insert into unmodified "Untitled(n)" buffer, or create
 *  new buffer inst and add to treeview.
 */
void file_open (gpointer data, gchar *filename)
{
    mainwin_t *app = data;
    kinst_t *inst = app->einst[app->focused]->inst; /* current focused buf */
    gint cc;
    gboolean modified;

    if (!filename)
        return;

    gchar *posixfn = get_posix_filename (filename);

    if (!posixfn) {
        g_warning ("get_posix_filename() failed");
        return;
    }

    /* get character-count and whether buffer modified */
    cc = gtk_text_buffer_get_char_count (GTK_TEXT_BUFFER(inst->buf));
    modified = gtk_text_buffer_get_modified (GTK_TEXT_BUFFER(inst->buf));

    /* character-count zero and buffer not modified
     * insert file in current inst->buf
     */
    if (cc == 0 && modified == FALSE) {
        check_untitled_remove (data);   /* if Untitled(n), clear n */
        inst_reset_state (inst);        /* reset all inst value except buf */
        inst->filename = posixfn;       /* get POSIX filename */
        split_fname (inst);             /* buf_new_inst not called - split */

        /* insert file in buffer, set name displayed in tree */
        if (buffer_insert_file (data, inst, NULL))
            treeview_setname (data, inst);

        ibar_eol_chk_default (data);    /* check non-default eol */
    }
    else {  /* add new inst to tree and insert file in new buffer */
        kinst_t *newinst = treeview_append (app, filename);
        if (newinst) {
            /* TODO add check if inst->readonly before allowing insert of
             * file into existing buffer if existing buffer is readonly
             */
            newinst->readonly = app->roflag;    /* set readonly flag for buf */
            buffer_insert_file (data, newinst, NULL);
            /* TODO/FIXME validate einst coordination here to
             * coordinate statusbar association with buffer, and
             * potentially word completion association with view and buffer.
             * (statusbar fixed with move of "changed" and "mark_set")
             */
        }
        else {
            /* TODO add error dialog or infobar */
            g_error ("file_open() newinst NULL");
        }
    }
}

void buffer_write_file (gpointer data)
{
    mainwin_t *app = data;
    einst_t *einst = app->einst[app->focused];
    kinst_t *inst = einst->inst;
    GtkWidget *view = einst->view;
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(inst->buf);
    GtkTextIter start, end;
    GError *err = NULL;
    gchar *text = NULL;
    gboolean result;

    while (gtk_events_pending())    /* process all pending events */
        gtk_main_iteration();

    /* disable text view and get contents of buffer */
    gtk_widget_set_sensitive (view, FALSE);

    /* get start and end iters for buffer */
    gtk_text_buffer_get_start_iter (buffer, &start);
    gtk_text_buffer_get_end_iter (buffer, &end);

    /* get text from buffer */
    text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);

    /* restore test view sensitivity */
    gtk_widget_set_sensitive (view, TRUE);

    /* set the contents of the file to the text from the buffer */
    result = g_file_set_contents (inst->filename, text, -1, &err);

    if (result == FALSE) {
        /* error saving file, show message to user */
        err_dialog (err->message);
        g_error_free (err);
    }
    else {
        gtk_text_buffer_set_modified (buffer, FALSE);
        gtkate_window_set_title (NULL, app);
        inst->modified = FALSE;
#ifndef HAVEMSWIN
        if (inst->filemode)     /* restore file mode, UID, GID */
            g_chmod (inst->filename, inst->filemode);
        if (inst->fileuid && inst->filegid)
            chown ((const char *)inst->filename, (uid_t)inst->fileuid,
                    (gid_t)inst->filegid);
#endif
    }

    /* free text */
    g_free (text);

}

/** file_save in new 'filename' or in existing if NULL.
 *  if 'filename' is provided, Save-As 'filename', otherwise if
 *  'filename' is NULL, Save in existing inst->filename or prompt
 *  with file_save dialog. cancel and add file monitoring as
 *  required. save file stats for existing file, update status.
 */
void file_save (gpointer data, gchar *filename)
{
    mainwin_t *app = data;
    kinst_t *inst = app->einst[app->focused]->inst;
    gchar *posixfn = NULL;

    if (filename) {                     /* file_save_as new filename */
        posixfn = get_posix_filename (filename);
        if (!posixfn) {
            g_warning ("get_posix_filename(fielname) failed");
            g_free (filename);
            return;
        }

        if (inst->filename) {           /* if existing file, free filename */
            inst_free_filename (inst);

            // if (!file_monitor_cancel (inst))    /* cancel existing monitor */
            //v{ /* handle error */ }
        }
        else
            check_untitled_remove (data);

        inst->filename = posixfn;       /* assign to app->filename */
        split_fname (inst);             /* split filename */
        treeview_setname (data, inst);  /* set name in tree */
        g_free (filename);
    }
    else {  /* regular save */
        if (!inst->filename) {  /* check if "Untitled(n)" */
            gchar *newname = NULL;
            while (!(newname = get_save_filename (data))) {
                if (dlg_yes_no_msg (data, "Error: Get save filename canceled!\n"
                                    "Do you want to cancel save?",
                                    "Warning - Save Canceled", FALSE))
                    return;
            }
            posixfn = get_posix_filename (newname);
            if (!posixfn) {
                g_warning ("get_posix_filename(newname) failed");
                g_free (newname);
                return;
            }
            check_untitled_remove (data);   /* unset "Untitled(n)" */
            inst->filename = posixfn;       /* assign to app->filename */
            split_fname (inst);             /* split filename */
            treeview_setname (data, inst);  /* set name in tree */
            g_free (newname);
        }
    }

    // app->mfp_savecmd = TRUE;                /* set flag TRUE - save in progress */

    /* FIXME move functionality here or rewite to just use inst filename */
    buffer_write_file (data);               /* write to file app->filename */

    // if (!app->mfp_handler)                  /* if not monitoring file */
    //     file_monitor_add (app);             /* setup monitoring on new name */

    file_get_stats (inst);                      /* check/save file stats */

    status_set_default (data);              /* restore statusbar */

    if (app->highlight)                     /* if syntax highlight enabled */
        sourceview_guess_language (data);   /* guess source language */
}

/** file_close - close the currently focused file.
 *  if file shown in multiple editor views, close all associated views,
 *  if only file in tree, clear buffer, set "Untitled".
 */
void file_close (gpointer data)
{
    if (treeview_remove_selected (data) == FALSE)
        g_error ("treeview_remove_selected() - failed\n");

}

/** remove leading, interleaved and trailing whitespace, in place.
 *  the original start address is preserved but due to reindexing,
 *  the contents of the original are not preserved. newline chars
 *  are removed. returns TRUE if changes made, FALSE otherwise.
 */
gboolean joinlines (gchar *s)
{
    if (!s || !*s) return FALSE;

    gchar *p = s, *wp = s;      /* pointer and write-pointer */
    gboolean changed = FALSE, xnl = FALSE;

    while (*p) {
        while (*p == '\n') {    /* check for '\n' */
            while (wp > s && (*(wp - 1) == ' ' || *(wp - 1) == '\t'))
                wp--;           /* remove preceeding whitespace */
            *wp++ = ' ';        /* replace '\n' with ' ' */
            p++;
            changed = TRUE;
            xnl = TRUE;     /* set replaced '\n' flag */
        }
        if (p > s && xnl) { /* if not at beginning & just replaced */
            while (*p && (*p == ' ' || *p == '\t')) {
                p++;        /* replace following whitespace */
                changed = TRUE;
            }
            if (!*p)        /* bail on nul-byte */
                break;
            xnl = FALSE;    /* unset replaced   */
        }
        *wp++ = *p;         /* copy non-ws char */
        p++;                /* advance to next  */
    }
    while (wp > s && (*(wp - 1) == ' ' || *(wp - 1) == '\t')) {
        wp--;               /* trim trailing ws */
        changed = TRUE;
    }
    *wp = 0;    /* nul-terminate */

    return changed;
}

/** call (*fn) for each character in selection */
void selection_for_each_char (GtkTextBuffer *buffer, gboolean (*fn) (gchar *))
{
    GtkTextIter start, end;
    gchar *text = NULL;

    /* validate selection exists */
    if (!gtk_text_buffer_get_selection_bounds (buffer, &start, &end)) {
        err_dialog ("Error: Selection Required.\n\n"
                    "gtk_text_buffer_get_selection_bounds()\n"
                    "requires selected text before being called.");
        return;
    }

    if (!(text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE))) {
        err_dialog ("Error: gtk_text_buffer_get_text()\n\n"
                    "Failed to return pointer to alloced\n"
                    "block of memory containing selection.");
        return;
    }

    if (fn (text)) {
        gtk_text_buffer_begin_user_action (buffer);

        if (gtk_text_buffer_delete_selection (buffer, FALSE, TRUE))
            gtk_text_buffer_insert_at_cursor (buffer, text, -1);

        gtk_text_buffer_end_user_action (buffer);
    }

    g_free (text);
}

