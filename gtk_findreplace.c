// #include "gtk_windef.h"
#include "gtk_findreplace.h"

/* find replace keypress handler */
static gboolean on_fr_keypress (GtkWidget *widget, GdkEventKey *event,
                                gpointer data);

/** globals (avoid unnecessary storage in mainwin_t) */
static GtkWidget *findrepwin;   /* main dialog window */
static guint dlgid;             /* dialog ID (DLGFIND 1, DLGREPL 2) */
static gboolean findcbchgd;     /* find combo box changed */
static gboolean replcbchgd;     /* replace combo box changed */
static GtkWidget *btnfind;      /* find button (set sensitive) */
static GtkWidget *btnreplace;   /* replace button (set sensitive) */
static GtkWidget *btnregex;     /* regex button (set sensitive) */
static GtkWidget *btnplace;     /* placeholder button (set sensitive) */
static GtkWidget *entryfind;    /* find text entry */
static GtkWidget *entryreplace; /* replace text entry */

GtkWidget *create_find_dlg (gpointer data)
{
    mainwin_t *app = data;

    GtkWidget *vbox;
    GtkWidget *frame1;
    GtkWidget *frame2;
    GtkWidget *table;
    GtkWidget *table2;
    GtkWidget *label;
    GtkWidget *hbtweak;
    GtkWidget *hbox;

    GtkWidget *btnclose;

    GtkWidget *chkregex;
    GtkWidget *chkcase;
    GtkWidget *chkwhole;
    GtkWidget *chkfrom;
    GtkWidget *chkback;
    GtkWidget *chkselect;

    guint i;

    /* create toplevel window */
    if (!(findrepwin = gtk_window_new (GTK_WINDOW_TOPLEVEL))) {
        err_dialog ("create_find_dlg() gtk_window_new failure.");
        return NULL;
    }
    gtk_window_set_position (GTK_WINDOW (findrepwin), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size (GTK_WINDOW (findrepwin), 264, 264);
    gtk_window_set_title (GTK_WINDOW (findrepwin), "Find Text");
    /* for inclusion in main app set modal and transient_for (to keep on top)
     * gtk_window_set_modal (GtkWindow *window, gboolean modal);
     * gtk_window_set_transient_for(GtkWindow *window, GtkWindow *parent)
     */
    gtk_window_set_modal (GTK_WINDOW(findrepwin), TRUE);
    gtk_window_set_transient_for (GTK_WINDOW(findrepwin),
                                    GTK_WINDOW(app->window));
    gtk_container_set_border_width (GTK_CONTAINER (findrepwin), 5);
    g_signal_connect (findrepwin, "destroy",
		      G_CALLBACK (btnclose_activate), app);

    dlgid = DLGFIND;    /* set dialog id to find dialog */
    findcbchgd = FALSE; /* initialize changed to FALSE */

    /* main vbox container
     * spacing profided on frames as containers
     */
    vbox = gtk_vbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (findrepwin), vbox);
    gtk_widget_show (vbox);

    /* frame1 - Search Box */
    frame1 = gtk_frame_new (NULL);
    gtk_frame_set_label (GTK_FRAME (frame1), "Find");
    gtk_frame_set_label_align (GTK_FRAME (frame1), 0.0, 0.5);
    gtk_frame_set_shadow_type (GTK_FRAME (frame1), GTK_SHADOW_ETCHED_OUT);
    gtk_container_set_border_width (GTK_CONTAINER (frame1), 5);

    /* table inside frame1 */
    table = gtk_table_new (3, 2, TRUE);
    gtk_table_set_row_spacings (GTK_TABLE (table), 5);
    gtk_table_set_col_spacings (GTK_TABLE (table), 3);
    gtk_container_set_border_width (GTK_CONTAINER (table), 5);
    gtk_container_add (GTK_CONTAINER (frame1), table);

    /* label for combo entry */
    label = gtk_label_new_with_mnemonic ("_Text to Find:");
    hbtweak = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbtweak), label, FALSE, FALSE, 0);
    gtk_table_attach_defaults (GTK_TABLE (table), hbtweak, 0, 1, 0, 1);
    gtk_widget_show (hbtweak);
    gtk_table_set_row_spacing (GTK_TABLE (table), 0, 0);

    /* combo box entry GTK_COMBO_BOX_TEXT */
    entryfind = gtk_combo_box_text_new_with_entry();
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), entryfind);
    gtk_table_attach_defaults (GTK_TABLE (table), entryfind, 0, 2, 1, 2);
    gtk_table_set_row_spacing (GTK_TABLE (table), 1, 10);
    for (i = 0; i < app->nfentries; i++)
        gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(entryfind),
                                        app->findtext[i]);

    /* regex checkbox */
    chkregex = gtk_check_button_new_with_mnemonic ("Regular e_xpression");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkregex), app->optregex);
    gtk_table_attach_defaults (GTK_TABLE (table), chkregex, 0, 1, 2, 3);

    /* regex edit button */
    btnregex = gtk_button_new_with_mnemonic ("_Edit...");
    gtk_widget_set_size_request (btnregex, 80, 24);
    gtk_widget_set_sensitive (btnregex, app->optregex);
    hbtweak = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_end (GTK_BOX (hbtweak), btnregex, FALSE, FALSE, 0);
    gtk_table_attach_defaults (GTK_TABLE (table), hbtweak, 1, 2, 2, 3);

    // gtk_container_add (GTK_CONTAINER (frame1), app->entryfind);
    gtk_widget_show (entryfind);
    gtk_widget_show (label);
    gtk_widget_show (hbtweak);
    gtk_widget_show (chkregex);
    gtk_widget_show (btnregex);
    gtk_widget_show (table);
    gtk_widget_show (frame1);

    gtk_box_pack_start (GTK_BOX (vbox), frame1, FALSE, FALSE, 0);

    /* frame2 - Options */
    frame2 = gtk_frame_new (NULL);
    gtk_frame_set_label (GTK_FRAME (frame2), "Options");
    gtk_frame_set_label_align (GTK_FRAME (frame2), 0.0, 0.5);
    gtk_frame_set_shadow_type (GTK_FRAME (frame2), GTK_SHADOW_ETCHED_OUT);
    gtk_container_set_border_width (GTK_CONTAINER (frame2), 5);
    gtk_widget_show (frame2);

    gtk_box_pack_start (GTK_BOX (vbox), frame2, FALSE, FALSE, 0);

    /* table in frame2 */
    table2 = gtk_table_new (3, 2, TRUE);
    gtk_table_set_row_spacings (GTK_TABLE (table2), 5);
    gtk_table_set_col_spacings (GTK_TABLE (table2), 3);
    gtk_container_set_border_width (GTK_CONTAINER (table2), 5);
    gtk_container_add (GTK_CONTAINER (frame2), table2);

    /* options checkboxs */
    chkcase = gtk_check_button_new_with_mnemonic ("C_ase sensitive");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkcase), app->optcase);
    gtk_table_attach_defaults (GTK_TABLE (table2), chkcase, 0, 1, 0, 1);
    gtk_widget_show (chkcase);

    chkwhole = gtk_check_button_new_with_mnemonic ("_Whole words only");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkwhole), app->optwhole);
    gtk_table_attach_defaults (GTK_TABLE (table2), chkwhole, 0, 1, 1, 2);
    gtk_widget_show (chkwhole);

    chkfrom = gtk_check_button_new_with_mnemonic ("From c_ursor");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkfrom), app->optfrom);
    gtk_table_attach_defaults (GTK_TABLE (table2), chkfrom, 0, 1, 2, 3);
    gtk_widget_show (chkfrom);

    chkback = gtk_check_button_new_with_mnemonic ("Find _backwards");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkback), app->optback);
    gtk_table_attach_defaults (GTK_TABLE (table2), chkback, 1, 2, 0, 1);
    gtk_widget_show (chkback);

    chk_existing_selection (app);   /* sets app->optselect, create marks */
    chkselect = gtk_check_button_new_with_mnemonic ("_Selected text");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkselect), app->optselect);
    // gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkselect), TRUE);
    gtk_table_attach_defaults (GTK_TABLE (table2), chkselect, 1, 2, 1, 2);
    gtk_widget_show (chkselect);

    gtk_widget_show (table2);

    /* hbox (replacing table) button spacing */
    hbox = gtk_hbox_new (FALSE, 0);
    gtk_box_set_spacing (GTK_BOX (hbox), 5);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);

    /* button sizes 80 x 24 */
    btnfind = gtk_button_new_with_mnemonic ("_Find");
    btnfind = btnfind;     /* FIXME - remove btnfind? */
    gtk_widget_set_size_request (btnfind, 80, 24);
    // gtk_box_pack_end (GTK_BOX (hbox), btnfind, FALSE, FALSE, 0); // (moved after btnclose)
    gtk_widget_set_sensitive (btnfind, findcbchgd);
    gtk_widget_show (btnfind);

    btnclose = gtk_button_new_with_mnemonic ("_Close");
    gtk_widget_set_size_request (btnclose, 80, 24);

    gtk_box_pack_end (GTK_BOX (hbox), btnclose, FALSE, FALSE, 0);
    gtk_box_pack_end (GTK_BOX (hbox), btnfind, FALSE, FALSE, 0); // pack on rt-side.
    gtk_widget_show (btnclose);

    gtk_box_pack_end (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
    gtk_widget_show (hbox);

    g_signal_connect (findrepwin, "show",
                      G_CALLBACK (on_window_show), app);

    g_signal_connect (chkregex, "toggled",
                      G_CALLBACK (chkregex_toggled), app);

    g_signal_connect (chkcase, "toggled",
                      G_CALLBACK (chkcase_toggled), app);

    g_signal_connect (chkwhole, "toggled",
                      G_CALLBACK (chkwhole_toggled), app);

    g_signal_connect (chkfrom, "toggled",
                      G_CALLBACK (chkfrom_toggled), app);

    g_signal_connect (chkback, "toggled",
                      G_CALLBACK (chkback_toggled), app);

    g_signal_connect (chkselect, "toggled",
                      G_CALLBACK (chkselect_toggled), app);

    g_signal_connect (btnfind, "clicked",
                      G_CALLBACK (btnfind_activate), app);

    g_signal_connect (btnclose, "clicked",
                      G_CALLBACK (btnclose_activate), app);

    g_signal_connect (findrepwin, "key_press_event",
                      G_CALLBACK (on_fr_keypress), app);

    g_signal_connect (entryfind, "changed",
                      G_CALLBACK (entry_find_activate), app);

    // g_signal_connect_swapped (btnclose, "clicked",
    //                           G_CALLBACK (delete_event),
    //                           window);
    gtk_widget_show (findrepwin);
    // gtk_widget_show_all (findrepwin);

    return (findrepwin);
}

