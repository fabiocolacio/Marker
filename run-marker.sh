#!/bin/bash
# Wrapper script to run Marker with local schemas for development

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

# Set the schema directory to the build directory
export GSETTINGS_SCHEMA_DIR="$BUILD_DIR"

# Set development mode flag
export MARKER_DEV_MODE=1

# Convert relative paths to absolute paths before changing directory
ARGS=()
for arg in "$@"; do
  if [[ -e "$arg" ]]; then
    # If the argument is an existing file or directory, convert to absolute path
    ARGS+=("$(realpath "$arg")")
  else
    # Otherwise, keep the argument as-is
    ARGS+=("$arg")
  fi
done

# Change to build directory so relative paths work
cd "$BUILD_DIR"

# Run marker from the build directory with converted paths
exec "./marker" "${ARGS[@]}"