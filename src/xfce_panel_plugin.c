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
#include "xfce_panel_plugin_dialogs.h"
#include <gtk/gtkx.h>
#include <glib/gi18n.h>
#include <signal.h>
#include <errno.h>
#include "config.h"

static GObject *properties = NULL;
static GMutex mutex;
static GPid pid = 0;
static gboolean embeded = FALSE;

static gboolean determine_orient(DockbarXPlugin *dbx_plugin);
static void run_plug(DockbarXPlugin *dbx_plugin);

static void block_panel_autohide(GObject *gobject, GParamSpec *pspec, gpointer user_data);
static gboolean on_size_changed(XfcePanelPlugin *plugin, int size, gpointer user_data);
static void on_orientation_changed(XfcePanelPlugin *plugin, GtkOrientation orientation, DockbarXPlugin *dbx_plugin);
static void on_screen_position_changed(XfcePanelPlugin *plugin, XfceScreenPosition position, DockbarXPlugin *dbx_plugin);
static void on_plug_added(GtkSocket *socket, DockbarXPlugin *dbx_plugin);
static gboolean on_plug_removed(GtkSocket *socket, DockbarXPlugin *dbx_plugin);
static void on_free_data(XfcePanelPlugin *plugin, DockbarXPlugin *dbx_plugin);
static void on_configure_plugin(XfcePanelPlugin *plugin, gpointer user_data);
static void on_about(XfcePanelPlugin *plugin, gpointer user_data);

static gboolean dbx_plugin_check(G_GNUC_UNUSED GdkScreen *screen) {
    GError *error = NULL;
    if (xfconf_init(&error) == FALSE) {
        g_critical("Failed to init xfconf: %s", error->message);
        g_error_free(error);
        return FALSE;
    }
    properties = g_object_new(PLUGIN_PROPERTIES_TYPE, NULL);
    if (properties == NULL) {
        g_critical("Failed to create properties gobject");
        return FALSE;
    }
    return TRUE;
}

static void dbx_plugin_construct(XfcePanelPlugin *plugin) {
    DockbarXPlugin *dbx_plugin;

    dbx_plugin = g_slice_new0(DockbarXPlugin);
    dbx_plugin->plugin = plugin;
    dbx_plugin->xfc = xfconf_channel_new_with_property_base("xfce4-panel", xfce_panel_plugin_get_property_base(plugin));
    dbx_plugin->props = properties;

    prop_bind_xfconf(dbx_plugin->xfc, properties);
    prop_connect_block_ah(properties, G_CALLBACK(block_panel_autohide), plugin);

    create_dialogs(dbx_plugin);
    xfce_panel_plugin_menu_show_configure(plugin);
    xfce_panel_plugin_menu_show_about(plugin);

    g_signal_connect(G_OBJECT(plugin), "configure-plugin", G_CALLBACK(on_configure_plugin), NULL);
    g_signal_connect(G_OBJECT(plugin), "about", G_CALLBACK(on_about), NULL);
    g_signal_connect(G_OBJECT(plugin), "size-changed", G_CALLBACK(on_size_changed), NULL);
    g_signal_connect(G_OBJECT(plugin), "orientation-changed", G_CALLBACK(on_orientation_changed), dbx_plugin);
    g_signal_connect(G_OBJECT(plugin), "screen-position-changed", G_CALLBACK(on_screen_position_changed), dbx_plugin);
    g_signal_connect(G_OBJECT(plugin), "free-data", G_CALLBACK(on_free_data), dbx_plugin);

    dbx_plugin->socket = gtk_socket_new();
    gtk_container_add(GTK_CONTAINER(plugin), dbx_plugin->socket);
    dbx_plugin->socket_id = gtk_socket_get_id(GTK_SOCKET(dbx_plugin->socket));
    g_signal_connect(dbx_plugin->socket, "plug-added", G_CALLBACK(on_plug_added), dbx_plugin);
    g_signal_connect(dbx_plugin->socket, "plug-removed", G_CALLBACK(on_plug_removed), dbx_plugin);

    gtk_widget_show_all(GTK_WIDGET(plugin));

    g_mutex_init(&mutex);

    determine_orient(dbx_plugin);
    run_plug(dbx_plugin);
}
XFCE_PANEL_PLUGIN_REGISTER_WITH_CHECK(dbx_plugin_construct, dbx_plugin_check);

// Starts DBX when the plugin starts, or when something kills it.
static void run_plug(DockbarXPlugin *dbx_plugin) {
    if (g_mutex_trylock(&mutex) == FALSE) {
        return;
    }
    gint unique_id = xfce_panel_plugin_get_unique_id(dbx_plugin->plugin);
    if (pid != 0) {
        if (kill(pid, SIGINT) == -1) {
            g_warning("Failed to stop DockbarX plug process %d: %s", pid, strerror(errno));
        }
    }
    gchar *argv[7] = {
        "python3",
        DOCKBARX_PATH "/xfce4-panel-plug",
        "-s",
        g_strdup_printf("%lu", dbx_plugin->socket_id),
        "-i",
        g_strdup_printf("%d", unique_id),
        NULL
    };
    GError *error = NULL;
    if (g_spawn_async(NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, &pid, &error) == FALSE) {
        GtkWidget *dialog = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, _("Failed to start DockbarX plug."));
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        g_error_free(error);
        pid = 0;
    }
    g_free(argv[3]);
    g_free(argv[5]);
    g_mutex_unlock(&mutex);
}

