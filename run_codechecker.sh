#!/bin/bash

REPORTS_DIR=".reports-codechecker"

# This script use custom docker image with codechecker installed
# You can build it with Dockerfile from this repository
# https://github.com/madamskip1/CodeChecker-docker

docker run --rm -v $(pwd):$(pwd) -w $(pwd) --user "$(id -u):$(id -g)" --cpus 2 codechecker analyze --config codechecker_config.yml -o $REPORTS_DIR/build/ build/compile_commands.json

docker run --rm -v $(pwd):$(pwd) -w $(pwd) --user "$(id -u):$(id -g)" codechecker parse --export html  -o $REPORTS_DIR/html $REPORTS_DIR/build ; echo "0"
docker run --rm -v $(pwd):$(pwd) -w $(pwd) --user "$(id -u):$(id -g)" codechecker parse $REPORTS_DIR/build