GtkWidget *create_replace_dlg (gpointer data)
{
    mainwin_t *app = data;

    GtkWidget *vbox;            /* vbox container   */
    GtkWidget *frame1;
    GtkWidget *framer;
    GtkWidget *frame2;
    GtkWidget *table;
    GtkWidget *table2;
    GtkWidget *label;
    GtkWidget *hbtweak;
    GtkWidget *hbox;

    GtkWidget *btnclose;

    GtkWidget *chkregex;
    GtkWidget *chkplace;
    GtkWidget *chkcase;
    GtkWidget *chkwhole;
    GtkWidget *chkfrom;
    GtkWidget *chkback;
    GtkWidget *chkselect;
    GtkWidget *chkprompt;

    guint i;

    /* create toplevel window */
    if (!(findrepwin = gtk_window_new (GTK_WINDOW_TOPLEVEL))) {
        err_dialog ("create_replace_dlg() gtk_window_new failure.");
        return NULL;
    }
    gtk_window_set_position (GTK_WINDOW (findrepwin), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size (GTK_WINDOW (findrepwin), 264, 264);
    gtk_window_set_title (GTK_WINDOW (findrepwin), "Replace Text");
    gtk_window_set_modal (GTK_WINDOW(findrepwin), TRUE);
    gtk_window_set_transient_for (GTK_WINDOW(findrepwin),
                                    GTK_WINDOW(app->window));
    gtk_container_set_border_width (GTK_CONTAINER (findrepwin), 5);
    g_signal_connect (findrepwin, "destroy",
		      G_CALLBACK (btnclose_activate), app);

    dlgid = DLGREPL;    /* set dialog id to replace dialog */
    findcbchgd = FALSE; /* initialize combobox changed to FALSE */
    replcbchgd = FALSE; /* initialize combobox changed to FALSE */

    /* main vbox container
     * spacing profided on frames as containers
     */
    vbox = gtk_vbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (findrepwin), vbox);
    gtk_widget_show (vbox);

    /* frame1 - Search Box */
    frame1 = gtk_frame_new (NULL);
    gtk_frame_set_label (GTK_FRAME (frame1), "Find");
    gtk_frame_set_label_align (GTK_FRAME (frame1), 0.0, 0.5);
    gtk_frame_set_shadow_type (GTK_FRAME (frame1), GTK_SHADOW_ETCHED_OUT);
    gtk_container_set_border_width (GTK_CONTAINER (frame1), 5);

    /* table inside frame1 */
    table = gtk_table_new (3, 2, TRUE);
    gtk_table_set_row_spacings (GTK_TABLE (table), 5);
    gtk_table_set_col_spacings (GTK_TABLE (table), 3);
    gtk_container_set_border_width (GTK_CONTAINER (table), 5);
    gtk_container_add (GTK_CONTAINER (frame1), table);

    /* label for combo entry */
    label = gtk_label_new_with_mnemonic ("_Text to find:");
    hbtweak = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbtweak), label, FALSE, FALSE, 0);
    gtk_table_attach_defaults (GTK_TABLE (table), hbtweak, 0, 1, 0, 1);
    gtk_widget_show (hbtweak);
    gtk_table_set_row_spacing (GTK_TABLE (table), 0, 0);

    /* combo box entry (GtkComboBoxText *) */
    entryfind = gtk_combo_box_text_new_with_entry();
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), entryfind);
    gtk_table_attach_defaults (GTK_TABLE (table), entryfind, 0, 2, 1, 2);
    gtk_table_set_row_spacing (GTK_TABLE (table), 1, 10);
    for (i = 0; i < app->nfentries; i++)
        gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(entryfind),
                                        app->findtext[i]);

    /* regex checkbox */
    chkregex = gtk_check_button_new_with_mnemonic ("Regular e_xpression");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkregex), app->optregex);
    gtk_table_attach_defaults (GTK_TABLE (table), chkregex, 0, 1, 2, 3);

    /* regex edit button - set active state FALSE - gray Edit... button */
    btnregex = gtk_button_new_with_mnemonic ("_Edit...");
    gtk_widget_set_size_request (btnregex, 80, 24);
    if (!app->optregex)
        gtk_widget_set_sensitive (btnregex, FALSE);
    hbtweak = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_end (GTK_BOX (hbtweak), btnregex, FALSE, FALSE, 0);
    gtk_table_attach_defaults (GTK_TABLE (table), hbtweak, 1, 2, 2, 3);

    // gtk_container_add (GTK_CONTAINER (frame1), entryfind);
    gtk_widget_show (entryfind);
    gtk_widget_show (label);
    gtk_widget_show (hbtweak);
    gtk_widget_show (chkregex);
    gtk_widget_show (btnregex);
    gtk_widget_show (table);
    gtk_widget_show (frame1);

    gtk_box_pack_start (GTK_BOX (vbox), frame1, FALSE, FALSE, 0);

    /* framer - Replace Box */
    framer = gtk_frame_new (NULL);
    gtk_frame_set_label (GTK_FRAME (framer), "Replace With");
    gtk_frame_set_label_align (GTK_FRAME (framer), 0.0, 0.5);
    gtk_frame_set_shadow_type (GTK_FRAME (framer), GTK_SHADOW_ETCHED_OUT);
    gtk_container_set_border_width (GTK_CONTAINER (framer), 5);

    /* table inside framer */
    table = gtk_table_new (3, 2, TRUE);
    gtk_table_set_row_spacings (GTK_TABLE (table), 5);
    gtk_table_set_col_spacings (GTK_TABLE (table), 3);
    gtk_container_set_border_width (GTK_CONTAINER (table), 5);
    gtk_container_add (GTK_CONTAINER (framer), table);

    /* label for combo entry */
    label = gtk_label_new_with_mnemonic ("Replace_ment text:");
    hbtweak = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbtweak), label, FALSE, FALSE, 0);
    gtk_table_attach_defaults (GTK_TABLE (table), hbtweak, 0, 1, 0, 1);
    gtk_widget_show (hbtweak);
    gtk_table_set_row_spacing (GTK_TABLE (table), 0, 0);

    /* combo box entry (GtkComboBoxText *) */
    entryreplace = gtk_combo_box_text_new_with_entry();
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), entryreplace);
    gtk_table_attach_defaults (GTK_TABLE (table), entryreplace, 0, 2, 1, 2);
    gtk_table_set_row_spacing (GTK_TABLE (table), 1, 10);
    for (i = 0; i < app->nrentries; i++)
        gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(entryreplace),
                                        app->reptext[i]);

    /* regex checkbox */
    chkplace = gtk_check_button_new_with_mnemonic ("Use p_laceholders");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkplace), app->optplace);
    gtk_table_attach_defaults (GTK_TABLE (table), chkplace, 0, 1, 2, 3);

    /* regex edit button */
    btnplace = gtk_button_new_with_mnemonic ("Insert Place_holder");
    gtk_widget_set_size_request (btnplace, 110, 24);
    if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (chkplace)))
        gtk_widget_set_sensitive (btnplace, FALSE);
    hbtweak = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_end (GTK_BOX (hbtweak), btnplace, FALSE, FALSE, 0);
    gtk_table_attach_defaults (GTK_TABLE (table), hbtweak, 1, 2, 2, 3);

    // gtk_container_add (GTK_CONTAINER (framer), entryreplace);
    gtk_widget_show (entryreplace);
    gtk_widget_show (label);
    gtk_widget_show (hbtweak);
    gtk_widget_show (chkplace);
    gtk_widget_show (btnplace);
    gtk_widget_show (table);
    gtk_widget_show (framer);

    gtk_box_pack_start (GTK_BOX (vbox), framer, FALSE, FALSE, 0);

    /* frame2 - Options */
    frame2 = gtk_frame_new (NULL);
    gtk_frame_set_label (GTK_FRAME (frame2), "Options");
    gtk_frame_set_label_align (GTK_FRAME (frame2), 0.0, 0.5);
    gtk_frame_set_shadow_type (GTK_FRAME (frame2), GTK_SHADOW_ETCHED_OUT);
    gtk_container_set_border_width (GTK_CONTAINER (frame2), 5);
    gtk_widget_show (frame2);

    gtk_box_pack_start (GTK_BOX (vbox), frame2, FALSE, FALSE, 0);

    /* table in frame2 */
    table2 = gtk_table_new (3, 2, TRUE);
    gtk_table_set_row_spacings (GTK_TABLE (table2), 5);
    gtk_table_set_col_spacings (GTK_TABLE (table2), 3);
    gtk_container_set_border_width (GTK_CONTAINER (table2), 5);
    gtk_container_add (GTK_CONTAINER (frame2), table2);

    /* options checkboxs */
    chkcase = gtk_check_button_new_with_mnemonic ("C_ase sensitive");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkcase), app->optcase);
    gtk_table_attach_defaults (GTK_TABLE (table2), chkcase, 0, 1, 0, 1);
    gtk_widget_show (chkcase);

    chkwhole = gtk_check_button_new_with_mnemonic ("_Whole words only");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkwhole), app->optwhole);
    gtk_table_attach_defaults (GTK_TABLE (table2), chkwhole, 0, 1, 1, 2);
    gtk_widget_show (chkwhole);

    chkfrom = gtk_check_button_new_with_mnemonic ("From c_ursor");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkfrom), app->optfrom);
    gtk_table_attach_defaults (GTK_TABLE (table2), chkfrom, 0, 1, 2, 3);
    gtk_widget_show (chkfrom);

    chkback = gtk_check_button_new_with_mnemonic ("Find _backwards");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkback), app->optback);
    gtk_table_attach_defaults (GTK_TABLE (table2), chkback, 1, 2, 0, 1);
    gtk_widget_show (chkback);

    chk_existing_selection (app);   /* sets app->optselect, create marks */
    chkselect = gtk_check_button_new_with_mnemonic ("_Selected text");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkselect), app->optselect);
    gtk_table_attach_defaults (GTK_TABLE (table2), chkselect, 1, 2, 1, 2);
    gtk_widget_show (chkselect);

    chkprompt = gtk_check_button_new_with_mnemonic ("_Prompt on replace");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkprompt), app->optprompt);
    gtk_table_attach_defaults (GTK_TABLE (table2), chkprompt, 1, 2, 2, 3);
    gtk_widget_show (chkprompt);

    gtk_widget_show (table2);

    /* hbox (replacing table) button spacing */
    hbox = gtk_hbox_new (FALSE, 0);
    gtk_box_set_spacing (GTK_BOX (hbox), 5);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);

    btnreplace = gtk_button_new_with_mnemonic ("_Replace");
    btnreplace = btnreplace;   /* FIXME - remove btnreplace? */
    gtk_widget_set_size_request (btnreplace, 80, 24);
    // gtk_box_pack_end (GTK_BOX (hbox), btnreplace, FALSE, FALSE, 0); // (moved after btnclose)
    gtk_widget_set_sensitive (btnreplace, findcbchgd);
    gtk_widget_show (btnreplace);

    btnclose = gtk_button_new_with_mnemonic ("_Close");
    gtk_widget_set_size_request (btnclose, 80, 24);

    gtk_box_pack_end (GTK_BOX (hbox), btnclose, FALSE, FALSE, 0);
    gtk_box_pack_end (GTK_BOX (hbox), btnreplace, FALSE, FALSE, 0); // pack on rt-side.
    gtk_widget_show (btnclose);

    gtk_box_pack_end (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
    gtk_widget_show (hbox);

    g_signal_connect (findrepwin, "show",
                      G_CALLBACK (on_window_show), app);

    g_signal_connect (chkregex, "toggled",
                      G_CALLBACK (chkregex_toggled), app);

    g_signal_connect (chkplace, "toggled",
                      G_CALLBACK (chkplace_toggled), app);

    g_signal_connect (chkcase, "toggled",
                      G_CALLBACK (chkcase_toggled), app);

    g_signal_connect (chkwhole, "toggled",
                      G_CALLBACK (chkwhole_toggled), app);

    g_signal_connect (chkfrom, "toggled",
                      G_CALLBACK (chkfrom_toggled), app);

    g_signal_connect (chkback, "toggled",
                      G_CALLBACK (chkback_toggled), app);

    g_signal_connect (chkselect, "toggled",
                      G_CALLBACK (chkselect_toggled), app);

    g_signal_connect (chkprompt, "toggled",
                      G_CALLBACK (chkprompt_toggled), app);

    g_signal_connect (btnreplace, "clicked",
                      G_CALLBACK (btnreplace_activate), app);

    g_signal_connect (btnclose, "clicked",
                      G_CALLBACK (btnclose_activate), app);

    /* pass keypress to handler */
    g_signal_connect (findrepwin, "key_press_event",
                      G_CALLBACK (on_fr_keypress), app);

    /* set replace button sensitive, require entries in both */
    g_signal_connect (entryfind, "changed",
                      G_CALLBACK (entry_set_find_sensitive), app);

    g_signal_connect (entryfind, "changed",
                      G_CALLBACK (entry_replace_activate), app);

    g_signal_connect (entryreplace, "changed",
                      G_CALLBACK (entry_set_repl_sensitive), app);

    g_signal_connect (entryreplace, "changed",
                      G_CALLBACK (entry_replace_activate), app);

    gtk_widget_show (findrepwin);
    // gtk_widget_show_all (findrepwin);

    return (findrepwin);
}

