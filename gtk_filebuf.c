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
//             buffer_get_eol (inst);          /* detect EOL, LF, CRLF, CR */

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
//         status_set_default (app);

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

/** open file and instert into unmodified "Untitled(n)" buffer, or create
 *  new buffer inst and add to treeview.
 */
void file_open (gpointer data, gchar *filename)
{
    mainwin_t *app = data;
    kinst_t *inst = app->einst[app->focused]->inst;
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
    }
    else {  /* add new inst to tree and insert file in new buffer */
        kinst_t *newinst = treeview_append (app, filename);
        if (newinst) {
            // app->nfiles++;  /* update file count (no - done in get_new_inst) */
            buffer_insert_file (data, newinst, NULL);
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
        // gtkwrite_window_set_title (NULL, app);
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

    // status_set_default (app);               /* restore statusbar */

    // if (app->highlight)                     /* if syntax highlight enabled */
    //     sourceview_guess_language (app);    /* guess source language */
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

