#!/bin/sh
if [ ! -f "get-deps-from-apt.sh" ]; then
    echo "Error: ./get-deps-from-apt.sh is missing. You must run this from the directory that contains it."
    exit 1
fi
if [ -f "`command -v apt`" ]; then
    ./get-deps-from-apt.sh
elif [ -f "`command -v apt-get`" ]; then
    ./get-deps-from-apt.sh --command apt-get
else
    echo "Error: Only apt and apt-get are implemented but neither are present. See README.md for what dependencies to install manually."
    exit 1
fi
