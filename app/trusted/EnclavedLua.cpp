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
void check_string(lua_State *L, void *arg) {
    int n = lua_gettop(L);

    for (int i = 1; i < n+1; i++)
        luaL_checklstring(L, 1, NULL);
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
int ecall_execfunc(LuaSGX_Arg *args, LuaSGX_Arg **ret, size_t len) {
    char pdata[BUFSIZ];
    const char *buff;
    char ebuff[BUFSIZ];
    size_t ds, result_size, sz;
    int nargs = 0;

    /* extract function code */
    decrypt(args->buffer, pdata, ds = std::min(sizeof(pdata)-1, args->size));
    pdata[ds] = 0;

    /* assign pcode (function) to global x */
    std::string c = std::string("x=") + pdata;
    ecall_execute("enclaved_lua_code", c.c_str(), c.size());

    lua_getglobal(L, "x");

    /* extract args */
    while (args->next != NULL) {
        args = args->next;
        decrypt(args->buffer, pdata, ds = std::min(sizeof(pdata)-1, args->size));
        pdata[ds] = 0;
        lua_pushstring(L, pdata);
        nargs += 1;
    }

    /* process Lua code, push all results on Lua state */
    int call_status = lua_pcall(L, nargs, LUA_MULTRET, 0);

    /* check result values (strings only) */
    /* TODO do it by other means */
    int check_status = luaD_rawrunprotected(L, check_string, NULL);

    if (call_status || check_status) {
        /* handle error msg */
        std::string msg("Error: ");

        if (lua_type(L, -1) == LUA_TSTRING)
            msg += lua_tostring(L, -1);
        else
            msg += "error msg is not a string (but should be one)";

        if (check_status)
            msg += " (error when retrieving return value, must be a string)";

        /* encrypt error message */
        encrypt(msg.c_str(), ebuff, result_size = std::min(msg.size(), len));

        /* store error message */
        *ret = new LuaSGX_Arg;
        (*ret)->buffer = (char*) memcpy((char*) malloc(result_size), ebuff, result_size);
        (*ret)->size = result_size;
        (*ret)->next = NULL;

        lua_settop(L, 0);
        return LUA_ERRRUN;
    }

    /* get number of elements to return */
    int n = lua_gettop(L);

    if (n == 0) {
        /* no element to return */
        *ret = NULL;

        lua_settop(L, 0);
        return LUA_OK;
    }

    /* store elements otherwise */
    *ret = new LuaSGX_Arg;
    LuaSGX_Arg *value = *ret;

    /* store return values */
    for (int i = 1; i < n + 1; i++) {
        // printf("return value %s\n", luaL_checklstring(L, i, NULL));

        /* encrypt return value */
        buff = luaL_checklstring(L, i, &sz);
        encrypt(buff, ebuff, result_size = std::min(sz+1, len));
        // printf("encrypted value %s, size %d, %d, %d\n", buff, sz, len, result_size);

        /* store return value */
        value->buffer = (char*) memcpy((char*) malloc(result_size), ebuff, result_size);
        value->size = result_size;
        // printf("value->buffer %s, value->size %d\n", value->buffer , value->size);

        if (i == n) {
            /* last element, end the list */
            value->next = NULL;
        } else {
            /* chain next element */
            value->next = new LuaSGX_Arg;
            value = value->next;
        }
    }

    lua_settop(L, 0);
    return LUA_OK;
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
