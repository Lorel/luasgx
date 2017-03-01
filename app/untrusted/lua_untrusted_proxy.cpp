#include <lua_untrusted_proxy.h>
#include <libgen.h>
#include <signal.h>
#include <unistd.h>

#ifndef NONENCLAVE
#include <sgx_initenclave.h>
#include <EnclavedLua_u.h>

//------------------------------------------------------------------------------
/* Global EID shared by multiple threads */
sgx_enclave_id_t global_eid = 0;
//------------------------------------------------------------------------------
#endif

//------------------------------------------------------------------------------
void ocall_print( const char* str ) {
    printf("\033[96m%s\033[0m", str);
}

//------------------------------------------------------------------------------
void ocall_outresult( const char* e, size_t len ) {

}

//------------------------------------------------------------------------------
void ctrlc_handler( int s ) {
    printf("(%d) Goodbye!\n", s);
    exit(1);
}

//------------------------------------------------------------------------------
#ifdef ONLYINTERPRETER
/* Application entry */
#include <fstream>
#include <iostream>
#include <utils.h>
int SGX_CDECL main(int argc, char *argv[]) {
    (void)(argc);

#ifndef NONENCLAVE
    /* Changing dir to where the executable is.*/
    char absolutePath [MAX_PATH];
    char *ptr = NULL;

    ptr = realpath(dirname(argv[0]),absolutePath);

    if( chdir(absolutePath) != 0)
            abort();

    /* Initialize the enclave */
    if(initialize_enclave( global_eid, "EnclavedLua.signed.so",
                                           "enclave.lua.token" ) < 0) {
        return -1;
    }
#endif
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = ctrlc_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT,&sigIntHandler,NULL);
//==============================================================================
    if( argc < 2 ) { std::cerr << "Usage: " << argv[0] << " [FILE]\n"; exit(0); }

    ecall_luainit( global_eid );
    Logger stats; 
    stats.init("lua_bmark.log");
    for( int fidx = 1; fidx < argc; ++fidx ) { 
        char *fname = argv[fidx];
        std::string line, wholefile;
        std::ifstream luafile( fname );
        while( std::getline(luafile, line) ) wholefile += line + '\n';
        if( fidx + 1 == argc ) {
        stats.start();
        ecall_execute( global_eid, fname, wholefile.c_str(), wholefile.size() );
        stats.finish();
        } else
        ecall_add_execution( global_eid, fname, wholefile.c_str(), wholefile.size() );
    }
    stats.close();
    ecall_luaclose( global_eid );
//==============================================================================
#ifndef NONENCLAVE
    sgx_destroy_enclave(global_eid);
#endif
    return 0;
}
#endif

