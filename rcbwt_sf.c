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
// TurboRC: Range Coder - fsm
// Reference: http://cbloomrants.blogspot.com/2017/01/order-0-estimators-for-data-compression.html

#include <stdio.h>             
#include "conf.h"   
#include "turborc.h"   

#define RC_MACROS
#define RC_BITS 15      	// RC_SIZE=64 + RC_IO=32 : set in turborc_.h
#include "turborc_.h"
#include "mbc_sf.h"       	// fsm predictor

extern fsm_t fsm[];
#include "rcbwt_.c"    // template functions 
