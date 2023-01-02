#!/usr/bin/python3
#
#   xfce4-dockbarx-plug
#
#   Copyright (C) 2008-2013 Aleksey Shaferov
#   Copyright (C) 2008-2016 Trent McPheron
#   Copyright (C) 2008-2020 Matias Sars
#   Copyright (C) 2020      Ted Alff
#   Copyright (C) 2020-2023 Xu Zhen
#
#   This file is part of DockbarX Xfce Panel Plugin.
#
#   DockbarX Xfce Panel Plugin is free software: you can redistribute it
#   and/or modify it under the terms of the GNU General Public License as
#   published by the Free Software Foundation, either version 3 of the
#   License, or (at your option) any later version.
#
#   DockbarX Xfce Panel Plugin is distributed in the hope that it will be
#   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with dockbar.  If not, see <http://www.gnu.org/licenses/>.

from dockbarx.log import *; log_to_file()
import sys
sys.stderr = StdErrWrapper()
sys.stdout = StdOutWrapper()
import io
import traceback

import gi
gi.require_version("Gtk", "3.0")
from gi.repository import Gtk
from gi.repository import Gdk
from gi.repository import GdkPixbuf
from gi.repository import GLib
import cairo
import dbus
import signal
import urllib.parse

from optparse import OptionParser
import os


