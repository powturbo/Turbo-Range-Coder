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
#ifndef TURBORC_H_
#define TURBORC_H_
#include <stddef.h>
  #if defined(__GNUC__)
#define _PACKED         __attribute__ ((packed))
  #else
#define _PACKED
  #endif
//----------- BWT -------------------
#define BWT_RDONLY  (1<<30) // input is read only, no overwrite
#define BWT_BWT16   (1<<29) // 16 bits bwt
#define BWT_PREP8   (1<<28) // preprocessor output 8-16 bits 
#define BWT_LZP     (1<<27) // Force lzp
#define BWT_VERBOSE (1<<26) // verbose
#define BWT_COPY    (1<<25) // memcpy in to out in case of no compression
#define BWT_RATIO   (1<<24) // No ratio check
#define BWT_NUTF8   (1<<23) // No utf8-preprocessing

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
//-------------- s: simple predictor -----------------------------------------------------------------------------------
size_t rcsenc(      unsigned char *in, size_t inlen,  unsigned char *out ); // order 0
size_t rcsdec(      unsigned char *in, size_t outlen, unsigned char *out );

size_t rccsenc(     unsigned char *in, size_t inlen,  unsigned char *out ); // order 8bits (=order1)
size_t rccsdec(     unsigned char *in, size_t outlen, unsigned char *out );

size_t rcc2senc(    unsigned char *in, size_t inlen,  unsigned char *out ); // order 16bits (=order2)
size_t rcc2sdec(    unsigned char *in, size_t outlen, unsigned char *out );

size_t rcxsenc(     unsigned char *in, size_t inlen,  unsigned char *out ); // order 8bits (=order1) sliding context
size_t rcxsdec(     unsigned char *in, size_t outlen, unsigned char *out );

size_t rcx2senc(    unsigned char *in, size_t inlen,  unsigned char *out ); // order 8bits (=order1) sliding context
size_t rcx2sdec(    unsigned char *in, size_t outlen, unsigned char *out );

size_t rcsenc16(    unsigned char *in, size_t inlen,  unsigned char *out ); // 16 bits order 0
size_t rcsdec16(    unsigned char *in, size_t outlen, unsigned char *out );

size_t rcsenc32(    unsigned char *in, size_t inlen,  unsigned char *out ); // 32-bits order 0
size_t rcsdec32(    unsigned char *in, size_t outlen, unsigned char *out );

size_t rccsenc16(   unsigned char *in, size_t inlen,  unsigned char *out ); // 16-bits order 1
size_t rccsdec16(   unsigned char *in, size_t outlen, unsigned char *out );

size_t rccsenc32(   unsigned char *in, size_t inlen,  unsigned char *out ); // 32-bits order 1
size_t rccsdec32(   unsigned char *in, size_t outlen, unsigned char *out );

size_t rcc2senc32(  unsigned char *in, size_t inlen,  unsigned char *out ); // 32-bits order 2
size_t rcc2sdec32(  unsigned char *in, size_t outlen, unsigned char *out );

size_t rcmsenc(     unsigned char *in, size_t inlen,  unsigned char *out ); // context mixing: o1 mixer/sse
size_t rcmsdec(     unsigned char *in, size_t outlen, unsigned char *out );

size_t rcm2senc(    unsigned char *in, size_t inlen,  unsigned char *out ); // context mixing: o2 mixer/sse
size_t rcm2sdec(    unsigned char *in, size_t outlen, unsigned char *out );

size_t rcmrsenc(    unsigned char *in, size_t inlen,  unsigned char *out ); // context mixing / run aware
size_t rcmrsdec(    unsigned char *in, size_t outlen, unsigned char *out );
size_t rcmrrsenc(    unsigned char *in, size_t inlen,  unsigned char *out ); // context mixing / run > 2 aware 
size_t rcmrrsdec(    unsigned char *in, size_t outlen, unsigned char *out );

