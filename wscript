#!/usr/bin/env python3
#
# Copyright (c) 2011- Trent McPheron <twilightinzero@gmail.com>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

# For creating a source archive.
APPNAME = 'xfce4-dockbarx-plugin'
VERSION = '0.6'

# Required waf stuff.
top = '.'
out = 'build'

def options (ctx):
    ctx.load('compiler_c')
    ctx.load('vala')

def configure (ctx):
    # Strip extraneous slash from prefix.
    if ctx.options.prefix[-1] == '/' :
        ctx.options.prefix += ctx.options.prefix[-1]

    # Check for required stuff.
    ctx.load('compiler_c')
    ctx.env.VALA_MINVER = (0, 42, 0)
    ctx.load('vala')
    ctx.env.append_value('CFLAGS', '-DGETTEXT_PACKAGE="gtk30"')
    args = '--cflags --libs'
    ctx.check_cfg(package = 'glib-2.0', atleast_version = '2.42',
        uselib_store = 'GLIB', mandatory = True, args = args)
    ctx.check_cfg(package = 'gtk+-3.0', atleast_version = '3.22',
        uselib_store = 'GTK', mandatory = True, args = args)
    ctx.check_cfg(package = 'libxfce4panel-2.0', atleast_version = '4.12',
        uselib_store = 'XFCE4PANEL', mandatory = True, args = args)
    ctx.check_cfg(package = 'libxfconf-0', atleast_version = '4.12',
        uselib_store = 'XFCONF', mandatory = True, args = args)

def build (ctx):
    # Compile the program.
    ctx.program(
        features     = 'c cshlib',
        is_lib       = True,
        vapi_dirs    = 'vapi',
        source       = ctx.path.ant_glob('src/*.vala'),
        packages     = 'glib-2.0 gtk+-3.0 libxfce4panel-2.0 libxfconf-0',
        target       = 'dockbarx',
        install_path = '${PREFIX}/lib/xfce4/panel/plugins/',
        use       = 'GLIB GTK XFCE4PANEL XFCONF')

    # Install other files.
    ctx(
        features = 'subst',
        source = 'data/dockbarx.desktop.in',
        target = 'data/dockbarx.desktop')
    ctx.install_files(
        '${PREFIX}/share/xfce4/panel/plugins/',
        'data/dockbarx.desktop')
    ctx.install_files(
        '${PREFIX}/share/dockbarx/themes/',
        'data/Mouse.tar.gz')
    ctx.install_files(
        '${PREFIX}/share/dockbarx/themes/',
        'data/Mouse-4.tar.gz')
    ctx.install_files(
        '${PREFIX}/share/dockbarx/themes/',
        'data/Mouse-6.tar.gz')
    ctx.install_files(
        '${PREFIX}/share/dockbarx/themes/',
        'data/MouseNeo.tar.gz')
    ctx.install_files(
        '${PREFIX}/share/dockbarx/themes/',
        'data/MouseNeo-4.tar.gz')
    ctx.install_files(
        '${PREFIX}/share/dockbarx/themes/',
        'data/MouseNeo-6.tar.gz')
    ctx.install_as(
        '/usr/share/xfce4/panel/plugins/xfce4-dockbarx-plug',
        'src/xfce4-dockbarx-plug.py',
        chmod=0o755)

def checkinstall (ctx):
    ctx.exec_command('checkinstall' +
     ' --pkgname=' + APPNAME + ' --pkgversion=' + VERSION +
     ' --provides=' + APPNAME + ' --requires=dockbarx' +
     ' --deldoc=yes --deldesc=yes --delspec=yes --backup=no' +
     ' --exclude=/home -y ./waf install')
