#include "gtk_statusbar.h"

void status_menuitem_label (GtkMenuItem *menuitem, gpointer data)
{
    mainwin_t *app = data;
    einst_t *einst = app->einst[app->focused];

    if (einst->cid)     /* pop previous statusbar entry */
        gtk_statusbar_pop (GTK_STATUSBAR (einst->sbar), einst->cid);

    gchar *str;                 /* create string from menu item */
    str = g_strdup_printf ("menuitem : %s",
                           gtk_menu_item_get_label (menuitem));

    einst->cid = gtk_statusbar_get_context_id (GTK_STATUSBAR (einst->sbar), str);

    gtk_statusbar_push (GTK_STATUSBAR (einst->sbar), einst->cid, str);

    g_free (str);
}

void status_pop (GtkWidget *widget, gpointer data)
{
    mainwin_t *app = data;
    einst_t *einst = app->einst[app->focused];

    if (einst->cid)
        gtk_statusbar_pop (GTK_STATUSBAR (einst->sbar), einst->cid);

    if (widget) {}
}

void status_set_default (gpointer data)
{
    mainwin_t *app = data;
    einst_t *einst = app->einst[app->focused];
    kinst_t *inst = einst->inst;

    extern const gchar *bomstr[];
    const gchar *langname;
    gchar *status;
    gint lines;

    /* Fri Jul 13 2018 18:57:03 CDT moved line/col update here from
     * buffer_update_pos () to prevent line number jumping on scrolled-
     * window scrolling when bottom of screen reached.
     */
    GtkTextIter iter;
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER (inst->buf);
    GtkTextMark *ins = gtk_text_buffer_get_insert (buffer);

    /* get iter at 'insert' mark */
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, ins);

    /* update line, lines & col */
    inst->line = gtk_text_iter_get_line (&iter);
    lines = gtk_text_buffer_get_line_count (buffer);
    inst->col = gtk_text_iter_get_line_offset (&iter);

    if (inst->language)
        langname = gtk_source_language_get_name (inst->language);
    else
        langname = "Plain Text";

    status = g_strdup_printf (" line:%5d / %d  col:%4d  |  %s  |  "
                            "%s  |  %s  |  %s",
                            inst->line + 1, lines, inst->col + 1,
                            app->overwrite ? "OVR" : "INS",
                            app->eolnm[app->eol], bomstr[app->bom],
                            langname);

    if (einst->cid)     /* pop previous statusbar entry */
        gtk_statusbar_pop (GTK_STATUSBAR(einst->sbar), einst->cid);

    einst->cid = gtk_statusbar_get_context_id (GTK_STATUSBAR (einst->sbar),
                                                status);
    gtk_statusbar_push (GTK_STATUSBAR(einst->sbar), einst->cid, status);

    g_free (status);
}
