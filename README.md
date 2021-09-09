TurboRC: Turbo Range Coder[![Build Status][travisDevBadge]](https://app.travis-ci.com/powturbo/Turbo-Range-Coder)

[travisBadge]: https://api.travis-ci.com/powturbo/Turbo-Range-Coder.svg?branch=master

======================================
* **Fastest Range Coder / Arithmetic Coder**
  * 100% C (C++ headers). 
  * OS/Arch: Linux amd/intel, arm, PowerPC, s390x, MacOs. Windows: Mingw, visual c++
  * No other Range Coder / Arithmetic Coder encode or decode faster with better compression
  * Up to 3 times faster than the next fastest range coder with similar compression ratio
  * Can work as bitwise or/and as multisymbol range coder
  * 32 or 64 bits range coder. Big+Little endian
  * Renormalization output 8,16 or 32 bits 
  * Easy connection to bit, nibble or byte predictors. Several built-in predictors
  * stdin/stdout file compressor included
  * Built-in order0, order1, Run Length Encoding, VLC/Gamma Coding, Move-To-Front/QLFC
  * Fast full BWT:Burrows-Wheeler compression/decompression w/
    QLFC:Quantized Local Frequency Coding and lzp preprocessing
  * new (2021.09): BWT libsais from libbsc + divsufsort included
  * lzp preprocessor
  * static + adaptive cdf - cumulative distribution functions

## Benchmark
   see also [Entropy Coder Benchmark](https://sites.google.com/site/powturbo/entropy-coder) 

        ./turborc -e0   inputfile           "benchmark all basic functions
        ./turborc -e20  inputfile           "byte gamma coding + rc
        ./turborc -e11,14 inputfile


###### File: enwik9bwt generated BWT (wikipedia XML 1GB )
		
|C Size|ratio%|C MB/s|D MB/s|Name / 2020-01|
|--------:|-----:|--------:|--------:|---------------------------------------|
|167,395,956|17.13|**82**|**120**|25-rcqlfcs  Move-To-Front/QLFC              |
|171,300,484|17.13|**92**|**91**|24-rcssxrle  RLE                              |
|172,462,692|17.25|**112**|**108**|22-rcsxrle RLE                              |
|173,418,060|17.34|   48|  44|14-rcssx o8bits (strong)|
|175,618,792|17.56|   64|  50|12-rcsx  o8bits sliding context          |
|176,983,264|17.70|   61|  58|13-rcss  o0 (strong)                     |
|183,243,104|18.32|  110| 102|23-rcssrle RLE	                           |
|182,958,556|18.30|**139**|**113**|21-rcsrle RLE 	                           |
|183,542,772|18.35|  81 | 73|11-rcs   o0 	                           |

###### File: QC (DNA 100MB)

|C Size|ratio%|C MB/s|D MB/s|Name / 2020-01|
|--------:|-----:|--------:|--------:|---------------------------------------|
|  838,416 | 0.83% | **909**|**1645**|24-rcssxrle RLE|
|  839,384 | 0.83% | **1283**|**1936**|22-rcsxrle RLE|
| 1,000,716 | 0.99% |   54|  55|14-rcssx o8bits sliding context (strong) 	|
| 1,041,592 | 1.03% |   76|  65|12-rcsx  o8bits sliding context 	        |
| 1,081,104 | 1.07% | 1263|1785|23-rcssrle RLE|
| 1,087,896 | 1.08% |**1456**|**2021**|21-rcsrle RLE|
| 3,388,684 | 3.36% |   73|  73|13-rcss  o0 (strong)|
| 4,145,920 | 4.10% |   92|  95|11-rcs   o0 	    |


## File Compression

#### Range Coder
        ./turborc -11 inputfile outputfile         "order 0 simple
        ./turborc -12 inputfile outputfile         "order 1 simple
        ./turborc -13 inputfile outputfile         "order 0 strong
        ./turborc -14 inputfile outputfile         "order 1 strong

#### Range Coder + RLE
        ./turborc -21 inputfile outputfile         "order 0 simple
        ./turborc -22 inputfile outputfile         "order 1 simple
        ./turborc -23 inputfile outputfile         "order 0 strong
        ./turborc -24 inputfile outputfile         "order 1 strong
        ./turborc -d inputfile outputfile          "decompress

#### Integer Raw File (binary 8,12,32 bits)
Variable integer coding: gamma coding + turborc</br>
Use gamma coding only, if most of the values are small

        ./turborc -30 inputfile outputfile         "Variable integer: gamma8
        ./turborc -31 inputfile outputfile         "Variable integer: gamma16
        ./turborc -32 inputfile outputfile         "Variable integer: gamma32
        ./turborc -d inputfile outputfile          "decompress

#### BWT (Burrows-Wheeler) + QLFC (Quantized Local Frequency Coding) + TurboRC

        ./turborc -26 inputfile outputfile         "bwt compression
        ./turborc -d inputfile outputfile          "decompress

## Compile:
        Download or clone TurboRC
		git clone git://github.com/powturbo/Turbo-Range-Coder.git
		cd Turbo-Range-Coder
        
###### Linux, MacOS, Windows (MingW), Clang,... (see also makefile)
		make
	or
		make BWT=1                                 "include BWT (default libsais)
     
###### Windows visual c++
		nmake /f makefile.vs

###### Windows visual studio c++
		project files in directory vs/vs2017

## Function usage:
See examples in "turborcs.c"

## Environment:
###### OS/Compiler (32 + 64 bits):
- Windows: Visual C++ (2017)
- Windows: MinGW-w64 makefile
- Linux amd/intel: GNU GCC (>=4.6)
- Linux amd/intel: Clang (>=3.2)
- Linux arm: aarch64
- MaxOS: XCode (>=9)
- PowerPC ppc64le
- IBM s390x

## References:

* **References:**
  * <a name="a"></a>[Entropy Coder Benchmark](https://sites.google.com/site/powturbo/entropy-coder) 

Last update:  09 SEP 2021