size_t rcrlesenc(   unsigned char *in, size_t inlen,  unsigned char *out ); // order 0 RLE (Run Length Encoding)
size_t rcrlesdec(   unsigned char *in, size_t outlen, unsigned char *out );

size_t rcrlesenc16( unsigned char *in, size_t inlen,  unsigned char *out ); // order 0 RLE 16 bits
size_t rcrlesdec16( unsigned char *in, size_t outlen, unsigned char *out );

size_t rcrle1senc(  unsigned char *in, size_t inlen,  unsigned char *out ); // order 1 RLE 
size_t rcrle1sdec(  unsigned char *in, size_t outlen, unsigned char *out );

size_t rcrle1senc16(unsigned char *in, size_t inlen,  unsigned char *out ); // order 1 RLE 16 bits
size_t rcrle1sdec16(unsigned char *in, size_t outlen, unsigned char *out );

size_t rcu3senc(    unsigned char *in, size_t inlen,  unsigned char *out ); // structured/segmented 3/5/8 varint
size_t rcu3sdec(    unsigned char *in, size_t outlen, unsigned char *out );

size_t rcqlfcsenc(  unsigned char *in, size_t inlen,  unsigned char *out ); // //--- QLFC :  Quantized Local Frequency Coding
size_t rcqlfcsdec(  unsigned char *in, size_t outlen, unsigned char *out );

size_t rc4senc(     unsigned char *in, size_t inlen,  unsigned char *out );// nibble adaptive
size_t rc4sdec(     unsigned char *in, size_t outlen, unsigned char *out );

size_t rc4csenc(    unsigned char *in, size_t inlen,  unsigned char *out ); // nibble static
size_t rc4csdec(    unsigned char *in, size_t outlen, unsigned char *out );

//--- Integer Compression: Gamma coding 
size_t rcgsenc8(    unsigned char *in, size_t inlen,  unsigned char *out ); // 8 bits
size_t rcgsdec8(    unsigned char *in, size_t outlen, unsigned char *out );
size_t rcgsenc16(   unsigned char *in, size_t inlen,  unsigned char *out ); // 16 bits 
size_t rcgsdec16(   unsigned char *in, size_t outlen, unsigned char *out );
size_t rcgsenc32(   unsigned char *in, size_t inlen,  unsigned char *out ); // 32 bits
size_t rcgsdec32(   unsigned char *in, size_t outlen, unsigned char *out );

size_t rcgzsenc8(    unsigned char *in, size_t inlen,  unsigned char *out ); // 8 bits  zigzag delta
size_t rcgzsdec8(    unsigned char *in, size_t outlen, unsigned char *out );
size_t rcgzsenc16(   unsigned char *in, size_t inlen,  unsigned char *out ); // 16 bits 
size_t rcgzsdec16(   unsigned char *in, size_t outlen, unsigned char *out );
size_t rcgzsenc32(   unsigned char *in, size_t inlen,  unsigned char *out ); // 32 bits
size_t rcgzsdec32(   unsigned char *in, size_t outlen, unsigned char *out );

//--- Integer Compression: Limited length Rice Coding
size_t rcrsenc8(    unsigned char *in, size_t inlen,  unsigned char *out ); // 8 bits
size_t rcrsdec8(    unsigned char *in, size_t outlen, unsigned char *out );
size_t rcrsenc16(   unsigned char *in, size_t inlen,  unsigned char *out ); // 16 bits 
size_t rcrsdec16(   unsigned char *in, size_t outlen, unsigned char *out );
size_t rcrsenc32(   unsigned char *in, size_t inlen,  unsigned char *out ); // 32 bits
size_t rcrsdec32(   unsigned char *in, size_t outlen, unsigned char *out );

size_t rcrzgsenc8(   unsigned char *in, size_t inlen,  unsigned char *out ); // 8 bits  zigzag delta
size_t rcrzsdec8(    unsigned char *in, size_t outlen, unsigned char *out );
size_t rcrzsenc16(   unsigned char *in, size_t inlen,  unsigned char *out ); // 16 bits 
size_t rcrzsdec16(   unsigned char *in, size_t outlen, unsigned char *out );
size_t rcrzsenc32(   unsigned char *in, size_t inlen,  unsigned char *out ); // 32 bits
size_t rcrzsdec32(   unsigned char *in, size_t outlen, unsigned char *out );

