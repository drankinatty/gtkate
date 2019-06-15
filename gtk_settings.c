#include "gtk_settings.h"

/** static globals to eliminate tracking unnecessary variables in mainwin_t */
static GtkWidget *settingswin;     /* settings window */

/** settings keypress handler */
static gboolean on_settings_keypress (GtkWidget *widget, GdkEventKey *event,
                                      gpointer data)
{
    switch (event->keyval)
    {
        case GDK_KEY_Escape:    /* close dialog if user presses ESC */
            settings_btncancel (widget, data);
            break;
    }

    return FALSE;   /* TRUE - no further processing */
}

/** settings window create */
GtkWidget *create_settings_dlg (gpointer data)
{
    mainwin_t *app = data;
    einst_t *einst = app->einst[app->focused];
    kinst_t *inst = einst->inst;

    /* variables */
    GtkWidget *vbox;            /* vbox - main container   */
    GtkWidget *notebook;        /* notebook in vbox */
    GtkWidget *vboxnb;          /* vboxnb - vbox in each notebook page */
    GtkWidget *frame;           /* frames to section vboxnb */
    GtkWidget *table;           /* table for each frame */
    GtkWidget *hbtweak;         /* hbox to left-justify table element */
    GtkWidget *label;           /* generic label */
    GtkWidget *toplabel;        /* label above settings */
    GtkWidget *hbox;            /* hbox container   */
    GtkWidget *btnok;           /* button OK */
    GtkWidget *btncancel;       /* button Cancel */
    GtkWidget *fontbtn;         /* Font Button */
    GtkWidget *chkdynwrap;      /* checkbox - dynamic word wrap */
    GtkWidget *chkshowdwrap;    /* checkbox - show wrap markers */
    GtkWidget *chksmarthe;      /* checkbox - smart home/end */
    GtkWidget *chkwraptxtcsr;   /* checkbox - wrap text cursor */
    GtkWidget *chkpgudmvscsr;   /* checkbox - PgUp/PgDn moves cursor */
    GtkWidget *chkcsrtarrow;    /* checkbox - use ctrl_shift_right_fix */
    GtkWidget *chkwinrestore;   /* checkbox - restore window size */
    GtkWidget *chkexpandtab;    /* checkbox - insert spaces for tab */
    GtkWidget *chksmartbs;      /* checkbox - smart backspace */
    GtkWidget *chkshowtabs;     /* checkbox - show tab markers */
    GtkWidget *spintab;         /* spinbutton - tabstop */
    GtkWidget *chkindentwspc;   /* checkbox - indent with spaces not tab */
    GtkWidget *chkindentauto;   /* checkbox - auto-indent new line */
    GtkWidget *chkindentmixd;   /* checkbox - Emacs mode mixed spaces/tabs */
    GtkWidget *spinindent;      /* spinbutton - indent width (softtab) */
    GtkWidget *chkposcurend;    /* checkbox - cursor position on open */
    GtkWidget *chkcmtusesngl;   /* checkbox - use single-line comment */
    // GtkWidget *commentbox;      /* combobox - holds single-line comment string */
    GtkWidget *cmbeoldefault;   /* combobox - set default EOL handling */
    GtkWidget *chkunsaved;      /* checkbox - prompt for unsaved on close */
    GtkWidget *chktrimendws;    /* checkbox - remove trailing whitespace */
    GtkWidget *chkposixeof;     /* checkbox - require POSIX end of file */
    GtkWidget *spinrecent;      /* spinbutton - no. files in recent chooser */
    GtkWidget *chklinehghlt;    /* checkbox - show current line highlight */
    GtkWidget *chkshowmargin;   /* checkbox - show right margin guide */
    GtkObject *adjmarginwidth;  /* adjustment - right marginwidth */
    GtkWidget *spinmarginwidth; /* spinbutton - right marginwidth */
    GtkWidget *chkenablecmplt;  /* checkbox - enable word completion */
    GtkObject *adjwordsize;     /* adjustment - right marginwidth */
    GtkWidget *spinwordsize; /* spinbutton - right marginwidth */

    GtkObject *adjtab;          /* adjustment - tab spinbutton */
    GtkObject *adjind;          /* adjustment - indent spinbutton */
    GtkObject *adjrec;          /* adjustment - recent spinbutton */

    gint wnwidth  = 500;    /* initial dialog width and height */
    gint wnheight = 520;    /* maintian wnwidth - pgwidth = 80 */
    gint pgwidth  = 420;    /* initial notebook page width and height */
    gint pgheight = 440;    /* (enforced by vboxnb width/height) */
                            /* MS Windows required additional space. */

    /* create toplevel window */
    if (!(settingswin = gtk_window_new (GTK_WINDOW_TOPLEVEL))) {
        err_dialog ("create_settings_dlg() gtk_window_new failure.");
        return NULL;
    }
    gtk_window_set_position (GTK_WINDOW (settingswin), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size (GTK_WINDOW (settingswin), wnwidth, wnheight);
    gtk_window_set_title (GTK_WINDOW (settingswin), "GtkWrite Settings");
    /* for inclusion in main app set modal and transient_for (to keep on top)
     * gtk_window_set_modal (GtkWindow *window, gboolean modal);
     * gtk_window_set_transient_for(GtkWindow *window, GtkWindow *parent)
     */
    gtk_window_set_modal (GTK_WINDOW(settingswin), TRUE);
    gtk_window_set_transient_for (GTK_WINDOW(settingswin),
                                    GTK_WINDOW(app->window));
    gtk_container_set_border_width (GTK_CONTAINER (settingswin), 5);
    g_signal_connect (settingswin, "destroy",
		      G_CALLBACK (settings_btncancel), app);

    /* main vbox container */
    vbox = gtk_vbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (settingswin), vbox);

    /* title over notebook, TODO: save label and change on page sel */
    // toplabel = gtk_label_new ("Checkboxes - immediate effect, Entries - on Ok");
    toplabel = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (toplabel),
        "<b>Checkboxes - immediate effect, Entries - on Ok</b>");
    gtk_misc_set_padding (GTK_MISC(toplabel), 0, 5);
    gtk_box_pack_start (GTK_BOX (vbox), toplabel, FALSE, FALSE, 0);
    gtk_widget_show (toplabel);

    /* Create a new notebook, place the position of the tabs left,
     * set tab-hborder and tab-vborder independently, kwinst menu
     * of tabs currently not currently enabled. (see notes below)
     */
    notebook = gtk_notebook_new ();
    gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_LEFT);
    /* set notebook tab border with g_object_set
     * (gtk_notebook_set_tab_[hv]border is deprecated)
     *
     * set both h & v borders to a single value:
     * g_object_set (GTK_NOTEBOOK (notebook), "tab-border", 4, NULL);
     *
     * set hborder and vborder independently:
     */
    g_object_set (GTK_NOTEBOOK (notebook), "tab-vborder", 5,
                                           "tab-hborder", 4, NULL);
    /* enable kwinst menu showing all tabs if user rt-clicks on tab
     * (not currently needed)
     *
     * gtk_notebook_popup_enable (GTK_NOTEBOOK (notebook));
     *
     * if enabled, you must replace:
     *   gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vboxnb, label);
     * with:
     *   gtk_notebook_append_page_menu (GTK_NOTEBOOK (notebook), vboxnb, label, NULL);
     * (already commented in code below)
     */

    gtk_box_pack_start (GTK_BOX (vbox), notebook, TRUE, TRUE, 0);
    // gtk_box_pack_start (GTK_BOX (vbox), notebook, FALSE, FALSE, 0);

    /* appearance - setting page */
    vboxnb = gtk_vbox_new (FALSE, 0);
    gtk_widget_set_size_request (vboxnb, pgwidth, pgheight);

    /* frame within page - font */
    frame = gtk_frame_new (NULL);
    gtk_frame_set_label (GTK_FRAME (frame), "Font Selection");
    gtk_frame_set_label_align (GTK_FRAME (frame), 0.0, 0.5);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
    gtk_widget_show (frame);

    /* table inside frame - font */
    table = gtk_table_new (1, 2, TRUE);
    gtk_table_set_row_spacings (GTK_TABLE (table), 5);
    gtk_table_set_col_spacings (GTK_TABLE (table), 3);
    gtk_container_set_border_width (GTK_CONTAINER (table), 5);
    gtk_container_add (GTK_CONTAINER (frame), table);
    gtk_widget_show (table);

    /* select font label is element 1 */
    label = gtk_label_new ("Select Font:");
    hbtweak = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbtweak), label, FALSE, FALSE, 0);
    gtk_table_attach_defaults (GTK_TABLE (table), hbtweak, 0, 1, 0, 1);
    gtk_widget_show (hbtweak);
    gtk_widget_show (label);

    /* font button calling font selection dialog */
    fontbtn = gtk_font_button_new_with_font (app->fontname);
    gtk_font_button_set_show_style (GTK_FONT_BUTTON(fontbtn), TRUE);
    gtk_font_button_set_show_size (GTK_FONT_BUTTON(fontbtn), TRUE);
    gtk_font_button_set_title (GTK_FONT_BUTTON(fontbtn),
                                "Select Editor Font & Size");
    gtk_table_attach_defaults (GTK_TABLE (table), fontbtn, 1, 2, 0, 1);
    gtk_widget_show (fontbtn);

    /* pack frame into notebook vboxnb */
    gtk_box_pack_start (GTK_BOX (vboxnb), frame, FALSE, FALSE, 0);

    /* frame within page - word wrap */
    frame = gtk_frame_new (NULL);
    gtk_frame_set_label (GTK_FRAME (frame), "Word Wrap");
    gtk_frame_set_label_align (GTK_FRAME (frame), 0.0, 0.5);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
    gtk_widget_show (frame);

    /* table inside frame */
    table = gtk_table_new (2, 2, TRUE);
    gtk_table_set_row_spacings (GTK_TABLE (table), 5);
    gtk_table_set_col_spacings (GTK_TABLE (table), 3);
    gtk_container_set_border_width (GTK_CONTAINER (table), 5);
    gtk_container_add (GTK_CONTAINER (frame), table);
    gtk_widget_show (table);

    /* options checkboxs */
    chkdynwrap = gtk_check_button_new_with_mnemonic ("_Dynamic word wrap");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkdynwrap), app->dynwrap);
    gtk_table_attach_defaults (GTK_TABLE (table), chkdynwrap, 0, 1, 0, 1);
    gtk_widget_show (chkdynwrap);

    chkshowdwrap = gtk_check_button_new_with_mnemonic ("Show _word wrap indicators");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkshowdwrap), app->showdwrap);
    gtk_table_attach_defaults (GTK_TABLE (table), chkshowdwrap, 0, 1, 1, 2);
    /* disable checkboxs until implemented? */
    // gtk_widget_set_sensitive (chkshowdwrap, FALSE);
    gtk_widget_show (chkshowdwrap);

    /* pack frame into notebook vboxnb */
    gtk_box_pack_start (GTK_BOX (vboxnb), frame, FALSE, FALSE, 0);

    /* frame within page - cursor & selection */
    frame = gtk_frame_new (NULL);
    gtk_frame_set_label (GTK_FRAME (frame), "Cursor & Selection");
    gtk_frame_set_label_align (GTK_FRAME (frame), 0.0, 0.5);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
    gtk_widget_show (frame);

    /* table inside frame */
    table = gtk_table_new (4, 2, TRUE);
    gtk_table_set_row_spacings (GTK_TABLE (table), 5);
    gtk_table_set_col_spacings (GTK_TABLE (table), 3);
    gtk_container_set_border_width (GTK_CONTAINER (table), 5);
    gtk_container_add (GTK_CONTAINER (frame), table);
    gtk_widget_show (table);

    /* options checkboxs */
    chksmarthe = gtk_check_button_new_with_mnemonic ("_Smart home & end movement");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chksmarthe), app->smarthe);
    gtk_table_attach_defaults (GTK_TABLE (table), chksmarthe, 0, 1, 0, 1);
    gtk_widget_show (chksmarthe);

    chkwraptxtcsr = gtk_check_button_new_with_mnemonic ("Wrap _cursor to next line");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkwraptxtcsr), app->wraptxtcsr);
    gtk_table_attach_defaults (GTK_TABLE (table), chkwraptxtcsr, 0, 1, 1, 2);
    gtk_widget_show (chkwraptxtcsr);

    chkpgudmvscsr = gtk_check_button_new_with_mnemonic ("PgUp/PgDn _moves text cursor");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkpgudmvscsr), app->pgudmvscsr);
    gtk_table_attach_defaults (GTK_TABLE (table), chkpgudmvscsr, 0, 1, 2, 3);
    gtk_widget_show (chkpgudmvscsr);

    chkcsrtarrow = gtk_check_button_new_with_mnemonic ("Ctrl + Shift + _Right-Arrow fix");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkcsrtarrow),
                                    app->ctrl_shift_right_fix);
    gtk_table_attach_defaults (GTK_TABLE (table), chkcsrtarrow, 0, 1, 3, 4);
    gtk_widget_show (chkcsrtarrow);

    /* pack frame into notebook vbox */
    gtk_box_pack_start (GTK_BOX (vboxnb), frame, FALSE, FALSE, 0);

    /* frame within page - cursor & selection */
    frame = gtk_frame_new (NULL);
    gtk_frame_set_label (GTK_FRAME (frame), "Visual Aids");
    gtk_frame_set_label_align (GTK_FRAME (frame), 0.0, 0.5);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
    gtk_widget_show (frame);

    /* table inside frame */
    table = gtk_table_new (2, 2, TRUE);
    gtk_table_set_row_spacings (GTK_TABLE (table), 5);
    gtk_table_set_col_spacings (GTK_TABLE (table), 3);
    gtk_container_set_border_width (GTK_CONTAINER (table), 5);
    gtk_container_add (GTK_CONTAINER (frame), table);
    gtk_widget_show (table);

    /* options checkboxs */
    chklinehghlt = gtk_check_button_new_with_mnemonic ("_Highlight current line");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chklinehghlt), app->linehghlt);
    gtk_table_attach_defaults (GTK_TABLE (table), chklinehghlt, 0, 1, 0, 1);
    gtk_widget_show (chklinehghlt);

    chkshowmargin = gtk_check_button_new_with_mnemonic ("Show right _margin");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkshowmargin), app->showmargin);
    gtk_table_attach_defaults (GTK_TABLE (table), chkshowmargin, 0, 1, 1, 2);
    gtk_widget_show (chkshowmargin);

    /* value, lower, upper, step_increment, page_increment, page_size
     * (as with statusbar, the value is line + 1)
     */
    adjmarginwidth = gtk_adjustment_new (app->marginwidth, 1.0, 120.0, 1.0, 4.0, 0.0);
    spinmarginwidth = gtk_spin_button_new (GTK_ADJUSTMENT(adjmarginwidth), 1.0, 0);
    gtk_table_attach_defaults (GTK_TABLE (table), spinmarginwidth, 1, 2, 1, 2);
    gtk_widget_show (spinmarginwidth);

    /* pack frame into notebook vbox */
    gtk_box_pack_start (GTK_BOX (vboxnb), frame, FALSE, FALSE, 0);

    /* frame within page - cursor & selection */
    frame = gtk_frame_new (NULL);
    gtk_frame_set_label (GTK_FRAME (frame), "Window Size");
    gtk_frame_set_label_align (GTK_FRAME (frame), 0.0, 0.5);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
    gtk_widget_show (frame);

    /* table inside frame */
    table = gtk_table_new (1, 2, TRUE);
    gtk_table_set_row_spacings (GTK_TABLE (table), 5);
    gtk_table_set_col_spacings (GTK_TABLE (table), 3);
    gtk_container_set_border_width (GTK_CONTAINER (table), 5);
    gtk_container_add (GTK_CONTAINER (frame), table);
    gtk_widget_show (table);

    /* options checkboxs */
    chkwinrestore = gtk_check_button_new_with_mnemonic ("_Restore window size at start");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkwinrestore), app->winrestore);
    gtk_table_attach_defaults (GTK_TABLE (table), chkwinrestore, 0, 1, 0, 1);
    gtk_widget_show (chkwinrestore);

    /* save window size button */
    GtkWidget *btn_savesize;
    if (app->winszsaved)
        btn_savesize = gtk_button_new_with_mnemonic ("Save Si_ze on Exit");
    else
        btn_savesize = gtk_button_new_with_mnemonic ("Save Current Si_ze");
    gtk_widget_set_size_request (btn_savesize, 80, 24);
    gtk_table_attach_defaults (GTK_TABLE (table), btn_savesize, 1, 2, 0, 1);
    gtk_widget_show (btn_savesize);

    /* pack frame into notebook vbox */
    gtk_box_pack_start (GTK_BOX (vboxnb), frame, FALSE, FALSE, 0);

    /* label for tab */
    /* TODO: set tab borders (hborder/vborder, etc..) */
    label = gtk_label_new ("Appearance");

    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vboxnb, label);
    /* use if gtk_notebook_popup_enable () set above */
    // gtk_notebook_append_page_menu (GTK_NOTEBOOK (notebook), vboxnb, label, NULL);
    gtk_widget_show (vboxnb);

    /* editing - setting page */
    vboxnb = gtk_vbox_new (FALSE, 0);
    gtk_widget_set_size_request (vboxnb, pgwidth, pgheight);

    /* frame within page */
    frame = gtk_frame_new (NULL);
    gtk_frame_set_label (GTK_FRAME (frame), "Tabulators");
    gtk_frame_set_label_align (GTK_FRAME (frame), 0.0, 0.5);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
    gtk_widget_show (frame);

    /* table inside frame */
    table = gtk_table_new (4, 2, TRUE);
    gtk_table_set_row_spacings (GTK_TABLE (table), 5);
    gtk_table_set_col_spacings (GTK_TABLE (table), 3);
    gtk_container_set_border_width (GTK_CONTAINER (table), 5);
    gtk_container_add (GTK_CONTAINER (frame), table);
    gtk_widget_show (table);

    /* options checkboxs */
    chkexpandtab = gtk_check_button_new_with_mnemonic ("_Insert spaces instead of tab");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkexpandtab), app->expandtab);
    gtk_table_attach_defaults (GTK_TABLE (table), chkexpandtab, 0, 1, 0, 1);
    gtk_widget_show (chkexpandtab);

    chksmartbs = gtk_check_button_new_with_mnemonic ("Smart _backspace (to tab)");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chksmartbs), app->smartbs);
    gtk_table_attach_defaults (GTK_TABLE (table), chksmartbs, 0, 1, 1, 2);
    gtk_widget_show (chksmartbs);

    chkshowtabs = gtk_check_button_new_with_mnemonic ("_Show tab markers");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkshowtabs), app->showtabs);
    gtk_table_attach_defaults (GTK_TABLE (table), chkshowtabs, 0, 1, 2, 3);
    gtk_widget_show (chkshowtabs);

    label = gtk_label_new ("Set tab size (spaces):");
    hbtweak = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbtweak), label, FALSE, FALSE, 0);
    gtk_table_attach_defaults (GTK_TABLE (table), hbtweak, 0, 1, 3, 4);
    gtk_widget_show (hbtweak);
    gtk_widget_show (label);

    /* value, lower, upper, step_increment, page_increment, page_size
     * (as with statusbar, the value is line + 1)
     */
    adjtab = gtk_adjustment_new (app->tabstop, 1.0, 80.0, 1.0, 4.0, 0.0);
    spintab = gtk_spin_button_new (GTK_ADJUSTMENT(adjtab), 1.0, 0);
    gtk_table_attach_defaults (GTK_TABLE (table), spintab, 1, 2, 3, 4);
    gtk_widget_show (spintab);

    /* pack frame into notebook vboxnb */
    gtk_box_pack_start (GTK_BOX (vboxnb), frame, FALSE, FALSE, 0);

    /* frame within page */
    frame = gtk_frame_new (NULL);
    gtk_frame_set_label (GTK_FRAME (frame), "Indentation Rules");
    gtk_frame_set_label_align (GTK_FRAME (frame), 0.0, 0.5);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
    gtk_widget_show (frame);

    /* table inside frame */
    table = gtk_table_new (4, 2, TRUE);
    gtk_table_set_row_spacings (GTK_TABLE (table), 5);
    gtk_table_set_col_spacings (GTK_TABLE (table), 3);
    gtk_container_set_border_width (GTK_CONTAINER (table), 5);
    gtk_container_add (GTK_CONTAINER (frame), table);
    gtk_widget_show (table);

    /* options checkboxs */
    chkindentwspc = gtk_check_button_new_with_mnemonic ("_Use spaces to indent");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkindentwspc), app->indentwspc);
    gtk_table_attach_defaults (GTK_TABLE (table), chkindentwspc, 0, 1, 0, 1);
    gtk_widget_show (chkindentwspc);

    chkindentauto = gtk_check_button_new_with_mnemonic ("_Auto-indent new lines");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkindentauto), app->indentauto);
    gtk_table_attach_defaults (GTK_TABLE (table), chkindentauto, 0, 1, 1, 2);
    gtk_widget_show (chkindentauto);

    chkindentmixd = gtk_check_button_new_with_mnemonic ("_Emacs style mixed mode");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkindentmixd), app->indentmixd);
    gtk_table_attach_defaults (GTK_TABLE (table), chkindentmixd, 0, 1, 2, 3);
    gtk_widget_show (chkindentmixd);

    label = gtk_label_new ("Set indent size (spaces):");
    hbtweak = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbtweak), label, FALSE, FALSE, 0);
    gtk_table_attach_defaults (GTK_TABLE (table), hbtweak, 0, 1, 3, 4);
    gtk_widget_show (hbtweak);
    gtk_widget_show (label);

    /* value, lower, upper, step_increment, page_increment, page_size
     * (page_size other than 0.0 is deprecated)
     */
    adjind = gtk_adjustment_new (app->softtab, 1.0, 80.0, 1.0, 2.0, 0.0);
    spinindent = gtk_spin_button_new (GTK_ADJUSTMENT(adjind), 1.0, 0);
    gtk_table_attach_defaults (GTK_TABLE (table), spinindent, 1, 2, 3, 4);
    gtk_widget_show (spinindent);

    /* pack frame into notebook vboxnb */
    gtk_box_pack_start (GTK_BOX (vboxnb), frame, FALSE, FALSE, 0);

    /* frame within page */
    frame = gtk_frame_new (NULL);
    gtk_frame_set_label (GTK_FRAME (frame), "Comment Settings");
    gtk_frame_set_label_align (GTK_FRAME (frame), 0.0, 0.5);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
    gtk_widget_show (frame);

    /* table inside frame */
    table = gtk_table_new (2, 2, TRUE);
    gtk_table_set_row_spacings (GTK_TABLE (table), 5);
    gtk_table_set_col_spacings (GTK_TABLE (table), 3);
    gtk_container_set_border_width (GTK_CONTAINER (table), 5);
    gtk_container_add (GTK_CONTAINER (frame), table);
    gtk_widget_show (table);

    /* options checkboxs (with description) */
    chkcmtusesngl = gtk_check_button_new_with_mnemonic ("Use single-line _comment");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkcmtusesngl), app->cmtusesingle);
    gtk_table_attach_defaults (GTK_TABLE (table), chkcmtusesngl, 0, 1, 0, 1);
    gtk_widget_show (chkcmtusesngl);
    label = gtk_label_new ("(overrides auto-block comment use)");
    gtk_table_attach_defaults (GTK_TABLE (table), label, 1, 2, 0, 1);
    gtk_widget_show (label);

    /* add single-line comment entry to be used no sourceview or to override */
    label = gtk_label_new ("Single-line Comment String:");
    hbtweak = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbtweak), label, FALSE, FALSE, 0);
    gtk_table_attach_defaults (GTK_TABLE (table), hbtweak, 0, 1, 1, 2);
    gtk_widget_show (hbtweak);
    gtk_widget_show (label);

    app->cmtentry = gtk_entry_new();
    const GtkBorder brd = { 2, 2, 2, 2 }; /* left, right, top, bottom */
    gtk_entry_set_max_length (GTK_ENTRY(app->cmtentry), 8);
    gtk_entry_set_has_frame (GTK_ENTRY(app->cmtentry), TRUE);
    gtk_entry_set_inner_border (GTK_ENTRY(app->cmtentry), &brd);
    if (inst->language && inst->comment_single)
        gtk_entry_set_text (GTK_ENTRY(app->cmtentry), inst->comment_single);
    else {
        gchar *comment = app->comment ? app->comment : "<unset>";
        gtk_entry_set_text (GTK_ENTRY(app->cmtentry), comment);
    }
    gtk_table_attach_defaults (GTK_TABLE (table), app->cmtentry, 1, 2, 1, 2);
    gtk_widget_show (app->cmtentry);

    /* show language and auto-lookup of comment-syntax */
    gtk_table_resize (GTK_TABLE (table), 4, 2);
    if (inst->language) {
        const gchar *lang = gtk_source_language_get_name (inst->language);
        gchar *msg;

        label = gtk_label_new ("GTK SourceView Comment Auto-detect/Lookup");
        hbtweak = gtk_hbox_new (FALSE, 0);
        gtk_box_pack_start (GTK_BOX (hbtweak), label, FALSE, FALSE, 0);
        gtk_table_attach_defaults (GTK_TABLE (table), hbtweak, 0, 2, 2, 3);
        gtk_widget_show (hbtweak);
        gtk_widget_show (label);

        if ((inst->comment_blk_beg && inst->comment_blk_end)) {
            if (inst->comment_single)
                msg = g_markup_printf_escaped ("<tt> </tt>language:<tt><b> %s    </b></tt>"
                                            "single-line:<tt><b> '%s'    </b></tt>"
                                            "block:<tt><b> '%s...%s'</b></tt>",
                                            lang, inst->comment_single,
                                            inst->comment_blk_beg, inst->comment_blk_end);
            else
                msg = g_markup_printf_escaped ("<tt> </tt>language:<tt><b> N/A    </b></tt>"
                                            "single-line:<tt><b> '%s'    </b></tt>"
                                            "block:<tt><b> '%s...%s'</b></tt>",
                                            lang,
                                            inst->comment_blk_beg, inst->comment_blk_end);
        }
        else if (inst->comment_single)
            msg = g_markup_printf_escaped ("<tt> </tt>language:<tt><b> %s    </b></tt>"
                                            "single-line:<tt><b> '%s'    </b></tt>"
                                            "block:<tt><b> 'none ... none'</b></tt>",
                                            lang, inst->comment_single);
        else {
            msg = g_markup_printf_escaped ("<tt> </tt>language:<tt><b> %s  </b></tt>"
                                        "(comment syntax not implemented)", lang);
        }

        label = gtk_label_new (NULL);
        hbtweak = gtk_hbox_new (FALSE, 0);
        gtk_box_pack_start (GTK_BOX (hbtweak), label, FALSE, FALSE, 0);
        gtk_label_set_markup (GTK_LABEL(label), msg);
        gtk_table_attach_defaults (GTK_TABLE (table), hbtweak, 0, 2, 3, 4);
        gtk_widget_show (hbtweak);
        gtk_widget_show (label);
        g_free (msg);
    }

    /* pack frame into notebook vboxnb */
    gtk_box_pack_start (GTK_BOX (vboxnb), frame, FALSE, FALSE, 0);

    /* label for tab */
    /* TODO: set tab borders (hborder/vborder, etc..) */
    label = gtk_label_new ("Editing");

    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vboxnb, label);
    /* use if gtk_notebook_popup_enable () set above */
    // gtk_notebook_append_page_menu (GTK_NOTEBOOK (notebook), vboxnb, label, NULL);
    gtk_widget_show (vboxnb);

    /* File Load/Save - setting page */
    vboxnb = gtk_vbox_new (FALSE, 0);
    gtk_widget_set_size_request (vboxnb, pgwidth, pgheight);

    /* frame within page - cursor positon on open */
    frame = gtk_frame_new (NULL);
    gtk_frame_set_label (GTK_FRAME (frame), "Position Cursor on Open");
    gtk_frame_set_label_align (GTK_FRAME (frame), 0.0, 0.5);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
    gtk_widget_show (frame);

    /* table inside frame */
    table = gtk_table_new (1, 2, TRUE);
    gtk_table_set_row_spacings (GTK_TABLE (table), 5);
    gtk_table_set_col_spacings (GTK_TABLE (table), 3);
    gtk_container_set_border_width (GTK_CONTAINER (table), 5);
    gtk_container_add (GTK_CONTAINER (frame), table);
    gtk_widget_show (table);

    /* options checkboxs */
    chkposcurend = gtk_check_button_new_with_mnemonic ("Positon cursor at _end");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkposcurend), app->poscurend);
    gtk_table_attach_defaults (GTK_TABLE (table), chkposcurend, 0, 1, 0, 1);
    gtk_widget_show (chkposcurend);

    /* pack frame into notebook vboxnb */
    gtk_box_pack_start (GTK_BOX (vboxnb), frame, FALSE, FALSE, 0);

    /* frame within page - cursor positon on open */
    frame = gtk_frame_new (NULL);
    gtk_frame_set_label (GTK_FRAME (frame), "Word Completion");
    gtk_frame_set_label_align (GTK_FRAME (frame), 0.0, 0.5);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
    gtk_widget_show (frame);

    /* table inside frame */
    table = gtk_table_new (2, 2, TRUE);
    gtk_table_set_row_spacings (GTK_TABLE (table), 5);
    gtk_table_set_col_spacings (GTK_TABLE (table), 3);
    gtk_container_set_border_width (GTK_CONTAINER (table), 5);
    gtk_container_add (GTK_CONTAINER (frame), table);
    gtk_widget_show (table);

    /* options checkboxs */
    chkenablecmplt = gtk_check_button_new_with_mnemonic ("Enable _Word Completion");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkenablecmplt),
                                    app->enablecmplt);
    gtk_table_attach_defaults (GTK_TABLE (table), chkenablecmplt, 0, 1, 0, 1);
    gtk_widget_show (chkenablecmplt);
    label = gtk_label_new ("(enabled/disabled on next editor use)");
    gtk_table_attach_defaults (GTK_TABLE (table), label, 1, 2, 0, 1);
    gtk_widget_show (label);

    label = gtk_label_new ("Minimum completion characters:");
    hbtweak = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbtweak), label, FALSE, FALSE, 0);
    gtk_table_attach_defaults (GTK_TABLE (table), hbtweak, 0, 1, 1, 2);
    gtk_widget_show (hbtweak);
    gtk_widget_show (label);
    /* value, lower, upper, step_increment, page_increment, page_size
     * (as with statusbar, the value is line + 1)
     */
    adjwordsize = gtk_adjustment_new (app->cmplwordsz, 2.0, 12.0, 1.0, 2.0, 0.0);
    spinwordsize = gtk_spin_button_new (GTK_ADJUSTMENT(adjwordsize), 1.0, 0);
    gtk_table_attach_defaults (GTK_TABLE (table), spinwordsize, 1, 2, 1, 2);
    gtk_widget_show (spinwordsize);

    /* pack frame into notebook vboxnb */
    gtk_box_pack_start (GTK_BOX (vboxnb), frame, FALSE, FALSE, 0);

    /* frame within page */
    frame = gtk_frame_new (NULL);
    gtk_frame_set_label (GTK_FRAME (frame), "End-of-Line Handling/Selection");
    gtk_frame_set_label_align (GTK_FRAME (frame), 0.0, 0.5);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
    gtk_widget_show (frame);

    /* table inside frame */
