INSTALL_PREFIX=/usr
BIN_DIR=$(INSTALL_PREFIX)/bin
APP_DIR=$(INSTALL_PREFIX)/share/com.github.fabiocolacio.marker
STYLES_DIR=$(APP_DIR)/styles

BIN_NAME=com.github.fabiocolacio.marker

MARKER_CFLAGS=-DSTYLES_DIR="\"/usr/share/com.github.fabiocolacio.marker"\" -DCONF_FILE="\"/usr/share/com.github.fabiocolacio.marker/marker.conf"\"

build: clean
	mkdir -p build/objects
	
	cd src/resources ; \
	glib-compile-resources marker.gresource.xml --target="../resources.c" --generate-source
	
	cc -c src/hoedown/autolink.c -o build/objects/hoedown_autolink.o
	cc -c src/hoedown/buffer.c -o build/objects/hoedown_buffer.o
	cc -c src/hoedown/document.c -o build/objects/hoedown_document.o
	cc -c src/hoedown/escape.c -o build/objects/hoedown_escape.o
	cc -c src/hoedown/html.c -o build/objects/hoedown_html.o
	cc -c src/hoedown/html_blocks.c -o build/objects/hoedown_html_blocks.o
	cc -c src/hoedown/html_smartypants.c -o build/objects/hoedown_html_smartypants.o
	cc -c src/hoedown/stack.c -o build/objects/hoedown_stack.o
	cc -c src/hoedown/version.c -o build/objects/hoedown_version.o
	
	cc -c src/marker.c -o build/objects/marker.o `pkg-config --cflags gtk+-3.0` $(MARKER_CFLAGS)
	cc -c src/marker-editor-window.c -o build/objects/marker-editor-window.o `pkg-config --cflags gtk+-3.0 gtksourceview-3.0 webkitgtk-3.0` $(MARKER_CFLAGS)
	cc -c src/marker-markdown.c -o build/objects/marker-markdown.o $(MARKER_CFLAGS)
	cc -c src/marker-prefs.c -o build/objects/marker-prefs.o `pkg-config --cflags gtk+-3.0 gtksourceview-3.0` $(MARKER_CFLAGS)
	cc -c src/marker-source-view.c -o build/objects/marker-source-view.o `pkg-config --cflags gtksourceview-3.0` $(MARKER_CFLAGS)
	cc -c src/marker-string.c -o build/objects/marker-string.o $(MARKER_CFLAGS)
	cc -c src/marker-widget.c -o build/objects/marker-widget.o `pkg-config --cflags gtk+-3.0` $(MARKER_CFLAGS)
	
	cc -c src/resources.c -o build/objects/resources.o `pkg-config --cflags glib-2.0`
	
	cc build/objects/*.o -o build/$(BIN_NAME) `pkg-config --libs gtk+-3.0 glib-2.0 gtksourceview-3.0 webkitgtk-3.0`
	
install:
	mkdir -p $(APP_DIR)
	cp -r data/styles $(APP_DIR)
	cp data/marker.config $(APP_DIR)
	cp data/com.github.fabiocolacio.marker.desktop $(INSTALL_PREFIX)/share/applications
	cp data/com.github.fabiocolacio.marker.svg $(INSTALL_PREFIX)/share/icons/hicolor/scalable/apps
	cp data/com.github.fabiocolacio.marker-symbolic.svg $(INSTALL_PREFIX)/share/icons/hicolor/symbolic/apps
	gtk-update-icon-cache -f -t $(INSTALL_PREFIX)/share/icons/hicolor
	cp build/$(BIN_NAME) $(BIN_DIR)
	
clean:
	rm -rf build
