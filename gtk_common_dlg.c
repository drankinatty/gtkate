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

gchar *get_open_filename (gpointer data)
{
    mainwin_t *app = data;
    kinst_t *inst = app->einst[app->focused]->inst;
    GtkWidget *chooser;
    gchar *filename=NULL;

    chooser = gtk_file_chooser_dialog_new ("Open File...",
                                            GTK_WINDOW (app->window),
                                            GTK_FILE_CHOOSER_ACTION_OPEN,
                                            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                            GTK_STOCK_OPEN, GTK_RESPONSE_OK,
                                            NULL);

    if (inst->filename) {
#ifdef DEBUG
        g_print ("get_open_filename() app->fpath: %s\napp->filename: %s\n",
                inst->fpath, inst->filename);
#endif
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

