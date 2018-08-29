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

