#include "gtk_filebuf.h"

/** file to set file stat information for filename */
void file_get_stats (const gchar *filename, kinst_t *file)
{
    struct stat sb;

    file->filemode = 0;
    file->fileuid = 0;
    file->filegid = 0;

    if (!filename) return;  /* validate filename */

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

    file->filemode = sb.st_mode;
    file->fileuid  = sb.st_uid;
    file->filegid  = sb.st_gid;
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

/* TODO - In work see TODO.txt outline */
void file_open (gpointer data, gchar *name)
{
    mainwin_t *app = data;
    kinst_t *inst = app->einst[app->focused]->inst;

    inst_reset_state (inst);    /* reset all inst value except buf */

    if (name) {}
}