//     table = gtk_table_new (2, 2, TRUE);
    table = gtk_table_new (1, 2, TRUE);
    gtk_table_set_row_spacings (GTK_TABLE (table), 5);
    gtk_table_set_col_spacings (GTK_TABLE (table), 3);
    gtk_container_set_border_width (GTK_CONTAINER (table), 5);
    gtk_container_add (GTK_CONTAINER (frame), table);
    gtk_widget_show (table);

    /* label left of combobox */
    label = gtk_label_new ("Select Global EOL Handling:");
    hbtweak = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbtweak), label, FALSE, FALSE, 0);
    gtk_table_attach_defaults (GTK_TABLE (table), hbtweak, 0, 1, 0, 1);
    gtk_widget_show (hbtweak);
    gtk_widget_show (label);

    /* end-of-line combobox */
    cmbeoldefault = gtk_combo_box_text_new ();
    // g_object_set (GTK_COMBO_BOX(cmbeoldefault), "xalign", 0.5, NULL);
    /* no property, must gtk_container_get_children (), find label */
    for (gint i = 0; i < EOLTXT_NO; i++)
        gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(cmbeoldefault), app->eoltxt[i]);
    gtk_combo_box_set_active (GTK_COMBO_BOX(cmbeoldefault), app->eoldefault);
    gtk_table_attach_defaults (GTK_TABLE (table), cmbeoldefault, 1, 2, 0, 1);
    gtk_widget_show (cmbeoldefault);

    /* label below combobox - TODO update on "changed" signal */
