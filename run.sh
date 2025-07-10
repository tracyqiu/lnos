#!/bin/bash

xhost +local:docker
sudo docker run --rm -it \
    -e DISPLAY=$DISPLAY \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    -p 1234:1234 \
    -v $(pwd):/workspace \
    lnos-dev /bin/bash -c "cd /workspace && make clean && make qemu"


