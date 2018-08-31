#include "gtk_appdata.h"

/** default values for main window before settings read from keyfile */
void mainwin_init (mainwin_t *app, char **argv)
{
    app->user           = g_get_user_name ();               /* system username */
#ifndef HAVEMSWIN
    app->usrdatadir     = g_strdup_printf ("%s/%s", g_get_user_data_dir(), CFGDIR);
    app->sysdatadir     = g_strdup (NIXSHARE);
    app->imgdir         = g_strdup_printf ("%s/%s", app->sysdatadir, IMGDIR);
#else
    gchar *usrdata      = get_posix_filename (g_get_user_data_dir());
    app->usrdatadir     = g_strdup_printf ("%s/%s", usrdata, CFGDIR);
    app->sysdatadir     = g_strdup (g_path_get_dirname(argv[0]));
    app->imgdir         = g_strdup_printf ("%s/%s", app->sysdatadir, IMGDIR);
    g_free (usrdata);
#endif
    app->window         = NULL;         /* main window pointer */
    app->toolbar        = NULL;         /* toolbar widget */
    app->vboxtree       = NULL;         /* vbox for treeview show/hide */
    app->treeview       = NULL;         /* treeview widget */
    app->vboxedit       = NULL;         /* vbox inside hpaned2 for ewin */

    /* allocate MAXSPLIT edit window instances */
    for (gint i = 0; i < MAXVIEW; i++) {
        app->einst[i] = g_slice_new (einst_t);
        app->einst[i]->ebox  = NULL;
        app->einst[i]->ibox  = NULL;
        app->einst[i]->swin  = NULL;
        app->einst[i]->view  = NULL;
        app->einst[i]->sbar  = NULL;
        app->einst[i]->inst  = NULL;
    }

    app->winwidth       = 840;          /* default window width   */
    app->winheight      = 800;          /* default window height  */
    app->winrestore     = FALSE;        /* restore window size */
    app->winszsaved     = FALSE;        /* win size saved */
    app->treewidth      = 180;          /* initial treeiew width */
    app->swbordersz     = 0;            /* scrolled_window border */

    app->nview          = 0;            /* no. of editor panes shown */
    app->nfiles         = 0;            /* no. of open files */
    app->focused        = 0;            /* focused einst index */

    app->showtoolbar    = TRUE;         /* toolbar is visible */
    app->showdocwin     = TRUE;         /* document tree is visible */

    app->fontname       = g_strdup ("DejaVu Sans Mono 8");

    app->tabstop        = 8;            /* number of spaces per tab */
    app->softtab        = 4;            /* soft tab stop size */
    app->tabstring      = NULL;         /* tabstring for indent */

    app->indentauto     = TRUE;         /* auto-indent on return */

    app->lineno         = FALSE;        /* show line numbers (sourceview) */
    app->linehghlt      = TRUE;         /* highlight current line */

    app->showmargin     = TRUE;         /* show margin at specific column */
    app->marginwidth    = 80;           /* initial right margin to display */

    app->nrecent        = 40;           /* no. of recent chooser files */
    app->nuntitled      = 0;            /* next "Untitled(n) in tree */

    app->cfgdir         = NULL;         /* user config dir */
    app->cfgfile        = NULL;         /* user config file */
    app->keyfile        = NULL;

    if (argv) {}
}

void mainwin_destroy (mainwin_t *app)
{
    /* save settings to keyfile */
    // context_write_keyfile (app);

    /* free allocated struct values */
    // app_free_filename (app);
    if (app->usrdatadir)    g_free (app->usrdatadir);
    if (app->sysdatadir)    g_free (app->sysdatadir);
    if (app->imgdir)        g_free (app->imgdir);
    if (app->fontname)      g_free (app->fontname);

    // if (app->tabstring)     g_free (app->tabstring);

    if (app->cfgdir)        g_free (app->cfgdir);
    if (app->cfgfile)       g_free (app->cfgfile);

    if (app->keyfile)       g_key_file_free (app->keyfile);

    for (gint i = 0; i < MAXVIEW; i++)
        g_slice_free (einst_t, app->einst[i]);
    /* free find/replace GList memory */
    // findrep_destroy (app);
}

