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
