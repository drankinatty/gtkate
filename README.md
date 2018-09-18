# gtkate
**GtKate** - Multi-Document Interface Text Editor written in C and Gtk+2.

GtKate is currently in interface design and the code is simply proof of concept at the moment. The interface takes the single-document interface of [GtkWrite](https://github.com/drankinatty/gtkwrite) and incorporates it into a mulit-document interface with a GtkTreeView of open documents that can be shown/hidden. The document window can be split into separate editor view with independent statusbar of the same or different files for editing (upto a current max of 4 views shown).

The inteface is currently created with a default single edit view shown. Each split will add an additional edit view of the currently focused document (by default) and the treeview can then be used to change the document shown in any split. The new edit view is added at the bottom of the right edit-window region and all splits are allocated equal space within the overall edit window. Any split may be removed (regardless of order or position) until a single view remains. The focused split is the one removed.

The initial code here and concept is to provide a menu, toolbar, hpaned region for the scrollable TreeView on left and TextView windows with statusbar on the right. The current code produces the inteface similar to the following for tweaking, e.g.


[GtKate Interface](http://paste.opensuse.org/52951577)

**Current Development State**

All widgets are just placeholders at the moment, but the initial interface is developing. Both toolbar and document tree can be hidden/shown. The edit window can be split in up to 4 horizontal panes showing any combination of files in the four views (e.g. from showing 4 different view of a single file to displaying 4 separate file one in each view). New-File logic is complete keeping track of up to 32 `"Untitled(n)"` new file instances. Any number of existing files can be opened (up to the memory available) and the filenames are displayed in the treeview document tree. File-Close is operational removing a file by closing all views currently displaying the file and removing the file from the document tree. File-Save is implemented. Goto dialog is implemented, Find/Replace is implemented. Settings dialog is implemented. GtkSourceview2 syntax highlight initial implemnentation. Word completion not yet implemented, but tested and working for single instance of buffer in sourceview, but not when switching buffers within an existing view. GFileMonitor'ing has yet been implemented. You can see [TODOtxt](TODO.txt) for a few additional notes.

**Compiling**

For Linux, all that is needed is `gcc/make/pkg-config` and `Gtk+2` and `GtkSourceview2`. (note some distributions package the headers and development files in separate packages, for instance `Gtk+2-dev` or `GtkSourceview2-dev`). You may want to create an out-of-source directory for building to prevent cluttering your sources with the object and executable files. Simply create a separate directory (e.g. `gtkate.build`) and then symlink the `Makefile` and `gtk_*` source and header files. All that is needed then is to change to the build directory and type:

     $ make

For building on Windows, see the notes on obtaining the precompiled Gtk libraries and header files in the [GtkWrite Readme.md](https://github.com/drankinatty/gtkwrite). (note: there is no `with=-DWGTKSOURCEVIEW2` needed with GtKate as it builds with GtkSourceview2 by default. To build with the windows susbsystem, simply include `ss=-DWIN` along with `make`, e.g.

    $ make ss=-DWIN

You will need to adjust the Makefile depending on whether you are using MinGW/Msys or TDM-MinGW.

**License, Image File, and Style Placement**

For details on where the image files, license file, styles and .gtkrc files should be placed for both Linux and Windows -- see the [GtkWrite Readme.md](https://github.com/drankinatty/gtkwrite) but adjust the directory names and image names from `gtkwrite` to `gtkate` and the license file is renamed `LICENSE` for this project (done by github by default).

**Branch Information**

Branches, the katedev branch will be the development branch for major changes leaving master in compilable state tracking more settled decisions regarding the interface and later for the full editor.