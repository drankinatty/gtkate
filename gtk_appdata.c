#include "gtk_appdata.h"

#define STYLE_DARK "gtkate"
#define STYLE_LIGHT "gtkate-light"

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

    app->treemodel      = NULL;         /* treemodel for treeview */

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

    /* initial views, files and focused einst */
    app->nview          = 0;            /* no. of editor panes shown */
    app->nfiles         = 0;            /* no. of open files */
    app->focused        = 0;            /* focused einst index */

    /* window layout/size */
    app->winwidth       = 840;          /* default window width   */
    app->winheight      = 800;          /* default window height  */
    app->treewidth      = 180;          /* initial treeiew width */
    app->swbordersz     = 0;            /* scrolled_window border */

    /* sourceview variables, menus & settings */
    app->langmgr        = gtk_source_language_manager_get_default();
    app->stylelist      = NULL;         /* sourceview styles menu list */
    app->hghltmenu      = NULL;         /* sourceview syntax highlight menu */
#ifdef HAVEMSWIN
    app->laststyle      = g_strdup(STYLE_LIGHT);  /* default light highlight style */
#else
    app->laststyle      = g_strdup(STYLE_DARK);   /* default dark highlight style */
#endif
    // app->laststyle      = NULL;
    app->highlight      = TRUE;         /* show syntax highlight */
    /* source completion variables & settings */
    app->completion     = NULL;         /* completion provider object */
    app->enablecmplt    = TRUE;         /* enable word completion */
    app->cmplwordsz     = 3;            /* completion minimum-word-size */

    /* find/replace defaults and initilization */
    app->findtext       = g_malloc0 (MAXLE * sizeof *(app->findtext));
    app->reptext        = g_malloc0 (MAXLE * sizeof *(app->reptext));
    app->nfentries      = 0;
    app->nrentries      = 0;
    app->fmax           = MAXLE;
    app->rmax           = MAXLE;
    app->optregex       = TRUE;  /* initial checkbox states */
    app->optplace       = FALSE;
    app->optcase        = TRUE;
    app->optwhole       = FALSE;
    app->optfrom        = FALSE;
    app->optback        = FALSE;
    app->optselect      = FALSE;
    app->optprompt      = FALSE;

    app->txtfound       = FALSE;
    app->last_pos       = NULL;
    app->markfrom       = NULL;
    app->selstart       = NULL;
    app->selend         = NULL;

    if (!(app->findtext && app->reptext)) {
        g_error ("findtest/reptext allocation failure.");
        gtk_main_quit();
    }

    app->printsettings  = NULL;         /* initialize print settings */
    app->printpgsetup   = NULL;         /* initialize page setup */
    app->margintop      = 0.5;          /* default margins */
    app->marginbottom   = 0.5;
    app->marginleft     = 1.0;
    app->marginright    = 0.5;

    /* settings flags/file information */
    app->showtoolbar    = TRUE;         /* toolbar is visible */
    app->showdocwin     = TRUE;         /* document tree is visible */
    app->winrestore     = FALSE;        /* restore window size */
    app->winszsaved     = FALSE;        /* win size saved */

    /* eol handling */
#ifndef HAVEMSWIN
    app->eolos          = LF;           /* default *nix line end - LF */
#else
    app->eolos          = CRLF;         /* default DOS/Win line end - CRLF */
