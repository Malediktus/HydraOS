#!/bin/bash

set -e

DIR="`dirname "${BASH_SOURCE[0]}"`"
cd "$DIR" || exit

find .. -type d -name 'build' -exec rm -r {} +
