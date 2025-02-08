#!/bin/bash

REPORTS_DIR=".reports-codechecker"

docker run --rm -v $(pwd):$(pwd) -w $(pwd) --user "$(id -u):$(id -g)" --cpus 2 codechecker analyze --config codechecker_config.yml -o $REPORTS_DIR/build/ build/compile_commands.json

docker run --rm -v $(pwd):$(pwd) -w $(pwd) --user "$(id -u):$(id -g)" codechecker parse --export html  -o $REPORTS_DIR/html $REPORTS_DIR/build ; echo "0"
docker run --rm -v $(pwd):$(pwd) -w $(pwd) --user "$(id -u):$(id -g)" codechecker parse $REPORTS_DIR/build
