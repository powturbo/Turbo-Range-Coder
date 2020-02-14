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
// TurboRC: Range Coder - Eugene shelwien

#include <stdio.h>             
#include "conf.h"   

#define RC_BITS 15     

#define eSCALE (1<<(RC_BITS+7)) //
static const int M_n0wr = (eSCALE/(5442+16));
static const int M_n0mw = (32+1) * (1);
static const int M_sxP0 = (11562+1) * (1);

static const int M_sxwr = (eSCALE/(7+16));
static const int M_sxmw = (1192+1) * (1);
static const int M_smP0 = (25088+1) * (1);

static const int M_smwr = (eSCALE/(38+16));
static const int M_smmw = (392+1) * (1);

#define RC_MACROS
#include "turborc_.h"
#include "mbc_sh.h" 

#define RCPRM0 M_n0wr
#define RCPRM1 M_n0mw

#define RC1PRM0 RCPRM0  
#define RC1PRM1 RCPRM1    
#include "turborcs_.c"