static gboolean determine_orient(DockbarXPlugin *dbx_plugin) {
    XfceScreenPosition pos = xfce_panel_plugin_get_screen_position(dbx_plugin->plugin);
    gchar *orient;
    gchar *orig_orient = prop_get_orient(dbx_plugin->props);
    switch (pos) {
        case XFCE_SCREEN_POSITION_S:
        case XFCE_SCREEN_POSITION_SE_H:
        case XFCE_SCREEN_POSITION_SW_H:
            orient = "bottom";
            break;
        case XFCE_SCREEN_POSITION_N:
        case XFCE_SCREEN_POSITION_NE_H:
        case XFCE_SCREEN_POSITION_NW_H:
            orient = "top";
            break;
        case XFCE_SCREEN_POSITION_W:
        case XFCE_SCREEN_POSITION_NW_V:
        case XFCE_SCREEN_POSITION_SW_V:
            orient = "left";
            break;
        case XFCE_SCREEN_POSITION_E:
        case XFCE_SCREEN_POSITION_NE_V:
        case XFCE_SCREEN_POSITION_SE_V:
            orient = "right";
            break;
        case XFCE_SCREEN_POSITION_FLOATING_H:
        case XFCE_SCREEN_POSITION_FLOATING_V:
        case XFCE_SCREEN_POSITION_NONE:
        default: {
            GtkOrientation panel_orient = xfce_panel_plugin_get_orientation(dbx_plugin->plugin);
            // Swap orientations if necessary.
            if (panel_orient == GTK_ORIENTATION_HORIZONTAL) {
                if (g_strcmp0(orig_orient, "left") == 0) {
                    orient = "bottom";
                } else if (g_strcmp0(orig_orient, "right") == 0) {
                    orient = "top";
                } else {
                    orient = orig_orient;
                }
            } else {
                if (g_strcmp0(orig_orient, "bottom") == 0) {
                    orient = "left";
                } else if (g_strcmp0(orig_orient, "top") == 0) {
                    orient = "right";
                } else {
                    orient = orig_orient;
                }
            }
            break;
        }
    }
    gboolean r;
    if (g_strcmp0(orig_orient, orient) != 0) {
        prop_set_orient(dbx_plugin->props, orient);
        r = TRUE;
    } else {
        r = FALSE;
    }
    g_free(orig_orient);
    return r;
}

static void reset_plug_orient(DockbarXPlugin *dbx_plugin) {
    if (determine_orient(dbx_plugin)) {
        if (pid != 0 && embeded) {
            kill(pid, SIGUSR1);
        } else {
            run_plug(dbx_plugin);
        }
    }
}
static void block_panel_autohide(GObject *gobject, G_GNUC_UNUSED GParamSpec *pspec, gpointer user_data) {
    gboolean block_ah = prop_get_block_ah(gobject);
    xfce_panel_plugin_block_autohide((XfcePanelPlugin*)user_data, block_ah);
}

static gboolean on_size_changed(G_GNUC_UNUSED XfcePanelPlugin *plugin, G_GNUC_UNUSED int size, G_GNUC_UNUSED gpointer user_data) {
    return TRUE;
}

static void on_orientation_changed(G_GNUC_UNUSED XfcePanelPlugin *plugin, G_GNUC_UNUSED GtkOrientation orientation, DockbarXPlugin *dbx_plugin) {
    reset_plug_orient(dbx_plugin);
}

static void on_screen_position_changed(G_GNUC_UNUSED XfcePanelPlugin *plugin, G_GNUC_UNUSED XfceScreenPosition position, DockbarXPlugin *dbx_plugin) {
    reset_plug_orient(dbx_plugin);
}

static void on_plug_added(G_GNUC_UNUSED GtkSocket *socket, DockbarXPlugin *dbx_plugin) {
    embeded = TRUE;
}

static gboolean on_plug_removed(G_GNUC_UNUSED GtkSocket *socket, DockbarXPlugin *dbx_plugin) {
    embeded = FALSE;
    run_plug(dbx_plugin);
    return TRUE;
}

static void on_free_data(XfcePanelPlugin *plugin, DockbarXPlugin *dbx_plugin) {
    gtk_widget_destroy(dbx_plugin->socket);
    g_slice_free(DockbarXPlugin, dbx_plugin);
    g_mutex_clear(&mutex);
    g_object_unref(properties);
    xfconf_shutdown();
}

static void on_configure_plugin(G_GNUC_UNUSED XfcePanelPlugin *plugin, G_GNUC_UNUSED gpointer user_data) {
    show_pref_dialog();
}

static void on_about(G_GNUC_UNUSED XfcePanelPlugin *plugin, G_GNUC_UNUSED gpointer user_data) {
    show_about_dialog();
}

