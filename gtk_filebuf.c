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
            file_get_stats (inst);
            gtk_text_buffer_set_modified (buffer, FALSE);   /* opened */
//             buffer_get_eol (inst);          /* detect EOL, LF, CRLF, CR */
//             /* add GFileMonitor watch on file - or it buf_new_inst? */
//             /* TODO check passing app or inst below */
//             if (!inst->filemonfn) {
//                 file_monitor_add (inst);
//             }
//             else if (!g_strcmp0 (inst->filemonfn, inst->filename) == 0) {
//                 if (file_monitor_cancel (inst))
//                     file_monitor_add (inst);
//             }
//             /* set syntax highlighting language */
//             if (app->highlight)
//                 sourceview_guess_language (inst);
        }
//         status_set_default (app);

        return TRUE;
    }
    return FALSE;
}

/** open file and instert into unmodified "Untitled(n)" buffer, or create
 *  new buffer inst and add to treeview.
 */
 /* FIXME - if there is only 1 file open and it is Untitled - then replace
  * buf only, no increment of nfiles
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

#ifdef DEBUG
    g_print ("file_open()\n"
             "  app->focused: %d\n"
             "  cc          : %d\n"
             "  modified    : %s\n",
             app->focused, cc, modified ? "true" : "false");
#endif
    /* character-count zero and buffer not modified
     * insert file in current inst->buf
     */
    if (cc == 0 && modified == FALSE) {
        check_untitled_remove (data);   /* if Untitled(n), clear n */
        inst_reset_state (inst);        /* reset all inst value except buf */
        inst->filename = posixfn;       /* get POSIX filename */
        split_fname (inst);             /* buf_new_inst not called - split */

#ifdef DEBUG
        g_print ("  filename    : %s\n"
                 "  fpath       : %s\n"
                 "  fname       : %s\n"
                 "  fext        : %s\n",
                 inst->filename, inst->fpath, inst->fname, inst->fext);
#endif
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
#ifdef DEBUG
    g_print ("\n");
#endif
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