//--- Integer Compression: Variable length encoding for large integers
size_t rcvsenc16(   unsigned char *in, size_t inlen,  unsigned char *out ); // 16 bits, 8 bits exponent 
size_t rcvsdec16(   unsigned char *in, size_t outlen, unsigned char *out );
size_t rcvsenc32(   unsigned char *in, size_t inlen,  unsigned char *out ); // 32 bits
size_t rcvsdec32(   unsigned char *in, size_t outlen, unsigned char *out );

size_t rcosenc16(   unsigned char *in, size_t inlen,  unsigned char *out ); // 16 bits, 8 bits exponent 
size_t rcosdec16(   unsigned char *in, size_t outlen, unsigned char *out );
size_t rcosenc32(   unsigned char *in, size_t inlen,  unsigned char *out ); // 32 bits
size_t rcosdec32(   unsigned char *in, size_t outlen, unsigned char *out );

size_t rcvdsenc16(   unsigned char *in, size_t inlen,  unsigned char *out ); // 16 bits, 8 bits exponent + map
size_t rcvdsdec16(   unsigned char *in, size_t outlen, unsigned char *out );
size_t rcvedsenc32(  unsigned char *in, size_t inlen,  unsigned char *out ); // 32 bits
size_t rcwveddec32(  unsigned char *in, size_t outlen, unsigned char *out );

size_t rcvesenc32(  unsigned char *in, size_t inlen,  unsigned char *out ); // 32 bits, 12 bits exponent
size_t rcvesdec32(  unsigned char *in, size_t outlen, unsigned char *out );

size_t rcv10senc32(  unsigned char *in, size_t inlen,  unsigned char *out ); // 32 bits, 10 bits exponent
size_t rcv10sdec32(  unsigned char *in, size_t outlen, unsigned char *out );

size_t rcvzsenc16(  unsigned char *in, size_t inlen,  unsigned char *out ); // 16 bits, 8 bits exponent + zigzag delta 
size_t rcvzsdec16(  unsigned char *in, size_t outlen, unsigned char *out );
size_t rcvzsenc32(  unsigned char *in, size_t inlen,  unsigned char *out ); // 32 bits, 8 bits exponent + zigzag delta 
size_t rcvzsdec32(  unsigned char *in, size_t outlen, unsigned char *out );

size_t rcvezsenc32( unsigned char *in, size_t inlen,  unsigned char *out ); // 32 bits, 12 bits exponent
size_t rcvezsdec32( unsigned char *in, size_t outlen, unsigned char *out );

size_t rcvgsenc16(  unsigned char *in, size_t inlen,  unsigned char *out ); // 16 bits, 8 bits exponent w/ gamma coding 
size_t rcvgsdec16(  unsigned char *in, size_t outlen, unsigned char *out );
size_t rcvgsenc32(  unsigned char *in, size_t inlen,  unsigned char *out ); // 32 bits, 8 bits exponent w/ gamma coding 
size_t rcvgsdec32(  unsigned char *in, size_t outlen, unsigned char *out );

size_t rcvgzsenc16( unsigned char *in, size_t inlen,  unsigned char *out ); // 16 bits, 8 bits exponent w/ gamma coding + zigzag delta 
size_t rcvgzsdec16( unsigned char *in, size_t outlen, unsigned char *out );
size_t rcvgzsenc32( unsigned char *in, size_t inlen,  unsigned char *out ); // 32 bits, 8 bits exponent w/ gamma coding + zigzag delta 
size_t rcvgzsdec32( unsigned char *in, size_t outlen, unsigned char *out );

