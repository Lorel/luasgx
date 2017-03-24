#!/usr/bin/env bash

IMAGE=luasgx:test-sim

rm -rf build_files/luasgx
mkdir -p build_files/luasgx
cp -rf ../../lua-5.3.2 build_files/luasgx/lua-5.3.2
cp -rf ../../lua-sgx build_files/luasgx/lua-sgx
cp -rf ../../lua-cjson build_files/luasgx/lua-cjson
cp -rf ../../luacsv build_files/luasgx/luacsv
cp -rf ../../sgx_common build_files/luasgx/sgx_common
cp -rf ../../app build_files/luasgx/app

docker build -t $IMAGE .