/** on window create/show function */
void on_window_show (GtkWidget *widget, gpointer data)
{
    mainwin_t *app = data;
    kinst_t *inst = app->einst[app->focused]->inst;

    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(inst->buf);
    GtkTextIter iter;

    if (app->markfrom) {    /* free reset existing mark */
        gtk_text_buffer_delete_mark (buffer, app->markfrom);
        app->markfrom = NULL;
    }

    /* get iter at insert, create app->markfrom text mark */
    gtk_text_buffer_get_iter_at_mark (buffer, &iter,
                            gtk_text_buffer_get_insert (buffer));
    app->markfrom = gtk_text_buffer_create_mark (buffer, "markfrom",
                                                &iter, FALSE);
    if (!app->markfrom)
    g_error ("setting 'markfrom' - FAILED!\n");

    if (widget) {}
}

/* entry helper callbacks for replace button set sensitive */
void entry_set_find_sensitive (GtkWidget *widget, gpointer data) {

    findcbchgd = TRUE;
    if (widget || data) {}
}

void entry_set_repl_sensitive (GtkWidget *widget, gpointer data) {

    replcbchgd = TRUE;
    if (widget || data) {}
}

/* entry comboboxes */
void entry_find_activate (GtkWidget *widget, gpointer data) {

    findcbchgd = TRUE; /* just set, or add set callback to find dlg */
                            /* and change to if (findcbchgd) */
    gtk_widget_set_sensitive (btnfind, findcbchgd);

    if (widget || data) {}
}

