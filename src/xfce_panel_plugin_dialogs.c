/*
 Copyright (C) 2022 Xu Zhen
 
 This file is part of DockbarX Xfce Panel Plugin.
 
 DockbarX Xfce Panel Plugin is free software: you can redistribute it and/or
 modify it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or (at your
 option) any later version.
 
 DockbarX Xfce Panel Plugin is distributed in the hope that it will be
 useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
 Public License for more details.
 
 You should have received a copy of the GNU General Public License along
 with this file. If not, see <http://www.gnu.org/licenses/>.
*/
#include "xfce_panel_plugin.h"
#include "xfce_panel_plugin_props.h"
#include <glib/gi18n.h>
#include "config.h"

static DockbarXPlugin *plugin;
static GtkWidget *pref_dialog;
static GtkWidget *pref_bottom_radio;
static GtkWidget *pref_top_radio;
static GtkWidget *pref_orient_frame;
static GtkWidget *pref_color_radio;
static GtkWidget *pref_image_radio;
static GtkWidget *pref_blend_radio;
static GtkWidget *pref_color_button;
static GtkWidget *pref_image_button;
static GtkWidget *pref_offset_spin;
static GtkWidget *pref_max_size_spin;
static GtkWidget *pref_expand_check;
static GtkWidget *about_dialog;

static void pref_bottom_radio_toggled(GtkToggleButton *togglebutton, GObject *props) {
    if (gtk_toggle_button_get_active(togglebutton)) {
        GtkOrientation orient = xfce_panel_plugin_get_orientation(plugin->plugin);
        prop_set_orient(props, (orient == GTK_ORIENTATION_HORIZONTAL) ? "bottom" : "left");
    }
}

static void pref_top_radio_toggled(GtkToggleButton *togglebutton, GObject *props) {
    if (gtk_toggle_button_get_active(togglebutton)) {
        GtkOrientation orient = xfce_panel_plugin_get_orientation(plugin->plugin);
        prop_set_orient(props, (orient == GTK_ORIENTATION_HORIZONTAL) ? "top" : "right");
    }
}

static void bgmode_toggled(GtkToggleButton *togglebutton, gpointer value) {
    if (gtk_toggle_button_get_active(togglebutton)) {
        prop_set_bgmode(plugin->props, (gintptr)value);
    }
}

static void pref_color_button_color_set(GtkColorButton *widget, GObject *props) {
    GdkRGBA rgba;
    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(widget), &rgba);
    gchar *color = gdk_rgba_to_string(&rgba);
    prop_set_color(props, color);
    g_free(color);
}

static void pref_image_button_file_set(GtkFileChooserButton *widget, GObject *props) {
    gchar *file = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widget));
    if (file != NULL) {
        prop_set_image(props, file);
        g_free(file);
    }
}

static void pref_offset_spin_value_changed(GtkSpinButton *spin_button, GObject *props) {
    prop_set_offset(props, gtk_spin_button_get_value_as_int(spin_button));
}

static void pref_max_size_spin_value_changed(GtkSpinButton *spin_button, GObject *props) {
    prop_set_max_size(props, gtk_spin_button_get_value_as_int(spin_button));
}

static void pref_expand_check_toggled(GtkToggleButton *togglebutton, GObject *props) {
    prop_set_expand(props, gtk_toggle_button_get_active(togglebutton));
}

