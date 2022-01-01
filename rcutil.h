/**
    Copyright (C) powturbo 2013-2022
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

#ifdef __cplusplus
extern "C" {
#endif
size_t lzpenc(unsigned char *__restrict in, size_t inlen,  unsigned char *__restrict out, unsigned lenmin);
size_t lzpdec(unsigned char *__restrict in, size_t outlen, unsigned char *__restrict out, unsigned lenmin);

unsigned char *rcqlfc(unsigned char *__restrict in, size_t n, unsigned char *__restrict out, unsigned char *__restrict r2c);

unsigned utf8enc(unsigned char *__restrict in, size_t inlen, unsigned char *__restrict out, unsigned flag);
unsigned utf8dec(unsigned char *__restrict in, size_t outlen, unsigned char *__restrict out);
#ifdef __cplusplus
}
#endif