//     label = gtk_label_new (app->eoltxt[app->eoldefault]);
//     hbtweak = gtk_hbox_new (FALSE, 0);
//     gtk_box_pack_start (GTK_BOX (hbtweak), label, FALSE, FALSE, 0);
//     gtk_table_attach_defaults (GTK_TABLE (table), hbtweak, 1, 2, 1, 2);
//     gtk_widget_show (hbtweak);
//     gtk_widget_show (label);

    /* pack frame into notebook vboxnb */
    gtk_box_pack_start (GTK_BOX (vboxnb), frame, FALSE, FALSE, 0);

    /* frame within page */
    frame = gtk_frame_new (NULL);
    gtk_frame_set_label (GTK_FRAME (frame), "User Notifications");
    gtk_frame_set_label_align (GTK_FRAME (frame), 0.0, 0.5);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
    gtk_widget_show (frame);

    /* table inside frame */
    table = gtk_table_new (1, 2, TRUE);
    gtk_table_set_row_spacings (GTK_TABLE (table), 5);
    gtk_table_set_col_spacings (GTK_TABLE (table), 3);
    gtk_container_set_border_width (GTK_CONTAINER (table), 5);
    gtk_container_add (GTK_CONTAINER (frame), table);
    gtk_widget_show (table);

    /* options checkboxs */
    chkunsaved = gtk_check_button_new_with_mnemonic ("Warn of _unsaved on close");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkunsaved), app->warnunsaved);
    gtk_table_attach_defaults (GTK_TABLE (table), chkunsaved, 0, 1, 0, 1);
    gtk_widget_show (chkunsaved);

    /* pack frame into notebook vboxnb */
    gtk_box_pack_start (GTK_BOX (vboxnb), frame, FALSE, FALSE, 0);

    /* frame within page */
    frame = gtk_frame_new (NULL);
    gtk_frame_set_label (GTK_FRAME (frame), "Automatic Cleanups on Load/Save");
    gtk_frame_set_label_align (GTK_FRAME (frame), 0.0, 0.5);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
    gtk_widget_show (frame);

    /* table inside frame */
    table = gtk_table_new (2, 2, TRUE);
    gtk_table_set_row_spacings (GTK_TABLE (table), 5);
    gtk_table_set_col_spacings (GTK_TABLE (table), 3);
    gtk_container_set_border_width (GTK_CONTAINER (table), 5);
    gtk_container_add (GTK_CONTAINER (frame), table);
    gtk_widget_show (table);

    /* options checkboxs */
    chktrimendws = gtk_check_button_new_with_mnemonic ("_Remove trailing whitespace");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chktrimendws), app->trimendws);
    gtk_table_attach_defaults (GTK_TABLE (table), chktrimendws, 0, 1, 0, 1);
    gtk_widget_show (chktrimendws);

    chkposixeof = gtk_check_button_new_with_mnemonic ("Require _POSIX end-of-file");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkposixeof), app->posixeof);
    gtk_table_attach_defaults (GTK_TABLE (table), chkposixeof, 0, 1, 1, 2);
    gtk_widget_show (chkposixeof);

    /* pack frame into notebook vboxnb */
    gtk_box_pack_start (GTK_BOX (vboxnb), frame, FALSE, FALSE, 0);

    /* frame within page */
    frame = gtk_frame_new (NULL);
    gtk_frame_set_label (GTK_FRAME (frame), "File Chooser Settings");
    gtk_frame_set_label_align (GTK_FRAME (frame), 0.0, 0.5);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
    gtk_widget_show (frame);

    /* table inside frame */
    table = gtk_table_new (1, 2, TRUE);
    gtk_table_set_row_spacings (GTK_TABLE (table), 5);
    gtk_table_set_col_spacings (GTK_TABLE (table), 3);
    gtk_container_set_border_width (GTK_CONTAINER (table), 5);
    gtk_container_add (GTK_CONTAINER (frame), table);
    gtk_widget_show (table);

    label = gtk_label_new ("No. Files in Recent Chooser:");
    hbtweak = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbtweak), label, FALSE, FALSE, 0);
    gtk_table_attach_defaults (GTK_TABLE (table), hbtweak, 0, 1, 0, 1);
    gtk_widget_show (hbtweak);
    gtk_widget_show (label);

    /* value, lower, upper, step_increment, page_increment, page_size
     * (page_size other than 0.0 is deprecated)
     */
    adjrec = gtk_adjustment_new (app->nrecent, 1.0, 80.0, 1.0, 2.0, 0.0);
    spinrecent = gtk_spin_button_new (GTK_ADJUSTMENT(adjrec), 1.0, 0);
    gtk_table_attach_defaults (GTK_TABLE (table), spinrecent, 1, 2, 0, 1);
    gtk_widget_show (spinrecent);

    /* pack frame into notebook vboxnb */
    gtk_box_pack_start (GTK_BOX (vboxnb), frame, FALSE, FALSE, 0);
    /* label for tab */
    /* TODO: set tab borders (hborder/vborder, etc..) */
    label = gtk_label_new ("File Load/Save");

    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vboxnb, label);
    /* use if gtk_notebook_popup_enable () set above */
    // gtk_notebook_append_page_menu (GTK_NOTEBOOK (notebook), vboxnb, label, NULL);
    gtk_widget_show (vboxnb);

    gtk_widget_show (notebook);     /* show the notebook within vbox */

    /* hbox (replacing table) button spacing */
    hbox = gtk_hbox_new (FALSE, 0);
    gtk_box_set_spacing (GTK_BOX (hbox), 5);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 10);

    /* button sizes 80 x 24 */
    btnok = gtk_button_new_with_mnemonic ("_Ok");
    gtk_widget_set_size_request (btnok, 80, 24);
    gtk_widget_show (btnok);

    btncancel = gtk_button_new_with_mnemonic ("_Cancel");
    gtk_widget_set_size_request (btncancel, 80, 24);

    gtk_box_pack_end (GTK_BOX (hbox), btncancel, FALSE, FALSE, 0);
    gtk_box_pack_end (GTK_BOX (hbox), btnok, FALSE, FALSE, 0); // pack on rt-side.
    gtk_widget_show (btncancel);

    gtk_box_pack_end (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
    gtk_widget_show (hbox);

    gtk_widget_show (vbox);         /* make all widgets visible */

    /* settings callbacks - TODO: add remaining callbacks */
    g_signal_connect (btncancel, "clicked",
                      G_CALLBACK (settings_btncancel), data);

    g_signal_connect (btnok, "clicked",
                      G_CALLBACK (settings_btnok), data);

    g_signal_connect (fontbtn, "font-set",
                      G_CALLBACK (settings_fontbtn), data);

    g_signal_connect (chkdynwrap, "toggled",
                      G_CALLBACK (chkdynwrap_toggled), data);

    g_signal_connect (chkshowdwrap, "toggled",
                      G_CALLBACK (chkshowdwrap_toggled), data);

    g_signal_connect (chksmarthe, "toggled",
                      G_CALLBACK (chksmarthe_toggled), data);

    g_signal_connect (chkwraptxtcsr, "toggled",
                      G_CALLBACK (chkwraptxtcsr_toggled), data);

    g_signal_connect (chkpgudmvscsr, "toggled",
                      G_CALLBACK (chkpgudmvscsr_toggled), data);

    g_signal_connect (chkcsrtarrow, "toggled",
                      G_CALLBACK (chkcsrtarrow_toggled), data);

    g_signal_connect (chklinehghlt, "toggled",
                      G_CALLBACK (chklinehghlt_toggled), data);

    g_signal_connect (chkshowmargin, "toggled",
                      G_CALLBACK (chkshowmargin_toggled), data);

    g_signal_connect (spinmarginwidth, "value-changed",
                      G_CALLBACK (spinmarginwidth_changed), data);

    g_signal_connect (chkenablecmplt, "toggled",
                      G_CALLBACK (chkenablecmplt_toggled), data);

    g_signal_connect (spinwordsize, "value-changed",
                      G_CALLBACK (spinwordsize_changed), data);

    g_signal_connect (chkwinrestore, "toggled",
                      G_CALLBACK (chkwinrestore_toggled), data);

    g_signal_connect (btn_savesize, "clicked",
                      G_CALLBACK (btnwinsavesize), data);

    g_signal_connect (chkexpandtab, "toggled",
                      G_CALLBACK (chkexpandtab_toggled), data);

    g_signal_connect (chksmartbs, "toggled",
                      G_CALLBACK (chksmartbs_toggled), data);

    g_signal_connect (chkshowtabs, "toggled",
                      G_CALLBACK (chkshowtabs_toggled), data);

    g_signal_connect (spintab, "value-changed",
                      G_CALLBACK (spintab_changed), data);

    g_signal_connect (chkindentwspc, "toggled",
                      G_CALLBACK (chkindentwspc_toggled), data);

    g_signal_connect (chkindentauto, "toggled",
                      G_CALLBACK (chkindentauto_toggled), data);

    g_signal_connect (chkindentmixd, "toggled",
                      G_CALLBACK (chkindentmixd_toggled), data);

    g_signal_connect (spinindent, "value-changed",
                      G_CALLBACK (spinindent_changed), data);

    g_signal_connect (chkcmtusesngl, "toggled",
                      G_CALLBACK (chkcmtusesngl_toggled), data);

    g_signal_connect (app->cmtentry, "activate",
                      G_CALLBACK (entry_comment_activate), data);

    g_signal_connect (chkposcurend, "toggled",
                      G_CALLBACK (chkposcurend_toggled), data);

    g_signal_connect (cmbeoldefault, "changed",
                      G_CALLBACK (cmbeoldefault_changed), data);

    g_signal_connect (chkunsaved, "toggled",
                      G_CALLBACK (chkunsaved_toggled), data);

    g_signal_connect (chktrimendws, "toggled",
                      G_CALLBACK (chktrimendws_toggled), data);

    g_signal_connect (chkposixeof, "toggled",
                      G_CALLBACK (chkposixeof_toggled), data);

    g_signal_connect (settingswin, "key_press_event",
                      G_CALLBACK (on_settings_keypress), data);

    g_signal_connect (spinrecent, "value-changed",
                      G_CALLBACK (spinrecent_changed), data);

    gtk_widget_show (settingswin); /* show the window */

    return (settingswin);
}

