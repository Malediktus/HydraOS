#!/bin/bash

set -e

if not command -v python3 &>/dev/null; then
    echo "error: python3 is required to run setup script"
    exit 1
fi

DIR="`dirname "${BASH_SOURCE[0]}"`"
cd "$DIR" || exit

python3 setup.py
