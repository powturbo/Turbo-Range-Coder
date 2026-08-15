/**
    Copyright (C) powturbo 2013-2026
    GPL v2 License

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
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
//--  tp.h - Byte/Nibble transpose for further compressing with lz77 or other compressors -------------------------------------

#ifdef __cplusplus
extern "C" {
#endif
// Syntax
// in    : Input buffer
// n     : Total number of bytes in input buffer
// out   : output buffer
// esize : element size in bytes (ex. 2, 4, 8, 16, ... )

//---------- High level functions with dynamic cpu detection and JIT scalar/sse/avx2 switching  
void tpenc(        unsigned char *__restrict in, unsigned n, unsigned char *__restrict out, unsigned esize); // byte tranpose
void tpzenc(       unsigned char *__restrict in, unsigned n, unsigned char *__restrict out, unsigned esize); // zigzag 
void tpxenc(       unsigned char *__restrict in, unsigned n, unsigned char *__restrict out, unsigned esize); // xor

void tp4enc(       unsigned char *__restrict in, unsigned n, unsigned char *__restrict out, unsigned esize); // Nibble transpose
void tp4zenc(      unsigned char *__restrict in, unsigned n, unsigned char *__restrict out, unsigned esize); 
void tp4xenc(      unsigned char *__restrict in, unsigned n, unsigned char *__restrict out, unsigned esize);

void tp2denc(      unsigned char *__restrict in,              unsigned nx, unsigned ny,              unsigned char *__restrict out, unsigned esize); //2D transpose
void tp3denc(      unsigned char *__restrict in,              unsigned nx, unsigned ny, unsigned nz, unsigned char *__restrict out, unsigned esize); //3D transpose
void tp4denc(      unsigned char *__restrict in, unsigned nw, unsigned nx, unsigned ny, unsigned nz, unsigned char *__restrict out, unsigned esize); //4D transpose

// --- reverse transpose ---------
void tpdec(        unsigned char *__restrict in, unsigned n, unsigned char *__restrict out, unsigned esize);
void tpzdec(       unsigned char *__restrict in, unsigned n, unsigned char *__restrict out, unsigned esize);
void tpxdec(       unsigned char *__restrict in, unsigned n, unsigned char *__restrict out, unsigned esize);

void tp4dec(       unsigned char *__restrict in, unsigned n, unsigned char *__restrict out, unsigned esize);
void tp4zdec(      unsigned char *__restrict in, unsigned n, unsigned char *__restrict out, unsigned esize);
void tp4xdec(      unsigned char *__restrict in, unsigned n, unsigned char *__restrict out, unsigned esize);

void tp2ddec(      unsigned char *__restrict in,              unsigned nx, unsigned ny,              unsigned char *__restrict out, unsigned esize); 
void tp3ddec(      unsigned char *__restrict in,              unsigned nx, unsigned ny, unsigned nz, unsigned char *__restrict out, unsigned esize);
void tp4ddec(      unsigned char *__restrict in, unsigned nw, unsigned nx, unsigned ny, unsigned nz, unsigned char *__restrict out, unsigned esize);

//---------- Low level functions --------------------------------------------------------------------------------------------
void tpenc2(       unsigned char *__restrict in, unsigned n, unsigned char *__restrict out); 
void tpenc128v2(   unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpenc256v2(   unsigned char *__restrict in, unsigned n, unsigned char *__restrict out); 
void tpenc3(       unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpenc4(       unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpenc128v4(   unsigned char *__restrict in, unsigned n, unsigned char *__restrict out); 
void tpzenc256v4(  unsigned char *__restrict in, unsigned n, unsigned char *__restrict out); 
void tpenc128v6(   unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpenc256v6(   unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpenc8(       unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpenc128v8(   unsigned char *__restrict in, unsigned n, unsigned char *__restrict out); 
void tpenc256v8(   unsigned char *__restrict in, unsigned n, unsigned char *__restrict out); 
void tpenc128v12(  unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpenc256v12(  unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpenc16(      unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpenc128v16(  unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpenc256v16(  unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);

void tpzenc2(      unsigned char *__restrict in, unsigned n, unsigned char *__restrict out); // zigzag
void tpzenc128v2(  unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpzenc256v2(  unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpzenc3(      unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpzenc4(      unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpzenc128v4(  unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpenc256v4(   unsigned char *__restrict in, unsigned n, unsigned char *__restrict out); 
void tpzenc8(      unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpzenc128v8(  unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpzenc256v8(  unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpzenc16(     unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);

void tpxenc2(      unsigned char *__restrict in, unsigned n, unsigned char *__restrict out); // xor
void tpxenc128v2(  unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpxenc256v2(  unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpxenc3(      unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpxenc4(      unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpxenc128v4(  unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpxenc256v4(  unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpxenc128v8(  unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpxenc8(      unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpxenc256v8(  unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpxenc16(     unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);

void tp2denc2(     unsigned char *__restrict in,              unsigned nx, unsigned ny,              unsigned char *__restrict out);  //2D
void tp2denc4(     unsigned char *__restrict in,              unsigned nx, unsigned ny,              unsigned char *__restrict out);
void tp2denc8(     unsigned char *__restrict in,              unsigned nx, unsigned ny,              unsigned char *__restrict out);  

void tp3denc2(     unsigned char *__restrict in,              unsigned nx, unsigned ny, unsigned nz, unsigned char *__restrict out); //3D
void tp3denc4(     unsigned char *__restrict in,              unsigned nx, unsigned ny, unsigned nz, unsigned char *__restrict out); 
void tp3denc8(     unsigned char *__restrict in,              unsigned nx, unsigned ny, unsigned nz, unsigned char *__restrict out);

void tp4denc2(     unsigned char *__restrict in, unsigned nw, unsigned nx, unsigned ny, unsigned nz, unsigned char *__restrict out); //4D
void tp4denc4(     unsigned char *__restrict in, unsigned nw, unsigned nx, unsigned ny, unsigned nz, unsigned char *__restrict out);
void tp4denc8(     unsigned char *__restrict in, unsigned nw, unsigned nx, unsigned ny, unsigned nz, unsigned char *__restrict out);

// --- reverse transpose -------------
void tpdec2(       unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpdec128v2(   unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpdec256v2(   unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpdec3(       unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpdec4(       unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpdec128v4(   unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpdec256v4(   unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpdec128v6(   unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpdec256v6(   unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpdec8(       unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpdec128v8(   unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpdec256v8(   unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpdec256v12(  unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpdec128v12(  unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpdec16(      unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpdec256v16(  unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpdec128v16(  unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);

void tpzdec2(      unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpzdec128v2(  unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpzdec256v2(  unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpzdec3(      unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpzdec4(      unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpzdec128v4(  unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpzdec256v4(  unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpzdec8(      unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpzdec128v8(  unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpzdec256v8(  unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpzdec16(     unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);

void tpxdec2(      unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpxdec128v2(  unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpxdec256v2(  unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpxdec3(      unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpxdec4(      unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpxdec128v4(  unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpxdec256v4(  unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpxdec8(      unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpxdec128v8(  unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpxdec256v8(  unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tpxdec16(     unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);

// --- Nibble transpose ---------------------------------------------
void tp4enc128v2(  unsigned char *__restrict in, unsigned n, unsigned char *__restrict out); 
void tp4enc256v2(  unsigned char *__restrict in, unsigned n, unsigned char *__restrict out); 
void tp4enc128v4(  unsigned char *__restrict in, unsigned n, unsigned char *__restrict out); 
void tp4enc256v4(  unsigned char *__restrict in, unsigned n, unsigned char *__restrict out); 
void tp4enc128v8(  unsigned char *__restrict in, unsigned n, unsigned char *__restrict out); 
void tp4enc256v8(  unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);

void tp4zenc128v2( unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tp4zenc256v2( unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tp4zenc128v4( unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tp4zenc256v4( unsigned char *__restrict in, unsigned n, unsigned char *__restrict out); 
void tp4zenc128v8( unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tp4zenc256v8( unsigned char *__restrict in, unsigned n, unsigned char *__restrict out); 

void tp4xenc128v2( unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tp4xenc256v2( unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tp4xenc128v4( unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tp4xenc256v4( unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tp4xenc128v8( unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tp4xenc256v8( unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);

void tp4dec128v2(  unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tp4dec256v2(  unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tp4dec128v4(  unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tp4dec256v4(  unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tp4dec128v8(  unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tp4dec256v8(  unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);

void tp4zdec128v2( unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tp4zdec256v2( unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tp4zdec128v4( unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tp4zdec256v4( unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tp4zdec128v8( unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tp4zdec256v8( unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);

void tp4xdec128v2( unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tp4xdec256v2( unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tp4xdec128v4( unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tp4xdec256v4( unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tp4xdec128v8( unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);
void tp4xdec256v8( unsigned char *__restrict in, unsigned n, unsigned char *__restrict out);

void tp2ddec2(     unsigned char *__restrict in,              unsigned nx, unsigned ny,              unsigned char *__restrict out); //2D
void tp2ddec4(     unsigned char *__restrict in,              unsigned nx, unsigned ny,              unsigned char *__restrict out);
void tp2ddec8(     unsigned char *__restrict in,              unsigned nx, unsigned ny,              unsigned char *__restrict out);

void tp3ddec2(     unsigned char *__restrict in,              unsigned nx, unsigned ny, unsigned nz, unsigned char *__restrict out); //3D
void tp3ddec4(     unsigned char *__restrict in,              unsigned nx, unsigned ny, unsigned nz, unsigned char *__restrict out);
void tp3ddec8(     unsigned char *__restrict in,              unsigned nx, unsigned ny, unsigned nz, unsigned char *__restrict out);

void tp4ddec2(     unsigned char *__restrict in, unsigned nw, unsigned nx, unsigned ny, unsigned nz, unsigned char *__restrict out); //4D
void tp4ddec4(     unsigned char *__restrict in, unsigned nw, unsigned nx, unsigned ny, unsigned nz, unsigned char *__restrict out);
void tp4ddec8(     unsigned char *__restrict in, unsigned nw, unsigned nx, unsigned ny, unsigned nz, unsigned char *__restrict out);

#ifdef __cplusplus
}
#endif

