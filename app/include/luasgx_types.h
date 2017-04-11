#ifndef LUASGX_TYPES_H
#define LUASGX_TYPES_H

typedef struct LuaSGX_Arg {
    const char *buffer;
    size_t size;
    struct LuaSGX_Arg *next;
} LuaSGX_Arg;

#endif
