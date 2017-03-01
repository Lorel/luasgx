#include <libc_proxy.h>
#include <file_mock.h>
#include <map>
#include <string>

static FILE *next_fd = stderr + 1;
extern "C" {
extern void printf(const char *fmt, ...);
}

struct FileDescriptor;
typedef std::map<FILE*,FileDescriptor*> MapFiles;
static MapFiles files_byids;
struct FileDescriptor {
    FILE* id;
    std::string data;
    std::string::iterator it;

    FileDescriptor() {
        id = next_fd++;
    }

    void set(const char *buff, size_t len) {
        data.assign(buff,len);
        it = data.begin();
        files_byids[id] = this;
    }
};
typedef std::map<std::string, FileDescriptor> MapFilesByName;
static MapFilesByName files;

void file_mock( const char *buff, size_t len, const char *fname ) {
    files[ std::string(fname) ].set(buff,len);
}

FILE* fmock_open( const char *fname ) {
    std::string flname(fname);
    if( strlen(fname) > 2 && fname[0] == '.' && fname[1] == '/' )
        flname.erase(0,2);
    if( files.find( flname ) == files.end() ) return NULL;
    return files[ flname ].id;
}

int fmock_close( FILE *f ) {
/*
    MapFilesByName::iterator it = files.begin();
    for(; it != files.end(); ++it )
        if( it->second.id == f ) { files.erase(it); break; }
    files_byids.erase(f);
*/
    return 0;
}

int fmock_feof(FILE *f) {
    return files_byids.find(f) == files_byids.end() ||
           files_byids[f]->it == files_byids[f]->data.end();
}

int fmock_getc(FILE *f) {
    if( fmock_feof(f) ) return EOF;
    return (int)*(files_byids[f]->it)++;
}

size_t fmock_fread(void *ptr, size_t size, size_t nmemb, FILE *f) {
    size_t ret = 0, pos = 0;
    MapFiles::iterator it;
    if( (it = files_byids.find(f)) == files_byids.end() ) return ret;

    FileDescriptor &fd = *it->second;
    char *p = (char*)ptr;
    while( fd.it != fd.data.end() && p - (char*)ptr < size*nmemb &&
           ( pos = fd.it - fd.data.begin() )+size <= fd.data.size() ) {
        memcpy( p++, fd.data.c_str() + pos, size );
        fd.it += size;
        ++ret;
    }
    return ret;
}

