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
// TurboRANS Range Asymmetric Systems : include header 
#define LIBAPI

typedef LIBAPI size_t (*fanscdfenc)(  unsigned char *in, size_t inlen, unsigned char *out);
typedef LIBAPI size_t (*fanscdfdec)(  unsigned char *in, size_t inlen, unsigned char *out);
typedef LIBAPI size_t (*fanscdf4senc)(unsigned char *in, size_t inlen, unsigned char *out, cdf_t *cdf);
typedef LIBAPI size_t (*fanscdf4sdec)(unsigned char *in, size_t inlen, unsigned char *out, cdf_t *cdf);

extern fanscdfenc _anscdfenc;
extern fanscdfdec _anscdfdec;
extern fanscdfenc _anscdf4enc;
extern fanscdfdec _anscdf4dec;

#ifdef __cplusplus
extern "C" {
#endif
void anscdfini(unsigned id);

LIBAPI size_t anscdf4senc(   unsigned char *in, size_t inlen,  unsigned char *out, cdf_t *cdf);
LIBAPI size_t anscdf4sdec(   unsigned char *in, size_t outlen, unsigned char *out, cdf_t *cdf);
                             
LIBAPI size_t anscdf4senc(   unsigned char *in, size_t inlen,  unsigned char *out, cdf_t *cdf);
LIBAPI size_t anscdf4sdec(   unsigned char *in, size_t outlen, unsigned char *out, cdf_t *cdf);
LIBAPI size_t anscdf4enc(    unsigned char *in, size_t inlen,  unsigned char *out);
LIBAPI size_t anscdf4dec(    unsigned char *in, size_t outlen, unsigned char *out);
LIBAPI size_t anscdfenc(     unsigned char *in, size_t inlen,  unsigned char *out);
LIBAPI size_t anscdfdec(     unsigned char *in, size_t outlen, unsigned char *out);
LIBAPI size_t anscdf1enc(    unsigned char *in, size_t inlen,  unsigned char *out); // o1
LIBAPI size_t anscdf1dec(    unsigned char *in, size_t outlen, unsigned char *out);
                             
LIBAPI size_t anscdfuenc16(  unsigned char *in, size_t inlen,  unsigned char *out); // vlc6 16 bits
LIBAPI size_t anscdfudec16(  unsigned char *in, size_t outlen, unsigned char *out);
LIBAPI size_t anscdfuzenc16( unsigned char *in, size_t inlen,  unsigned char *out); // vlc6 16 bits zigzag
LIBAPI size_t anscdfuzdec16( unsigned char *in, size_t outlen, unsigned char *out);
                             
LIBAPI size_t anscdfvenc16(  unsigned char *in, size_t inlen,  unsigned char *out); // vlc7 16 bits
LIBAPI size_t anscdfvdec16(  unsigned char *in, size_t outlen, unsigned char *out);
LIBAPI size_t anscdfvzenc16( unsigned char *in, size_t inlen,  unsigned char *out); // vlc7 16 bits zigzag
LIBAPI size_t anscdfvzdec16( unsigned char *in, size_t outlen, unsigned char *out);
                             
LIBAPI size_t anscdfvenc32(  unsigned char *in, size_t inlen,  unsigned char *out); // vlc7 32 bits
LIBAPI size_t anscdfvdec32(  unsigned char *in, size_t outlen, unsigned char *out);
LIBAPI size_t anscdfvzenc32( unsigned char *in, size_t inlen,  unsigned char *out); // vlc7 32 bits zigzag
LIBAPI size_t anscdfvzdec32( unsigned char *in, size_t outlen, unsigned char *out);

//-------------- direct function calls --------------------------------------------------------
LIBAPI size_t anscdf4senc0(  unsigned char *in, size_t inlen,  unsigned char *out, cdf_t *cdf);
LIBAPI size_t anscdf4sdec0(  unsigned char *in, size_t outlen, unsigned char *out, cdf_t *cdf);
LIBAPI size_t anscdf4sencs(  unsigned char *in, size_t inlen,  unsigned char *out, cdf_t *cdf);
LIBAPI size_t anscdf4sdecs(  unsigned char *in, size_t outlen, unsigned char *out, cdf_t *cdf);
LIBAPI size_t anscdf4sencx(  unsigned char *in, size_t inlen,  unsigned char *out, cdf_t *cdf);
LIBAPI size_t anscdf4sdecx(  unsigned char *in, size_t outlen, unsigned char *out, cdf_t *cdf);
                             
LIBAPI size_t anscdfenc0(    unsigned char *in, size_t inlen,  unsigned char *out);
LIBAPI size_t anscdfencs(    unsigned char *in, size_t inlen,  unsigned char *out);
LIBAPI size_t anscdfencx(    unsigned char *in, size_t inlen,  unsigned char *out);
LIBAPI size_t anscdfdec0(    unsigned char *in, size_t outlen, unsigned char *out);
LIBAPI size_t anscdfdecs(    unsigned char *in, size_t outlen, unsigned char *out);
LIBAPI size_t anscdfdecx(    unsigned char *in, size_t outlen, unsigned char *out);
                             
LIBAPI size_t anscdf1enc0(   unsigned char *in, size_t inlen,  unsigned char *out);
LIBAPI size_t anscdf1encs(   unsigned char *in, size_t inlen,  unsigned char *out);
LIBAPI size_t anscdf1encx(   unsigned char *in, size_t inlen,  unsigned char *out);
LIBAPI size_t anscdf1dec0(   unsigned char *in, size_t outlen, unsigned char *out);
LIBAPI size_t anscdf1decs(   unsigned char *in, size_t outlen, unsigned char *out);
LIBAPI size_t anscdf1decx(   unsigned char *in, size_t outlen, unsigned char *out);
                             
LIBAPI size_t anscdf4enc0(   unsigned char *in, size_t inlen,  unsigned char *out);
LIBAPI size_t anscdf4encs(   unsigned char *in, size_t inlen,  unsigned char *out);
LIBAPI size_t anscdf4encx(   unsigned char *in, size_t inlen,  unsigned char *out);
LIBAPI size_t anscdf4dec0(   unsigned char *in, size_t outlen, unsigned char *out);
LIBAPI size_t anscdf4decs(   unsigned char *in, size_t outlen, unsigned char *out);
LIBAPI size_t anscdf4decx(   unsigned char *in, size_t outlen, unsigned char *out);
                             
LIBAPI size_t anscdfuenc160( unsigned char *in, size_t inlen,  unsigned char *out);
LIBAPI size_t anscdfudec160( unsigned char *in, size_t outlen, unsigned char *out);
LIBAPI size_t anscdfuenc16s( unsigned char *in, size_t inlen,  unsigned char *out);
LIBAPI size_t anscdfudec16s( unsigned char *in, size_t outlen, unsigned char *out);
LIBAPI size_t anscdfuenc16x( unsigned char *in, size_t inlen,  unsigned char *out);
LIBAPI size_t anscdfudec16x( unsigned char *in, size_t outlen, unsigned char *out);
                             
LIBAPI size_t anscdfuzenc160(unsigned char *in, size_t inlen,  unsigned char *out);
LIBAPI size_t anscdfuzdec160(unsigned char *in, size_t outlen, unsigned char *out);
LIBAPI size_t anscdfuzenc16s(unsigned char *in, size_t inlen,  unsigned char *out);
LIBAPI size_t anscdfuzdec16s(unsigned char *in, size_t outlen, unsigned char *out);
LIBAPI size_t anscdfuzenc16x(unsigned char *in, size_t inlen,  unsigned char *out);
LIBAPI size_t anscdfuzdec16x(unsigned char *in, size_t outlen, unsigned char *out);
                             
LIBAPI size_t anscdfvenc160( unsigned char *in, size_t inlen,  unsigned char *out); // vlc7 16 bits
LIBAPI size_t anscdfvdec160( unsigned char *in, size_t outlen, unsigned char *out);
LIBAPI size_t anscdfvenc16s( unsigned char *in, size_t inlen,  unsigned char *out);
LIBAPI size_t anscdfvdec16s( unsigned char *in, size_t outlen, unsigned char *out);
LIBAPI size_t anscdfvenc16x( unsigned char *in, size_t inlen,  unsigned char *out);
LIBAPI size_t anscdfvdec16x( unsigned char *in, size_t outlen, unsigned char *out);

LIBAPI size_t anscdfvzenc160(unsigned char *in, size_t inlen,  unsigned char *out); // vlc7 16 bits zigzag
LIBAPI size_t anscdfvzdec160(unsigned char *in, size_t outlen, unsigned char *out);
LIBAPI size_t anscdfvzenc16s(unsigned char *in, size_t inlen,  unsigned char *out);
LIBAPI size_t anscdfvzdec16s(unsigned char *in, size_t outlen, unsigned char *out);
LIBAPI size_t anscdfvzenc16x(unsigned char *in, size_t inlen,  unsigned char *out);
LIBAPI size_t anscdfvzdec16x(unsigned char *in, size_t outlen, unsigned char *out);

LIBAPI size_t anscdfvenc320( unsigned char *in, size_t inlen,  unsigned char *out); // vlc7 32 bits
LIBAPI size_t anscdfvdec320( unsigned char *in, size_t outlen, unsigned char *out);
LIBAPI size_t anscdfvenc32s( unsigned char *in, size_t inlen,  unsigned char *out);
LIBAPI size_t anscdfvdec32s( unsigned char *in, size_t outlen, unsigned char *out);
LIBAPI size_t anscdfvenc32x( unsigned char *in, size_t inlen,  unsigned char *out);
LIBAPI size_t anscdfvdec32x( unsigned char *in, size_t outlen, unsigned char *out);

LIBAPI size_t anscdfvzenc320(unsigned char *in, size_t inlen,  unsigned char *out); // vlc7 32 bits zigzag
LIBAPI size_t anscdfvzdec320(unsigned char *in, size_t outlen, unsigned char *out);
LIBAPI size_t anscdfvzenc32s(unsigned char *in, size_t inlen,  unsigned char *out);
LIBAPI size_t anscdfvzdec32s(unsigned char *in, size_t outlen, unsigned char *out);
LIBAPI size_t anscdfvzenc32x(unsigned char *in, size_t inlen,  unsigned char *out);
LIBAPI size_t anscdfvzdec32x(unsigned char *in, size_t outlen, unsigned char *out);

//extern LIBAPI size_t ansbc(unsigned char *in, size_t inlen, unsigned char *out, unsigned outsize);
//extern LIBAPI size_t ansbd(unsigned char *in, size_t inlen, unsigned char *out);
#ifdef __cplusplus
}
#endif