#endif
    app->eol            = app->eolos;   /* initially set to operating system default */
    app->eoldefault     = OS_EOL;       /* global default for EOL, default use from file */
    app->oeol           = app->eol;     /* original end-of-line (for conversions) */
    app->eolchg         = FALSE;        /* no eol change until file open or user selects */
    app->eolstr[0]      = EOL_LF;       /* eol ending strings */
    app->eolstr[1]      = EOL_CRLF;
    app->eolstr[2]      = EOL_CR;
    app->eolnm[0]       = EOLNM_LF;     /* eol string names */
    app->eolnm[1]       = EOLNM_CRLF;
    app->eolnm[2]       = EOLNM_CR;
    app->eoltxt[0]      = EOLTXT_LF;    /* eol descriptions */
    app->eoltxt[1]      = EOLTXT_CRLF;
    app->eoltxt[2]      = EOLTXT_CR;
    app->eoltxt[3]      = EOLTXT_FILE;
    app->eoltxt[4]      = EOLTXT_OS;

    app->bom            = 0;            /* Byte Order Mark (0 none)
                                         * (1-15 corresponds index)
                                         */

    /* custom key handler flags */
    app->ctrl_shift_right_fix = TRUE;   /* Use custom key-handler */

    app->fontname       = g_strdup ("DejaVu Sans Mono 8");  /* default font */

    app->tabstop        = 8;            /* number of spaces per tab */
    app->softtab        = 4;            /* soft tab stop size */
    app->tabstring      = NULL;         /* tabstring for indent */
    app->expandtab      = TRUE;         /* insert spaces for tab */
    app->showtabs       = FALSE;        /* display tabulator markers */

    app->smartbs        = TRUE;         /* use smart backspace */
    app->smarthe        = TRUE;         /* smart home & end cursor */

    app->indentauto     = TRUE;         /* auto-indent on return */
    app->indentwspc     = TRUE;         /* indent w/spaces not tabs */
    app->indentmixd     = FALSE;        /* Emacs mode indent w/mixed spc/tabs */

    app->dynwrap        = TRUE;         /* use dynamic word wrap */
    app->showdwrap      = FALSE;        /* show dynamic word wrap indicators */
    app->wraptxtcsr     = TRUE;         /* wrap cursor to next line */
    app->pgudmvscsr     = TRUE;         /* PgUp/PgDn keys move cursor */

    app->lineno         = FALSE;        /* show line numbers (sourceview) */
    app->linehghlt      = TRUE;         /* highlight current line */

    app->showmargin     = TRUE;         /* show margin at specific column */
    app->marginwidth    = 80;           /* initial right margin to display */

    app->comment        = NULL;         /* single-line comment string */
    app->cmtentry       = NULL;         /* single-line comment entry */
    app->cmtusesingle   = FALSE;        /* single-line instead of block comment */

    app->overwrite      = FALSE;        /* ins/overwrite mode flag */
    app->poscurend      = FALSE;        /* scroll to end of file on open */

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
    inst->mfpmon = NULL;
    inst->mfpfn = NULL;
    inst->mfphandler = 0;

    inst->filename = NULL;
    inst->fname = NULL;
    inst->fpath = NULL;
    inst->fext = NULL;

    inst->filemode = 0;
    inst->fileuid = 0;
    inst->filegid = 0;

    inst->line = inst->col = 0;

    inst->language = NULL;
    // inst->lang_id = NULL;    /* get at time keyfile is written */

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

//     gtk_text_buffer_set_text (GTK_TEXT_BUFFER(inst->buf), "", -1);
//     gtk_text_buffer_set_modified (GTK_TEXT_BUFFER(inst->buf), FALSE);

    if (fn) {
        gchar *posixfn = get_posix_filename (fn);
        inst->filename = g_strdup (posixfn);
        split_fname (inst);
        /* set GFileMonitor watch */
    }

    return inst;
}

/** free memory for allocated buffer instance */
void buf_delete_inst (kinst_t *inst)
{
    if (!inst)
        return;

    /* free allocated members and zero remaining members */
    inst_reset_state (inst);

    g_slice_free (kinst_t, inst);   /* free slice */

    inst = NULL;    /* set pointer NULL */
}

void einst_reset (einst_t *einst)
{
    einst->ebox = NULL;
    einst->ibox = NULL;
    einst->swin = NULL;
    einst->view = NULL;
    einst->sbar = NULL;

    /* preserve kinst_t as it may be shown in multiple views */
}