void entry_replace_activate (GtkWidget *widget, gpointer data) {

    if (findcbchgd && replcbchgd) /* require both for replace */
        gtk_widget_set_sensitive (btnreplace, findcbchgd);

    if (widget || data) {}
}

/* option checkboxs */
void chkregex_toggled    (GtkWidget *widget, gpointer data)
{
    mainwin_t *app = data;

    app->optregex = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
    gtk_widget_set_sensitive (btnregex, app->optregex);
}

void chkplace_toggled   (GtkWidget *widget, gpointer data)
{
    mainwin_t *app = data;

    app->optplace = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
    gtk_widget_set_sensitive (btnplace, app->optplace);
}

void chkcase_toggled    (GtkWidget *widget, gpointer data)
{
    mainwin_t *app = data;

    app->optcase = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
}

void chkwhole_toggled   (GtkWidget *widget, gpointer data)
{
    mainwin_t *app = data;

    app->optwhole = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
}

void chkfrom_toggled    (GtkWidget *widget, gpointer data)
{
    mainwin_t *app = data;

    app->optfrom = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
}

void chkback_toggled    (GtkWidget *widget, gpointer data)
{
    mainwin_t *app = data;

    app->optback = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
}

void chkselect_toggled  (GtkWidget *widget, gpointer data)
{
    mainwin_t *app = data;

    app->optselect = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
}

