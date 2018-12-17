#include "gtk_common_dlg.h"

/** general use err_dialog, just pass errmsg. */
// void err_dialog (const gchar *errmsg)
// {
//     GtkWidget *dialog;
//
//     g_warning (errmsg); /* log to terminal window */
//
//     /* create an error dialog and display modally to the user */
//     dialog = gtk_message_dialog_new (NULL,
//                                     GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
//                                     GTK_MESSAGE_ERROR,
//                                     GTK_BUTTONS_OK,
//                                     errmsg);
//
//     gtk_window_set_title (GTK_WINDOW (dialog), "Error!");
//     gtk_dialog_run (GTK_DIALOG (dialog));
//     gtk_widget_destroy (dialog);
// }

/** general use err_dialog, just pass errmsg. */
void err_dialog_win (gpointer data, const gchar *errmsg)
{
    mainwin_t *app = data;
    GtkWidget *dialog;

    g_warning (errmsg); /* log to terminal window */

    /* create an error dialog and display modally to the user */
    dialog = gtk_message_dialog_new (GTK_WINDOW (app->window),
                                    GTK_DIALOG_MODAL |
                                        GTK_DIALOG_DESTROY_WITH_PARENT,
                                    GTK_MESSAGE_ERROR,
                                    GTK_BUTTONS_OK,
                                    errmsg);

    gtk_window_set_title (GTK_WINDOW (dialog), "Error!");
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
}

/** font_select_dialog used to set textview font.
 *  create a new pango font description and calls
 *  gtk_widget_modify_font to set textview font.
 */
void font_select_dialog (GtkWidget *widget, gpointer data)
{
    mainwin_t *app = data;
    GtkResponseType result;

    GtkWidget *dialog = gtk_font_selection_dialog_new ("Select Font");

    /* set initial font name if (!wanted) use default */
    if (!gtk_font_selection_dialog_set_font_name (
            GTK_FONT_SELECTION_DIALOG (dialog), app->fontname))
        gtk_font_selection_dialog_set_font_name (
            GTK_FONT_SELECTION_DIALOG (dialog), "Monospace 8");

    result = gtk_dialog_run (GTK_DIALOG(dialog));

    if (result == GTK_RESPONSE_OK || result == GTK_RESPONSE_APPLY) {

        if (app->fontname)
            g_free (app->fontname);

        PangoFontDescription *font_desc;
        app->fontname = gtk_font_selection_dialog_get_font_name (
                                GTK_FONT_SELECTION_DIALOG (dialog));

        if (!app->fontname) {
            err_dialog ("error: invalid font returned.");
            return;
        }

        font_desc = pango_font_description_from_string (app->fontname);

        /* loop over each shown textview changing font */
        for (gint i = 0; i < app->nview; i++)
            gtk_widget_modify_font (app->einst[i]->view, font_desc);

        pango_font_description_free (font_desc);
    }
    gtk_widget_destroy (dialog);

    if (widget) {}  /* stub */
}

void dlg_info (const gchar *msg, const gchar *title)
{
    GtkWidget *dialog;

    dialog = gtk_message_dialog_new (NULL,
                                    GTK_DIALOG_MODAL |
                                    GTK_DIALOG_DESTROY_WITH_PARENT,
                                    GTK_MESSAGE_INFO,
                                    GTK_BUTTONS_CLOSE,
                                    msg);

    gtk_window_set_title (GTK_WINDOW (dialog), title);
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
}

void dlg_info_win (gpointer data, const gchar *msg, const gchar *title)
{
    mainwin_t *app = data;
    GtkWidget *dialog;

    dialog = gtk_message_dialog_new (GTK_WINDOW (app->window),
                                    GTK_DIALOG_MODAL |
                                    GTK_DIALOG_DESTROY_WITH_PARENT,
                                    GTK_MESSAGE_INFO,
                                    GTK_BUTTONS_CLOSE,
                                    msg);

    gtk_window_set_title (GTK_WINDOW (dialog), title);
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
}

gboolean dlg_yes_no_msg (gpointer data, const gchar *msg, const gchar *title,
                            gboolean default_return)
{
    mainwin_t *app = data;
    GtkWidget *window = data ? app->window : NULL;
    gboolean ret = default_return ? TRUE : FALSE;
    GtkWidget *dialog;

    dialog = gtk_message_dialog_new (GTK_WINDOW (window),
                                    GTK_DIALOG_MODAL |
                                    GTK_DIALOG_DESTROY_WITH_PARENT,
                                    GTK_MESSAGE_QUESTION,
                                    GTK_BUTTONS_YES_NO,
                                    msg);

    gtk_window_set_title (GTK_WINDOW (dialog), title);
    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_NO)
    {
        ret = FALSE;    /* don't save */
    }
    else ret = TRUE;    /* save */

    gtk_widget_destroy (dialog);

    return ret;
}

/** ib_response_ok - default callback for show_info_bar functions.
 *  callback for infobar with "_Ok" button to hide infobar.
 */
