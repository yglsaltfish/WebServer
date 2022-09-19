#!/bin/bash
#初始变量设定
set -x
set -e
Source_Dir=`pwd`
Build_Dir=${Build_Dir:-build}

mkdir -p $Build_Dir \
    && cd $Build_Dir \
    && cmake $Source_Dir \
    && make $*