void chkprompt_toggled  (GtkWidget *widget, gpointer data)
{
    mainwin_t *app = data;

    app->optprompt = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
}

/* dialog buttons */
void btnregex_activate   (GtkWidget *widget, gpointer data)
{
    if (widget || data) {}
}

void btnplace_activate   (GtkWidget *widget, gpointer data)
{
    if (widget || data) {}
}

/** check if selection exists when find/replace dlg called,
 *  create marks for beginning and end to restore after
 *  find/replace operations
 */
void chk_existing_selection (gpointer data)
{
    mainwin_t *app = data;
    kinst_t *inst = app->einst[app->focused]->inst;

    GtkTextIter sel_start, sel_end;
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(inst->buf);
    gboolean selected = FALSE;

    /* check whether existing selection active */
    selected = gtk_text_buffer_get_selection_bounds (buffer,
                                            &sel_start, &sel_end);

    if (selected && !gtk_text_iter_equal (&sel_start, &sel_end)) {
        app->optselect = TRUE;
        app->selstart = gtk_text_buffer_create_mark (buffer,
                                    "selstart", &sel_start, FALSE);
        app->selend = gtk_text_buffer_create_mark (buffer,
                                    "selend", &sel_end, FALSE);
    }
    else {
        app->optselect = FALSE;
        app->selstart = app->selend = NULL;
    }
}

/** case insensitive forward search for implementation without
 *  GtkSourceView.
 */
gboolean gtk_text_iter_forward_search_nocase (GtkTextIter *iter,
                                            const gchar *text,
                                            GtkTextSearchFlags flags,
                                            GtkTextIter *mstart,
                                            GtkTextIter *mend)
{
    gunichar c;
    gchar *lctext = g_strdup (text);   /* copy text */
    gsize textlen = g_strlen (text);   /* get length */

    str2lower (lctext);

    for (;;) {              /* iterate over all chars in range */

        gsize len = textlen;               /* get char at iter */
        c = g_unichar_tolower (gtk_text_iter_get_char (iter));

        if (c == (gunichar)lctext[0]) /* compare 1st in lctext */
        {
            *mstart = *iter;      /* set start iter to current */

            for (gsize i = 0; i < len; i++)
            {
                c = g_unichar_tolower (gtk_text_iter_get_char (iter));

                /* compare/advance -- order IS important */
                if (c != (gunichar)lctext[i] ||
                    !gtk_text_iter_forward_char (iter))
                    goto next;              /* start next search */
            }
            *mend = *iter;                  /* set end iter */
            if (lctext) g_free (lctext);    /* free lctext  */
            return TRUE;                    /* return true  */
        }
        next:;  /* if at end of selecton break */
        if (!gtk_text_iter_forward_char (iter))
            break;
    }
    if (lctext) g_free (lctext);    /* free lctext */

    if (mstart || mend || flags) {}

    return FALSE;
}

/** case insensitive backward search for implementation without
 *  GtkSourceView.
 */