size_t rcv8senc16(   unsigned char *in, size_t inlen,  unsigned char *out ); // 16 bits
size_t rcv8sdec16(   unsigned char *in, size_t outlen, unsigned char *out );
size_t rcv8senc32(   unsigned char *in, size_t inlen,  unsigned char *out ); // 32 bits
size_t rcv8sdec32(   unsigned char *in, size_t outlen, unsigned char *out );

size_t rcv8zsenc16(  unsigned char *in, size_t inlen,  unsigned char *out ); // 16 bits
size_t rcv8zsdec16(  unsigned char *in, size_t outlen, unsigned char *out );
size_t rcv8zsenc32(  unsigned char *in, size_t inlen,  unsigned char *out ); // 32 bits
size_t rcv8zsdec32(  unsigned char *in, size_t outlen, unsigned char *out );

size_t rcv16senc32(   unsigned char *in, size_t inlen,  unsigned char *out ); // 32 bits
size_t rcv16sdec32(   unsigned char *in, size_t outlen, unsigned char *out );

//----------------ss: dual predictor ----------------------------------------------------------------------------
size_t rcssenc(      unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1 ); // order 0
size_t rcssdec(      unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1 );

size_t rccssenc(     unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1 ); // order 8bits (=order1)
size_t rccssdec(     unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1 ); 

size_t rcc2ssenc(    unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1 );  // order 16bits (=order2)
size_t rcc2ssdec(    unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1 ); 

size_t rcxssenc(     unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1 );  // order 8bits (=order1) sliding context
size_t rcxssdec(     unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1 ); 

size_t rcx2ssenc(    unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1 );  // order 8bits (=order1) sliding context
size_t rcx2ssdec(    unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1 ); 

size_t rcssenc16(    unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1 );  // 16 bits order 0
size_t rcssdec16(    unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1 ); 

size_t rcssenc32(    unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1 );  // 32-bits order 0
size_t rcssdec32(    unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1 ); 

size_t rccssenc16(   unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1 );  // 16-bits order 1
size_t rccssdec16(   unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1 ); 

size_t rccssenc32(   unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1 );  // 32-bits order 1
size_t rccssdec32(   unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1 ); 

size_t rcc2ssenc32(  unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1 );  // 32-bits order 2
size_t rcc2ssdec32(  unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1 ); 

size_t rcmssenc(     unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1  ); // context mixing: o1 mixer/sse
size_t rcmssdec(     unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1  );

size_t rcm2ssenc(    unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1  ); // context mixing: o2 mixer/sse
size_t rcm2ssdec(    unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1  );

size_t rcmrssenc(    unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1  ); // context mixing / run aware
size_t rcmrssdec(    unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1  );
size_t rcmrrssenc(   unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1  ); // context mixing / run > 2 aware
size_t rcmrrssdec(   unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1  );

size_t rcrlessenc(   unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1 );  // order 0 RLE (Run Length Encoding)
size_t rcrlessdec(   unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1 ); 

size_t rcrlessenc16( unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1 );  // order 0 RLE 16 bits
size_t rcrlessdec16( unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1 ); 

size_t rcrle1ssenc(  unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1 );  // order 1 RLE 
size_t rcrle1ssdec(  unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1 ); 

size_t rcrle1ssenc16(unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1 );  // order 1 RLE 16 bits
size_t rcrle1ssdec16(unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1 ); 

size_t rcu3ssenc(    unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1 );  // structured/segmented 3/5/8 varint
size_t rcu3ssdec(    unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1 ); 

size_t rcqlfcssenc(  unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1 );  // //--- QLFC :  Quantized Local Frequency Coding
size_t rcqlfcssdec(  unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1 ); 

size_t rc4ssenc(     unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1 ); // nibble adaptive
size_t rc4ssdec(     unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1 ); 

size_t rc4cssenc(    unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1 );  // nibble static
size_t rc4cssdec(    unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1 ); 

//--- Integer Compression: Gamma coding 
size_t rcgssenc8(    unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1 );  // 8 bits
size_t rcgssdec8(    unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1 ); 
size_t rcgssenc16(   unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1 );  // 16 bits 
size_t rcgssdec16(   unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1 ); 
size_t rcgssenc32(   unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1 );  // 32 bits
size_t rcgssdec32(   unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1 ); 

