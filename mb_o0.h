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
// TurboRC: Range Coder 

/* bitwise adaptive range coder, can be used as order 0..N */
  #if (RC_SIZE - RC_IO)/2 >= RC_BITS
#define _RCENORM1(_op_) 
#define _RCDNORM1(_ip_) 
  #else
#define _RCENORM1(_op_) _rcenorm_(rcrange,rclow, _op_)
#define _RCDNORM1(_ip_) _rcdnorm_(rcrange,rccode,_ip_)
  #endif 

  #if (RC_SIZE - RC_IO)/4 >= RC_BITS
#define _RCENORM2(_op_) 
#define _RCDNORM2(_ip_) 
  #else
#define _RCENORM2(_op_) _rcenorm_(rcrange,rclow, _op_)
#define _RCDNORM2(_ip_) _rcdnorm_(rcrange,rccode,_ip_)
  #endif

// 8 bits
#define mb8enc(rcrange,rclow, _m_,_prm0_,_prm1_,_op_, _x_) do {\
  unsigned _x = 1<<8 | _x_;\
  { _rcenorm_(rcrange,rclow,_op_); unsigned _mp = mbu_p(&_m_[    1]); mbu_e(rcrange,rclow, _mp, &_m_[    1], _prm0_,_prm1_,_op_, _x & (1<<7));}\
  { _RCENORM1(_op_);               unsigned _mp = mbu_p(&_m_[_x>>7]); mbu_e(rcrange,rclow, _mp, &_m_[_x>>7], _prm0_,_prm1_,_op_, _x & (1<<6));}\
  { _RCENORM2(_op_);               unsigned _mp = mbu_p(&_m_[_x>>6]); mbu_e(rcrange,rclow, _mp, &_m_[_x>>6], _prm0_,_prm1_,_op_, _x & (1<<5));}\
  { _RCENORM1(_op_);               unsigned _mp = mbu_p(&_m_[_x>>5]); mbu_e(rcrange,rclow, _mp, &_m_[_x>>5], _prm0_,_prm1_,_op_, _x & (1<<4));}\
  { _rcenorm_(rcrange,rclow,_op_); unsigned _mp = mbu_p(&_m_[_x>>4]); mbu_e(rcrange,rclow, _mp, &_m_[_x>>4], _prm0_,_prm1_,_op_, _x & (1<<3));}\
  { _RCENORM1(_op_);               unsigned _mp = mbu_p(&_m_[_x>>3]); mbu_e(rcrange,rclow, _mp, &_m_[_x>>3], _prm0_,_prm1_,_op_, _x & (1<<2));}\
  { _RCENORM2(_op_);               unsigned _mp = mbu_p(&_m_[_x>>2]); mbu_e(rcrange,rclow, _mp, &_m_[_x>>2], _prm0_,_prm1_,_op_, _x & (1<<1));}\
  { _RCENORM1(_op_);               unsigned _mp = mbu_p(&_m_[_x>>1]); mbu_e(rcrange,rclow, _mp, &_m_[_x>>1], _prm0_,_prm1_,_op_, _x & (1<<0));}\
} while(0)

#define mb8dec(rcrange,rccode, _m_,_prm0_,_prm1_,_ip_, _x_) { \
  _x_ = 1;                                                                             _rcdnorm_(rcrange,rccode,_ip_);\
  { mbu_d(rcrange,rccode, mbu_p(&_m_[  1]), &_m_[  1], _prm0_,_prm1_, _x_); _RCDNORM1(_ip_); }\
  { mbu_d(rcrange,rccode, mbu_p(&_m_[_x_]), &_m_[_x_], _prm0_,_prm1_, _x_); _RCDNORM2(_ip_); }\
  { mbu_d(rcrange,rccode, mbu_p(&_m_[_x_]), &_m_[_x_], _prm0_,_prm1_, _x_); _RCDNORM1(_ip_); }\
  { mbu_d(rcrange,rccode, mbu_p(&_m_[_x_]), &_m_[_x_], _prm0_,_prm1_, _x_); _rcdnorm_(rcrange,rccode,_ip_); }\
  { mbu_d(rcrange,rccode, mbu_p(&_m_[_x_]), &_m_[_x_], _prm0_,_prm1_, _x_); _RCDNORM1(_ip_); }\
  { mbu_d(rcrange,rccode, mbu_p(&_m_[_x_]), &_m_[_x_], _prm0_,_prm1_, _x_); _RCDNORM2(_ip_); }\
  { mbu_d(rcrange,rccode, mbu_p(&_m_[_x_]), &_m_[_x_], _prm0_,_prm1_, _x_); _RCDNORM1(_ip_); }\
  { mbu_d(rcrange,rccode, mbu_p(&_m_[_x_]), &_m_[_x_], _prm0_,_prm1_, _x_); }\
}