static kinst_t *kinst_init (kinst_t *inst)
{
    if (!inst)
        return NULL;

    inst->buf = NULL;

    inst->filename = NULL;
    inst->fname = NULL;
    inst->fpath = NULL;
    inst->fext = NULL;

    inst->line = inst->col = 0;

    inst->lang_id = NULL;

    inst->comment_single = NULL;
    inst->comment_blk_beg = NULL;
    inst->comment_blk_end = NULL;

    return inst;
}

/** create and initialize a new buffer instance to add to tree */
kinst_t *buf_new_inst (const gchar *fn)
{
    kinst_t *inst = g_slice_new (kinst_t);

    if (!inst)
        return NULL;

    kinst_init (inst);

    inst->buf = gtk_source_buffer_new (NULL);

    if (fn) {
        inst->filename = g_strdup (fn);
        split_fname (inst);
    }

    return inst;
}

/** free memory for allocated buffer instance */
void buf_delete_inst (kinst_t *inst)
{
    if (!inst)
        return;

    inst_free_filename (inst);

    /*
    if (inst->filename) g_free (inst->filename);
    if (inst->fname)    g_free (inst->fname);
    if (inst->fpath)    g_free (inst->fpath);
    if (inst->fext)     g_free (inst->fext);
    */

    g_slice_free (kinst_t, inst);
}

/** fast strlen function */
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

