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
#ifndef _XFCE4_DOCKBARX_PLUGIN_H_
#define _XFCE4_DOCKBARX_PLUGIN_H_

#pragma once

#include <libxfce4panel/libxfce4panel.h>
#include <xfconf/xfconf.h>
#include <glib-object.h>

typedef struct {
    XfcePanelPlugin *plugin;
    XfconfChannel *xfc;
    GtkWidget *socket;
    ulong socket_id;
    GObject *props;
} DockbarXPlugin;

#endif
