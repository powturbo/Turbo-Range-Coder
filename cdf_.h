#define MXV(_i_, _j_) _j_ + ((_j_ <= _i_)?0:(1<<RC_BITS)-16)
#define MIXIN16(_i_) { MXV(_i_,0), MXV(_i_,1), MXV(_i_, 2), MXV(_i_, 3), MXV(_i_, 4), MXV(_i_, 5), MXV(_i_, 6), MXV(_i_, 7),\
                       MXV(_i_,8), MXV(_i_,9), MXV(_i_,10), MXV(_i_,11), MXV(_i_,12), MXV(_i_,13), MXV(_i_,14), MXV(_i_,15) }

static cdf_t mixin16[16][16] = {
 MIXIN16( 0), MIXIN16( 1), MIXIN16( 2), MIXIN16( 3), MIXIN16( 4), MIXIN16( 5), MIXIN16( 6), MIXIN16( 7),
 MIXIN16( 8), MIXIN16( 9), MIXIN16(10), MIXIN16(11), MIXIN16(12), MIXIN16(13), MIXIN16(14), MIXIN16(15)
};

#define CDF16DEC0(_m_)     cdf_t _m_[17];      { int j;                          for(j = 0; j <= 16; j++)    _m_[j] = j << (RC_BITS-4); }
#define CDF16DEC1(_m_,_n_) cdf_t _m_[_n_][17]; { int i,j; for(i=0; i < _n_; i++) for(j = 0; j <= 16; j++) _m_[i][j] = j << (RC_BITS-4); }
  #if 0
#define IC 10 //10
#define MIXD ( ((1u<<RC_BITS)-1) & ~((1<<5)-1) )  //#define MIXD 0x7fd9 //((1u<<ANS_BITS)-1-28) //
#define CDF16DEF __m256i mv = _mm256_set1_epi16(MIXD), v0=_mm256_set_epi16(15*IC,14*IC,13*IC,12*IC,11*IC,10*IC, 9*IC, 8*IC, 7*IC, 6*IC, 5*IC, 4*IC, 3*IC, 2*IC, 1*IC, 0)
  #else
#define CDF16DEF  
  #endif
#define RATE16 7

  #ifdef __AVX2__
  #if 1

#define cdf16upd(_m_, _x_) {\
  __m256i _vm0 = _mm256_loadu_si256((const __m256i *)(_m_));\
 _mm256_storeu_si256((const __m256i *)(_m_), _mm256_add_epi16(_vm0, _mm256_srai_epi16(_mm256_sub_epi16(_mm256_loadu_si256((const __m256i *)mixin16[_x_]), _vm0), RATE16)));\
}
/*#define cdf16upd2(_m0_, _x0_, _m1_,_x1_) {\
  __m256i _vm0 = _mm256_loadu_si256((const __m256i *)(_m0_));\
  __m256i _vm1 = _mm256_loadu_si256((const __m256i *)(_m1_));\
  __m256i _vx0 = _mm256_loadu_si256((const __m256i *)mixin16[_x0_]);\
	      _vm0 = _mm256_add_epi16(_vm0, _mm256_srai_epi16(_mm256_sub_epi16(_vx0, _vm0), RATE16));\
  __m256i _vx1 = _mm256_loadu_si256((const __m256i *)mixin16[_x1_]);\
	      _vm1 = _mm256_add_epi16(_vm1, _mm256_srai_epi16(_mm256_sub_epi16(_vx1, _vm1), RATE16));\
  _mm256_storeu_si256((const __m256i *)(_m0_), _vm0);\
  _mm256_storeu_si256((const __m256i *)(_m1_), _vm1);\
}*/
#else
	
#define cdf16upd(_m_, _x_) {\
  __m256i _m0 = _mm256_loadu_si256((const __m256i *)_m_);\
  __m256i _g0 = _mm256_cmpgt_epi16(_m0, _mm256_set1_epi16(_m_[_x_]));\
  _m0 = _mm256_add_epi16(_m0,_mm256_srai_epi16(_mm256_add_epi16(_mm256_sub_epi16(v0,_m0),_mm256_and_si256(_g0,mv)), RATE16));\
  _mm256_storeu_si256((__m256i *)_m_, _m0);\
}
#endif

  #elif defined(__SSE2__) || defined(__ARM_NEON) || defined(__powerpc64__)
#define cdf16upd(_m_, _y_) {\
  __m128i _vx0 = _mm_loadu_si128((const __m128i *) mixin16[_y_]);\
  __m128i _vm0 = _mm_loadu_si128((const __m128i *)(_m_));\
  __m128i _vx1 = _mm_loadu_si128((const __m128i *)&mixin16[_y_][8]); \
  __m128i _vm1 = _mm_loadu_si128((const __m128i *)&(_m_)[8]);\
	_vm0 = _mm_add_epi16(_vm0, _mm_srai_epi16(_mm_sub_epi16(_vx0, _vm0), RATE16));\
	_vm1 = _mm_add_epi16(_vm1, _mm_srai_epi16(_mm_sub_epi16(_vx1, _vm1), RATE16));\
	_mm_storeu_si128((const __m128i *)( _m_),  _vm0);\
	_mm_storeu_si128((const __m128i *)&_m_[8], _vm1);\
}
  #else
#define cdf16upd(_m_, _x_) { int _i; for(_i = 0; _i < 16; _i++) _m_[_i] += (mixin16[_x_][_i] - _m_[_i]) >> RATE16; }
  #endif