size_t rcgzssenc8(    unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1 );  // 8 bits  zigzag delta
size_t rcgzssdec8(    unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1 ); 
size_t rcgzssenc16(   unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1 );  // 16 bits 
size_t rcgzssdec16(   unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1 ); 
size_t rcgzssenc32(   unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1 );  // 32 bits
size_t rcgzssdec32(   unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1 ); 

//--- Integer Compression: Limited length Rice Coding
size_t rcrssenc8(    unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1 );  // 8 bits
size_t rcrssdec8(    unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1 ); 
size_t rcrssenc16(   unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1 );  // 16 bits 
size_t rcrssdec16(   unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1 ); 
size_t rcrssenc32(   unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1 );  // 32 bits
size_t rcrssdec32(   unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1 ); 

size_t rcrzgssenc8(   unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1 );  // 8 bits  zigzag delta
size_t rcrzssdec8(    unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1 ); 
size_t rcrzssenc16(   unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1 );  // 16 bits 
size_t rcrzssdec16(   unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1 ); 
size_t rcrzssenc32(   unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1 );  // 32 bits
size_t rcrzssdec32(   unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1 ); 

//--- Integer Compression: Variable length encoding for large integers
size_t rcvssenc16(   unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1 );  // 16 bits, 8 bits exponent 
size_t rcvssdec16(   unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1 ); 
size_t rcvssenc32(   unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1 );  // 32 bits
size_t rcvssdec32(   unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1 ); 

size_t rcossenc16(   unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1 );  // 16 bits, 8 bits exponent 
size_t rcossdec16(   unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1 ); 
size_t rcossenc32(   unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1 );  // 32 bits
size_t rcossdec32(   unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1 ); 

size_t rcvessenc32(  unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1 );  // 32 bits, 12 bits exponent
size_t rcvessdec32(  unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1 ); 

size_t rcvzssenc16(  unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1 );  // 16 bits, 8 bits exponent + zigzag delta 
size_t rcvzssdec16(  unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1 ); 
size_t rcvzssenc32(  unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1 );  // 32 bits, 8 bits exponent + zigzag delta 
size_t rcvzssdec32(  unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1 ); 

size_t rcvezssenc32( unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1 );  // 32 bits, 12 bits exponent
size_t rcvezssdec32( unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1 ); 

size_t rcvgssenc16(  unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1 );  // 16 bits, 8 bits exponent w/ gamma coding 
size_t rcvgssdec16(  unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1 ); 
size_t rcvgssenc32(  unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1 );  // 32 bits, 8 bits exponent w/ gamma coding 
size_t rcvgssdec32(  unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1 ); 

size_t rcvgzssenc16( unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1 );  // 16 bits, 8 bits exponent w/ gamma coding + zigzag delta 
size_t rcvgzssdec16( unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1 ); 
size_t rcvgzssenc32( unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1 );  // 32 bits, 8 bits exponent w/ gamma coding + zigzag delta 
size_t rcvgzssdec32( unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1 ); 

size_t rcv8ssenc16(   unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1 ); // 16 bits
size_t rcv8ssdec16(   unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1 );
size_t rcv8ssenc32(   unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1  ); // 32 bits
size_t rcv8ssdec32(   unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1  );

size_t rcv8zssenc16(  unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1 ); // 16 bits
size_t rcv8zssdec16(  unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1 );
size_t rcv8zssenc32(  unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1  ); // 32 bits
size_t rcv8zssdec32(  unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1  );

//size_t rcv16ssenc32(   unsigned char *in, size_t inlen,  unsigned char *out, unsigned prm0, unsigned prm1  ); // 32 bits
//size_t rcv16ssdec32(   unsigned char *in, size_t outlen, unsigned char *out, unsigned prm0, unsigned prm1  );

//----------------sf: fsm predictor ----------------------------------------------------------------------------
#pragma pack(1)
typedef struct fsm_t { unsigned short p,s[2]; } _PACKED fsm_t;
#pragma pack() 