void settings_btncancel (GtkWidget *widget, gpointer data)
{
    gtk_widget_destroy (settingswin);

    if (widget || data) {}
}

void settings_btnok (GtkWidget *widget, gpointer data)
{
    mainwin_t *app = data;
    einst_t *einst = app->einst[app->focused];
    GtkWidget *view = einst->view;

    entry_comment_activate (app->cmtentry, app);
    if (app->showmargin)
        gtk_source_view_set_right_margin_position (GTK_SOURCE_VIEW(view),
                                                    app->marginwidth);
    gtk_widget_destroy (settingswin);

    if (widget) {}
}

void settings_fontbtn (GtkWidget *widget, gpointer data)
{
    mainwin_t *app = data;
    einst_t *einst = app->einst[app->focused];
    GtkWidget *view = einst->view;

    const gchar *newfont = gtk_font_button_get_font_name (GTK_FONT_BUTTON(widget));
    PangoFontDescription *font_desc;

    if (!newfont) {
        err_dialog ("error: invalid font returned.");
        return;
    }

    if (app->fontname)
        g_free (app->fontname);

    if (!(app->fontname = g_strdup (newfont))) {
        err_dialog ("error:\ncopy of newfont to app->fontname failed.");
        return;
    }

    font_desc = pango_font_description_from_string (app->fontname);

    gtk_widget_modify_font (view, font_desc);
    pango_font_description_free (font_desc);
}

