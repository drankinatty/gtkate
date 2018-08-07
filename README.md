# gtkate
**GtKate** - Multi-Document Interface Text Editor written in C and Gtk+2.

GtKate is currently in interface design and the code is simply proof of concept at the moment. The interface takes the single-document interface of GtkWrite and incorporates it into a mulit-document interface with a GtkTreeView of open documents that can be shown/hidden. The initial concept is to provide a menu, toolbar, hpaned region for the scrollable TreeView and TextView windows and a statusbar on the bottom. The current code produces the inteface for tweaking, e.g.


![GtKate Interface](http://paste.opensuse.org/52951577)


Basically all widgets are just placeholders at the moment, but the initial interface is encouraging. Both toolbar and document tree can be hidden/shown with current accelerators `[Ctrl+t]` and `[Ctrl+d]`, respectively for temporary testing.