void ib_response_ok (GtkInfoBar *bar, gint response_id, gpointer data)
{
    mainwin_t *app = data;
    einst_t *einst = app->einst[app->focused];

    gtk_widget_hide (GTK_WIDGET(bar));

    /* if called by default from show_info_bar_choice, check and
     * restore sensitive setting for textview.
     */
    if (!gtk_widget_get_sensitive (einst->view))
        gtk_widget_set_sensitive (einst->view, TRUE);

    /* grab focus for textview */
    gtk_widget_grab_focus (einst->view);

    /* reset infobar flags */
    einst->ibflags = 0;

    if (response_id) {}
}

/* TODO - add guchar bitfiled for label-selectable, textview-sensitive
 * and any other needed values, rather than adding multiple parameters.
 * declare enum GTK_IBAR_LABEL_SELECT, GTK_IBAR_VIEW_SENSITIVE, etc.
 * can add as app->ibflags and avoid additional parameter altogether.
 * (implementation done for ibar_handle_quit() Wed Jan 17 2018 02:35:34)
 */
/** show infobar with msg, msgtype, btndef, and callback fn_response.
 *  btndef and fn_response can both be NULL, if btndef is NULL,
 *  "_OK" is used by default. btndef passes a pointer to array of
 *  ibbtndef containing btntext and resource_id pairs, the last
 *  btntext must be an empty-string sentinel. the fn_response must
 *  handle the response_id returned from the infobar.
 */
void show_info_bar_choice (const gchar *msg, gint msgtype,
                            ibbtndef *btndef,
                            void (*fn_response)(GtkInfoBar *bar,
                                    gint response_id,
                                    gpointer data),
                            gpointer data)
{
    mainwin_t *app = data;
    einst_t *einst = app->einst[app->focused];

    GtkWidget *infobar;             /* the infobar widget */
    GtkInfoBar *bar;                /* a GtkInfoBar* reference */

    GtkWidget *message_label;       /* test to display in infobar */
    GtkWidget *content_area;        /* content_area of infobar */
    GtkWidget *hbox;                /* hbox for content_area */

    infobar = gtk_info_bar_new ();  /* create new infobar */
    gtk_widget_set_no_show_all (infobar, TRUE); /* set no show all */
    bar = GTK_INFO_BAR (infobar);   /* create reference for convenience */
    content_area = gtk_info_bar_get_content_area (bar); /* get content_area */

    hbox = gtk_hbox_new (FALSE, 0); /* create hbox for content w/border */
    gtk_container_set_border_width (GTK_CONTAINER(hbox), 5);

    /* set label text, add hbox to content_area, add label to hbox */
    // message_label = gtk_label_new (msg);
    message_label = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL(message_label), msg);
    gtk_container_add (GTK_CONTAINER (content_area), hbox);
    gtk_box_pack_start (GTK_BOX(hbox), message_label, FALSE, FALSE, 0);
    gtk_widget_show (message_label);
    gtk_widget_show (hbox);

    /* change message foreground color as needed */
    if (msgtype < GTK_MESSAGE_ERROR) {
        GdkColor color;
        gdk_color_parse ("black", &color);
        gtk_widget_modify_fg (message_label, GTK_STATE_NORMAL, &color);
    }

    /* add buttons to infobar */
    if (btndef && *(btndef->btntext)) {
        for (; *(btndef->btntext); btndef++)
            gtk_info_bar_add_button (bar, btndef->btntext, btndef->response_id);
    }
    else
        gtk_info_bar_add_button (bar, "_OK", GTK_RESPONSE_OK);

    /* choose type of infobar */
    gtk_info_bar_set_message_type (bar, msgtype);

    /* pack infobar into vbox in parent (passed a pointer data) */
    gtk_box_pack_start(GTK_BOX(einst->ibox), infobar, FALSE, TRUE, 0);

    /* connect response handler */
    if (fn_response)                /* connect custom handler  */
        g_signal_connect (bar, "response", G_CALLBACK (fn_response), app);
    else                            /* connect default handler */
        g_signal_connect (bar, "response", G_CALLBACK (ib_response_ok), app);

    /* set label in infobar selectable */
    if (einst->ibflags & IBAR_LABEL_SELECT)
        gtk_label_set_selectable (GTK_LABEL(message_label), TRUE);

    /* set text_view sensitive */
    if (einst->ibflags & IBAR_VIEW_SENSITIVE)
        gtk_widget_set_sensitive (einst->view, FALSE);

    /* set visible flag */
    einst->ibflags |= IBAR_VISIBLE;

    gtk_widget_show (infobar);  /* show the infobar */
}

