#! /bin/bash

set -x 

TOP_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
LOCAL_BUILD_CACHE="${TOP_DIR}/.cache"
BIN_DIR="${TOP_DIR}/bin"

function do_build {
  echo "Starting envoy binary build and backing up previous binary..."
  mv ${BIN_DIR}/envoy ${BIN_DIR}/envoy.bak
  pushd ${TOP_DIR}
  bazel build -c ${1} --disk_cache=${LOCAL_BUILD_CACHE} //:envoy
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
  bazel clean --disk_cache=${LOCAL_BUILD_CACHE}
}

git config --global --add safe.directory /src

mkdir -p ${LOCAL_BUILD_CACHE}
mkdir -p ${BIN_DIR}
case "${1}" in
  release) do_build opt;;
  debug) do_build dbg;;
  test) do_test;;
  clean) do_clean;;
  *) do_build fastbuild;;
esac
