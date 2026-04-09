#!/bin/bash
# Continuously run the crawler bot with metric collection.
# Automatically finds its own location and jmoria.

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

RUNNER="$SCRIPT_DIR/crawl_metric.sh"
while true; do "$RUNNER" ; sleep 2; done

