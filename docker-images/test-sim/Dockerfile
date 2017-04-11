FROM ubuntu:14.04.1

ENV BUILD_PACKAGES cmake dh-autoreconf git gcc g++ libboost-chrono1.54-dev libboost-system1.54-dev libcrypto++-dev libprotobuf-dev libprotobuf-c0-dev make ocaml-nox protobuf-compiler protobuf-c-compiler python wget
ENV UTIL_PACKAGES strace
ENV DEPENDENCIES_PACKAGES libboost-chrono1.54.0 libboost-system1.54.0 libcrypto++9 libcurl4-openssl-dev liblua5.3-dev libprotobuf8 libprotobuf-c0

ENV LINUX_SGX_GIT https://github.com/01org/linux-sgx.git
ENV LINUX_SGX_COMMIT f4005be591a82b1bedfbf8021cec8929a3911bb1

ENV LUA53_REPO deb http://ppa.launchpad.net/ubuntu-cloud-archive/mitaka-staging/ubuntu trusty main

ENV LUASGX_REPO git@github.com:Lorel/luasgx.git

ENV SGX_BUILD_DIR /root/sgx-build
ENV SGX_SDK_PARENT_DIR /opt/intel
ENV SGX_SDK_DIR $SGX_SDK_PARENT_DIR/sgxsdk
ENV LUA_BUILD_DIR /root/lua-build
ENV LUASGX_BUILD_DIR /root/luasgx
ENV RUNTIME_DIR /root/app

ENV LUASGX_REPO_KEY /root/.ssh/id_rsa

ADD build_files/.ssh/id_luasgx $LUASGX_REPO_KEY
ADD build_files/docker_patches $SGX_BUILD_DIR

# TODO: --allow-unauthenticated added because libxml2 liblua5.3-0 liblua5.3-dev cannot be authenticated
#       should be fetched from http://ppa.launchpad.net/ubuntu-cloud-archive/mitaka-staging/ubuntu
#       find a way to authenticate them

RUN echo $LUA53_REPO >> /etc/apt/sources.list && \
  apt-get update && \
  apt-get install -y --allow-unauthenticated $BUILD_PACKAGES $UTIL_PACKAGES $DEPENDENCIES_PACKAGES
  #  && \

ENV LINUX_SGX_COMMIT 525cefa817457d8a412d64677d347ebe64f0f77a
RUN mkdir -p $SGX_BUILD_DIR && \
  cd $SGX_BUILD_DIR && \
  git init && \
  git remote add origin $LINUX_SGX_GIT && \
  git fetch origin  && \
  git checkout $LINUX_SGX_COMMIT && \
  git reset --hard && \
  patch linux/installer/bin/install-sgx-psw.bin.tmpl < 0001-Remove-modprobe-lsmod-for-use-in-docker-container.patch && \
  patch linux/installer/common/psw/install.sh < 0002-Remove-unnecessary-sudo-from-PSW-install-script.patch && \
  patch psw/ae/aesm_service/source/aesm/application/main.cpp < 0003-Remove-daemonization-of-AESM.patch && \
  patch linux/installer/bin/install-sgx-sdk.bin.tmpl < 0004-use_INSTALL_PATH_var_for-ssdk_install.patch && \
  ./download_prebuilt.sh && \
  make && \
  cd linux/installer/bin && \
  ./build-installpkg.sh psw && \
  ./sgx_linux_x64_psw_1.8.100.37689.bin && \
  ./build-installpkg.sh sdk && \
  INSTALL_PATH=$SGX_SDK_PARENT_DIR ./sgx_linux_x64_sdk_1.8.100.37689.bin && \
  rm -rfv $SGX_BUILD_DIR
  #  && \

ENV SGX_MODE SIMULATION_MODE

# RUN chmod 400 $LUASGX_REPO_KEY && \
#   ssh-keyscan -H github.com >> /root/.ssh/known_hosts && \
#   cd /root && \
#   mkdir -p $LUASGX_BUILD_DIR && \
#   git clone $LUASGX_REPO $LUASGX_BUILD_DIR && \
#   cd $LUASGX_BUILD_DIR/lua-sgx && \
#   make && \
#   cd $LUASGX_BUILD_DIR/app && \
#   make && \
#   mkdir -p $RUNTIME_DIR && \
#   mv $LUASGX_BUILD_DIR/app/luasgx $RUNTIME_DIR/luasgx && \
#   mv $LUASGX_BUILD_DIR/app/EnclavedLua.signed.so $RUNTIME_DIR/EnclavedLua.signed.so
  #  && \
  # rm -rfv $LUASGX_REPO_KEY $LUASGX_BUILD_DIR && \
  # apt-get -y remove --auto-remove $BUILD_PACKAGES && \
  # apt-get clean && \
  # rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

# build from local files
ADD build_files/luasgx $LUASGX_BUILD_DIR
RUN cd $LUASGX_BUILD_DIR/lua-sgx && \
  make && \
  cd $LUASGX_BUILD_DIR/app && \
  make && \
  mkdir -p $RUNTIME_DIR && \
  mv $LUASGX_BUILD_DIR/app/luasgx $RUNTIME_DIR/luasgx && \
  mv $LUASGX_BUILD_DIR/app/EnclavedLua.signed.so $RUNTIME_DIR/EnclavedLua.signed.so

# link SGX simulation shared libraries
RUN ln -s /opt/intel/sgxsdk/lib64/libsgx_urts_sim.so /lib/x86_64-linux-gnu/libsgx_urts_sim.so
RUN ln -s /opt/intel/sgxsdk/lib64/libsgx_uae_service_sim.so /lib/x86_64-linux-gnu/libsgx_uae_service_sim.so

ADD build_files/start.sh $RUNTIME_DIR

WORKDIR $RUNTIME_DIR

VOLUME ["/root/worker"]

ENTRYPOINT ["./start.sh"]