void chkdynwrap_toggled (GtkWidget *widget, gpointer data)
{
    mainwin_t *app = data;
    GtkWidget *view = app->einst[app->focused]->view;

    app->dynwrap = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (view),
            app->dynwrap ? GTK_WRAP_WORD : GTK_WRAP_NONE);
}

void chkshowdwrap_toggled (GtkWidget *widget, gpointer data)
{
    mainwin_t *app = data;

    app->showdwrap = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
}

void chksmarthe_toggled (GtkWidget *widget, gpointer data)
{
    mainwin_t *app = data;

    app->smarthe = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));

    for (gint i = 0; i < app->nview; i++) {
        GtkWidget *view = app->einst[i]->view;
        if (app->smarthe)
            gtk_source_view_set_smart_home_end (GTK_SOURCE_VIEW(view),
                                        GTK_SOURCE_SMART_HOME_END_BEFORE);
        else
            gtk_source_view_set_smart_home_end (GTK_SOURCE_VIEW(view),
                                        GTK_SOURCE_SMART_HOME_END_DISABLED);
    }
}

void chkwraptxtcsr_toggled (GtkWidget *widget, gpointer data)
{
    mainwin_t *app = data;

    app->wraptxtcsr = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
}

void chkpgudmvscsr_toggled (GtkWidget *widget, gpointer data)
{
    mainwin_t *app = data;

    app->pgudmvscsr = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
}