size_t rcsfenc(      unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm);  // order 0
size_t rcsfdec(      unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm); 

size_t rccsfenc(     unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm);  // order 8bits (=order1)
size_t rccsfdec(     unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm); 

size_t rcc2sfenc(    unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm);  // order 16bits (=order2)
size_t rcc2sfdec(    unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm); 

size_t rcxsfenc(     unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm);  // order 8bits (=order1) sliding context
size_t rcxsfdec(     unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm); 

size_t rcx2sfenc(    unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm);  // order 8bits (=order1) sliding context
size_t rcx2sfdec(    unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm); 

size_t rcsfenc16(    unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm);  // 16 bits order 0
size_t rcsfdec16(    unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm); 

size_t rcsfenc32(    unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm);  // 32-bits order 0
size_t rcsfdec32(    unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm); 

size_t rccsfenc16(   unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm);  // 16-bits order 1
size_t rccsfdec16(   unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm); 

size_t rccsfenc32(   unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm);  // 32-bits order 1
size_t rccsfdec32(   unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm); 

size_t rcc2sfenc32(  unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm);  // 32-bits order 2
size_t rcc2sfdec32(  unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm); 

size_t rcmsfenc(     unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm);  // context mixing: o1 mixer/sse
size_t rcmsfdec(     unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm); 

size_t rcm2sfenc(    unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm);  // context mixing: o2 mixer/sse
size_t rcm2sfdec(    unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm); 

size_t rcmrsfenc(    unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm);  // context mixing / run aware
size_t rcmrsfdec(    unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm); 
size_t rcmrrsfenc(   unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm);  // context mixing / run > 2 aware
size_t rcmrrsfdec(   unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm); 

size_t rcrlesfenc(   unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm);  // order 0 RLE (Run Length Encoding)
size_t rcrlesfdec(   unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm); 

size_t rcrlesfenc16( unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm);  // order 0 RLE 16 bits
size_t rcrlesfdec16( unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm); 

size_t rcrle1sfenc(  unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm);  // order 1 RLE 
size_t rcrle1sfdec(  unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm); 

size_t rcrle1sfenc16(unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm);  // order 1 RLE 16 bits
size_t rcrle1sfdec16(unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm); 

size_t rcu3sfenc(    unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm);  // structured/segmented 3/5/8 varint
size_t rcu3sfdec(    unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm); 

size_t rcqlfcsfenc(  unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm);  // //--- QLFC :  Quantized Local Frequency Coding
size_t rcqlfcsfdec(  unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm); 

size_t rc4sfenc(     unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm); // nibble adaptive
size_t rc4sfdec(     unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm); 

size_t rc4csfenc(    unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm);  // nibble static
size_t rc4csfdec(    unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm); 

//--- Integer Compression: Gamma coding 
size_t rcgsfenc8(    unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm);  // 8 bits
size_t rcgsfdec8(    unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm); 
size_t rcgsfenc16(   unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm);  // 16 bits 
size_t rcgsfdec16(   unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm); 
size_t rcgsfenc32(   unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm);  // 32 bits
size_t rcgsfdec32(   unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm); 

size_t rcgzsfenc8(    unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm);  // 8 bits  zigzag delta
size_t rcgzsfdec8(    unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm); 
size_t rcgzsfenc16(   unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm);  // 16 bits 
size_t rcgzsfdec16(   unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm); 
size_t rcgzsfenc32(   unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm);  // 32 bits
size_t rcgzsfdec32(   unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm); 

//--- Integer Compression: Limited length Rice Coding
size_t rcrsfenc8(    unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm);  // 8 bits
size_t rcrsfdec8(    unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm); 
size_t rcrsfenc16(   unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm);  // 16 bits 
size_t rcrsfdec16(   unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm); 
size_t rcrsfenc32(   unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm);  // 32 bits
size_t rcrsfdec32(   unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm); 

