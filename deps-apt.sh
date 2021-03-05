#!/bin/sh
echo "* Installing dockbarx deps (this is only for docx standalone command; xfce4-dockbarx-plugin deps come next)..."
apt install -y git build-essential

apt install -y gir1.2-keybinder-3.0 gir1.2-pango-1.0 gir1.2-wnck-3.0 python3-cairo python3-dbus python3-distutils python3-gi python3-gi-cairo python3-pil python3-polib python3-xdg python3-xlib
apt install -y gir1.2-zeitgeist-2.0 and zeitgeist
# ^ "to access latest and most used documents"
# apt install indicator-application
# ^ no installation candidate, so:
apt install -y ayatana-indicator-application
# "to use the appindicator applet with DockX"
apt install -y python3-pyudev
# ^ "(>= 0.15), to use the battery status applet with DockX"
apt install -y python3-lxml
# ^ to use the settings migrating tool


echo "* Installing xfce4-dockbarx-plugin deps (switched to version 0.48 of vala since that is the version on Buster)..."
apt install -y valac libvala-0.48-dev
apt install -y libgtk-3-dev
apt install -y libxfce4panel-2.0
apt install -y libxfconf-0-dev