gboolean gtk_text_iter_backward_search_nocase (GtkTextIter *iter,
                                            const gchar *text,
                                            GtkTextSearchFlags flags,
                                            GtkTextIter *mstart,
                                            GtkTextIter *mend)
{
    gunichar c;
    gchar *lctext = g_strdup (text);   /* copy text */
    gsize textlen = g_strlen (text);   /* get length */

    str2lower (lctext);     /* convert to lower-case */
    *mend = *iter;        /* initialize end iterator */

    while (gtk_text_iter_backward_char (iter)) {

        gsize len = textlen - 1;  /* index for last in lctext */
        c = g_unichar_tolower (gtk_text_iter_get_char (iter));

        if (c == (gunichar)lctext[len]) /* initial comparison */
        {
            /* iterate over remaining chars in lctext/compare */
            while (len-- && gtk_text_iter_backward_char (iter))
            {
                c = g_unichar_tolower (gtk_text_iter_get_char (iter));

                if (c != (gunichar)lctext[len]) {
                    /* reset iter to right of char */
                    gtk_text_iter_forward_char (iter);
                    goto prev;
                }
            }
            *mstart = *iter; /* set start iter before last char */
            if (lctext) g_free (lctext);        /* free lctext */
            return TRUE;                    /* return success */
        }
        prev:;
        *mend = *iter;   /* set end iter after next search char */
    }
    if (lctext) g_free (lctext);    /* free lctext */

    if (mstart || mend || flags) {}

    return FALSE;   /* no match */
}

/** common find function for both find/replace dialogs, locates text
 *  within app->buffer and sets app->last_pos for next search begin,
 *  test match against dialog options, sets app->txtfound on success.
 */
void find (gpointer data, const gchar *text)
{
    if (!text || !*text) return;

    mainwin_t *app = data;
    einst_t *einst = app->einst[app->focused];
    kinst_t *inst = einst->inst;

    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(inst->buf);
    GtkTextIter iter, mstart, mend;
    gboolean found = FALSE, wrapped = FALSE;

    /* start infinite loop here, loop until all options satisfied or end
     * of buffer reached, then break setting app->txtfound as needed.
     * TODO: handle if existing selection was made from end->start, requires
     * checking iter from gtk_text_buffer_get_insert and selstart/selend.
     * TODO: activate find/replace on [enter]/[kp_return] from findtext and
     * replacetext combo box entry.
     * TODO: F3 - btnfind_activate (think about Prompt on Replace)
     ^ TODO: case insensitive doesn't keep last whole-word highlighted if
     * subsequent search does not match.
     */
    for (;;) {  /* loop until word found matching search options or end */

        // gtk_text_buffer_begin_user_action (GTK_TEXT_BUFFER(buffer));

        if (!app->last_pos) {                /* find first occurrence  */
            if (wrapped) {                  /* search again from start */
                if (!app->optback) {       /* search again forward dir */
                    if (app->optselect) { /* search again in selection */
                        gtk_text_buffer_get_iter_at_mark (buffer, &iter,
                                                        app->selstart);
                    }
                    else if (app->optfrom) { /* search again at cursor */
                        gtk_text_buffer_get_iter_at_mark (buffer, &iter,
                                                        app->markfrom);
                    }
                    else {    /* search again from beginning of buffer */
                        gtk_text_buffer_get_start_iter (buffer, &iter);
                    }
                }
                else {             /* search again backward directions */
                    if (app->optselect) {    /* search again selection */
                        gtk_text_buffer_get_iter_at_mark (buffer, &iter,
                                                        app->selend);
                    }
                    else if (app->optfrom) {    /* search again cursor */
                        gtk_text_buffer_get_iter_at_mark (buffer, &iter,
                                                        app->markfrom);
                    }
                    else {          /* search again from end of buffer */
                        gtk_text_buffer_get_end_iter (buffer, &iter);
                    }
                }
                wrapped = FALSE;
            }
            else if (!app->optback) {               /* forward search  */
                if (app->optfrom) {           /* if search from cursor */
                    gtk_text_buffer_get_iter_at_mark (buffer, &iter,
                                    gtk_text_buffer_get_insert (buffer));
                }
                else if (app->optselect)   /* search from select start */
                    gtk_text_buffer_get_iter_at_mark (buffer, &iter,
                                                        app->selstart);
                else                       /* otherwise get start iter */
                    gtk_text_buffer_get_start_iter (buffer, &iter);
            }
            else {                                 /* backwards search */
                if (app->optfrom)                /* get iter at insert */
                    gtk_text_buffer_get_iter_at_mark (buffer, &iter,
                                    gtk_text_buffer_get_insert (buffer));
                else if (app->optselect)         /* get iter at selend */
                    gtk_text_buffer_get_iter_at_mark (buffer, &iter,
                                                        app->selend);
                else                         /* start at end of buffer */
                    gtk_text_buffer_get_end_iter (buffer, &iter);
            }
        }
        else                                /* find next occurrence  */
            gtk_text_buffer_get_iter_at_mark (buffer, &iter, app->last_pos);

        /* case sensitive forward/reverse search from iter for 'text' setting
         * iters mstart & mend pointing to first & last char in matched text.
         */
        if (app->optback) {     /* search backward */
            if (app->optcase) { /* case sensitive  */
                found = gtk_text_iter_backward_search (&iter, text, 0,
                                                    &mstart, &mend, NULL);
            }
            else {  /* case insensitive backwards search */
                found = gtk_source_iter_backward_search (&iter, text,
                                            GTK_SOURCE_SEARCH_CASE_INSENSITIVE,
                                            &mstart, &mend, NULL);
            }
        }
        else {                  /* search forward */
            if (app->optcase) { /* case sensitive  */
                found = gtk_text_iter_forward_search (&iter, text, 0,
                                                    &mstart, &mend, NULL);
            }
            else {  /* case insensitive forward search */
                found = gtk_source_iter_forward_search (&iter, text,
                                            GTK_SOURCE_SEARCH_CASE_INSENSITIVE,
                                            &mstart, &mend, NULL);
            }
        }

        if (found)  /* text found in buffer, now refine match w/options */
        {
            /* if optselect, in both forward/backward search check iter
             * against mend/mstart to determine if search is within
             * original selection range.
             */
            if (app->optselect) {   /* if srch in selected-text */
                GtkTextIter iterx;
                if (app->optback) { /* if searching backwards */
                    gtk_text_buffer_get_iter_at_mark (buffer, &iterx,
                                                        app->selstart);
                    if (gtk_text_iter_compare (&mstart, &iterx) < 0 ) {
                        delete_mark_last_pos (app);
                        app->txtfound = FALSE;
                        /* handle search reached beginning of selection */
                        // status_update_str (app, "Search reached beginning of selection...");
                        return; /* or btnclose_activate (NULL, app); */
                    }
                }
                else {  /* for forward search in selection */
                    gtk_text_buffer_get_iter_at_mark (buffer, &iterx,
                                                        app->selend);
                    if (gtk_text_iter_compare (&iterx, &mend) < 0 ) {
                        delete_mark_last_pos (app);
                        app->txtfound = FALSE;
                        /* handle search reached end of selection */
                        // status_update_str (app, "Search reached end of selection...");
                        return; /* or btnclose_activate (NULL, app); */
                    }
                }
            }   /* TODO: restore original selection if no find in range */

            /* create or move last_pos GtkTextMark */
            if (app->last_pos == NULL)
                app->last_pos = gtk_text_buffer_create_mark (buffer, "last_pos",
                                          app->optback ? &mstart : &mend, FALSE);
            else
                gtk_text_buffer_move_mark (buffer, app->last_pos,
                                           app->optback ? &mstart : &mend);

            /* flags checking whether match is at word start/end */
            gboolean word_start = gtk_text_iter_starts_word (&mstart),
                    word_end = gtk_text_iter_ends_word (&mend);

            /* handle whole-word search option */
            if (app->optwhole) {
                if (!word_start || !word_end) { /* not word start/end */
                    /* place cursor at end to cancel select-highlight */
                    gtk_text_buffer_place_cursor (buffer, &mend);
                    continue;   /* find next text match */
                }
            }

            app->txtfound = TRUE;   /* match found satisfying options */

            /* select range, setting start, end iters, last_pos mark */
            gtk_text_buffer_select_range (buffer, &mstart, &mend);

            /* scroll window to mark to insure match is visible */
#ifndef TOMARK
            gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (einst->view),
                                        app->last_pos, 0.0, TRUE, 0.95, 0.8);
#else
            gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW (einst->view),
                                                app->last_pos);
#endif
            break;
        }
        else {  /* 'text' not found in buffer */

            /* there was a previous find in text, and search was from cursor
             * or confined to a selection
             */
            if (app->txtfound) {
                app->txtfound = FALSE;

                /* TODO: closing dialog removes selection depending on
                 * how fast the user presses enter or clicks OK. find
                 * solution for timing issue (I've seen posts on this )
                 */
                if (app->optselect || app->optfrom) {
                    if (dlg_yes_no_msg (app, "Search reached end of range.\n\n"
                                        "Continue search from beginning?",
                                        "Search Term Not Found",
                                        FALSE)) {

                        /* reset last position */
                        delete_mark_last_pos (app);
                        wrapped = TRUE;
                        goto wrapsrch;
                    }
                    else    /* close the dialog */
                        btnclose_activate (NULL, app);
                }
                else {  /* search from beginning or end */
                    /* reset last position */
                    delete_mark_last_pos (app);
                    // status_update_str (app, "No further matches in search area...");
                }
            }
            else {  /* no match ever found */

                // status_update_str (app, "String not found in search area...");
                // dlg_info ("No text matched within search.",
                //             "Search Term Not Found");
            }

            break;

            wrapsrch:;
        }
        // gtk_text_buffer_end_user_action (buffer);
    }
}

