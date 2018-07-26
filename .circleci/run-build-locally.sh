#!/usr/bin/env bash

# This script is used to run CircleCI build on local machine without pushing through remote repo.

curl --user ${CIRCLE_TOKEN}: \
    --request POST \
    --form revision=$1 \
    --form config=@config.yml \
    --form notify=false \
        https://circleci.com/api/v1.1/project/github/MMquant/bfx-cpp-api/tree/develop
