SGX_SDK=/opt/intel/sgxsdk

CC     := gcc
CXX    := g++
TARGET := libluasgx.a
BINDIR := bin
OBJDIR := obj
SRCDIR := src

SSOURCES = 
CXXSOURCES = file_mock.cpp
CSOURCES = libc_proxy.c lauxlib.c ldblib.c liolib.c \
           lbaselib.c lapi.c ldebug.c lvm.c \
           ltable.c ltm.c lmem.c lstring.c lgc.c \
           lobject.c lfunc.c lstate.c lzio.c \
           linit.c lopcodes.c lmathlib.c ltablib.c \
           loslib.c lutf8lib.c lundump.c llex.c \
           lctype.c ldump.c ldo.c lparser.c lcode.c \
           lstrlib.c loadlib.c lcorolib.c lua_parser.c
OBJECTS = $(addprefix $(OBJDIR)/, \
			$(CSOURCES:.c=.o) $(SSOURCES:.s=.o) $(CXXSOURCES:.cpp=.o))
CJSONDIR = ../lua-cjson
CJSONOBJS = fpconv.o lua_cjson.o strbuf.o

SGX_COMMON_CFLAGS := -m64
ifeq ($(SGX_DEBUG), 1)
    SGX_COMMON_CFLAGS += -O0 -g
else
    SGX_COMMON_CFLAGS += -O2
endif

INCPATHS := $(addprefix -I, $(SRCDIR) $(SGX_SDK)/include/tlibc \
                            $(SGX_SDK)/include /usr/include/x86_64-linux-gnu/)
Flags_JUSTC  := -Wno-implicit-function-declaration -std=c11
Flags_COMMON := $(SGX_COMMON_CFLAGS) -nostdinc -fvisibility=hidden -fpie -fstack-protector -fno-builtin-printf $(INCPATHS)
CFLAGS       := $(Flags_COMMON) $(Flags_JUSTC)
CXXFLAGS     := $(Flags_COMMON) -I$(SGX_SDK)/include/stlport -std=c++11 \
                -nostdinc++ 

.PHONY: all clean

all: $(BINDIR)/$(TARGET) 

$(BINDIR)/$(TARGET) : $(OBJECTS) $(CJSONOBJS) | $(BINDIR)
	@ar rcs $@ $^
	@echo "Packing lib\t=>\t$@"

$(OBJDIR)/%.o : $(SRCDIR)/%.s | $(OBJDIR)
	@$(CC) $(CFLAGS) -c $< -o $@ 
	@echo "CC (Enclave)\t<=\t$<" 

$(OBJDIR)/%.o : $(SRCDIR)/%.c | $(OBJDIR)
	@$(CC) $(CFLAGS) -c $< -o $@ 
	@echo "CC (Enclave)\t<=\t$<" 

$(OBJDIR)/%.o : $(SRCDIR)/%.cpp | $(OBJDIR)
	@$(CXX) $(CXXFLAGS) -c $< -o $@ 
	@echo "CXX (Enclave)\t<=\t$<"

$(CJSONOBJS) : %.o : $(CJSONDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -DSGX_COMPILE 
	@echo "CC (Enclave)\t<=\t$<" 

$(BINDIR):
	@mkdir $(BINDIR)

$(OBJDIR):
	@mkdir $(OBJDIR)

clean:
	@rm -rf $(BINDIR) $(OBJDIR)
	@echo "Cleaned $(BINDIR) $(OBJDIR)"
