#ifndef _SGX_COMMON_UTILS_H_
#define _SGX_COMMON_UTILS_H_

#include <string.h>
#ifndef NONENCLAVE_MATCHING
#include <sgx_tcrypto.h>
#else
#include <crypto++/aes.h>
using CryptoPP::AES;
#include <crypto++/ccm.h>
using CryptoPP::CTR_Mode;
#include <crypto++/filters.h>
using CryptoPP::StringSource;
using CryptoPP::StringSink;
using CryptoPP::StreamTransformationFilter;
#endif

//------------------------------------------------------------------------------
inline bool isgraphorspace( char c ) {
    return (c >= 0x09 && c <= 0x0D) || (c >= 0x20 && c <= 0x7E );
}

//------------------------------------------------------------------------------
inline bool is_cipher( const char *buff, size_t len ) {
    size_t validchr;
    for( size_t i = 0; i < len; ++i )
        if( !isgraphorspace(buff[i]) ) return true;
    return false;
}

//------------------------------------------------------------------------------
inline void decrypt( const char *src, char *dst,  size_t len ) {
    uint8_t key[16], iv[16];
    memset(key,0,16); memset(iv,0,16);
    key[0] = 'a'; key[15] = '5';
    iv[0] = 'x'; iv[15]= '?';
#ifndef NONENCLAVE_MATCHING
    sgx_aes_ctr_decrypt(&key, (const uint8_t*)src, len, iv, 128, (uint8_t*)dst);
#else
    CTR_Mode< AES >::Decryption d;
    std::string recov, cipher(src,len);
    d.SetKeyWithIV( key, sizeof(key), iv );
    StringSource( cipher, true, new StreamTransformationFilter(d,
                                                       new StringSink(recov)));
    memcpy(dst, recov.c_str(), std::min(len,recov.size()));
#endif
}
//------------------------------------------------------------------------------
inline void encrypt( const char *src, char *dst, size_t len ) {
    uint8_t key[16], iv[16];
    memset(key,0,16); memset(iv,0,16);
    key[0] = 'a'; key[15] = '5';
    iv[0] = 'x'; iv[15]= '?';
#ifndef NONENCLAVE_MATCHING
    sgx_aes_ctr_encrypt(&key, (const uint8_t*)src, len, iv, 128, (uint8_t*)dst);
#else
#endif
}

#endif

