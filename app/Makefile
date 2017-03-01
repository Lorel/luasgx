SGX_SDK=/opt/intel/sgxsdk
SGX_MODE ?= HW
#SGX_MODE ?= SIMULATION_MODE
SGX_PRERELEASE ?=1

CC = gcc
CXX = g++
ENCLVNAME = EnclavedLua
LUADIR = ../lua-sgx
SGXCOMMONDIR = ../../sgx_common
UNTRUSTED_DIR = untrusted
TRUSTED_DIR = trusted
UNTRLUADIR = ../lua-5.3.2
UNTRCJSON = ../lua-cjson

SGX_COMMON_CFLAGS := -I$(SGXCOMMONDIR) -m64
SGX_LIBRARY_PATH := $(SGX_SDK)/lib64
SGX_ENCLAVE_SIGNER := $(SGX_SDK)/bin/x64/sgx_sign
SGX_EDGER8R := $(SGX_SDK)/bin/x64/sgx_edger8r

App_Trusted_IncPaths := $(addprefix -I, $(UNTRUSTED_DIR) $(SGX_SDK)/include \
                          $(SGXCOMMONDIR))
App_C_Flags := $(SGX_COMMON_CFLAGS) -fPIC -Wno-attributes $(App_Trusted_IncPaths)

ifeq ($(SGX_DEBUG), 1)
ifeq ($(SGX_PRERELEASE), 1)
$(error Cannot set SGX_DEBUG and SGX_PRERELEASE at the same time!!)
endif
endif

ifeq ($(SGX_DEBUG), 1)
    SGX_COMMON_CFLAGS += -O0 -g
    App_C_Flags += -DDEBUG -UNDEBUG -UEDEBUG
else
    SGX_COMMON_CFLAGS += -O2
    ifeq ($(SGX_PRERELEASE), 1)
        App_C_Flags += -DNDEBUG -DEDEBUG -UDEBUG
    else
        App_C_Flags += -DNDEBUG -UEDEBUG -UDEBUG
    endif
endif

App_Cpp_Flags := $(App_C_Flags) -std=c++11 -I$(UNTRLUADIR)/src
Trusted_IncPaths := $(addprefix -I, $(LUADIR)/src $(SGX_SDK)/include/tlibc \
                                 $(SGX_SDK)/include $(TRUSTED_DIR)\
                                 /usr/include/x86_64-linux-gnu/)

Flags_Just_For_C := -Wno-implicit-function-declaration -std=c11
Common_C_Cpp_Flags := $(SGX_COMMON_CFLAGS) -nostdinc -fvisibility=hidden -fpie -fstack-protector $(Trusted_IncPaths) -fno-builtin-printf -I.
C_Flags := $(Flags_Just_For_C) $(Common_C_Cpp_Flags)
Cpp_Flags := $(Common_C_Cpp_Flags) -I$(SGX_SDK)/include/stlport -std=c++11 -nostdinc++  

App_Link_Flags := $(SGX_COMMON_CFLAGS) -L$(SGX_LIBRARY_PATH) -lcryptopp -Wl,-E -ldl

ifneq ($(SGX_MODE), HW)
    Trts_Library_Name := sgx_trts_sim
    Service_Library_Name := sgx_tservice_sim
    App_Link_Flags += -lsgx_uae_service_sim -lpthread
    Urts_Library_Name := sgx_urts_sim
else
    Trts_Library_Name := sgx_trts
    Service_Library_Name := sgx_tservice
    App_Link_Flags += -lsgx_uae_service
    Urts_Library_Name := sgx_urts
endif

App_Link_Flags += -l$(Urts_Library_Name) -lboost_system -lboost_chrono
Crypto_Library_Name := sgx_tcrypto

Trusted_Link_Flags := $(SGX_COMMON_CFLAGS) -Wl,--no-undefined -nostdlib \
    -nodefaultlibs -nostartfiles -L$(SGX_LIBRARY_PATH) -L$(LUADIR)/bin \
    -Wl,--whole-archive -l$(Trts_Library_Name) -lsgx_tsetjmp -Wl,--no-whole-archive \
    -Wl,--start-group -lsgx_tstdc -lsgx_tstdcxx -l$(Crypto_Library_Name) \
    -l$(Service_Library_Name) -lluasgx -Wl,--end-group \
    -Wl,-Bstatic -Wl,-Bsymbolic -Wl,--no-undefined \
    -Wl,-pie,-eenclave_entry -Wl,--export-dynamic  \
    -Wl,--defsym,__ImageBase=0 \
    -Wl,--version-script=$(TRUSTED_DIR)/$(ENCLVNAME).lds

