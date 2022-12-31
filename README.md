# xfce4-dockbarx-plugin
### ver. 0.7

## About xfce4-dockbarx-plugin
xfce4-dockbarx-plugin is free software. Please see the file COPYING for details. For building and installation instructions please see the INSTALL file. For information on the authors of this program, see AUTHORS and THANKS.

xfce4-dockbarx-plugin is a pair of programs--a gtk socket in C and a gtk plug in Python--that work together to embed DockbarX into xfce4-panel. See THANKS for details on DockbarX. Because this is not a port and just grabs the pre-existing DockbarX, you immediately benefit from any updates made to DBX, and already have all the pre-existing functionality.

## Using xfce4-dockbarx-plugin
Add DockbarX to the panel and it will automatically start. If you don't want to customize it, no further configuration is necessary.

If you wish to configure it, you must invoke the configuration dialog through the panel preferences window. You can then configure a distinct background for the plugin, change the maximum size, or set it to expand. The plugin will automatically detect panel orientation and will reconfigure itself on the fly.

There is only one gotcha for automatic panel blending; namely, if you use the image style, you still need to make sure the offset is properly configured if you have an image that necessitates it.

## Any extras?
This plugin includes a DockbarX theme called Mouse, created by me for use with xfce4-panel (but should work fine on Gnome/Mate panels, AWN, or DockX). There are two variant versions for varying levels of x/ythickness on the panel widgets to make them match up nicely. If a theme gives different panel widgets differing x/ythickness, its author is a sick bastard.

## Okay, I'm sold! Gimme the goods!
Some distros already have it packaged in some form:
* Arch Linux / Manjaro users can install from the [AUR](https://aur.archlinux.org/packages/xfce4-dockbarx-plugin/).
* Ubuntu users can install from the [Dockbar PPA](https://launchpad.net/~xuzhen666/+archive/ubuntu/dockbarx).
* The stable source release can be found on [Xfce-Look](http://xfce-look.org/content/show.php?content=157865).

If you want to (or have to) install from source, you need the following dependencies:

* CMake >= 3.1
* GLib >= 2.38
* GTK+3 >= 3.12
* Xfce4-Panel >= 4.12
* Xfconf >= 4.12
* DockbarX >= 1.0-beta2

For Debian/Ubuntu users, the building dependencies could be installed by using:

    sudo apt install gcc make cmake libglib2.0-dev libgtk-3-dev libxfce4panel-2.0-dev libxfconf-0-dev

To configure, build, and install, run these commands:

    mkdir build
    cd build
    cmake ..
    make
    sudo make install

The panel will probably not detect the plugin unless you install it in the /usr prefix, so instead do the configure step with `cmake -DCMAKE_INSTALL_PREFIX=/usr ..`

## Awesome! Who do I need to thank for all this?
* Aleksey Shaferov is the original Dockbar developer.
* Matias Särs is the developer of the DockbarX fork.
* The developers of the Python languages are to be thanked, of course.
* The build system is cmake, so all the guys working on that are to thank for keeping this out of autohell.
* Trent McPheron is the original developer of this beautifully hacky xfce4 panel plugin that really should not work as well as it does.
* And the github community to whom I entrust the future of this plugin.

## I want to make the plugin better!
Awesome! Fork the repo and send me pull requests! I will probably merge any request I get. The future of this plugin is in your hands now. :)
