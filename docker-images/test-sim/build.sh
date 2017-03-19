#!/usr/bin/env bash

IMAGE=luasgx:test-sim

mkdir -p build_files/luasgx
cp -rfv ../../lua-5.3.2 build_files/luasgx/lua-5.3.2
cp -rfv ../../lua-sgx build_files/luasgx/lua-sgx
cp -rfv ../../lua-cjson build_files/luasgx/lua-cjson
cp -rfv ../../luacsv build_files/luasgx/luacsv
cp -rfv ../../sgx_common build_files/luasgx/sgx_common
cp -rfv ../../app build_files/luasgx/app

docker build -t $IMAGE .