// 7 bits
#define mb7enc(rcrange,rclow, _m_,_prm0_,_prm1_,_op_, _x_) do {\
  unsigned _x = 1<<7 | _x_;\
  { _rcenorm_(rcrange,rclow,_op_); unsigned _mp = mbu_p(&_m_[    1]); mbu_e(rcrange,rclow, _mp, &_m_[    1], _prm0_,_prm1_,_op_, _x & (1<<6));}\
  { _RCENORM1(_op_);               unsigned _mp = mbu_p(&_m_[_x>>6]); mbu_e(rcrange,rclow, _mp, &_m_[_x>>6], _prm0_,_prm1_,_op_, _x & (1<<5));}\
  { _RCENORM2(_op_);               unsigned _mp = mbu_p(&_m_[_x>>5]); mbu_e(rcrange,rclow, _mp, &_m_[_x>>5], _prm0_,_prm1_,_op_, _x & (1<<4));}\
  { _RCENORM1(_op_);               unsigned _mp = mbu_p(&_m_[_x>>4]); mbu_e(rcrange,rclow, _mp, &_m_[_x>>4], _prm0_,_prm1_,_op_, _x & (1<<3));}\
  { _rcenorm_(rcrange,rclow,_op_); unsigned _mp = mbu_p(&_m_[_x>>3]); mbu_e(rcrange,rclow, _mp, &_m_[_x>>3], _prm0_,_prm1_,_op_, _x & (1<<2));}\
  { _RCENORM1(_op_);               unsigned _mp = mbu_p(&_m_[_x>>2]); mbu_e(rcrange,rclow, _mp, &_m_[_x>>2], _prm0_,_prm1_,_op_, _x & (1<<1));}\
  { _RCENORM2(_op_);               unsigned _mp = mbu_p(&_m_[_x>>1]); mbu_e(rcrange,rclow, _mp, &_m_[_x>>1], _prm0_,_prm1_,_op_, _x & (1<<0));}\
} while(0)

#define mb7dec(rcrange,rccode, _m_,_prm0_,_prm1_,_ip_, _x_) { \
  _x_ = 1;                                                                             _rcdnorm_(rcrange,rccode,_ip_);\
  { mbu_d(rcrange,rccode, mbu_p(&_m_[  1]), &_m_[  1], _prm0_,_prm1_, _x_); _RCDNORM1(_ip_); }\
  { mbu_d(rcrange,rccode, mbu_p(&_m_[_x_]), &_m_[_x_], _prm0_,_prm1_, _x_); _RCDNORM2(_ip_); }\
  { mbu_d(rcrange,rccode, mbu_p(&_m_[_x_]), &_m_[_x_], _prm0_,_prm1_, _x_); _RCDNORM1(_ip_); }\
  { mbu_d(rcrange,rccode, mbu_p(&_m_[_x_]), &_m_[_x_], _prm0_,_prm1_, _x_); _rcdnorm_(rcrange,rccode,_ip_); }\
  { mbu_d(rcrange,rccode, mbu_p(&_m_[_x_]), &_m_[_x_], _prm0_,_prm1_, _x_); _RCDNORM1(_ip_); }\
  { mbu_d(rcrange,rccode, mbu_p(&_m_[_x_]), &_m_[_x_], _prm0_,_prm1_, _x_); _RCDNORM2(_ip_); }\
  { mbu_d(rcrange,rccode, mbu_p(&_m_[_x_]), &_m_[_x_], _prm0_,_prm1_, _x_);                  }\
}

