#!/bin/bash

set -e

SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
BUILDDIR="$SCRIPTDIR/cmake-build-fuzz"

"$BUILDDIR"/fuzz_message "$@" > /dev/null
