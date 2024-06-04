#!/usr/bin/env bash

# This script builds the vcpkg dependency management system
set -e

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# shellcheck source=/dev/null
. "${DIR}/../Meta/shell_include.sh"

exit_if_running_as_root "Do not run BuildVcpkg.sh as root, parts of your Toolchain directory will become root-owned"

GIT_REPO="https://github.com/microsoft/vcpkg.git"
GIT_REV="01f602195983451bc83e72f4214af2cbc495aa94" # 2024.05.24
PREFIX_DIR="$DIR/Local/vcpkg"

mkdir -p "$DIR/Tarballs"
pushd "$DIR/Tarballs"
    [ ! -d vcpkg ] && git clone $GIT_REPO

    cd vcpkg
    git fetch origin
    git checkout $GIT_REV

    ./bootstrap-vcpkg.sh -disableMetrics

    mkdir -p "$PREFIX_DIR/bin"
    cp vcpkg "$PREFIX_DIR/bin"
popd
