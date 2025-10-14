#!/bin/bash
# RunMyModel 0.4.0 - Wrapper script
# This script calls the actual run.sh in the app directory

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
exec "$SCRIPT_DIR/app/run.sh" "$@"

