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
#ifndef _XFCE_PANEL_PLUGIN_PROPS_H__
#define _XFCE_PANEL_PLUGIN_PROPS_H__

#include <glib-object.h>

#define PLUGIN_PROPERTIES_TYPE (plugin_properties_get_type())
#define PLUGIN_PROPERTIES(o) (G_TYPE_CHECK_INSTANCE_CAST((o), PLUGIN_PROPERTIES_TYPE, PluginProperties))

typedef struct {
    GObject parent;
} PluginProperties;

typedef struct {
    GObjectClass parent;
} PluginPropertiesClass;

GType plugin_properties_get_type() G_GNUC_CONST;

#endif