void btnfind_activate (GtkWidget *widget, gpointer data)
{
    mainwin_t *app = data;

    /* TODO: call chk_existing_selection to set selstart and selend
     * text marks to be used on restoring the selection bounds at the end
     * of the function. This will facilitate further searches or replacements
     * without having to set selection bounds manually. So the same for the
     * replace dialog. Just a single function taking "app" as a parameter so
     * it holds the selstart and selend GktTextMarks. Make sure the set
     * selection bounds is the last possible call so that nothing else can
     * occur in the interim to dismiss the selection.
     */
    guint i;

    /* get find & replace entries */
    gchar *findtext    = gtk_combo_box_text_get_active_text (
                            GTK_COMBO_BOX_TEXT(entryfind));

    /* add new terms to find/replace lists (no dups) */
    for (i = 0; i < app->nfentries; i++)
        if (g_strcmp0 (app->findtext[i], findtext) == 0) goto fdup;

    /* add to array of entries  & increment indexes */
    app->findtext[app->nfentries++] = findtext;
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(entryfind), findtext);

    chk_realloc_ent (app);  /* check/realloc find/rep text */

  fdup:

    find (data, findtext);

    if (widget) {}
}

void btnreplace_activate (GtkWidget *widget, gpointer data)
{
    mainwin_t *app = data;
    kinst_t *inst = app->einst[app->focused]->inst;

    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(inst->buf);
    guint i;

    /* get find & replace entries */
    gchar *findtext    = gtk_combo_box_text_get_active_text (
                            GTK_COMBO_BOX_TEXT(entryfind));
    gchar *replacetext = gtk_combo_box_text_get_active_text (
                            GTK_COMBO_BOX_TEXT(entryreplace));

    /* add new terms to find/replace lists (no dups) */
    for (i = 0; i < app->nfentries; i++)
        if (g_strcmp0 (app->findtext[i], findtext) == 0) goto fdup;

    /* add to array of entries  & increment indexes */
    app->findtext[app->nfentries++] = findtext;
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(entryfind), findtext);

  fdup:
    for (i = 0; i < app->nrentries; i++)
        if (g_strcmp0 (app->reptext[i], replacetext) == 0) goto rdup;
    /* add to array of entries  & increment indexes */
    app->reptext[app->nrentries++] = replacetext;
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(entryreplace), replacetext);

  rdup:

    chk_realloc_ent (app);  /* check/realloc find/rep text */

    if (app->optprompt) {   /* if option prompt_on_replace do once */
        find (app, findtext);

        /* delete text between input/select marks, replace with replacetext */
        if (app->txtfound) {

            /* TODO: add dialog prompting for replace, or maybe separate
             * [find] [replace] buttons on dialog
             */
            gtk_text_buffer_begin_user_action (buffer);   /* begin user action */
            buffer_replace_selection (app, replacetext);  /* replace text      */
            gtk_text_buffer_end_user_action (buffer);     /* end user action   */
        }
    }
    else {                  /* otherwise, replace all occurrences */
        gtk_text_buffer_begin_user_action (buffer);     /* begin user action */
        for (;;) {
            find (app, findtext);
            if (app->txtfound)
                buffer_replace_selection (app, replacetext); /* replace text */
            else
                break;
        }
        gtk_text_buffer_end_user_action (buffer);       /* end user action   */
    }

    if (widget) {}
}