// 6 bits
#define mb6enc(rcrange,rclow, _m_,_prm0_,_prm1_,_op_, _x_) do {\
  unsigned _x = 1<<6 | _x_;\
  { _rcenorm_(rcrange,rclow,_op_); unsigned _mp = mbu_p(&_m_[    1]); mbu_e(rcrange,rclow, _mp, &_m_[    1], _prm0_,_prm1_,_op_, _x & (1<<5)); }\
  { _RCENORM1(_op_);               unsigned _mp = mbu_p(&_m_[_x>>5]); mbu_e(rcrange,rclow, _mp, &_m_[_x>>5], _prm0_,_prm1_,_op_, _x & (1<<4)); }\
  { _RCENORM2(_op_);               unsigned _mp = mbu_p(&_m_[_x>>4]); mbu_e(rcrange,rclow, _mp, &_m_[_x>>4], _prm0_,_prm1_,_op_, _x & (1<<3)); }\
  { _RCENORM1(_op_);               unsigned _mp = mbu_p(&_m_[_x>>3]); mbu_e(rcrange,rclow, _mp, &_m_[_x>>3], _prm0_,_prm1_,_op_, _x & (1<<2)); }\
  { _rcenorm_(rcrange,rclow,_op_); unsigned _mp = mbu_p(&_m_[_x>>2]); mbu_e(rcrange,rclow, _mp, &_m_[_x>>2], _prm0_,_prm1_,_op_, _x & (1<<1)); }\
  { _RCENORM1(_op_);               unsigned _mp = mbu_p(&_m_[_x>>1]); mbu_e(rcrange,rclow, _mp, &_m_[_x>>1], _prm0_,_prm1_,_op_, _x & (1<<0)); }\
} while(0)

#define mb6dec(rcrange,rccode, _m_,_prm0_,_prm1_,_ip_, _x_) { \
  _x_ = 1;                                                                             _rcdnorm_(rcrange,rccode,_ip_);\
  { mbu_d(rcrange,rccode, mbu_p(&_m_[  1]), &_m_[  1], _prm0_,_prm1_, _x_); _RCDNORM1(_ip_); }\
  { mbu_d(rcrange,rccode, mbu_p(&_m_[_x_]), &_m_[_x_], _prm0_,_prm1_, _x_); _RCDNORM2(_ip_); }\
  { mbu_d(rcrange,rccode, mbu_p(&_m_[_x_]), &_m_[_x_], _prm0_,_prm1_, _x_); _RCDNORM1(_ip_); }\
  { mbu_d(rcrange,rccode, mbu_p(&_m_[_x_]), &_m_[_x_], _prm0_,_prm1_, _x_); _rcdnorm_(rcrange,rccode,_ip_); }\
  { mbu_d(rcrange,rccode, mbu_p(&_m_[_x_]), &_m_[_x_], _prm0_,_prm1_, _x_); _RCDNORM1(_ip_); }\
  { mbu_d(rcrange,rccode, mbu_p(&_m_[_x_]), &_m_[_x_], _prm0_,_prm1_, _x_);                  }\
}

// 5 bits
#define mb5enc(rcrange,rclow, _m_,_prm0_,_prm1_,_op_, _x_) do {\
  unsigned _x = 1<<5 | (_x_);\
  { _rcenorm_(rcrange,rclow,_op_); unsigned _mp = mbu_p(&_m_[    1]); mbu_e(rcrange,rclow, _mp, &_m_[    1], _prm0_,_prm1_,_op_, (_x & (1<<4))); }\
  { _RCENORM1(_op_);               unsigned _mp = mbu_p(&_m_[_x>>4]); mbu_e(rcrange,rclow, _mp, &_m_[_x>>4], _prm0_,_prm1_,_op_, (_x & (1<<3))); }\
  { _RCENORM2(_op_);               unsigned _mp = mbu_p(&_m_[_x>>3]); mbu_e(rcrange,rclow, _mp, &_m_[_x>>3], _prm0_,_prm1_,_op_, (_x & (1<<2))); }\
  { _RCENORM1(_op_);               unsigned _mp = mbu_p(&_m_[_x>>2]); mbu_e(rcrange,rclow, _mp, &_m_[_x>>2], _prm0_,_prm1_,_op_, (_x & (1<<1))); }\
  { _rcenorm_(rcrange,rclow,_op_); unsigned _mp = mbu_p(&_m_[_x>>1]); mbu_e(rcrange,rclow, _mp, &_m_[_x>>1], _prm0_,_prm1_,_op_, (_x & (1<<0))); }\
} while(0)

