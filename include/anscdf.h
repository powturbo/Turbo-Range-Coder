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

typedef LIBAPI unsigned *(*fanscdfenc)(unsigned char *in, unsigned inlen, unsigned char *out, unsigned blksize);
typedef LIBAPI unsigned *(*fanscdfdec)(unsigned char *in, unsigned inlen, unsigned char *out, unsigned blksize);

extern fanscdfenc _anscdfenc;
extern fanscdfdec _anscdfdec;
extern fanscdfenc _anscdf4enc;
extern fanscdfdec _anscdf4dec;

#ifdef __cplusplus
extern "C" {
#endif
void anscdfini(unsigned id);
LIBAPI unsigned anscdfenc(  unsigned char *in, unsigned inlen,  unsigned char *out, unsigned blksize);
LIBAPI unsigned anscdfdec(  unsigned char *in, unsigned outlen, unsigned char *out, unsigned blksize);
LIBAPI unsigned anscdf4enc( unsigned char *in, unsigned inlen,  unsigned char *out, unsigned blksize);
LIBAPI unsigned anscdf4dec( unsigned char *in, unsigned outlen, unsigned char *out, unsigned blksize);

//-------------- direct function calls --------------------------------------------------------
LIBAPI unsigned anscdfenc0( unsigned char *in, unsigned inlen,  unsigned char *out, unsigned blksize);
LIBAPI unsigned anscdfencs( unsigned char *in, unsigned inlen,  unsigned char *out, unsigned blksize);
LIBAPI unsigned anscdfencx( unsigned char *in, unsigned inlen,  unsigned char *out, unsigned blksize);
LIBAPI unsigned anscdfdec0( unsigned char *in, unsigned outlen, unsigned char *out, unsigned blksize);
LIBAPI unsigned anscdfdecs( unsigned char *in, unsigned outlen, unsigned char *out, unsigned blksize);
LIBAPI unsigned anscdfdecx( unsigned char *in, unsigned outlen, unsigned char *out, unsigned blksize);

LIBAPI unsigned anscdf4enc0(unsigned char *in, unsigned inlen,  unsigned char *out, unsigned blksize);
LIBAPI unsigned anscdf4encs(unsigned char *in, unsigned inlen,  unsigned char *out, unsigned blksize);
LIBAPI unsigned anscdf4encx(unsigned char *in, unsigned inlen,  unsigned char *out, unsigned blksize);
LIBAPI unsigned anscdf4dec0(unsigned char *in, unsigned outlen, unsigned char *out, unsigned blksize);
LIBAPI unsigned anscdf4decs(unsigned char *in, unsigned outlen, unsigned char *out, unsigned blksize);
LIBAPI unsigned anscdf4decx(unsigned char *in, unsigned outlen, unsigned char *out, unsigned blksize);
//extern LIBAPI unsigned ansbc(unsigned char *in, unsigned inlen, unsigned char *out, unsigned outsize, unsigned blksize);
//extern LIBAPI unsigned ansbd(unsigned char *in, unsigned inlen, unsigned char *out, unsigned blksize);
#ifdef __cplusplus
}
#endif