void btnclose_activate (GtkWidget *widget, gpointer data)
{
    mainwin_t *app = data;
    kinst_t *inst = app->einst[app->focused]->inst;

    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(inst->buf);
    GtkTextIter ins, bound; /* iterators to restore selection bounds */

    /* free app mem - destruct here */
    app->txtfound = FALSE;  /* reset found & last_pos */
    if (app->last_pos)
        gtk_text_buffer_delete_mark (buffer, app->last_pos);
    app->last_pos = NULL;
    findcbchgd = FALSE;
    replcbchgd = FALSE;

    /** TODO: finish hanlding word selection in 'replace' and then restore
     *        the original selection bounds here so available for additional
     *        searches/replaces without requiring a user re-selection if
     *        additional searches on the same text are desired.
     */
    if (app->selstart && app->selend) { /* both selection marks present */
        /* get iters at original selection bounds */
        gtk_text_buffer_get_iter_at_mark (buffer, &ins, app->selstart);
        gtk_text_buffer_get_iter_at_mark (buffer, &bound, app->selend);

        /* select range between between original insert and selection marks */
        gtk_text_buffer_select_range (buffer, &ins, &bound);
    }
    if (app->selstart) {    /* delete marks used during search */
        gtk_text_buffer_delete_mark (buffer, app->selstart);
        app->selstart = NULL;   /* must be set NULL for next chk */
    }
    if (app->selend) {
        gtk_text_buffer_delete_mark (buffer, app->selend);
        app->selend = NULL;
    }
    if (app->markfrom) {
        gtk_text_buffer_delete_mark (buffer, app->markfrom);
        app->markfrom = NULL;
    }

    dlgid = 0; /* reset dialog id to default */

    /* call common gtk_widget_destroy (could move all there) */
    gtk_widget_destroy (findrepwin);
    if (widget) {}
}

void chk_realloc_ent (gpointer data)
{
    mainwin_t *app = data;

    /* check/realloc find/rep text */
    if (app->nfentries == app->fmax) {
        gchar **tmp = g_realloc (app->findtext,
                                 2 * app->nfentries * sizeof *(app->findtext));
        if (!tmp) {
            err_dialog ("btnreplace_activate()\nvirtual memory exhausted.");
            // findrep_destroy (app); /* TODO: graceful exit */
            // context_destroy (app);
            gtk_main_quit();
        }
        app->fmax += MAXLE;
    }

    if (app->nrentries == app->rmax) {
        gchar **tmp = g_realloc (app->reptext,
                                 2 * app->nrentries * sizeof *(app->reptext));
        if (!tmp) {
            err_dialog ("btnreplace_activate()\nvirtual memory exhausted.");
            // findrep_destroy (app); /* TODO: graceful exit */
            // context_destroy (app);
            gtk_main_quit();
        }
        app->rmax += MAXLE;
    }
}

void dumpopts (gpointer data)
{
    mainwin_t *app = data;

    g_print ("\n checkbox states:\n\n");
    g_print ("  optregex  : %s\n", app->optregex  ? "true" : "false");
    g_print ("  optplace  : %s\n", app->optplace  ? "true" : "false");
    g_print ("  optcase   : %s\n", app->optcase   ? "true" : "false");
    g_print ("  optwhole  : %s\n", app->optwhole  ? "true" : "false");
    g_print ("  optfrom   : %s\n", app->optfrom   ? "true" : "false");
    g_print ("  optback   : %s\n", app->optback   ? "true" : "false");
    g_print ("  optselect : %s\n", app->optselect ? "true" : "false");
    g_print ("  optprompt : %s\n", app->optprompt ? "true" : "false");
}

/* find replace keypress handler */
/* TODO: add handling by combobox and skip handler? (see snip below) */
static gboolean on_fr_keypress (GtkWidget *widget, GdkEventKey *event,
                                gpointer data)
{
    switch (event->keyval)
    {
        case GDK_KEY_Escape:
            btnclose_activate (widget, data);
            // return TRUE;    /* return TRUE - no further processing */
            break;
        case GDK_KP_Enter:
            if (dlgid == DLGFIND && findcbchgd)
                btnfind_activate (widget, data);
            else if (dlgid == DLGREPL && findcbchgd && replcbchgd)
                btnreplace_activate (widget, data);
            return TRUE;
        case GDK_Return:
            if (dlgid == DLGFIND && findcbchgd)
                btnfind_activate (widget, data);
            else if (dlgid == DLGREPL && findcbchgd && replcbchgd)
                btnreplace_activate (widget, data);
            return TRUE;
    }

    return FALSE;
}

/** delete text between input/select marks, making last_pos the same as
 *  insert_mark using last_pos to create iter, then insert replacetext.
 */
void buffer_replace_selection (gpointer data, const gchar *replacetext)
{
    mainwin_t *app = data;
    kinst_t *inst = app->einst[app->focused]->inst;

    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(inst->buf);
    GtkTextIter iter;

    gtk_text_buffer_delete_selection (buffer, FALSE, TRUE);
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, app->last_pos);
    gtk_text_buffer_insert (buffer, &iter, replacetext, -1);
}
