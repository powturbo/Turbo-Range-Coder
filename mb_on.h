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
// TurboRC: Range Coder - Sliding context order N context bits bitwise range coder 
// Unlike other entropy coders, you can specify the context in bits (4 - 24 bits)
// and not only in bytes.

// Predictor declaration. _cxbits_ must be >= 4 
#define MBC_DEF(  _m_, _cxbits_) mbu       _m_[1<<(_cxbits_+1)][1<<(4+2)]
// Predictor init
#define MBC_INIT( _m_, _cxbits_) mbu_init2(_m_,1<<(_cxbits_+1), 1<<(4+2))
#define MBC_DEC(  _m_, _cxbits_) MBC_DEF(  _m_, _cxbits_); MBC_INIT( _m_, _cxbits_)

#define LITSX8(_cx_, _cxbits_) (((_cx_) >> _cxbits_) << 4) |
#define MBC_(_cx_, _cxbits_) (_cxbits_==8?(unsigned char)(_cx_):((_cx_) & ((1<<_cxbits_)-1)))

// Encode char 'x' with context 'cx' of size 'cxbits' bits
#define mbcenc(rcrange,rclow, _cx_, _cxbits_, _m_,_prm0_,_prm1_,_op_, _x_) {\
  unsigned _y = _x_;                                                /*encode high nibble*/\
  mbu *_mh = _m_[              MBC_(_cx_, _cxbits_)]; unsigned _cxh = LITSX8(_cx_, _cxbits_) ((_y) >> 4);  _cx_ = _cx_ << 4 | (_y)>>4;\
  { mbu *_m = &_mh[(_cxh >> 2) & 0x3c | 3]; mbu_enc(rcrange,rclow, _m,_prm0_,_prm1_,_op_, _cxh & (1<<3)); }\
  { mbu *_m = &_mh[(_cxh >> 1) & 0x3c | 2]; mbu_enc(rcrange,rclow, _m,_prm0_,_prm1_,_op_, _cxh & (1<<2)); }\
  { mbu *_m = &_mh[(_cxh     ) & 0x3c | 1]; mbu_enc(rcrange,rclow, _m,_prm0_,_prm1_,_op_, _cxh & (1<<1)); }\
  { mbu *_m = &_mh[(_cxh << 1) & 0x3c    ]; mbu_enc(rcrange,rclow, _m,_prm0_,_prm1_,_op_, _cxh & (1<<0)); }\
                                                                    /*slide context then encode low nibble*/\
  mbu *_ml = _m_[(1<<_cxbits_)+MBC_(_cx_, _cxbits_)]; unsigned _cxl = LITSX8(_cx_, _cxbits_) ((_y) & 0xf); _cx_ = _cx_ << 4 | (_y) & 0xf;\
  { mbu *_m = &_ml[(_cxl >> 2) & 0x3c | 3]; mbu_enc(rcrange,rclow, _m,_prm0_,_prm1_,_op_, _cxl & (1<<3)); }\
  { mbu *_m = &_ml[(_cxl >> 1) & 0x3c | 2]; mbu_enc(rcrange,rclow, _m,_prm0_,_prm1_,_op_, _cxl & (1<<2)); }\
  { mbu *_m = &_ml[(_cxl     ) & 0x3c | 1]; mbu_enc(rcrange,rclow, _m,_prm0_,_prm1_,_op_, _cxl & (1<<1)); }\
  { mbu *_m = &_ml[(_cxl << 1) & 0x3c    ]; mbu_enc(rcrange,rclow, _m,_prm0_,_prm1_,_op_, _cxl & (1<<0)); }\
}

// Decode char with context 'cx' of size 'cxbits' bits. Decoded char = (unsigned char)cx  
#define mbcdec(rcrange,rccode, _cx_, _cxbits_, _m_,_prm0_,_prm1_,_ip_) { \
  mbu      *_mh = &_m_[MBC_(_cx_, _cxbits_)],*_m;\
  unsigned _y = (_cx_) >> _cxbits_;\
  _m = &_mh[(_y & 0xf) << 2 | 3]; mbu_dec(rcrange,rccode, _m,_prm0_,_prm1_,_ip_, _y); /* high nibble*/\
  _m = &_mh[(_y & 0xf) << 2 | 2]; mbu_dec(rcrange,rccode, _m,_prm0_,_prm1_,_ip_, _y);\
  _m = &_mh[(_y & 0xf) << 2 | 1]; mbu_dec(rcrange,rccode, _m,_prm0_,_prm1_,_ip_, _y);\
  _m = &_mh[(_y & 0xf) << 2    ]; mbu_dec(rcrange,rccode, _m,_prm0_,_prm1_,_ip_, _y); _cx_ = _cx_ << 4 | _y & 0xf; _mh = &_m_[1<<_cxbits_ | MBC_(_cx_, _cxbits_)]; _y = (_cx_) >> _cxbits_;\
  _m = &_mh[(_y & 0xf) << 2 | 3]; mbu_dec(rcrange,rccode, _m,_prm0_,_prm1_,_ip_, _y); /* low nibble*/\
  _m = &_mh[(_y & 0xf) << 2 | 2]; mbu_dec(rcrange,rccode, _m,_prm0_,_prm1_,_ip_, _y);\
  _m = &_mh[(_y & 0xf) << 2 | 1]; mbu_dec(rcrange,rccode, _m,_prm0_,_prm1_,_ip_, _y);\
  _m = &_mh[(_y & 0xf) << 2    ]; mbu_dec(rcrange,rccode, _m,_prm0_,_prm1_,_ip_, _y); _cx_ = _cx_ << 4 | _y & 0xf;\
}
