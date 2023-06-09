/**
    Copyright (C) powturbo 2013-2023
    GPL v3 License

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

    - homepage : https://sites.google.com/site/powturbo/
    - github   : https://github.com/powturbo
    - twitter  : https://twitter.com/powturbo
    - email    : powturbo [_AT_] gmail [_DOT_] com
**/
// TurboRC Range Coder : include header 
#if defined(_MSC_VER) && (_MSC_VER < 1600)
  #if !defined(_STDINT) && !defined(_MSC_STDINT_H_)
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;
  #endif
#else
#include <stdint.h>
#endif
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif
size_t lzpenc(unsigned char *__restrict in, size_t inlen,  unsigned char *__restrict out, unsigned lenmin, unsigned h_bits);
size_t lzpdec(unsigned char *__restrict in, size_t outlen, unsigned char *__restrict out, unsigned lenmin, unsigned h_bits);

unsigned char *rcqlfc(unsigned char *__restrict in, size_t n, unsigned char *__restrict out, unsigned char *__restrict r2c);

size_t utf8enc(unsigned char *__restrict in, size_t inlen,  unsigned char *__restrict out, unsigned flag);
size_t utf8dec(unsigned char *__restrict in, size_t outlen, unsigned char *__restrict out);
void memrev(unsigned char a[], unsigned n);

//----------- 24 = 3x8 -----------------
unsigned  delta8l24(uint8_t *in, size_t n);
void      delta8e24(uint8_t *in, size_t n, uint8_t *out);
void      delta8d24(uint8_t *in, size_t n, uint8_t *out);


#if defined(__clang__) && defined(__is_identifier)
  #if !__is_identifier(_Float16)
    #undef FLT16_BUILTIN
  #endif
#elif defined(FLT16_MAX)
#define FLT16_BUILTIN
#endif
//------- Quantization : b number of bits quantized in out ----------------
  #if defined(FLT16_BUILTIN) 
void fpquant8e16( _Float16 *in, size_t n, uint8_t  *out, unsigned b, _Float16 *pfmin, _Float16 *pfmax);
void fpquant16e16(_Float16 *in, size_t n, uint16_t *out, unsigned b, _Float16 *pfmin, _Float16 *pfmax);
  #endif                                                                                              
																									  
void fpquant8e32(    float *in, size_t n, uint8_t  *out, unsigned b, float    *pfmin,    float *pfmax);
void fpquant16e32(   float *in, size_t n, uint16_t *out, unsigned b, float    *pfmin,    float *pfmax);
void fpquant32e32(   float *in, size_t n, uint32_t *out, unsigned b, float    *pfmin,    float *pfmax);
																									  
void fpquant8e64(   double *in, size_t n, uint8_t  *out, unsigned b, double   *pfmin,   double *pfmax);
void fpquant16e64(  double *in, size_t n, uint16_t *out, unsigned b, double   *pfmin,   double *pfmax);
void fpquant32e64(  double *in, size_t n, uint32_t *out, unsigned b, double   *pfmin,   double *pfmax);
void fpquant64e64(  double *in, size_t n, uint64_t *out, unsigned b, double   *pfmin,   double *pfmax);
																									  
  #if defined(FLT16_BUILTIN)                                                                          
void fpquant8d16( uint8_t  *in, size_t n, _Float16 *out, unsigned b, _Float16   fmin, _Float16   fmax);
void fpquant16d16(uint16_t *in, size_t n, _Float16 *out, unsigned b, _Float16   fmin, _Float16   fmax);
  #endif                                                                                              
																									  
void fpquant8d32( uint8_t  *in, size_t n, float    *out, unsigned b, float      fmin,    float   fmax);
void fpquant16d32(uint16_t *in, size_t n, float    *out, unsigned b, float      fmin,    float   fmax);
void fpquant32d32(uint32_t *in, size_t n, float    *out, unsigned b, float      fmin,    float   fmax);
																									  
void fpquant8d64( uint8_t  *in, size_t n, double   *out, unsigned b, double     fmin,   double   fmax);
void fpquant16d64(uint16_t *in, size_t n, double   *out, unsigned b, double     fmin,   double   fmax);
void fpquant32d64(uint32_t *in, size_t n, double   *out, unsigned b, double     fmin,   double   fmax);
void fpquant64d64(uint64_t *in, size_t n, double   *out, unsigned b, double     fmin,   double   fmax);

#ifdef __cplusplus
}
#endif