# A very minimal plug application that loads DockbarX
# so that the embed plugin can, well, embed it.
class DockBarXFCEPlug(Gtk.Plug):

    def __init__ (self, app):
        import dockbarx.dockbar as db
        self.app = app
        self.bus = None
        self.xfconf = None
        self.dbx_prop = None
        self.panel_prop = None
        self.mode = None

        parser = OptionParser()
        parser.add_option("-s", "--socket", default = 0, help = "Socket ID")
        parser.add_option("-i", "--plugin_id", default = -1, help = "Plugin ID")
        (options, args) = parser.parse_args()

        # Sanity checks.
        if options.socket == 0:
            sys.exit("This program needs to be run by the XFCE DBX plugin.")
        if options.plugin_id == -1:
            sys.exit("We need to know the plugin id of the DBX socket.")

        Gtk.Plug.__init__(self)
        self.construct(int(options.socket))
        self.connect("destroy", self.destroy)
        self.get_settings().connect("notify::gtk-theme-name",self.theme_changed)
        self.set_app_paintable(True)
        gtk_screen = Gdk.Screen.get_default()
        visual = gtk_screen.get_rgba_visual()
        if visual is None: visual = gtk_screen.get_system_visual()
        self.set_visual(visual)

        # This should cause the widget to get themed like a panel.
        self.set_name("Xfce4PanelDockBarX")
        style_context = self.get_style_context()
        style_context.add_class("xfce4-panel")
        self.show()

        self.bus = dbus.SessionBus()
        self.connect_xfconf_dbus()
        self.dbx_prop = "/plugins/plugin-" + options.plugin_id + "/"
        self.panel_prop = [k for (k, v) in
         self.xfconf.GetAllProperties("xfce4-panel", "/panels").items()
         if "plugin-ids" in k and int(options.plugin_id) in v][0][:-10]

        fdo = self.bus.get_object("org.freedesktop.DBus",
                                  "/org/freedesktop/DBus")
        fdo.connect_to_signal("NameOwnerChanged",
                              self.xfconf_dbus_changed,
                              dbus_interface="org.freedesktop.DBus")

        self.dockbar = db.DockBar(self)
        self.dockbar.set_orient(self.get_orient())
        self.config_bg()
        self.dockbar.set_expose_on_clear(True)
        self.dockbar.load()
        self.add(self.dockbar.get_container())
        self.dockbar.set_max_size(self.get_size())
        self.show_all()
        self.block_autohide_patch()


        self.connect("draw", self.on_draw)

    def connect_xfconf_dbus(self):
        self.xfconf = dbus.Interface(self.bus.get_object(
         "org.xfce.Xfconf", "/org/xfce/Xfconf"), "org.xfce.Xfconf")
        self.bus.add_signal_receiver(self.xfconf_changed, "PropertyChanged",
         "org.xfce.Xfconf", "org.xfce.Xfconf", "/org/xfce/Xfconf")

    def disconnect_xfconf_dbus(self):
        self.bus.remove_signal_receiver(self.xfconf_changed, "PropertyChanged",
         "org.xfce.Xfconf", "org.xfce.Xfconf", "/org/xfce/Xfconf")
        self.xfconf = None

    def xfconf_dbus_changed(self, name, previous_owner, current_owner):
        if str(name) == "org.xfce.Xfconf":
            if previous_owner == "" and current_owner !="":
                self.connect_xfconf_dbus()
            if previous_owner != "" and current_owner == "":
                self.disconnect_xfconf_dbus()

    # Convenience methods.
    def xfconf_get (self, prop_base, prop, default=None):
        if self.xfconf is None:
            self.connect_xfconf_dbus()
        if self.xfconf.PropertyExists("xfce4-panel", prop_base + prop):
            retval = self.xfconf.GetProperty("xfce4-panel", prop_base + prop)
            return retval
        else:
            return default
    def xfconf_get_dbx (self, prop, default=None):
        return self.xfconf_get(self.dbx_prop, prop, default)
    def xfconf_get_panel (self, prop, default=None):
        return self.xfconf_get(self.panel_prop, prop, default)

    def xfconf_changed (self, channel, prop, val):
        if channel != "xfce4-panel": return
        if self.panel_prop in prop and self.mode == 2:
            self.pattern_from_dbus()
        elif self.dbx_prop in prop:
            if "orient" in prop:  self.dockbar.set_orient(self.get_orient())
            elif "mode" in prop:  self.config_bg()
            elif "max-size" in prop:
                self.dockbar.set_max_size(self.get_size())
            elif "expand" in prop:
                self.dockbar.set_max_size(self.get_size(val))
            elif "block-autohide" in prop:
                pass  # This is one way comm from the plug to the socket.
            elif self.mode == 0 and ("color" in prop or "alpha" in prop):
                color = Gdk.RGBA()
                color.parse(self.xfconf_get_dbx("color", "#000"))
                self.color_pattern(color)
            elif self.mode == 1 and ("image" in prop or "offset" in prop):
                self.image_pattern(self.xfconf_get_dbx("image", ""))
            else:
                self.pattern_from_dbus()
        self.queue_draw()

    # The only function that sets anything in xfconf. It's a lazy way to
    # communicate with the vala socket, but it does work!
    def set_block_autohide (self):
        if self.xfconf is None:
            self.connect_xfconf_dbus()
        self.xfconf.SetProperty("xfce4-panel", self.dbx_prop +
         "block-autohide", self.dockbar.globals.get_shown_popup() != None or
          self.dockbar.globals.gtkmenu != None)

    # Terrible monkey patching... but this allows inhibiting autohide!
    def block_autohide_patch (self):
        import dockbarx.common as com
        def new_setattr (obj, name, value):
            super(com.Globals, obj).__setattr__(name, value)
            if name in ("gtkmenu", "shown_popup"):
                self.set_block_autohide()
        com.Globals.__setattr__ = new_setattr

    def theme_changed (self, obj=None, prop=None):
        if self.mode == 2:
            self.pattern_from_dbus()
            self.queue_draw()

    def config_bg (self):
        self.mode = self.xfconf_get_dbx("mode", 2)
        if self.mode == 1:
            self.image_pattern(self.xfconf_get_dbx("image", ""))
        elif self.mode == 0:
            color = Gdk.RGBA()
            color.parse(self.xfconf_get_dbx("color", "#000"))
            self.color_pattern(color)
        else:
            self.pattern_from_dbus()

    def color_pattern (self, color):
        if Gdk.Screen.get_default().get_rgba_visual() is None:
            color.alpha = 1
        self.pattern = cairo.SolidPattern(color.red, color.green, color.blue, color.alpha)

    def image_pattern (self, image, from_dbus=False):
        if (image == ""):
            self.pattern = None
            return
        self.offset = self.xfconf_get_dbx("offset", 0)
        try:
            pixbuf = GdkPixbuf.Pixbuf.new_from_file(image)
            surface = Gdk.cairo_surface_create_from_pixbuf(pixbuf, 0)
            self.pattern = cairo.SurfacePattern(surface)
            self.pattern.set_extend(cairo.EXTEND_REPEAT)
            tx = self.offset if self.orient in ("up", "down") else 0
            ty = self.offset if self.orient in ("left", "right") else 0
            matrix = cairo.Matrix(x0=tx, xy=ty)
            self.pattern.set_matrix(matrix)
        except:
            traceback.print_exc()
            print("Failed to load image.")
            if from_dbus:
                self.pattern = None
            else:
                self.pattern_from_dbus()
            return

    def pattern_from_dbus (self):
        bgstyle = self.xfconf_get_panel("background-style", 0)
        if bgstyle == 2:
            image = self.xfconf_get_panel("background-image", "")
            if image.startswith("file://"):
                image = urllib.parse.unquote(urllib.parse.urlparse(image).path)
            if os.path.isfile(image):
                self.image_pattern(image, from_dbus=True)
            else:
                self.pattern = None
        elif bgstyle == 1:
            col = self.xfconf_get_panel("background-rgba", None)
            if col is None:
                # xfce4-panel < 4.14
                col = self.xfconf_get_panel("background-color", [0, 0, 0, 0])
                col = [v / 65535.0 for v in col]
                col[3] = self.xfconf_get_panel("background-alpha", 100) / 100.0
            self.color_pattern(Gdk.RGBA(col[0], col[1], col[2], col[3]))
        else:
            self.pattern = None

    def get_orient (self):
        self.orient = self.xfconf_get_dbx("orient", "down")

        # Let's make sure our parameters are actually valid.
        if not (self.orient == "bottom" or self.orient == "top" or
         self.orient == "down" or self.orient == "up" or
         self.orient == "left" or self.orient == "right"):
            self.orient = "down"

        # Change it to DBX-specific terminology.
        if self.orient == "bottom": self.orient = "down"
        if self.orient == "top": self.orient = "up"

        return self.orient

    def get_size (self, expand = None):
        if expand is None:
            expand = self.xfconf_get_dbx("expand", False)
        if expand:
            return 32767
        max_size = self.xfconf_get_dbx("max-size", 0)
        if max_size < 1: max_size = 32767
        return max_size

    # Dockbar calls back with this function when it is reloaded
    # since the old container has been destroyed in the reload
    # and needs to be added again.
    def readd_container (self, container):
        if self.get_child() != container:
            self.add(container)
        self.dockbar.set_max_size(self.get_size())
        container.show()

    # Imitates xfce4-panel's expose event.
    def on_draw (self, widget, ctx):
        a = widget.get_allocation()
        if self.pattern is None:
            context = widget.get_style_context()
            Gtk.render_background(context, ctx, a.x, a.y, a.width, a.height)
            return
        ctx.save()
        ctx.set_antialias(cairo.ANTIALIAS_NONE)
        ctx.set_operator(cairo.OPERATOR_SOURCE)
        ctx.rectangle(a.x, a.y, a.width, a.height)
        ctx.clip()
        ctx.set_source(self.pattern)
        ctx.paint()
        ctx.restore()

    def destroy (self, widget, data=None):
        if hasattr(self.dockbar, "destroy"):
            self.dockbar.destroy()
        self.app.quit()

    # signal handlers
    def on_sigint (self, *args):
        self.destroy(self)
        return 0 # G_SOURCE_REMOVE
    def on_sigusr1 (self, *args):
        # orientation changed
        self.dockbar.set_orient(self.get_orient())
        self.readd_container(self.dockbar.get_container())
        return 1 # G_SOURCE_CONTINUE

if __name__ == '__main__':
    app = Gtk.Application(application_id="org.dockbarx.xfce4panel.plugin")
    window = DockBarXFCEPlug(app)
    app.connect("activate", lambda e: app.add_window(window))
    GLib.unix_signal_add(GLib.PRIORITY_DEFAULT, signal.SIGINT, window.on_sigint)
    GLib.unix_signal_add(GLib.PRIORITY_DEFAULT, signal.SIGUSR1, window.on_sigusr1)
    app.run()