void inst_reset_state (kinst_t *inst)
{
    inst_free_filename (inst);

    inst->line = inst->col = 0;

    inst->lang_id = NULL;

    inst->comment_single = NULL;
    inst->comment_blk_beg = NULL;
    inst->comment_blk_end = NULL;
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

/* TODO - win32 check URI conversion */
/** convert uri to filename.
 *  returns pointer to beginning of filename within uri.
 */
gchar *uri_to_filename (const gchar *uri)
{
    if (!uri || !*uri) return NULL;

    gchar *p = (gchar *)uri;
    gsize len = g_strlen (uri);

    if (len < 4) return NULL;
    /* TODO - check beginning of URI and adjust action based on content */
    for (p += 2; *p; p++) {
        if (*(p - 2) == '/' && *(p - 1) == '/' && *p == '/')
            return p;
    }

    return NULL;
}

/** get_posix_filename returns a pointer to an allocated POSIX filename.
 *  the user is responsible for calling g_free on the return.
 */
gchar *get_posix_filename (const gchar *fn)
{
    if (!fn) return NULL;

    gsize len = g_strlen (fn);
    gchar *posixfn = g_malloc0 (len * 2),
        *wp = posixfn;

    if (!wp) {
        /* handle error */
        return NULL;
    }

    /* convert fn to POSIX filename format in posixfn */
    for (const gchar *p = fn; *p; p++) {
        if (*p == '\\')
            *wp++ = '/';        /* replace backslash with forward slash */
        else if (*p == ' ') {
            *wp++ = '\\';       /* escape before all spaces */
            *wp++ = *p;
        }
        else
            *wp++ = *p;         /* copy remaining chars unchanged */
    }
    *wp = 0;

    return posixfn;
}

/** simple boolean stack function using the bits of STKMAX
 *  unsigned integers to provide stack storage
 */
/** clear stack - zero integers. */
void bstack_clear (mainwin_t *app)
{
    app->bindex = 0;
    for (gint i = 0; i < STKMAX; i++)
        app->bstack[i] = 0;
}

/** stack_push set bit at bindex to 0 or 1 based on v.
 *  increment bindex after setting bit.
 */
int bstack_push (mainwin_t *app, gint v)
{
    guint arrbits = (sizeof app->bstack) * CHAR_BIT,
        elebits = (sizeof app->bindex) * CHAR_BIT,
        arridx = app->bindex/elebits;

    if (app->bindex == arrbits) {   /* check bstack full */
        g_print ("bstack full\n");
        return 0;
    }

    if (v)  /* set bit at bindex */
        app->bstack[arridx] |= (1u << (app->bindex % elebits));
    else    /* clear bit at index */
        app->bstack[arridx] &= ~(1u << (app->bindex % elebits));

    return ++app->bindex;
}

/** stack_pop - decrement bindex, get bit at bindex, clear bit. */
int bstack_pop (mainwin_t *app)
{
    if (!app->bindex) {             /* check bstack empty */
        g_print ("bstack empty\n");
        return -1;
    }

    guint elebits = (sizeof app->bindex) * CHAR_BIT,
        arridx = --app->bindex/elebits,
        /* get bit at bindex */
        v = (app->bstack[arridx] >> (app->bindex % elebits)) & 1;

    /* clear bit at bindex */
    app->bstack[arridx] &= ~(1u << (app->bindex % elebits));

    return v;
}

/** stack_last - get bit for last pushed value. */
int bstack_last (mainwin_t *app)
{
    if (!app->bindex) {             /* check bstack empty */
        g_print ("bstack empty\n");
        return -1;
    }

    guint elebits = (sizeof app->bindex) * CHAR_BIT,
        arridx = (app->bindex - 1)/elebits;

    /* get bit at bindex */
    return (app->bstack[arridx] >> ((app->bindex - 1) % elebits)) & 1;
}

/** Untitled(n) bitfield management */
gint bit_check (guint *bf, gint n) { return (*bf >> n) & 1; }

gint untitled_get_next (mainwin_t *app)
{
    for (gint i = 0; i < (gint)(sizeof app->nuntitled * CHAR_BIT); i++)
        if (!bit_check (&app->nuntitled, i)) {
            app->nuntitled |= (1u << i);
            return i;
        }

    g_warning ("untitled_get_next() error: max untitled files reached.");

    return -1;
}

void untitled_remove (mainwin_t *app, gint n)
{
    app->nuntitled &= ~(1u << n);
}

/** date & time functions */

/** returns allocated date/time string in with
 *  %F %T format (e.g. 2018-07-12 03:12:09)
 *  user is responsible for calling g_free()
 */
gchar *get_local_datetime (void)
{
    gchar *s = NULL;
    gint Y, m, d, H, M, S;
    GDateTime *gdt = g_date_time_new_now_local ();

    g_date_time_get_ymd (gdt, &Y, &m, &d);
    H = g_date_time_get_hour (gdt);
    M = g_date_time_get_minute (gdt);
    S = g_date_time_get_second (gdt);

    s = g_strdup_printf ("%4d-%02d-%02d %02d:%02d:%02d", Y, m, d, H, M, S);

    g_date_time_unref (gdt);

    return s;
}

/* TODO - get_user_cfgfile() add error checks */
/**  form key_file filename from config_dir and defines
 */
char *get_user_cfgfile (mainwin_t *app)
{
    gchar *cfgdir;

    if ((cfgdir = (gchar *)g_get_user_config_dir())) {
        app->cfgdir = g_strdup_printf ("%s/%s", cfgdir, CFGDIR);
        if (!app->cfgdir)
            return NULL;
        return (app->cfgfile =
                g_strdup_printf ("%s/%s", app->cfgdir, CFGFILE));
    }

    return NULL;
}

/** general use err_dialog, just pass errmsg. */
void err_dialog (const gchar *errmsg)
{
    GtkWidget *dialog;

    g_warning (errmsg); /* log to terminal window */

    /* create an error dialog and display modally to the user */
    dialog = gtk_message_dialog_new (NULL,
                                    GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                    GTK_MESSAGE_ERROR,
                                    GTK_BUTTONS_OK,
                                    errmsg);

    gtk_window_set_title (GTK_WINDOW (dialog), "Error!");
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
}

/** creates a new pixbuf from filename.
 *  you are responsible for calling g_object_unref() on
 *  the pixbuf when done.
 */
GdkPixbuf *create_pixbuf_from_file (const gchar *filename)
{
    GdkPixbuf *pixbuf;
    GError *error = NULL;
    pixbuf = gdk_pixbuf_new_from_file (filename, &error);

    if (!pixbuf) {
        g_warning (error->message); /* log to terminal window */
        g_error_free (error);
    }

    return pixbuf;
}

