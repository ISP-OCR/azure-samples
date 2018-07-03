#!/bin/sh

export LD_LIBRARY_PATH=/usr/local/lib64

IMAGE_FILE_PATH=$1
if [ "$IMAGE_FILE_PATH" = "" ]; then
 IMAGE_FILE_PATH="./image.jpg"
fi

LICENSE_FILE_PATH=$2
if [ "$LICENSE_FILE_PATH" = "" ]; then
 LICENSE_FILE_PATH="./key.txt"
fi

echo "${IMAGE_FILE_PATH} ${LICENSE_FILE_PATH}"
./MyNumberSample ${IMAGE_FILE_PATH} ${LICENSE_FILE_PATH} 3