App_Cpp_Sources := $(UNTRUSTED_DIR)/lua_untrusted_proxy.cpp \
                   $(SGXCOMMONDIR)/sgx_errlist.cpp \
                   $(SGXCOMMONDIR)/sgx_initenclave.cpp \
                   $(UNTRUSTED_DIR)/lua.cpp \
                   $(SGXCOMMONDIR)/crypto.cpp \
                   $(SGXCOMMONDIR)/utils.cpp
App_C_Sources := $(UNTRUSTED_DIR)/$(ENCLVNAME)_u.c
App_Objects = $(App_C_Sources:.c=.o) $(App_Cpp_Sources:.cpp=.o)

ifeq ($(ONELUAVM),1)
    App_Objects := $(filter-out $(UNTRUSTED_DIR)/lua.o,$(App_Objects))
    App_Cpp_Flags += -DONLYINTERPRETER
endif

TrustedSources = $(addprefix $(TRUSTED_DIR)/, $(ENCLVNAME)_t.c) ../luacsv/csv.c
TrustedCppSrcs = $(addprefix $(TRUSTED_DIR)/, $(ENCLVNAME).cpp)
TrustedObjects = $(TrustedSources:.c=.o) $(TrustedCppSrcs:.cpp=.o)

.PHONY: all clean luasgx 

all: $(ENCLVNAME).signed.so luasgx

############## UNTRUSTED #######################################################
luasgx : $(App_Objects) $(UNTRLUADIR)/src/liblua.a $(UNTRCJSON)/cjson.a
	@$(CXX) -o $@ $^ $(App_Link_Flags)
	@echo "LINK (App)\t=>\t$@"

$(UNTRCJSON)/cjson.a:
	@$(MAKE) -C $(UNTRCJSON)

$(UNTRLUADIR)/src/liblua.a:
	@$(MAKE) -C $(UNTRLUADIR)/src linux 

$(SGXCOMMONDIR)/%.o : $(SGXCOMMONDIR)/%.cpp
	@$(CXX) $(App_Cpp_Flags) -c $< -o $@ 
	@echo "CXX (App)\t<=\t$@" 

$(UNTRUSTED_DIR)/%.o : $(UNTRUSTED_DIR)/%.cpp
	@$(CXX) $(App_Cpp_Flags) -c $< -o $@ 
	@echo "CXX (App)\t<=\t$@" 

$(UNTRUSTED_DIR)/%_u.o : $(UNTRUSTED_DIR)/%_u.c
	@$(CC) $(App_C_Flags) -c $< -o $@ 
	@echo "CC (App)\t<=\t$@" 

%_u.c : $(TRUSTED_DIR)/$(ENCLVNAME).edl $(SGX_EDGER8R)
	@cd $(dir $@) && $(SGX_EDGER8R) --untrusted ../$< --search-path ../$(dir $<) --search-path $(SGX_SDK)/include
	@echo "EDGER (App)\t=>\t$@"

############## TRUSTED #########################################################
%.o : %.s
	@$(CC) $(C_Flags) -c $< -o $@ 
	@echo "CC (Enclave)\t<=\t$@" 

%.o : %.c
	@$(CC) $(C_Flags) -c $< -o $@ 
	@echo "CC (Enclave)\t<=\t$@" 

%.o : %.cpp
	@$(CXX) $(Cpp_Flags) -c $< -o $@ 
	@echo "CXX (Enclave)\t<=\t$@"

$(ENCLVNAME).so : $(TrustedObjects) $(LUADIR)/bin/libluasgx.a
	$(CXX) $(filter-out %.a,$^) -o $@ $(Trusted_Link_Flags)
	@echo "LINK (Enclave)\t=>\t$@"

$(LUADIR)/bin/libluasgx.a:
	$(MAKE) -C $(LUADIR)

%_t.c : %.edl $(SGX_EDGER8R)
	@cd $(dir $@) && $(SGX_EDGER8R) --trusted ../$< --search-path ../$(dir $<) --search-path $(SGX_SDK)/include
	@echo "EDGER (Enclave)\t=>\t$@"

$(ENCLVNAME).signed.so : $(ENCLVNAME).so
	@$(SGX_ENCLAVE_SIGNER) sign -enclave $< -config $(TRUSTED_DIR)/$(ENCLVNAME).config.xml -out $@ -key $(TRUSTED_DIR)/private_key.pem
	@echo "SIGN (Enclave)\t=>\t$@"

################################################################################

clean:
	rm -f $(TrustedObjects) $(App_Objects) $(TRUSTED_DIR)/*_t.{c,h} $(UNTRUSTED_DIR)/*_u.{c,h} app $(ENCLVNAME)*so luasgx.a

