#!/usr/bin/env seed
// Returns: 0
// STDIN:
// STDOUT:\{"name":"set_opacity","return_type":"void","args":\{"0":\{"type":"double"\}\}\}
// STDERR:

Seed.import_namespace("Gtk");
Gtk.init(null, null);
win = new Gtk.Window();
Seed.print(JSON.stringify(Seed.introspect(win.set_opacity)));