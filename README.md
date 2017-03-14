# LuaSGX

## Dependencies

On Ubuntu 14:

```bash
$ apt-get install cmake dh-autoreconf git gcc g++ libboost-chrono1.54-dev libboost-system1.54-dev libcrypto++-dev libcurl4-openssl-dev liblua5.3-dev libprotobuf-dev libprotobuf-c0-dev make ocaml-nox protobuf-c-compiler python wget
$ mkdir sgx && cd sgx && git init && git remote add origin https://github.com/01org/linux-sgx.git
$ git fetch origin && git checkout f4005be591a82b1bedfbf8021cec8929a3911bb1 && git reset --hard
$ ./download_prebuilt.sh && make
$ cd linux/installer/bin && ./build-installpkg.sh sdk
$ INSTALL_PATH=/opt/intel ./sgx_linux_x64_sdk_1.6.100.34922.bin
```

## Build

```bash
$ git clone git@github.com:Lorel/luasgx.git
$ cd luasgx/lua-sgx
$ make
$ cd ../app
$ make
```