void chkcsrtarrow_toggled (GtkWidget *widget, gpointer data)
{
    mainwin_t *app = data;

    app->ctrl_shift_right_fix =
                gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
}

void chklinehghlt_toggled (GtkWidget *widget, gpointer data)
{
    mainwin_t *app = data;
    einst_t *einst = app->einst[app->focused];
    GtkWidget *view = einst->view;

    app->linehghlt = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
    gtk_source_view_set_highlight_current_line (GTK_SOURCE_VIEW(view),
                                                app->linehghlt);
}

void chkshowmargin_toggled (GtkWidget *widget, gpointer data)
{
    mainwin_t *app = data;
    einst_t *einst = app->einst[app->focused];
    GtkWidget *view = einst->view;

    app->showmargin = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
    gtk_source_view_set_show_right_margin (GTK_SOURCE_VIEW (view),
                                            app->showmargin);
}

void spinmarginwidth_changed (GtkWidget *widget, gpointer data)
{
    mainwin_t *app = data;

    app->marginwidth = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(widget));
}

void chkenablecmplt_toggled (GtkWidget *widget, gpointer data)
{
    mainwin_t *app = data;
    einst_t *einst = app->einst[app->focused];
    GtkWidget *view = einst->view;

    app->enablecmplt = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
    gtk_source_view_set_highlight_current_line (GTK_SOURCE_VIEW(view),
                                                app->enablecmplt);
}

