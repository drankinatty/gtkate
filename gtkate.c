#include "gtk_mainwin.h"

int main (int argc, char **argv) {

#ifndef GLIB236
    g_type_init();
#endif
    mainwin_t *mwin = NULL;
    mwin = g_slice_new (mainwin_t);     /* allocate mainwin struct */
    mainwin_init (mwin, argv);          /* initialize mainwin values */

    // gtk_init (&argc, &argv);        /* terminates if can't initialize */
    if (!gtk_init_check (&argc, &argv)) {   /* same but doesn't exit */
        g_fprintf (stderr, "error: gtk_init failed.\n");
        return 1;
    }

    if (!create_window (mwin)) {        /* create the main window   */
        g_fprintf (stderr, "error: create_window failed.\n");
        return 1;
    }

    gtk_main ();

    mainwin_destroy (mwin);             /* free allocated members */
    g_slice_free (mainwin_t, mwin);     /* free mainwin struct */

    return 0;
}