#define mb5dec(rcrange,rccode, _m_,_prm0_,_prm1_,_ip_, _x_) { \
  _x_ = 1;                                                       				 _rcdnorm_(rcrange,rccode,_ip_);\
  { unsigned _mp = mbu_p(&_m_[  1]); mbu_d(rcrange,rccode, _mp, &_m_[  1], _prm0_,_prm1_, _x_); _RCDNORM1(_ip_);}\
  { unsigned _mp = mbu_p(&_m_[_x_]); mbu_d(rcrange,rccode, _mp, &_m_[_x_], _prm0_,_prm1_, _x_); _RCDNORM2(_ip_);}\
  { unsigned _mp = mbu_p(&_m_[_x_]); mbu_d(rcrange,rccode, _mp, &_m_[_x_], _prm0_,_prm1_, _x_); _RCDNORM1(_ip_);}\
  { unsigned _mp = mbu_p(&_m_[_x_]); mbu_d(rcrange,rccode, _mp, &_m_[_x_], _prm0_,_prm1_, _x_); _rcdnorm_(rcrange,rccode,_ip_); }\
  { unsigned _mp = mbu_p(&_m_[_x_]); mbu_d(rcrange,rccode, _mp, &_m_[_x_], _prm0_,_prm1_, _x_);}\
}

// 4 bits
#define mb4enc(rcrange,rclow, _m_,_prm0_,_prm1_,_op_, _x_) do {\
  unsigned _x = 1<<4 | _x_;\
  { _rcenorm_(rcrange,rclow,_op_); unsigned _mp = mbu_p(&_m_[    1]); mbu_e(rcrange,rclow, _mp, &_m_[    1], _prm0_,_prm1_,_op_, _x & (1<<3));}\
  { _RCENORM1(_op_);               unsigned _mp = mbu_p(&_m_[_x>>3]); mbu_e(rcrange,rclow, _mp, &_m_[_x>>3], _prm0_,_prm1_,_op_, _x & (1<<2));}\
  { _RCENORM2(_op_);               unsigned _mp = mbu_p(&_m_[_x>>2]); mbu_e(rcrange,rclow, _mp, &_m_[_x>>2], _prm0_,_prm1_,_op_, _x & (1<<1));}\
  { _RCENORM1(_op_);               unsigned _mp = mbu_p(&_m_[_x>>1]); mbu_e(rcrange,rclow, _mp, &_m_[_x>>1], _prm0_,_prm1_,_op_, _x & (1<<0));}\
} while(0)

#define mb4dec(rcrange,rccode, _m_,_prm0_,_prm1_,_ip_, _x_) { \
  _x_ = 1;                                                       					           _rcdnorm_(rcrange,rccode,_ip_);\
  { unsigned _mp = mbu_p(&_m_[  1]); mbu_d(rcrange,rccode, _mp, &_m_[  1],_prm0_,_prm1_, _x_); _RCDNORM1(_ip_); }\
  { unsigned _mp = mbu_p(&_m_[_x_]); mbu_d(rcrange,rccode, _mp, &_m_[_x_],_prm0_,_prm1_, _x_); _RCDNORM2(_ip_); }\
  { unsigned _mp = mbu_p(&_m_[_x_]); mbu_d(rcrange,rccode, _mp, &_m_[_x_],_prm0_,_prm1_, _x_); _RCDNORM1(_ip_); }\
  { unsigned _mp = mbu_p(&_m_[_x_]); mbu_d(rcrange,rccode, _mp, &_m_[_x_],_prm0_,_prm1_, _x_); }\
}

// 3 bits
#define mb3enc(rcrange,rclow, _m_,_prm0_,_prm1_,_op_, _x_) do {\
  unsigned _x = 1<<3 | _x_;\
  { _rcenorm_(rcrange,rclow,_op_); unsigned _mp = mbu_p(&_m_[    1]); mbu_e(rcrange,rclow, _mp, &_m_[    1], _prm0_,_prm1_,_op_, _x & (1<<2));}\
  { _RCENORM1(_op_);               unsigned _mp = mbu_p(&_m_[_x>>2]); mbu_e(rcrange,rclow, _mp, &_m_[_x>>2], _prm0_,_prm1_,_op_, _x & (1<<1));}\
  { _RCENORM2(_op_);               unsigned _mp = mbu_p(&_m_[_x>>1]); mbu_e(rcrange,rclow, _mp, &_m_[_x>>1], _prm0_,_prm1_,_op_, _x & (1<<0));}\
} while(0)

