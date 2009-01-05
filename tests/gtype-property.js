#!/usr/bin/env seed
// Returns: 0
// STDIN:
// STDOUT:1\.000000\n0\.000000
// STDERR:
   // Returns: 0
   // STDIN:
   // STDOUT:In klass init\nIn constructor for \[object HelloWindow\]\nPrototypes!\nIn map, verifying widget\.title : Hello!
   // STDERR:

Seed.import_namespace("Gtk");
Seed.import_namespace("GObject");
Gtk.init(null, null);

HelloWindowType = {
parent: Gtk.Window.type,
name: "HelloWindow",
class_init: function(klass, prototype)
{
	klass.install_property(GObject.param_spec_boolean("test",
													  "test property",
													  "A test property!",
													  false,
													  GObject.ParamFlags.Readable | GObject.ParamFlags.Writable));
},
instance_init: function(klass)
{
	
}};

HelloWindow = new GType(HelloWindowType);
w = new HelloWindow({test: true});
Seed.print(w.test);
w = new HelloWindow();
Seed.print(w.test);
	  