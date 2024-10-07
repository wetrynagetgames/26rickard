#!/usr/bin/env bash

set -eo pipefail

script_path=$(CDPATH='' cd -P -- "$(dirname -- "$0")" && pwd -P)
CDPATH='' cd "$script_path/.."

if [ "$(uname -s)" = "Darwin" ]; then
    # MacOS's find does not support '-executable' OR '-perm /mode'.
    BAD_FILES=$(find Base/res/ -type f -perm +111)
else
    BAD_FILES=$(find Base/res/ -type f -executable)
fi

if [ -n "${BAD_FILES}" ]
then
    echo "These files are marked as executable, but are in directories that do not commonly"
    echo "contain executables. Please double-check the permissions of these files:"
    echo "${BAD_FILES}" | xargs ls -ld
    exit 1
fi