static void create_pref_dialog() {
    pref_dialog = gtk_dialog_new_with_buttons("DockbarX Plugin Preferences", NULL, GTK_DIALOG_MODAL,
                                              _("_DockbarX Settings"), 1,
                                              _("_Close"), GTK_RESPONSE_CLOSE,
                                              NULL);
    xfce_panel_plugin_take_window(plugin->plugin, GTK_WINDOW(pref_dialog));
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(pref_dialog));
    gtk_box_set_spacing(GTK_BOX(content), 12);
    gtk_widget_set_margin_start(content, 12);
    gtk_widget_set_margin_end(content, 12);
    gtk_widget_set_margin_top(content, 12);
    gtk_widget_set_margin_bottom(content, 12);
    gtk_orientable_set_orientation(GTK_ORIENTABLE(content), GTK_ORIENTATION_VERTICAL);

    pref_orient_frame = gtk_frame_new(_("Orientation"));
    GtkWidget *orient_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    pref_bottom_radio = gtk_radio_button_new(NULL);
    pref_top_radio = gtk_radio_button_new_from_widget(GTK_RADIO_BUTTON(pref_bottom_radio));
    gtk_widget_set_margin_start(orient_box, 12);
    gtk_widget_set_margin_end(orient_box, 8);
    gtk_widget_set_margin_top(orient_box, 4);
    gtk_widget_set_margin_bottom(orient_box, 4);
    gtk_box_pack_start(GTK_BOX(orient_box), pref_bottom_radio, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(orient_box), pref_top_radio, TRUE, TRUE, 0);
    gtk_container_add(GTK_CONTAINER(pref_orient_frame), orient_box);
    gtk_box_pack_start(GTK_BOX(content), pref_orient_frame, FALSE, FALSE, 0);

    pref_blend_radio = gtk_radio_button_new_with_label(NULL, _("Blend with panel"));
    gtk_box_pack_start(GTK_BOX(content), pref_blend_radio, TRUE, FALSE, 0);

    GtkWidget *color_frame = gtk_frame_new(NULL);
    pref_color_radio = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(pref_blend_radio), _("Solid color"));
    GtkWidget *color_table = gtk_grid_new();
    GtkWidget *color_label = gtk_label_new(_("Color:"));
    pref_color_button = gtk_color_button_new();
    gtk_frame_set_label_widget(GTK_FRAME(color_frame), pref_color_radio);
    gtk_widget_set_margin_start(color_table, 12);
    gtk_widget_set_margin_end(color_table, 8);
    gtk_widget_set_margin_top(color_table, 4);
    gtk_widget_set_margin_bottom(color_table, 4);
    gtk_grid_set_column_spacing(GTK_GRID(color_table), 8);
    gtk_widget_set_hexpand(pref_color_button, TRUE);
    gtk_color_chooser_set_use_alpha(GTK_COLOR_CHOOSER(pref_color_button), TRUE);
    gtk_grid_attach(GTK_GRID(color_table), color_label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(color_table), pref_color_button, 1, 0, 1, 1);
    gtk_container_add(GTK_CONTAINER(color_frame), color_table);
    gtk_box_pack_start(GTK_BOX(content), color_frame, FALSE, FALSE, 0);

    GtkWidget *image_frame = gtk_frame_new(NULL);
    pref_image_radio = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(pref_color_radio), _("Background image"));
    GtkWidget *image_table = gtk_grid_new();
    GtkWidget *image_label = gtk_label_new(_("Image:"));
    pref_image_button = gtk_file_chooser_button_new(_("Select background image"), GTK_FILE_CHOOSER_ACTION_OPEN);
    GtkWidget *offset_label = gtk_label_new(_("Offset:"));
    pref_offset_spin = gtk_spin_button_new_with_range(-32767, 32767, 1);
    gtk_frame_set_label_widget(GTK_FRAME(image_frame), pref_image_radio);
    gtk_widget_set_margin_start(image_table, 12);
    gtk_widget_set_margin_end(image_table, 8);
    gtk_widget_set_margin_top(image_table, 4);
    gtk_widget_set_margin_bottom(image_table, 4);
    gtk_grid_set_column_spacing(GTK_GRID(image_table), 8);
    gtk_grid_set_row_spacing(GTK_GRID(image_table), 4);
    gtk_widget_set_hexpand(pref_image_button, TRUE);
    gtk_widget_set_hexpand(pref_offset_spin, TRUE);
    gtk_grid_attach(GTK_GRID(image_table), image_label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(image_table), offset_label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(image_table), pref_image_button, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(image_table), pref_offset_spin, 1, 1, 1, 1);
    gtk_container_add(GTK_CONTAINER(image_frame), image_table);
    gtk_box_pack_start(GTK_BOX(content), image_frame, FALSE, FALSE, 0);

    GtkWidget *size_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
    GtkWidget *max_size_label = gtk_label_new(_("Max size:"));
    pref_max_size_spin = gtk_spin_button_new_with_range(0, 32767, 1);
    pref_expand_check = gtk_check_button_new_with_label(_("Expand"));
    gtk_box_pack_start(GTK_BOX(size_box), max_size_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(size_box), pref_max_size_spin, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(size_box), pref_expand_check, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), size_box, FALSE, FALSE, 0);

    g_signal_connect(pref_bottom_radio, "toggled", G_CALLBACK(pref_bottom_radio_toggled), plugin->props);
    g_signal_connect(pref_top_radio, "toggled", G_CALLBACK(pref_top_radio_toggled), plugin->props);

    g_signal_connect(pref_color_radio, "toggled", G_CALLBACK(bgmode_toggled), (gpointer)0);
    g_signal_connect(pref_image_radio, "toggled", G_CALLBACK(bgmode_toggled), (gpointer)1);
    g_signal_connect(pref_blend_radio, "toggled", G_CALLBACK(bgmode_toggled), (gpointer)2);

    g_signal_connect(pref_color_button, "color-set", G_CALLBACK(pref_color_button_color_set), plugin->props);

    g_signal_connect(pref_image_button, "file-set", G_CALLBACK(pref_image_button_file_set), plugin->props);

    g_signal_connect(pref_offset_spin, "value-changed", G_CALLBACK(pref_offset_spin_value_changed), plugin->props);
    g_signal_connect(pref_max_size_spin, "value-changed", G_CALLBACK(pref_max_size_spin_value_changed), plugin->props);

    g_signal_connect(pref_expand_check, "toggled", G_CALLBACK(pref_expand_check_toggled), plugin->props);

    gtk_widget_show_all(content);
    gtk_window_set_resizable(GTK_WINDOW(pref_dialog), FALSE);
}

