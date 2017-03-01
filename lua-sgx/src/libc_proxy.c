#include <string.h>
#include "libc_proxy.h"
#include <file_mock.h>
#define TRACE_LIBC_CALLS

/* STDIO */
FILE *const stdin  = (FILE*)666;
FILE *const stdout = (FILE*)667;
FILE *const stderr = (FILE*)668;

FILE *fopen(const char *path, const char *mode) {
    FILE *ret = fmock_open(path);
#ifdef TRACE_LIBC_CALLS
    if( ret == NULL )
    printf("FILE *fopen(const char *path='%s', const char *mode='%s')\n", path, mode);
#endif
    return ret;
}

FILE *freopen(const char *path, const char *mode, FILE *stream) {
#ifdef TRACE_LIBC_CALLS
    printf("FILE *freopen(const char *path='%s', const char *mode='%s', FILE *stream)\n", path, mode);
#endif
}

int fclose(FILE *f) {
    return fmock_close(f);
}

int remove(const char *pathname) {
#ifdef TRACE_LIBC_CALLS
    printf("remove(const char *pathname='%s')\n",pathname);
#endif
}

int rename(const char *oldpath, const char *newpath) {
#ifdef TRACE_LIBC_CALLS
    printf("int rename(const char *oldpath='%s', const char *newpath='%s')\n", oldpath, newpath);
#endif
}

int feof(FILE *f) {
    return fmock_feof(f);
}

int ferror(FILE *f) {
    return 0;
}

int fflush(FILE *f) {
    return 0;
}

void clearerr(FILE *f) {
#ifdef TRACE_LIBC_CALLS
    printf("void clearerr(FILE *f)\n");
#endif
}

int fseek(FILE *f, long offset, int whence) {
#ifdef TRACE_LIBC_CALLS
    printf("int fseek(FILE *f, long offset, int whence)\n");
#endif
}

long ftell(FILE *f) {
#ifdef TRACE_LIBC_CALLS
    printf("long ftell(FILE *f)\n");
#endif
}

int getc(FILE *f) {
    int ret = fmock_getc(f);
#ifdef TRACE_LIBC_CALLS
    if( ret == EOF )
    printf("int getc(FILE *f)\n");
#endif
    return ret;
}

int ungetc(int c, FILE *f) {
#ifdef TRACE_LIBC_CALLS
    printf("int ungetc(int c, FILE *f)\n");
#endif
}

char *fgets(char *s, int size, FILE *stream) {
#ifdef TRACE_LIBC_CALLS
    printf("char *fgets(char *s, int size, FILE *stream)\n");
#endif
    return NULL;
}

int fputc(int c, FILE *stream) {
#ifdef TRACE_LIBC_CALLS
    printf("int fputc(int c, FILE *stream)\n");
#endif
    return NULL;
}

#include <stdarg.h>
int fprintf(FILE *stream, const char *format, ...) {
#ifdef TRACE_LIBC_CALLS
    if(stream == stdout || stream == stderr ) {
        char buf[1024] = {'\0'};
        va_list ap;
        va_start(ap, format);
        vsnprintf(buf, 1024, format, ap);
        va_end(ap);
        ocall_print(buf);
    } else
        printf("int fprintf(FILE *stream='%d', const char *format, ...)\n",(int)stream);
#endif
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t ret = fmock_fread(ptr,size,nmemb,stream);
#ifdef TRACE_LIBC_CALLS
    if( !ret )
    printf("size_t fread(void *ptr, size_t size='%d', size_t nmemb='%d', FILE *stream='%d')\n",size, nmemb, (int)stream);
#endif
    return ret;
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream) {
#ifdef TRACE_LIBC_CALLS
    if(stream == stdout || stream == stderr) {
        printf((const char*)ptr);
    } else
        printf("size_t fwrite(const void *ptr, size_t size='%d', size_t nmemb='%d', FILE *stream='%d')\n",size, nmemb, (int)stream);
#endif
    return nmemb;
}

int setvbuf(FILE *stream, char *buf, int mode, size_t size) {
#ifdef TRACE_LIBC_CALLS
    printf("int setvbuf(FILE *stream, char *buf, int mode, size_t size)\n");
#endif
}

char *tmpnam(char *s) {
#ifdef TRACE_LIBC_CALLS
    printf("char *tmpnam(char *s)\n");
#endif
}

FILE *tmpfile(void) {
#ifdef TRACE_LIBC_CALLS
    printf("FILE *tmpfile(void)\n");
#endif
}

/* STDLIB */
int rand (void) {
#ifdef TRACE_LIBC_CALLS
    printf("int rand (void)\n");
#endif
}

void srand (unsigned seed) {
#ifdef TRACE_LIBC_CALLS
    printf("void srand (unsigned seed)\n");
#endif
}

char *getenv (const char *name) {
    return NULL;
}

int system (const char *command) {
#ifdef TRACE_LIBC_CALLS
    printf("int system (const char *command)\n");
#endif
}

void exit (int status) {
#ifdef TRACE_LIBC_CALLS
    printf("void exit (int status='%d')\n",status);
#endif
}

/* STRING */
char *strcpy (char *dest, const char *src) { strncpy(dest,src,strlen(src)); }

/* TIME */
time_t mktime (struct tm *t) {
#ifdef TRACE_LIBC_CALLS
    printf("time_t mktime (struct tm *t)\n");
#endif
}

struct tm *gmtime (const time_t *timep) {
#ifdef TRACE_LIBC_CALLS
    printf("struct tm *gmtime (const time_t *timep)\n");
#endif
}

struct tm *localtime (const time_t *timep) {
#ifdef TRACE_LIBC_CALLS
    printf("struct tm *localtime (const time_t *timep)\n");
#endif
}

time_t time (time_t *__timer) {
#ifdef TRACE_LIBC_CALLS
    printf("time_t time (time_t *__timer)\n");
#endif
}

clock_t clock (void) {
#ifdef TRACE_LIBC_CALLS
    printf("clock_t clock (void)\n");
#endif
}

/* LOCALE */
char *setlocale (int category, const char *locale) {
#ifdef TRACE_LIBC_CALLS
    printf("char *setlocale (int category, const char *locale)\n");
#endif
}

struct lconv *localeconv(void) {
    static struct lconv l;
    l.decimal_point=".";
    l.thousands_sep=l.grouping=l.int_curr_symbol=l.currency_symbol=l.mon_decimal_point=l.mon_thousands_sep=l.mon_grouping=l.positive_sign=l.negative_sign="";
    l.int_frac_digits=l.frac_digits=l.p_cs_precedes=l.p_sep_by_space=l.n_cs_precedes=l.n_sep_by_space=l.p_sign_posn=l.n_sign_posn=l.int_p_cs_precedes=l.int_p_sep_by_space=l.int_n_cs_precedes=l.int_n_sep_by_space=l.int_p_sign_posn=l.int_n_sign_posn=127; 
    return &l;
}

