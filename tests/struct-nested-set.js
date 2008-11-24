#!/usr/local/bin/seed
// Returns: 0
// STDIN:
// STDOUT:\[object seed_union\]\n0\.000000\n300\.000000
// STDERR:
Seed.import_namespace("Gdk");

e = new Gdk.Event();
Seed.print(e);

e.expose.type = Gdk.EventType.expose;
Seed.print(e.expose.type);

r = new Gdk.Rectangle();
r.x = 300;
e.rectangle = r;
Seed.print(e.rectangle.x);