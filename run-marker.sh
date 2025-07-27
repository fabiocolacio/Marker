#!/bin/bash
# Wrapper script to run Marker with local schemas for development

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

# Set the schema directory to the build directory
export GSETTINGS_SCHEMA_DIR="$BUILD_DIR"

# Set development mode flag
export MARKER_DEV_MODE=1

# Change to build directory so relative paths work
cd "$BUILD_DIR"

# Run marker from the build directory
exec "./marker" "$@"