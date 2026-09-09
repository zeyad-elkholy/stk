#!/bin/sh
# Use this script to run your program LOCALLY.
#
# Note: Changing this script WILL NOT affect how CodeCrafters runs your program.
#
# Learn more: https://codecrafters.io/program-interface


# Copied from .codecrafters/compile.sh
#
# - Edit this to change how your program compiles locally
# - Edit .codecrafters/compile.sh to change how your program compiles remotely

# Copied from .codecrafters/run.sh
#
# - Edit this to change how your program runs locally
# - Edit .codecrafters/run.sh to change how your program runs remotely
set -e

(
  cd "$(dirname "$0")"
  cmake -B build -S . > /dev/null
  cmake --build build > /dev/null
)

exec "$(dirname "$0")/build/shell" "$@"
