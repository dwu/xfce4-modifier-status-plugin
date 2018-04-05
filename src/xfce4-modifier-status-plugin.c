#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include <libxfce4panel/xfce-panel-plugin.h>
#include <xfconf/xfconf.h>

#include "properties-dialog.h"

typedef struct {
  GtkWidget *indicator;
  gchar *symbols;
  guint32 num_characters;
  gchar *font;
  GtkBuilder *properties_dialog_builder;
  GtkWindow *settings;
  XfconfChannel *xfconf_channel;
  GdkKeymap *keymap;
} AppContext;

static AppContext *app_context = NULL;

GString* get_modifier_string(guint modifier_state) {
  GString *result = g_string_new("");

  for (int i=0; i<=8; i++ ) {
        if (modifier_state & (1 << i)) {
            g_string_append(result, g_utf8_substring(app_context->symbols, i, i+1));
        }
    }

  return result;
}

/* Called each time a modifier key is pressed or released to update the status
  display */
void modifier_state_changed(GdkKeymap *keymap, gpointer data) {
  GString *label_text = get_modifier_string(gdk_keymap_get_modifier_state(keymap));
  gtk_label_set_text(GTK_LABEL(app_context->indicator), label_text->str);
  g_string_free(label_text, TRUE);
}

void load_and_update_config() {
  app_context->num_characters = xfconf_channel_get_uint(app_context->xfconf_channel, "/num-characters", 7);
  app_context->symbols = xfconf_channel_get_string(app_context->xfconf_channel, "/symbols", "⇧⇪⌃⌥①3⌘5");
  app_context->font = xfconf_channel_get_string(app_context->xfconf_channel, "/font", "Sans 12");

  PangoAttrList *indicator_attr = gtk_label_get_attributes(GTK_LABEL(app_context->indicator));
  if (indicator_attr == NULL) {
    indicator_attr = pango_attr_list_new();
  }
  PangoFontDescription *font = pango_font_description_from_string(app_context->font);
  pango_attr_list_change(indicator_attr, pango_attr_font_desc_new(font));
  gtk_label_set_attributes(GTK_LABEL(app_context->indicator), indicator_attr);

  gtk_label_set_width_chars(GTK_LABEL(app_context->indicator), app_context->num_characters);

  /* Update modifiers once after loading config to make sure the current symbols
    are being used */
  modifier_state_changed(app_context->keymap, app_context->indicator);
}

void on_cancel_button_clicked(GtkButton *button, gpointer data) {
  gtk_widget_hide(GTK_WIDGET(app_context->settings));
}

void on_apply_button_clicked(GtkButton *button, gpointer data) {
  /* Validate symbol list. GdkModifierType defines 8 keyboard modifiers (see
    https://developer.gnome.org/gdk3/stable/gdk3-Windows.html#GdkModifierType).
    Each modifier is represented as one character in the status display. */
  if (gtk_entry_get_text_length(GTK_ENTRY(gtk_builder_get_object(app_context->properties_dialog_builder, "symbols_entry"))) < 8) {
    return;
  }

  xfconf_channel_set_string(
    app_context->xfconf_channel,
    "/symbols",
    gtk_entry_get_text(GTK_ENTRY(gtk_builder_get_object(app_context->properties_dialog_builder, "symbols_entry"))));
  xfconf_channel_set_uint(
    app_context->xfconf_channel,
    "/num-characters",
    (guint32)gtk_adjustment_get_value(GTK_ADJUSTMENT(gtk_builder_get_object(app_context->properties_dialog_builder, "num_characters_adjustment"))));
  xfconf_channel_set_string(
    app_context->xfconf_channel,
    "/font",
    gtk_font_chooser_get_font(GTK_FONT_CHOOSER(gtk_builder_get_object(app_context->properties_dialog_builder, "font_chooser"))));

  load_and_update_config();

  gtk_widget_hide(GTK_WIDGET(app_context->settings));
}

void on_symbols_changed(GtkEditable *editable, gpointer user_data) {
  GtkEntry *symbols = GTK_ENTRY(gtk_builder_get_object(app_context->properties_dialog_builder, "symbols_entry"));

  if (gtk_entry_get_text_length(symbols) < 8) {
    gtk_entry_set_icon_from_icon_name(symbols, GTK_ENTRY_ICON_SECONDARY, "dialog-error");
    gtk_entry_set_icon_tooltip_text(symbols, GTK_ENTRY_ICON_SECONDARY, "Symbol list must be 8 characters long.");
  } else {
     gtk_entry_set_icon_from_icon_name(symbols, GTK_ENTRY_ICON_SECONDARY, NULL);
  }
}