size_t rcrzgsfenc8(   unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm);  // 8 bits  zigzag delta
size_t rcrzsfdec8(    unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm); 
size_t rcrzsfenc16(   unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm);  // 16 bits 
size_t rcrzsfdec16(   unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm); 
size_t rcrzsfenc32(   unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm);  // 32 bits
size_t rcrzsfdec32(   unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm); 

//--- Integer Compression: Variable length encoding for large integers
size_t rcvsfenc16(   unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm);  // 16 bits, 8 bits exponent 
size_t rcvsfdec16(   unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm); 
size_t rcvsfenc32(   unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm);  // 32 bits
size_t rcvsfdec32(   unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm); 

size_t rcosfenc16(   unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm);  // 16 bits, 8 bits exponent 
size_t rcosfdec16(   unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm); 
size_t rcosfenc32(   unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm);  // 32 bits
size_t rcosfdec32(   unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm); 

size_t rcvesfenc32(  unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm);  // 32 bits, 12 bits exponent
size_t rcvesfdec32(  unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm); 

size_t rcvzsfenc16(  unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm);  // 16 bits, 8 bits exponent + zigzag delta 
size_t rcvzsfdec16(  unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm); 
size_t rcvzsfenc32(  unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm);  // 32 bits, 8 bits exponent + zigzag delta 
size_t rcvzsfdec32(  unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm); 

size_t rcvezsfenc32( unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm);  // 32 bits, 12 bits exponent
size_t rcvezsfdec32( unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm); 

size_t rcvgsfenc16(  unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm);  // 16 bits, 8 bits exponent w/ gamma coding 
size_t rcvgsfdec16(  unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm); 
size_t rcvgsfenc32(  unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm);  // 32 bits, 8 bits exponent w/ gamma coding 
size_t rcvgsfdec32(  unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm); 

size_t rcvgzsfenc16( unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm);  // 16 bits, 8 bits exponent w/ gamma coding + zigzag delta 
size_t rcvgzsfdec16( unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm); 
size_t rcvgzsfenc32( unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm);  // 32 bits, 8 bits exponent w/ gamma coding + zigzag delta 
size_t rcvgzsfdec32( unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm); 

size_t rcv8sfenc16(  unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm ); // 16 bits, 8 bits exponent 
size_t rcv8sfdec16(  unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm );
size_t rcv8sfenc32(  unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm ); // 32 bits
size_t rcv8sfdec32(  unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm );

size_t rcv8zsfenc16( unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm ); // 16 bits, 8 bits exponent 
size_t rcv8zsfdec16( unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm );
size_t rcv8zsfenc32( unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm ); // 32 bits
size_t rcv8zsfdec32( unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm );

size_t rcv16sfenc32(   unsigned char *in, size_t inlen,  unsigned char *out, fsm_t *fsm);  // 32 bits
size_t rcv16sfdec32(   unsigned char *in, size_t outlen, unsigned char *out, fsm_t *fsm); 

//----------------- CDF: cumulative distribution functions --------------------------------------------------------
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

size_t rccdfdec8(   unsigned char *in, size_t outlen, unsigned char *out); //integer 0-299 
size_t rccdfenc8(   unsigned char *in, size_t inlen,  unsigned char *out); 
size_t rccdfidec8(  unsigned char *in, size_t outlen, unsigned char *out);
size_t rccdfienc8(  unsigned char *in, size_t inlen,  unsigned char *out); 

void trcini(void);
//------ predictor ids (used in turborc.c)
#define RC_PRD_S    1  // simple
#define RC_PRD_SS   2  // dual speed
#define RC_PRD_SF   3  // fsm
#define RC_PRD_LAST RC_PRD_SF
//----- bwt --------------------------
size_t rcbwtenc(   unsigned char *in, size_t inlen,  unsigned char *out, unsigned lev, unsigned thnum, unsigned lenmin); //---- bwt - compressor 
size_t rcbwtdec(   unsigned char *in, size_t outlen, unsigned char *out, unsigned lev, unsigned thnum );

#ifdef __cplusplus
}
#endif
#endif
