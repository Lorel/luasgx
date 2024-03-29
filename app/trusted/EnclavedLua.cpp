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
void get_Lua_string(lua_State *L, void *arg) {
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
size_t ecall_execfunc(LuaSGX_Arg *args, char *buff, size_t len) {
    char pdata[BUFSIZ];
    size_t ds, result_size;
    int nargs = 0;

    // extract function code
    decrypt(args->buffer, pdata, ds = std::min(sizeof(pdata)-1, args->size));
    pdata[ds] = 0;

    // assign pcode (function) to global x
    std::string c = std::string("x=") + pdata;
    ecall_execute("enclaved_lua_code", c.c_str(), c.size());

    lua_getglobal(L, "x");

    // extract args
    while (args->next != NULL) {
        args = args->next;
        decrypt(args->buffer, pdata, ds = std::min(sizeof(pdata)-1, args->size));
        pdata[ds] = 0;
        lua_pushstring(L, pdata);
        nargs += 1;
    }

    // process Lua code, return only one result
    // (use lua_pcall(L, nargs, LUA_MULTRET,0) otherwise)
    int call_status = lua_pcall(L, nargs, 1, 0);

    // retrieve result (string)
    Buff b;
    int get_status = luaD_rawrunprotected(L, get_Lua_string, &b);

    // handle error msg
    std::string msg("Error: ");

    if (call_status || get_status) {
        if (lua_type(L, -1) == LUA_TSTRING)
            msg += lua_tostring(L, -1);
        else
            msg += "not a string (but should be one)";

        if (get_status)
            msg += " (error when retrieving return value, must be a string)";

        // replace buffer content with error message
        b.buff = msg.c_str();
        b.len = msg.size();
    }

    // store encrypted response
    encrypt(b.buff, buff, result_size = std::min(b.len, len));

    lua_settop(L, 0);
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