void spinwordsize_changed (GtkWidget *widget, gpointer data)
{
    mainwin_t *app = data;

    guint newcmplwordsz = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(widget));

    if (app->cmplwordsz != newcmplwordsz) {
        /* get list of completion providers associated with completion object */
        GList *providers = gtk_source_completion_get_providers (app->completion);
        app->cmplwordsz = newcmplwordsz;    /* assign new minimum-word-size */

        /* iterate through providers setting minimum-word-size property */
        while (providers) {
            g_object_set (providers->data, "minimum-word-size",
                            app->cmplwordsz, NULL);
            providers = providers->next;
        }
    }
}

void chkwinrestore_toggled (GtkWidget *widget, gpointer data)
{
    mainwin_t *app = data;

    app->winrestore = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
}

void btnwinsavesize (GtkWidget *widget, gpointer data)
{
    mainwin_t *app = data;

    /* set toggle on button so if winszsaved, button text is
     * "Save Size on Exit" instead of "Save Current Size".
     */

    if (app->winszsaved) {
        app->winszsaved = FALSE;
        gtk_button_set_label (GTK_BUTTON (widget),
                                "Save Current Si_ze");
        gtk_button_set_use_underline (GTK_BUTTON (widget), TRUE);
    }
    else {
        /* get window size */
        gtk_window_get_size (GTK_WINDOW (app->window), &(app->winwidth),
                            &(app->winheight));

        app->winszsaved = TRUE;
        gtk_button_set_label (GTK_BUTTON (widget),
                                "Save Si_ze on Exit");
        gtk_button_set_use_underline (GTK_BUTTON (widget), TRUE);
    }

    if (widget) {}
}

