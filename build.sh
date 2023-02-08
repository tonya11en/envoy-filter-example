#! /bin/bash

set -x 

TOP_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
LOCAL_BUILD_CACHE="${TOP_DIR}/.cache"
BIN_DIR="${TOP_DIR}/bin"
BUILD_TYPE="dbg"

function do_build {
  echo "Starting envoy binary build and backing up previous binary..."
  mv ${BIN_DIR}/envoy ${BIN_DIR}/envoy.bak
  pushd ${TOP_DIR}
  set -x
  bazel build -c ${BUILD_TYPE} --disk_cache=${LOCAL_BUILD_CACHE} //:envoy
  popd
  cp ${TOP_DIR}/bazel-bin/envoy ${BIN_DIR}
}

function do_test {
  pushd ${TOP_DIR}
  set -x
  bazel test //...
  popd
}

function do_clean {
  bazel clean
}

mkdir -p ${LOCAL_BUILD_CACHE}
mkdir -p ${BIN_DIR}
while getopts release:test:dev:clean: flag
do
    case "${flag}" in
        release) BUILD_TYPE="opt"; exit $(do_build);;
        test) exit $(do_test);;
        clean) exit $(do_clean);;
    esac
done

# Catch-all will just run a dev build.
do_build
