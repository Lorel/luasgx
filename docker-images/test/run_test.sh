#!/usr/bin/env bash

IMAGE=lorel/zmqrxlua:sgx

docker run --rm --privileged --device /dev/isgx -it -v $(pwd):/root/worker $IMAGE test.lua
