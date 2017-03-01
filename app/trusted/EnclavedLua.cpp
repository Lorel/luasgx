#include <EnclavedLua_t.h>
#include <lauxlib.h>
#include <lualib.h>
#include <string.h>
#include <lua_parser.h>
#include <sgx_utiles.h>
#include <file_mock.h>
#include <algorithm>
#include <string>

/* 
 * printf: 
 *   Invokes OCALL to display the enclave buffer to the terminal.
 */
//------------------------------------------------------------------------------
extern "C" {
void printf(const char *fmt, ...) {
    char buf[BUFSIZ] = {'\0'};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, BUFSIZ, fmt, ap);
    va_end(ap);
    ocall_print(buf);
}
}

//====================== ECALLS ================================================
lua_State *L = 0;
extern "C" { 
    extern int luaopen_cjson(lua_State *l);
    extern int luaopen_csv(lua_State *l);
}
void ecall_luainit() {
    L = luaL_newstate();  /* create state */
    if (L == NULL) {
        ocall_print("cannot create state: not enough memory");
        return;
    }
    luaL_requiref(L, "cjson", luaopen_cjson, 1);
    luaL_requiref(L, "ccsv",  luaopen_csv, 1); 
    luaL_openlibs(L);

    lua_createtable(L, 0, 2);
    lua_pushstring(L, "_lua_interpreter_"); lua_rawseti(L,-2,-1);
    lua_pushstring(L, "_lua_script_");      lua_rawseti(L,-2, 0);
    lua_setglobal(L,"arg");

    lua_settop(L,0);
}

//------------------------------------------------------------------------------
void ecall_luaclose() {
    lua_close(L);
}

#include <ldo.h>
struct Buff{ const char *buff; size_t len;};
//------------------------------------------------------------------------------
void test(lua_State *L, void *arg) {
    Buff *b = (Buff*)arg;
    b->buff = luaL_checklstring( L, 1, &b->len );
}

//------------------------------------------------------------------------------
static void stackDump (lua_State *L) {
    int i;
    int top = lua_gettop(L);
    for (i = 1; i <= top; i++) {  /* repeat for each level */
        int t = lua_type(L, i);
        switch (t) {
        case LUA_TSTRING:  /* strings */
            printf("`%s'", lua_tostring(L, i));
            break;

        case LUA_TBOOLEAN:  /* booleans */
            printf(lua_toboolean(L, i) ? "true" : "false");
            break;

        case LUA_TNUMBER:  /* numbers */
            printf("%g", lua_tonumber(L, i));
            break;

        default:  /* other values */
            printf("%s", lua_typename(L, t));
            break;
        }
        printf(" * ");  /* put a separator */
    }
}

//------------------------------------------------------------------------------
size_t ecall_execfunc( const char *ccode, size_t csz, const char *cdata, size_t dsz, char *buff, size_t len ) {
    char pcode[BUFSIZ], pdata[BUFSIZ];
    size_t cs, ds, result_size;
    decrypt( ccode, pcode, cs = std::min(sizeof(pcode)-1,csz) );
    decrypt( cdata, pdata, ds = std::min(sizeof(pdata)-1,dsz) );
    pcode[cs] = 0; pdata[ds] = 0;
    
    std::string c = std::string("x=") + pcode;
    ecall_execute("abc",c.c_str(),c.size());

    lua_getglobal(L,"x");
    lua_pushstring( L, pdata );
    lua_pcall(L,1,1,0);

    Buff b;
    int status = luaD_rawrunprotected( L, test, &b );
    std::string msg("Error: ");
    if( status ) {
        if( lua_type(L,-1) == LUA_TSTRING ) msg += lua_tostring(L,-1);
        b.buff = msg.c_str();
        b.len = msg.size();
    }

    encrypt( b.buff, buff, result_size = std::min(b.len,len) );
    lua_settop(L,0);
    return result_size;
}

//------------------------------------------------------------------------------
void ecall_add_execution( const char *fname, const char *e, size_t len ) {
    file_mock( e, len, fname );
}

//------------------------------------------------------------------------------
void ecall_execute( const char *fname, const char *e, size_t len ) {
    file_mock( e, len, fname );
    int st = handle_script(L,fname);
}

