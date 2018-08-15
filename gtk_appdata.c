#include "gtk_appdata.h"

/** fast strlen function */
/*
static gsize g_strlen (const gchar *s)
{
    gsize len = 0;
    for(;;) {
        if (s[0] == 0) return len;
        if (s[1] == 0) return len + 1;
        if (s[2] == 0) return len + 2;
        if (s[3] == 0) return len + 3;
        s += 4, len += 4;
    }
}
*/

/** inst_free_filename, free all filename components. */
void inst_free_filename (kinst_t *inst)
{
    if (inst->filename) g_free (inst->filename);
    if (inst->fname) g_free (inst->fname);
    if (inst->fext) g_free (inst->fext);
    if (inst->fpath) g_free (inst->fpath);

    inst->filename = NULL;
    inst->fname = NULL;
    inst->fext = NULL;
    inst->fpath = NULL;
}

/** split app->filename into path, filename, extension.
 *  this can be called numerous times during editing, so
 *  the struct is passed and all components are updated.
 */
void split_fname (kinst_t *inst)
{
    if (!inst->filename)
        return;

    gchar *ep = inst->filename;
    gchar *sp = ep;
    gchar *p = ep;

    /* free memory for existing components */
    if (inst->fname) g_free (inst->fname);
    if (inst->fext)  g_free (inst->fext);
    if (inst->fpath) g_free (inst->fpath);
    inst->fname = inst->fext = inst->fpath = NULL;

    for (; *ep; ep++);  /* end of string */

    /* separate extension */
    p = ep;
    while (*sp == '.') sp++;            /* handle dot files */
    for (; p > sp && *p != '.'; p--);   /* find last '.' */

    if (p != sp)    /* not dot file with no extension */
        inst->fext = g_strdup (p + 1);   /* set fext */

    p = ep; /* separate path */
    for (; p > inst->filename && *p != '/'; p--);

    if (p == inst->filename) {
        if (*p == '/') {    /* handle root path */
            inst->fname = g_strdup (inst->filename + 1);
            inst->fpath = g_strdup ("/");
        }
        else    /* no path */
            inst->fname = g_strdup (inst->filename);
        return;
    }

    /* separate normal /path/filename */
    inst->fname = g_strdup (p + 1);
    inst->fpath = g_strndup (inst->filename, p - inst->filename);
}

