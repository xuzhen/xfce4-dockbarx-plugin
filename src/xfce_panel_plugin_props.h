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
#ifndef _XFCE_PANEL_PLUGIN_PROPS_H_
#define _XFCE_PANEL_PLUGIN_PROPS_H_

#include <glib-object.h>
#include <xfconf/xfconf.h>

#define PLUGIN_PROPERTIES_TYPE (plugin_properties_get_type())
#define PLUGIN_PROPERTIES(o) (G_TYPE_CHECK_INSTANCE_CAST((o), PLUGIN_PROPERTIES_TYPE, PluginProperties))

typedef struct {
    GObject parent;
} PluginProperties;

typedef struct {
    GObjectClass parent;
} PluginPropertiesClass;

GType plugin_properties_get_type() G_GNUC_CONST;

gint prop_get_bgmode(GObject *object);
void prop_set_bgmode(GObject *object, gint bgmode);
gchar *prop_get_color(GObject *object);
void prop_set_color(GObject *object, const gchar *color);
gchar *prop_get_image(GObject *object);
void prop_set_image(GObject *object, const gchar *image);
gint prop_get_offset(GObject *object);
void prop_set_offset(GObject *object, gint offset);
gint prop_get_max_size(GObject *object);
void prop_set_max_size(GObject *object, gint max_size);
gchar *prop_get_orient(GObject *object);
void prop_set_orient(GObject *object, const gchar *orient);
gboolean prop_get_expand(GObject *object);
void prop_set_expand(GObject *object, gboolean expand);
gboolean prop_get_block_ah(GObject *object);
void prop_set_block_ah(GObject *object, gboolean block_ah);
void prop_connect_block_ah(GObject *object, GCallback cb, gpointer data);

void prop_bind_xfconf(XfconfChannel *channel, GObject *object);

#endif
