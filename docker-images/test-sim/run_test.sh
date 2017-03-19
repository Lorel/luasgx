#!/usr/bin/env bash

IMAGE=luasgx:test-sim

docker run --rm --privileged -it -v $(pwd):/root/worker $IMAGE test.lua
