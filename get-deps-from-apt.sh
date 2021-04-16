#!/bin/bash
install_bin="apt"
install_params="install -y"

usage(){
cat <<END

USAGE:
get-deps-from-apt.sh --command apt-get --params "install -y"

command: (default: "apt") the install command
params: (default: "install -y") params for the install command

END
}
prev_name=
this_name=
# ^ set the values to nothing
custom_warning=
for arg in "$@"
do
    # The following two [[]] cases only work in bash:
    # if [[ $string = *" "* ]]; then
    if [[ $string =~ " " ]]; then
        first2=
        # avoid bad substitution
    else
        first2=${arg:0:2}
    fi
    echo "* param opener: \"$first2\""
    if [ "$first2" = "--" ]; then
        this_name=${arg:2}
        if [ ! -z "$prev_name" ]; then
            usage
            echo "Error: There was an unexpected option after $prev_name (expected value)."
            exit 1
        fi
    else
        this_name=
    fi
    if [ "@$prev_name" = "@command" ]; then
        if [ "@$arg" = "@apt" ]; then
            install_bin="apt"
        elif [ "@$arg" = "@apt-get" ]; then
            install_bin="apt-get"
        else
            install_bin="$arg"
            custom_warning="WARNING: The $arg command is for a package system not implemented in this script, so the package names may not be correct."
            echo "$custom_warning"
        fi
    elif [ "@$prev_name" = "@params" ]; then
        install_params="$arg"
    elif [ ! -z "$prev_name" ]; then
        usage
        echo "Error: the \"$prev_name\" option is unknown."
        exit 1
    fi
    prev_name="$this_name"
done
echo "* Installing dockbarx deps (this is only for docx standalone install_cmd; xfce4-dockbarx-plugin deps come next)..."
$install_bin $install_params git build-essential
if [ $? -ne 0 ]; then
    usage
    echo "Error: '$install_bin $install_params git build-essential' failed."
    if [ ! -z "$custom_warning" ]; then
        echo "$custom_warning"
    fi
    exit 1
else
    echo "* '$install_bin $install_params git build-essential'...OK"
fi
$install_bin $install_params gir1.2-keybinder-3.0 gir1.2-pango-1.0 gir1.2-wnck-3.0 python3-cairo python3-dbus python3-distutils python3-gi python3-gi-cairo python3-pil python3-polib python3-xdg python3-xlib
$install_bin $install_params gir1.2-zeitgeist-2.0 and zeitgeist
# ^ "to access latest and most used documents"
# $install_bin $install_params install indicator-application
# ^ no installation candidate, so:
$install_bin $install_params ayatana-indicator-application
# "to use the appindicator applet with DockX"
$install_bin $install_params python3-pyudev
# ^ "(>= 0.15), to use the battery status applet with DockX"
$install_bin $install_params python3-lxml
# ^ to use the settings migrating tool


echo "* Installing xfce4-dockbarx-plugin deps (switched to version 0.48 of vala since that is the version on Buster)..."
$install_bin $install_params valac libvala-0.48-dev
$install_bin $install_params libgtk-3-dev
$install_bin $install_params libxfce4panel-2.0
$install_bin $install_params libxfconf-0-dev

echo "Done"
echo
echo "You should now be able to use dockbarx (https://github.com/M7S/dockbarx.git) and this fork of xfce4-dockbarx-plugin from source."
echo "See INSTALL and README.md"
echo
