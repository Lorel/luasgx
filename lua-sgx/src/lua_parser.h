#ifndef _LUA_PARSER_DEFINED_
#define _LUA_PARSER_DEFINED_


#include <lua.h>
#ifdef __cplusplus
extern "C" {
#endif

int handle_script (lua_State *L, const char *fname);
int dostring (lua_State *L, const char *s, const char *name);
int docall (lua_State *L, int narg, int nres);

#ifdef __cplusplus
}
#endif


#endif 