/** simple infobar with OK button displaying msg of msgtype */
void show_info_bar_ok (const gchar *msg, gint msgtype, gpointer data)
{
    mainwin_t *app = data;
    einst_t *einst = app->einst[app->focused];

    GtkWidget *infobar;             /* the infobar widget */
    GtkInfoBar *bar;                /* a GtkInfoBar* reference */

    GtkWidget *message_label;       /* test to display in infobar */
    GtkWidget *content_area;        /* content_area of infobar */
    GtkWidget *hbox;                /* hbox for content_area */

    infobar = gtk_info_bar_new ();  /* create new infobar */
    gtk_widget_set_no_show_all (infobar, TRUE); /* set no show all */
    bar = GTK_INFO_BAR (infobar);   /* create reference for convenience */
    content_area = gtk_info_bar_get_content_area (bar); /* get content_area */

    hbox = gtk_hbox_new (FALSE, 0); /* create hbox for content w/border */
    gtk_container_set_border_width (GTK_CONTAINER(hbox), 5);

    /* set label text, add hbox to content_area, add label to hbox */
    message_label = gtk_label_new (msg);
    gtk_container_add (GTK_CONTAINER (content_area), hbox);
    gtk_box_pack_start (GTK_BOX(hbox), message_label, FALSE, FALSE, 0);
    gtk_widget_show (message_label);
    gtk_widget_show (hbox);

    /* change message foreground color as needed */
    if (msgtype < GTK_MESSAGE_ERROR) {
        GdkColor color;
        gdk_color_parse ("black", &color);
        gtk_widget_modify_fg (message_label, GTK_STATE_NORMAL, &color);
    }

    /* add button to close infobar */
    gtk_info_bar_add_button (bar, "_OK", GTK_RESPONSE_OK);

    /* choose type of infobar */
    gtk_info_bar_set_message_type (bar, msgtype);

    /* pack infobar into vbox in parent (passed a pointer data) */
    gtk_box_pack_start(GTK_BOX(einst->ibox), infobar, FALSE, TRUE, 0);

    /* connect response handler */
    g_signal_connect (bar, "response", G_CALLBACK (ib_response_ok), app);

    /* set label in infobar selectable */
    if (einst->ibflags & IBAR_LABEL_SELECT)
        gtk_label_set_selectable (GTK_LABEL(message_label), TRUE);

    /* set text_view sensitive FALSE */
    if (einst->ibflags & IBAR_VIEW_SENSITIVE)
        gtk_widget_set_sensitive (einst->view, FALSE);

    /* set visible flag */
    einst->ibflags |= IBAR_VISIBLE;

    gtk_widget_show (infobar);  /* show the infobar */
}

GSList *get_open_filename (gpointer data)
{
    mainwin_t *app = data;
    kinst_t *inst = app->einst[app->focused]->inst;
    GtkWidget *chooser;
    GtkWidget *read_only;
    GSList *filenames = NULL;

    chooser = gtk_file_chooser_dialog_new ("Open File...",
                                            GTK_WINDOW (app->window),
                                            GTK_FILE_CHOOSER_ACTION_OPEN,
                                            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                            GTK_STOCK_OPEN, GTK_RESPONSE_OK,
                                            NULL);

    if (inst->filename) {
        /* set current file path beginning choice */
        gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(chooser),
                                            inst->fpath);
        /* set current filename beginning choice */
        gtk_file_chooser_set_filename (GTK_FILE_CHOOSER(chooser),
                                        inst->filename);
    }

    /* set multi-select */
    gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER(chooser),
                                            TRUE);

    /* add read-only checkbox */
    read_only = gtk_check_button_new_with_label ("Open file read-only");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (read_only), app->roflag);
    gtk_widget_show (read_only);
    gtk_file_chooser_set_extra_widget (GTK_FILE_CHOOSER(chooser), read_only);

    if (gtk_dialog_run (GTK_DIALOG (chooser)) == GTK_RESPONSE_OK) {
        filenames = gtk_file_chooser_get_filenames (GTK_FILE_CHOOSER (chooser));
        app->roflag = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (read_only));
    }
    else {
        g_warning ("user canceled file-open.\n");
        // dlg_info ("user canceled file-open", "GTK_RESPONSE_CANCEL");
    }
    gtk_widget_destroy (chooser);

    return filenames;
}

gchar *get_save_filename (gpointer data)
{
    mainwin_t *app = data;
    kinst_t *inst = app->einst[app->focused]->inst;
    GtkWidget *chooser;
    gchar *filename = NULL;

    chooser = gtk_file_chooser_dialog_new ("Save File...",
                                            GTK_WINDOW (app->window),
                                            GTK_FILE_CHOOSER_ACTION_SAVE,
                                            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                            GTK_STOCK_SAVE, GTK_RESPONSE_OK,
                                            NULL);

    gtk_file_chooser_set_create_folders (GTK_FILE_CHOOSER(chooser), TRUE);
    gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER(chooser),
                                                    TRUE);
    if (inst->filename) {
        /* set current file path beginning choice */
        gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(chooser),
                                            inst->fpath);
        /* set current filename beginning choice */
        gtk_file_chooser_set_filename (GTK_FILE_CHOOSER(chooser),
                                        inst->filename);
    }

    if (gtk_dialog_run (GTK_DIALOG (chooser)) == GTK_RESPONSE_OK)
    {
        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));
    }
    gtk_widget_destroy (chooser);

    return filename;
}