#define mb3dec(rcrange,rccode, _m_,_prm0_,_prm1_,_ip_, _x_) { \
  _x_ = 1;                                                       					            _rcdnorm_(rcrange,rccode,_ip_);\
  { unsigned _mp = mbu_p(&_m_[  1]); mbu_d(rcrange,rccode, _mp, &_m_[  1], _prm0_,_prm1_, _x_); _RCDNORM1(_ip_);}\
  { unsigned _mp = mbu_p(&_m_[_x_]); mbu_d(rcrange,rccode, _mp, &_m_[_x_], _prm0_,_prm1_, _x_); _RCDNORM2(_ip_);}\
  { unsigned _mp = mbu_p(&_m_[_x_]); mbu_d(rcrange,rccode, _mp, &_m_[_x_], _prm0_,_prm1_, _x_); }\
}
            
// 2 bits
#define mb2enc(rcrange,rclow, _m_,_prm0_,_prm1_,_op_, _x_) do {\
  unsigned _x = 1<<2 | _x_;\
  { _rcenorm_(rcrange,rclow,_op_); unsigned _mp = mbu_p(&_m_[    1]); mbu_e(rcrange,rclow, _mp, &_m_[    1], _prm0_,_prm1_,_op_, _x & (1<<1));}\
  { _RCENORM1(_op_);               unsigned _mp = mbu_p(&_m_[_x>>1]); mbu_e(rcrange,rclow, _mp, &_m_[_x>>1], _prm0_,_prm1_,_op_, _x & (1<<0));}\
} while(0)

#define mb2dec(rcrange,rccode, _m_,_prm0_,_prm1_,_ip_, _x_) { \
  _x_ = 1;                                                       					            _rcdnorm_(rcrange,rccode,_ip_);\
  { unsigned _mp = mbu_p(&_m_[  1]); mbu_d(rcrange,rccode, _mp, &_m_[  1], _prm0_,_prm1_, _x_); _RCDNORM1(_ip_);}\
  { unsigned _mp = mbu_p(&_m_[_x_]); mbu_d(rcrange,rccode, _mp, &_m_[_x_], _prm0_,_prm1_, _x_); }\
}
                                                                      
// Static 
#define mbu_senc(rcrange,rclow, _mb_,_op_, _b_) do { unsigned _mbp = mbu_p(_mb_); rcbenc(rcrange,rclow , _mbp, ;, ;, _mb_, 0,0,_op_, _b_); } while(0)
#define mbu_sdec(rcrange,rccode,_mb_,_ip_, _x_) do { unsigned _mbp = mbu_p(_mb_); rcbdec(rcrange,rccode, _mbp, ;, ;, _mb_, 0,0,_ip_, _x_); } while(0)

#define mb4senc(rcrange,rclow, _m_, _op_, _x_) do {\
  unsigned _x = 1<<4 | _x_;\
  mbu_senc(rcrange,rclow, &_m_[    1], _op_, _x & (1<<3));\
  mbu_senc(rcrange,rclow, &_m_[_x>>3], _op_, _x & (1<<2));\
  mbu_senc(rcrange,rclow, &_m_[_x>>2], _op_, _x & (1<<1));\
  mbu_senc(rcrange,rclow, &_m_[_x>>1], _op_, _x & (1<<0));\
} while(0)

// Decode char x from memory buffer in
#define mb4sdec(rcrange,rccode, _m_, _ip_, _x_) {\
  _x_ = 1;\
  mbu_sdec(rcrange,rccode, &_m_[  1], _ip_, _x_);\
  mbu_sdec(rcrange,rccode, &_m_[_x_], _ip_, _x_);\
  mbu_sdec(rcrange,rccode, &_m_[_x_], _ip_, _x_);\
  mbu_sdec(rcrange,rccode, &_m_[_x_], _ip_, _x_);\
}