/* Use the selected font for displaying the symbols list in the settings dialog */
void update_symbols_font_in_config() {
  GtkEntry *symbols = GTK_ENTRY(gtk_builder_get_object(app_context->properties_dialog_builder, "symbols_entry"));

  PangoAttrList *symbols_attr = gtk_entry_get_attributes(symbols);
  if (symbols_attr == NULL) {
    symbols_attr = pango_attr_list_new();
  }

  GtkFontChooser *font_chooser = GTK_FONT_CHOOSER(gtk_builder_get_object(app_context->properties_dialog_builder, "font_chooser"));
  PangoFontDescription *font = pango_font_description_from_string(gtk_font_chooser_get_font(font_chooser));
  pango_attr_list_change(symbols_attr, pango_attr_font_desc_new(font));
  gtk_entry_set_attributes(symbols, symbols_attr);
}

void on_font_activated(GtkFontChooser *font_chooser, gchar *fontname, gpointer data) {
  update_symbols_font_in_config();
}

void plugin_configure() {
  if (app_context->settings != NULL && gtk_widget_is_visible(GTK_WIDGET(app_context->settings)))
    return;

  if (app_context->properties_dialog_builder == NULL) {
    app_context->properties_dialog_builder = gtk_builder_new_from_string (properties_dialog_ui, -1);
    app_context->settings = GTK_WINDOW(gtk_builder_get_object(app_context->properties_dialog_builder, "properties_dialog"));

    g_signal_connect(gtk_builder_get_object(app_context->properties_dialog_builder, "cancel_button"), "clicked", G_CALLBACK (on_cancel_button_clicked), NULL);
    g_signal_connect(gtk_builder_get_object(app_context->properties_dialog_builder, "apply_button"), "clicked", G_CALLBACK (on_apply_button_clicked), NULL);
    g_signal_connect(gtk_builder_get_object(app_context->properties_dialog_builder, "symbols_entry"), "changed", G_CALLBACK(on_symbols_changed), NULL);
    g_signal_connect(gtk_builder_get_object(app_context->properties_dialog_builder, "font_chooser"), "notify::font", G_CALLBACK(on_font_activated), NULL);

    g_signal_connect(app_context->settings, "destroy", G_CALLBACK(gtk_widget_hide), NULL);
  }

  /* Update the values shows in the settings dialog when it is shown. Property
    binding is currently not used becuase cancelling the settings dialog without
    applying any changes should be possible. */
  gtk_font_chooser_set_font(GTK_FONT_CHOOSER(gtk_builder_get_object(app_context->properties_dialog_builder, "font_chooser")), app_context->font);
  gtk_font_chooser_set_show_preview_entry(GTK_FONT_CHOOSER(gtk_builder_get_object(app_context->properties_dialog_builder, "font_chooser")), FALSE);
  gtk_adjustment_set_value(GTK_ADJUSTMENT(gtk_builder_get_object(app_context->properties_dialog_builder, "num_characters_adjustment")), app_context->num_characters);
  gtk_entry_set_text(GTK_ENTRY(gtk_builder_get_object(app_context->properties_dialog_builder, "symbols_entry")), app_context->symbols);
  update_symbols_font_in_config();

  gtk_widget_show(GTK_WIDGET(app_context->settings));
}

static void plugin_register(XfcePanelPlugin *plugin) {
    GtkWidget *frame;

    /* Initialize app_context */
    app_context = g_new0(AppContext, 1);

    /* Initialize xfconf and setup panel configuration */
    xfconf_init(NULL);
    app_context->xfconf_channel = xfconf_channel_new_with_property_base("xfce4-panel", "/plugins/keyboard-modifier-status");
    xfce_panel_plugin_menu_show_configure(plugin);
    g_signal_connect(plugin, "configure-plugin", G_CALLBACK(plugin_configure), NULL);

    /* Draw panel indicator */
    frame = gtk_frame_new(NULL);
    gtk_container_add(GTK_CONTAINER(plugin), frame);

    app_context->indicator = gtk_label_new("");
    gtk_container_add(GTK_CONTAINER(frame), app_context->indicator);

    app_context->keymap = gdk_keymap_get_for_display(gdk_display_get_default());
    
    /* Load config and wire signals */
    load_and_update_config();
    g_signal_connect(app_context->keymap, "state-changed", G_CALLBACK (modifier_state_changed), NULL);
   
    gtk_widget_show_all(frame);
}

XFCE_PANEL_PLUGIN_REGISTER(plugin_register);