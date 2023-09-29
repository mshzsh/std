#include <gtk/gtk.h>
#include <gdk/gdkwayland.h>
#include <wayland-client.h>
#include <stdlib.h>
#include <stdio.h>
#include <cstdlib>
#include <iostream>
#include <string>

// Wayland globals
static struct wl_display *display = NULL;
static struct wl_registry *registry = NULL;
static struct wl_compositor *compositor = NULL;
static struct wl_seat *seat = NULL;
static struct wl_surface *surface = NULL;

// GTK globals
static GtkWidget *panel;
static GtkWidget *textbox;

// Generate dynamic path
std::string getDynamicPath(const std::string& relativePath) {
    const char* homePath = std::getenv("HOME");
    if (homePath != nullptr) {
        std::string homeDirectory(homePath);
        std::string fullPath = homeDirectory + "/" + relativePath;
        return fullPath;
    } else {
        std::cerr << "HOME environment variable is not set." << std::endl;
        return "";
    }
}


static void registry_handle_global(
    void *data,
    struct wl_registry *registry,
    uint32_t id,
    const char *interface,
    uint32_t version
) {
    // Handle Wayland global objects here, e.g., wl_compositor, wl_seat
}

static void registry_handle_global_remove(
    void *data,
    struct wl_registry *registry,
    uint32_t name
) {
    // Handle removal of Wayland global objects here
}

static const struct wl_registry_listener registry_listener = {
    registry_handle_global,
    registry_handle_global_remove
};

static void on_textbox_activate(GtkEntry *entry, gpointer user_data) {
    const gchar *text = gtk_entry_get_text(entry);

    // Use the "wl-clipboard" command-line tool to copy text to clipboard
    char *command = g_strdup_printf("wl-copy \"%s\"", text);
    system(command);
    g_free(command);

    gtk_main_quit();
}


static void setup_wayland() {
    display = wl_display_connect(NULL);
    registry = wl_display_get_registry(display);
    wl_registry_add_listener(registry, &registry_listener, NULL);
    wl_display_roundtrip(display);
}

int main(int argc, char **argv) {
    gtk_init(&argc, &argv);

    // Initialize Wayland
    setup_wayland();

    // Create a GTK window (panel)
    panel = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(panel), "STD");
    gtk_window_set_default_size(GTK_WINDOW(panel), 800, 50);
    g_signal_connect(panel, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Create a GTK entry (textbox)
    textbox = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(textbox), "Type here...");
    gtk_widget_set_name(textbox, "textbox"); // Set a custom name for the textbox

    // Load and apply the CSS file to the entire application
    GtkCssProvider *cssProvider = gtk_css_provider_new();
    std::string stylesPath = getDynamicPath(".config/hypr/secure_text_display/styles.css");
    gtk_css_provider_load_from_path(cssProvider,stylesPath.c_str(), NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(cssProvider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_signal_connect(textbox, "activate", G_CALLBACK(on_textbox_activate), NULL);

    // Add the textbox to the panel
    gtk_container_add(GTK_CONTAINER(panel), textbox);

    // Show everything
    gtk_widget_show_all(panel);

    // Enter the GTK main loop
    gtk_main();

    // Cleanup Wayland
    wl_surface_destroy(surface);
    wl_seat_destroy(seat);
    wl_registry_destroy(registry);
    wl_display_disconnect(display);

    return 0;
}