static void create_about_dialog() {
    const gchar *authors[] = { "Aleksey Shaferov", "Matias Sars", "Trent McPheron", "Ted Alff", "Xu Zhen", NULL };
    const gchar *copyright = "Copyright (C) 2008-2013 Aleksey Shaferov\n"
                             "Copyright (C) 2008-2016 Trent McPheron\n"
                             "Copyright (C) 2008-2020 Matias Sars\n"
                             "Copyright (C) 2020 Ted Alff\n"
                             "Copyright (C) 2020-2022 Xu Zhen";
    const gchar *license = "This program is free software: you can redistribute it and/or "
                           "modify it under the terms of the GNU General Public License as published by "
                           "the Free Software Foundation, either version 3 of the License, or (at your "
                           "option) any later version.\n\n"
                           "This program is distributed in the hope that it will be "
                           "useful, but WITHOUT ANY WARRANTY; without even the implied warranty of "
                           "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General "
                           "Public License for more details.\n\n"
                           "You should have received a copy of the GNU General Public License along "
                           "with this program. If not, see <http://www.gnu.org/licenses/>.";
    about_dialog = gtk_about_dialog_new();
    GtkAboutDialog *d = GTK_ABOUT_DIALOG(about_dialog);
    gtk_about_dialog_set_program_name(d, "DockbarX Xfce Panel Plugin");
    gtk_about_dialog_set_version(d, PLUGIN_VERSION);
    gtk_about_dialog_set_copyright(d, copyright);
    gtk_about_dialog_set_comments(d, "Enjoy the DockbarX in Xfce panel.");
    gtk_about_dialog_set_license(d, license);
    gtk_about_dialog_set_wrap_license(d, TRUE);
    gtk_about_dialog_set_website(d, "https://github.com/xuzhen/xfce4-dockbarx-plugin");
    gtk_about_dialog_set_authors(d, authors);
    gtk_about_dialog_set_logo_icon_name(d, "dockbarx");
    xfce_panel_plugin_take_window(plugin->plugin, GTK_WINDOW(about_dialog));
}

void create_dialogs(DockbarXPlugin *dbx_plugin) {
    plugin = dbx_plugin;
    create_pref_dialog();
    create_about_dialog();
}

void show_pref_dialog() {
    // Bottom/Top change to Left/Right if the panel's vertical.
    GtkOrientation panel_orient = xfce_panel_plugin_get_orientation(plugin->plugin);
    if (panel_orient == GTK_ORIENTATION_VERTICAL) {
        gtk_button_set_label(GTK_BUTTON(pref_bottom_radio), _("Left"));
        gtk_button_set_label(GTK_BUTTON(pref_top_radio), _("Right"));
    } else {
        gtk_button_set_label(GTK_BUTTON(pref_bottom_radio), _("Bottom"));
        gtk_button_set_label(GTK_BUTTON(pref_top_radio), _("Top"));
    }

    XfceScreenPosition pos = xfce_panel_plugin_get_screen_position(plugin->plugin);
    if (pos == XFCE_SCREEN_POSITION_FLOATING_H || pos == XFCE_SCREEN_POSITION_FLOATING_V || pos == XFCE_SCREEN_POSITION_NONE) {
        gtk_widget_show(pref_orient_frame);
    } else {
        gtk_widget_hide(pref_orient_frame);
    }

    gchar *orient = prop_get_orient(plugin->props);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pref_bottom_radio), g_strcmp0(orient, "bottom") == 0 || g_strcmp0(orient, "left") == 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pref_top_radio), g_strcmp0(orient, "top") == 0 || g_strcmp0(orient, "right") == 0);
    g_free(orient);

    int bgmode = prop_get_bgmode(plugin->props);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pref_color_radio), bgmode == 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pref_image_radio), bgmode == 1);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pref_blend_radio), bgmode == 2);

    GdkRGBA rgba;
    gchar *color = prop_get_color(plugin->props);
    gdk_rgba_parse(&rgba, color);
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(pref_color_button), &rgba);
    g_free(color);

    gchar *image = prop_get_image(plugin->props);
    gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(pref_image_button), image);
    g_free(image);
        
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(pref_offset_spin), prop_get_offset(plugin->props));
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(pref_max_size_spin), prop_get_max_size(plugin->props));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pref_expand_check), prop_get_expand(plugin->props));

    while (TRUE) {
        gint result = gtk_dialog_run(GTK_DIALOG(pref_dialog));
        if (result > 0) {
            if (g_spawn_command_line_async("dbx_preference", NULL) == FALSE) {
                GtkWidget *msg_dialog = gtk_message_dialog_new(GTK_WINDOW(pref_dialog), 0, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, _("Failed to run DockbarX preferences"));
                gtk_dialog_run(GTK_DIALOG(msg_dialog));
                gtk_widget_destroy(msg_dialog);
            }
        } else {
            gtk_widget_hide(pref_dialog);
            break;
        }
    }
}

void show_about_dialog() {
    gtk_dialog_run(GTK_DIALOG(about_dialog));
    gtk_widget_hide(about_dialog);
}