void einst_move (einst_t *tgt, einst_t *src)
{
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
    /* TODO add GFileMonitor var reset */
    inst->line = inst->col = 0;

    inst->filemode = 0;
    inst->fileuid = 0;
    inst->filegid = 0;

    inst->language = NULL;
    // inst->lang_id = NULL;    /* get and write to keyfile */

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

/** fast strlen function */
gsize g_strlen (const gchar *s)
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

gboolean str2lower (gchar *str)
{
    if (!str) return FALSE;

    gchar *p = str;
    gboolean changed = FALSE;

    for (; *p; p++)
        if ('A' <= *p && *p <= 'Z') {
            *p ^= 32;
            changed = TRUE;
        }

    return changed;
}

gboolean str2upper (gchar *str)
{
    if (!str) return FALSE;

    gchar *p = str;
    gboolean changed = FALSE;

    for (; *p; p++)
        if ('a' <= *p && *p <= 'z') {
            *p ^= 32;
            changed = TRUE;
        }

    return changed;
}

gboolean str2title (gchar *str)
{
    if (!str) return FALSE;

    gchar *p = str;
    gboolean changed = FALSE;

    for (; *p; p++) {
        if (p == str) {
            if ('a' <= *p && *p <= 'z') {
                *p ^= 32;
                changed = TRUE;
            }
        }
        else if (*(p - 1) == ' ') {
            if ('a' <= *p && *p <= 'z') {
                *p ^= 32;
                changed = TRUE;
            }
        }
        else {
            if ('A' <= *p && *p <= 'Z') {
                *p ^= 32;
                changed = TRUE;
            }
        }
    }

    return changed;
}

/** delete the last_pos mark */
void delete_mark_last_pos (gpointer data)
{
    mainwin_t *app = data;
    kinst_t *inst = app->einst[app->focused]->inst;

    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(inst->buf);

    if (app->last_pos)
        gtk_text_buffer_delete_mark (buffer, app->last_pos);

    app->last_pos = NULL;
}

/** simple boolean stack function using the bits of STKMAX
 *  unsigned integers to provide stack storage
 */
/** clear stack - zero integers. */
void bstack_clear (gpointer data)
{
    mainwin_t *app = data;

    app->bindex = 0;
    for (gint i = 0; i < STKMAX; i++)
        app->bstack[i] = 0;
}

/** stack_push set bit at bindex to 0 or 1 based on v.
 *  increment bindex after setting bit.
 */
int bstack_push (gpointer data, gint v)
{
    mainwin_t *app = data;

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
int bstack_pop (gpointer data)
{
    mainwin_t *app = data;

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
int bstack_last (gpointer data)
{
    mainwin_t *app = data;

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

/* get next available 'n' for "Untitled(n)", set bit in nuntitled bitfield */
gint untitled_get_next (gpointer data)
{
    mainwin_t *app = data;

    for (gint i = 0; i < (gint)(sizeof app->nuntitled * CHAR_BIT); i++)
        if (!bit_check (&app->nuntitled, i)) {
            app->nuntitled |= (1u << i);
            return i;
        }

    g_warning ("untitled_get_next() error: max untitled files reached.");

    return -1;
}

/* clear bit 'n' in nuntitled bitfield */
void untitled_remove (gpointer data, gint n)
{
    mainwin_t *app = data;

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

/** set window title from treeview displayed name and buffer modified state */
void gtkate_window_set_title (GtkWidget *widget, gpointer data)
{
    /* TODO: create common set title function for all dialogs
     * (e.g. if (widget == app->window), then window title, else dialog
     */
    mainwin_t *app = data;
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    GtkTreeModel *model;

    /* set widget to main window if NULL */
    if (!widget)
        widget = app->window;

    /* initialize selection from treeview */
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(app->treeview));

    /* get treemodel and iter from selection */
    if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

        kinst_t *inst;
        gchar *name = NULL,
              *title = NULL;

        /* get column name and buffer instance given model & iter */
        gtk_tree_model_get (model, &iter, COLNAME, &name, COLINST, &inst,  -1);

        /* check/set modified state in window title text on selection change */
        if (gtk_text_buffer_get_modified (GTK_TEXT_BUFFER(inst->buf)))
            title = g_strdup_printf ("%s - %s [modified]", APPNAME, name);
        else
            title = g_strdup_printf ("%s - %s", APPNAME, name);

        /* set window title */
        gtk_window_set_title (GTK_WINDOW (widget), title);

        /* free allocated memory */
        g_free (name);
        g_free (title);
    }
    else
        g_warning ("gtkate_window_set_title() unable to set window title.");

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

