#!/bin/bash


if [ -z "${BUILD_PATH}" ]
then 
BUILD_PATH=/tmp/
fi

cd ${BUILD_PATH}/libsbml-5.20.2/build
make install