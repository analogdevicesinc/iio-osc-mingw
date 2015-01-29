#!/bin/bash

CURDIR=`realpath "$0"`
export CURDIR=`dirname "${CURDIR}"`

export MINGW_VERSION=i686
cd ${CURDIR}
. ./build.sh.inc

export MINGW_VERSION=x86_64
cd ${CURDIR}
. ./build.sh.inc
