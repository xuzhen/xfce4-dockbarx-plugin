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
from gi.repository import Gio
import cairo
import dbus
import signal
import urllib.parse

import os

DBUS_NAME="org.dockbar.plugins.xfce4panel"

# A very minimal plug application that loads DockbarX
# so that the embed plugin can, well, embed it.
class DockBarXFCEPlug(Gtk.Plug):

    def __init__ (self, app, socket, plugin_id):
        import dockbarx.dockbar as db
        self.app = app
        self.bus = None
        self.xfconf = None
        self.dbx_prop = None
        self.panel_prop = None
        self.mode = None

        Gtk.Plug.__init__(self)
        self.construct(socket)
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
        self.dbx_prop = "/plugins/plugin-%d/" % plugin_id
        self.panel_prop = [k for (k, v) in
         self.xfconf.GetAllProperties("xfce4-panel", "/panels").items()
         if "plugin-ids" in k and plugin_id in v][0][:-10]

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
            elif "color" in prop:
                if self.mode == 0:
                    color = Gdk.RGBA()
                    color.parse(self.xfconf_get_dbx("color", "#000"))
                    self.color_pattern(color)
            elif "image" in prop or "offset" in prop:
                if self.mode == 1:
                    self.image_pattern(self.xfconf_get_dbx("image", ""))
            else:
                self.pattern_from_dbus()
        self.queue_draw()

    def set_block_autohide (self):
        blocked = self.dockbar.globals.get_shown_popup() != None or \
                  self.dockbar.globals.gtkmenu != None
        self.app.notify_autohide(blocked)

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

class XfcePlugApp(Gtk.Application):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, application_id=DBUS_NAME, flags=Gio.ApplicationFlags.HANDLES_COMMAND_LINE, **kwargs)
        self.window = None
        self.socket = None
        self.plugin_id = None
        self.add_main_option("socket", ord("s"), GLib.OptionFlags.IN_MAIN, GLib.OptionArg.INT, "Socket ID", None)
        self.add_main_option("plugin_id", ord("i"), GLib.OptionFlags.IN_MAIN, GLib.OptionArg.INT, "Plugin ID", None)

    def do_startup(self):
        Gtk.Application.do_startup(self)
        self.register_dbus()
        
    def do_activate(self):
        if not self.window:
            self.window = DockBarXFCEPlug(self, self.socket, self.plugin_id)
            self.add_window(self.window)
            GLib.unix_signal_add(GLib.PRIORITY_DEFAULT, signal.SIGINT, self.window.on_sigint)
        self.window.present()

    def do_command_line(self, command_line):
        options = command_line.get_options_dict()
        options = options.end().unpack()
        if "socket" in options:
            self.socket = int(options["socket"])
        else:
            logger.error("This program needs to be run by the XFCE DBX plugin.")
            return 1
        if "plugin_id" in options:
            self.plugin_id = int(options["plugin_id"])
        else:
            logger.error("We need to know the plugin id of the DBX socket.")
            return 1
        self.activate()
        return 0

    def notify_autohide(self, blocked):
        dbus = self.get_dbus_connection()
        dbus_path = self.get_dbus_object_path()
        args = GLib.Variant.new_tuple(GLib.Variant.new_boolean(blocked))
        dbus.emit_signal(None, dbus_path, DBUS_NAME, "AutoHide", args)

    def register_dbus(self):
        dbus_xml = \
            "<node>" \
              "<interface name='%s'>" % DBUS_NAME + \
                "<method name='SetOrient'>" \
                  "<arg type='s' name='pos' direction='in'/>" \
                "</method>" \
                "<signal name='AutoHide'>" \
                  "<arg type='b' name='block'/>" \
                "</signal>" \
              "</interface>" \
            "</node>"
        info = Gio.DBusNodeInfo.new_for_xml(dbus_xml)
        iface = info.lookup_interface(DBUS_NAME)
        dbus = self.get_dbus_connection()
        dbus_path = self.get_dbus_object_path()
        dbus.register_object(dbus_path, iface, self.dbus_method_call, None, None);

    def dbus_method_call(self, connection, sender, object_path, interface_name, method_name, parameters, invocation):
        ret = None
        if self.window is None:
            err = Gio.DBusError.FAILED
            err_message = "Not ready yet"
        if method_name == "SetOrient":
            self.window.dockbar.set_orient(self.window.get_orient())
            self.window.readd_container(self.window.dockbar.get_container())
            ret = GLib.Variant.new_tuple()
        else:
            err = Gio.DBusError.UNKNOWN_METHOD
            err_message = "No such method: %s" % method_name

        if ret is not None:
            invocation.return_value(ret)
        else:
            invocation.return_error_literal(err.quark(), err, err_message)


if __name__ == '__main__':
    XfcePlugApp().run(sys.argv)
