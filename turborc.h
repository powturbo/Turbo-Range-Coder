/**
    Copyright (C) powturbo 2013-2020
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

//----------------------------- RC: Range Coder -------------------------------------------------------------
// Encoding:
// in:    input to be encoded
// inlen: input length in bytes
// out  : output buffer must at least equal to inlen
// return value: 
//  < inlen : length of the encoded buffer 
//  = inlen : input buffer is incompressible. Input copied to output

// Decoding:
// in:     encoded input to be decoded
// outlen: original (decompressed) input length in bytes
// out   : outlen bytes are decoded to the output buffer
//         The caller must first check outlen == original/decoded input length.         

//-------------- s: simple predictor -----------------------------------------------------------------------------------
size_t rcsenc(      unsigned char *in, size_t inlen,  unsigned char *out ); // order 0
size_t rcsdec(      unsigned char *in, size_t outlen, unsigned char *out );

size_t rcxsenc(     unsigned char *in, size_t inlen,  unsigned char *out ); // order 8bits
size_t rcxsdec(     unsigned char *in, size_t outlen, unsigned char *out );

size_t rc4senc(     unsigned char *in, size_t inlen,  unsigned char *out );// nibble adaptive
size_t rc4sdec(     unsigned char *in, size_t outlen, unsigned char *out );

size_t rc4csenc(    unsigned char *in, size_t inlen,  unsigned char *out ); // nibble static
size_t rc4csdec(    unsigned char *in, size_t outlen, unsigned char *out );

size_t rcrlesenc(   unsigned char *in, size_t inlen,  unsigned char *out ); // order 0     RLE : Run Length Encoding
size_t rcrlesdec(   unsigned char *in, size_t outlen, unsigned char *out );

size_t rcrlexsenc(  unsigned char *in, size_t inlen,  unsigned char *out ); // order 8bits RLE : Run Length Encoding 
size_t rcrlexsdec(  unsigned char *in, size_t outlen, unsigned char *out );

size_t rcqlfcsenc(  unsigned char *in, size_t inlen,  unsigned char *out ); // //--- QLFC :  Quantized Local Frequency Coding
size_t rcqlfcsdec(  unsigned char *in, size_t outlen, unsigned char *out );

size_t rcbwtsenc(   unsigned char *in, size_t inlen,  unsigned char *out, unsigned lev, unsigned thnum, unsigned lenmin); // //---- bwt - compressor 
size_t rcbwtsdec(   unsigned char *in, size_t outlen, unsigned char *out, unsigned lev, unsigned thnum );

//--- Integer Compression: Gamma coding 
size_t rcgsenc8(    unsigned char *in, size_t inlen,  unsigned char *out ); // 8 bits
size_t rcgsdec8(    unsigned char *in, size_t outlen, unsigned char *out );
size_t rcgsenc16(   unsigned char *in, size_t inlen,  unsigned char *out ); // 16 bits 
size_t rcgsdec16(   unsigned char *in, size_t outlen, unsigned char *out );
size_t rcgsenc32(   unsigned char *in, size_t inlen,  unsigned char *out ); // 32 bits
size_t rcgsdec32(   unsigned char *in, size_t outlen, unsigned char *out );

//----------------ss: Strong predictor ----------------------------------------------------------------------------
size_t rcssenc(     unsigned char *in, size_t inlen,  unsigned char *out ); // order 0
size_t rcssdec(     unsigned char *in, size_t outlen, unsigned char *out );

size_t rcxssenc(    unsigned char *in, size_t inlen,  unsigned char *out ); // order 8bits
size_t rcxssdec(    unsigned char *in, size_t outlen, unsigned char *out );

size_t rc4ssenc(    unsigned char *in, size_t inlen,  unsigned char *out );// nibble adaptive
size_t rc4ssdec(    unsigned char *in, size_t outlen, unsigned char *out );

size_t rc4cssenc(   unsigned char *in, size_t inlen,  unsigned char *out ); // nibble static
size_t rc4cssdec(   unsigned char *in, size_t outlen, unsigned char *out );

size_t rcrlessenc(  unsigned char *in, size_t inlen,  unsigned char *out ); // order 0     RLE : Run Length Encoding
size_t rcrlessdec(  unsigned char *in, size_t outlen, unsigned char *out );

size_t rcrlexssenc( unsigned char *in, size_t inlen,  unsigned char *out ); // order 8bits RLE : Run Length Encoding 
size_t rcrlexssdec( unsigned char *in, size_t outlen, unsigned char *out );

size_t rcqlfcssenc( unsigned char *in, size_t inlen,  unsigned char *out ); // //--- QLFC :  Quantized Local Frequency Coding
size_t rcqlfcssdec( unsigned char *in, size_t outlen, unsigned char *out );

size_t rcbwtssenc(  unsigned char *in, size_t inlen,  unsigned char *out, unsigned lev, unsigned thnum, unsigned lenmin); // //---- bwt - compressor 
size_t rcbwtssdec(  unsigned char *in, size_t outlen, unsigned char *out, unsigned lev, unsigned thnum );

//--- Integer Compression: Gamma coding 
size_t rcgssenc8(   unsigned char *in, size_t inlen,  unsigned char *out ); // 8 bits
size_t rcgssdec8(   unsigned char *in, size_t outlen, unsigned char *out );
size_t rcgssenc16(  unsigned char *in, size_t inlen,  unsigned char *out ); // 16 bits 
size_t rcgssdec16(  unsigned char *in, size_t outlen, unsigned char *out );
size_t rcgssenc32(  unsigned char *in, size_t inlen,  unsigned char *out ); // 32 bits
size_t rcgssdec32(  unsigned char *in, size_t outlen, unsigned char *out );

//----------------nz: Nanozip predictor ---------------------------------------------------------------------------
size_t rcnzenc(     unsigned char *in, size_t inlen,  unsigned char *out ); // order 0
size_t rcnzdec(     unsigned char *in, size_t outlen, unsigned char *out );

size_t rcxnzenc(    unsigned char *in, size_t inlen,  unsigned char *out ); // order 8bits
size_t rcxnzdec(    unsigned char *in, size_t outlen, unsigned char *out );

size_t rcrlenzenc(  unsigned char *in, size_t inlen,  unsigned char *out ); // order 0     RLE : Run Length Encoding
size_t rcrlenzdec(  unsigned char *in, size_t outlen, unsigned char *out );

size_t rcrlexnzenc( unsigned char *in, size_t inlen,  unsigned char *out ); // order 8bits RLE : Run Length Encoding 
size_t rcrlexnzdec( unsigned char *in, size_t outlen, unsigned char *out );

size_t rcqlfcnzenc( unsigned char *in, size_t inlen,  unsigned char *out ); // //--- QLFC :  Quantized Local Frequency Coding
size_t rcqlfcnzdec( unsigned char *in, size_t outlen, unsigned char *out );

//----------------sh: Eugene Shelwien predictor ------------------------------------------------------------------
size_t rcshenc(     unsigned char *in, size_t inlen,  unsigned char *out ); // order 0
size_t rcshdec(     unsigned char *in, size_t outlen, unsigned char *out );

size_t rcxshenc(    unsigned char *in, size_t inlen,  unsigned char *out ); // order 8bits
size_t rcxshdec(    unsigned char *in, size_t outlen, unsigned char *out );

size_t rcrleshenc(  unsigned char *in, size_t inlen,  unsigned char *out ); // order 0     RLE : Run Length Encoding
size_t rcrleshdec(  unsigned char *in, size_t outlen, unsigned char *out );

size_t rcrlexshenc( unsigned char *in, size_t inlen,  unsigned char *out ); // order 8bits RLE : Run Length Encoding 
size_t rcrlexshdec( unsigned char *in, size_t outlen, unsigned char *out );

size_t rcqlfcshenc( unsigned char *in, size_t inlen,  unsigned char *out ); // //--- QLFC :  Quantized Local Frequency Coding
size_t rcqlfcshdec( unsigned char *in, size_t outlen, unsigned char *out );

//----------------- cdf functions --------------------------------------------------------
typedef unsigned short cdf_t;

//--- cdf static functions --------------
int cdfini(unsigned char *in, size_t inlen, cdf_t *cdf, unsigned cdfnum); //example init static 

size_t rccdfsenc(   unsigned char *in, size_t inlen,  unsigned char *out, cdf_t *cdf, unsigned cdfnum); //Encode using cdf 
size_t rccdfsbdec(  unsigned char *in, size_t outlen, unsigned char *out, cdf_t *cdf, unsigned cdfnum); //Decode using cdf / binary search
size_t rccdfsldec(  unsigned char *in, size_t outlen, unsigned char *out, cdf_t *cdf, unsigned cdfnum); //Decode using cdf / linear search
size_t rccdfsvbdec( unsigned char *in, size_t outlen, unsigned char *out, cdf_t *cdf, unsigned cdfnum); //Decode using cdf 
size_t rccdfsvldec( unsigned char *in, size_t outlen, unsigned char *out, cdf_t *cdf, unsigned cdfnum);

size_t rccdfs2enc(  unsigned char *in, size_t inlen,  unsigned char *out, cdf_t *cdf, unsigned cdfnum); // interleaved 2 
size_t rccdfsl2dec( unsigned char *in, size_t outlen, unsigned char *out, cdf_t *cdf, unsigned cdfnum); 
size_t rccdfsb2dec( unsigned char *in, size_t outlen, unsigned char *out, cdf_t *cdf, unsigned cdfnum);

//--- cdf adaptive functions ------------
size_t rccdfenc(    unsigned char *in, size_t inlen,  unsigned char *out); // byte
size_t rccdfdec(    unsigned char *in, size_t outlen, unsigned char *out); 
size_t rccdfidec(   unsigned char *in, size_t outlen, unsigned char *out); // byte interleaved
size_t rccdfienc(   unsigned char *in, size_t inlen,  unsigned char *out);

size_t rccdf4enc(   unsigned char *in, size_t inlen,  unsigned char *out); //Nibble
size_t rccdf4dec(   unsigned char *in, size_t outlen, unsigned char *out); 

size_t rccdf4ienc(  unsigned char *in, size_t inlen,  unsigned char *out); //Nibble interleaved
size_t rccdf4idec(  unsigned char *in, size_t outlen, unsigned char *out); 

//----------------- misc. function --------------------------------------------------------
size_t lzpenc(unsigned char *in, size_t inlen,  unsigned char *out, unsigned lenmin);
size_t lzpdec(unsigned char *in, size_t outlen, unsigned char *out, unsigned lenmin);

unsigned char *rcqlfc(unsigned char *in, unsigned char *out, size_t n, unsigned char *b);

void trcini(void);

#ifdef __cplusplus
}
#endif

