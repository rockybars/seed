#!/usr/bin/env seed

// Import libraries that are used by the program
Seed.import_namespace("Gtk");
Seed.import_namespace("Gdk");
Seed.import_namespace("Gio");

// Pretty.js is John Resig's date display library. It downloaded it and
// put it in the same directory as this script. You can easily use a lot
// of existing JS libs and import them into Seed applications at runtime.
Seed.include("pretty.js");

// Initialize GTK+
Gtk.init(null, null);

// Create the main application window and set the title
var window = new Gtk.Window({"title": "Twitter Search", "border-width": 5});
// Make the program terminate when the window is closed
window.signal.hide.connect(Gtk.main_quit);

// Modify the default style so that TextView widgets look like labels
Gtk.rc_parse_string(
	'style "tv" {base[NORMAL] = @bg_color} widget_class "*GtkTextView" style "tv"');

// This function generates the GTK+ widgets that display the retrieved messages
function make_block(data) {
	var vbox = new Gtk.VBox({"spacing": 10, "border-width": 5});

	// The text styling for the heading is done with simple Pango markup.
	var heading = new Gtk.Label({
			"use-markup": true,
			"label": "<b><big>" + data.from_user + "</big></b> " +
			"(<small>" + prettyDate(data.created_at) + "</small>)"
		});

	// The message text is displayed in a TextView widget because the GTK+ label
	// widget completely sucks at wrapping text. The TextView will look like a
	// label because of the RC change at the top of the script.
	var message = new Gtk.TextView({"wrap-mode": 2, "editable": false});
	message.buffer.text = data.text;
  
	heading.set_alignment(0, 0);
	vbox.pack_start(heading);
	vbox.pack_start(message);

	var frame = new Gtk.Frame({"border-width": 5});
	frame.add(vbox);
	return frame;
}

// Create the message container and put it in a scrollable window
var messages = new Gtk.VBox();
var scroll = new Gtk.ScrolledWindow();
scroll.add_with_viewport(messages);
scroll.set_policy(1, 1);

// Create the input textbox and the search button
var textbox = new Gtk.Entry();
var button = new Gtk.Button({"label": "_Search", "use-underline": true});


function async_callback(source, result)
{
	var stream = source.read_finish(result);
	var dstream = new Gio.DataInputStream.c_new(stream);
	var data = JSON.parse(dstream.read_until("", 0));
	
	messages.foreach(function(m) {messages.remove(m)});
	data.results.forEach(function(m) {messages.pack_start(make_block(m))});
	
	messages.show_all();
};

// Define the behavior for the button press by associating an anonymous
// function with the button's click signal handler
button.signal.clicked.connect(function(w) {
		var twitter = 
			Gio.file_new_for_uri("http://search.twitter.com/search.json?q="
								 + textbox.get_text());
		
		twitter.read_async(0, null, async_callback);
	});


// Pack the remaining widgets into the window layout
var searchbox = new Gtk.HBox();
searchbox.pack_start(textbox, true, true);
searchbox.pack_start(button);

var layout = new Gtk.VBox({"spacing": 5});
layout.pack_start(searchbox);
layout.pack_start(scroll, true, true);

window.add(layout);
window.show_all();

// Start the main GTK+ loop and initiate the program
Gtk.main();