void chkexpandtab_toggled (GtkWidget *widget, gpointer data)
{
    mainwin_t *app = data;

    app->expandtab = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
}

void chksmartbs_toggled (GtkWidget *widget, gpointer data)
{
    mainwin_t *app = data;

    app->smartbs = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
}

void chkshowtabs_toggled (GtkWidget *widget, gpointer data)
{
    mainwin_t *app = data;

    app->showtabs = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
}

void spintab_changed (GtkWidget *widget, gpointer data)
{
    mainwin_t *app = data;

    app->tabstop = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(widget));
}

void chkindentwspc_toggled (GtkWidget *widget, gpointer data)
{
    mainwin_t *app = data;

    app->indentwspc = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
}

void chkindentauto_toggled (GtkWidget *widget, gpointer data)
{
    mainwin_t *app = data;

    app->indentauto = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));

    for (gint i = 0; i < app->nview; i++) {
        GtkWidget *view = app->einst[i]->view;
        gtk_source_view_set_auto_indent (GTK_SOURCE_VIEW(view),
                                            app->indentauto);
    }
}

void chkindentmixd_toggled (GtkWidget *widget, gpointer data)
{
    mainwin_t *app = data;

    app->indentmixd = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
}

void spinindent_changed (GtkWidget *widget, gpointer data)
{
    mainwin_t *app = data;

    app->softtab = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(widget));
}

void chkcmtusesngl_toggled (GtkWidget *widget, gpointer data)
{
    mainwin_t *app = data;

    app->cmtusesingle = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
}

/** get new single-line comment string */
void entry_comment_activate (GtkWidget *widget, gpointer data)
{
    mainwin_t *app = data;

    const gchar *text;
    gsize len;

    text = gtk_entry_get_text (GTK_ENTRY (widget));
    len = g_strlen (text);

    if (app->comment)
        g_free (app->comment);

    if (text[len-1] != ' ')
        app->comment = g_strdup_printf ("%s ", text);
    else
        app->comment = g_strdup (text);

    gtk_entry_set_text (GTK_ENTRY (widget), app->comment);
}

/** position cursor at end checkbox */
void chkposcurend_toggled (GtkWidget *widget, gpointer data)
{
    mainwin_t *app = data;

    app->poscurend = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
}

/** Handle EOL settings combobox selections */
void cmbeoldefault_changed (GtkWidget *widget, gpointer data)
{
    mainwin_t *app = data;

    gchar *selected =
            gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT (widget));

    /* loop over app->eoltxt[] to match selected to get index */
    for (gint i = 0; i < EOLTXT_NO; i++)
        if (g_strcmp0 (selected, app->eoltxt[i]) == 0) {
            app->eoldefault = i;
            break;
        }

    /* if selection to LF, CRLF, or CR made, update menu, convert file */
    if (app->eoldefault < FILE_EOL && app->eoldefault != app->eol) {
        if (app->eoldefault == LF)  /* trigger the LF callback */
            gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (app->eolLFMi),
                                            TRUE);
        else if (app->eoldefault == CRLF)   /* trigger the CRLF callback */
            gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (app->eolCRLFMi),
                                            TRUE);
        else    /* trigger the CR callback */
            gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (app->eolCRMi),
                                            TRUE);
        app->eol = app->eoldefault; /* update app->eol & app->oeol to current */
        app->oeol = app->eol;
    }   /* if selection to OS_EOL, convert to app->eolos using same method */
    else if (app->eoldefault == OS_EOL && app->eol != app->eolos) {
        if (app->eolos == LF)
            gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (app->eolLFMi),
                                            TRUE);
        else if (app->eolos == CRLF)
            gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (app->eolCRLFMi),
                                            TRUE);
        else
            gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (app->eolCRMi),
                                            TRUE);
        app->eol = app->eolos;  /* update values */
        app->oeol = app->eol;
    }

//     g_print ("settings - eoldefault: %s eoltxt[%d] : %s\n",
//             selected, app->eoldefault, app->eoltxt[app->eoldefault]);
}

/** prompt for unsaved on close */
void chkunsaved_toggled (GtkWidget *widget, gpointer data)
{
    mainwin_t *app = data;

    app->warnunsaved = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
}

/** remove trailing whitespace checkbox */
void chktrimendws_toggled (GtkWidget *widget, gpointer data)
{
    mainwin_t *app = data;

    app->trimendws = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
}

/** require POSIX end-of-line (end-of-file) checkbox */
void chkposixeof_toggled (GtkWidget *widget, gpointer data)
{
    mainwin_t *app = data;

    app->posixeof = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
}

/** set number of files shown in recent-chooser dialog */
void spinrecent_changed (GtkWidget *widget, gpointer data)
{
    mainwin_t *app = data;

    app->nrecent = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(widget));
}

