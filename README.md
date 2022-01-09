TurboRC: Turbo Range Coder

======================================
* **Fastest Range Coder / Arithmetic Coder**
  * 100% C (C++ headers). 
  * OS/Arch: Linux amd/intel, arm, PowerPC, s390x, MacOs+Apple M1. Windows: Mingw, visual c++
  * No other Range Coder / Arithmetic Coder encode or decode faster with better compression
  * Up to 3 times faster than the next fastest range coder with similar compression ratio
  * Can work as bitwise or/and as multisymbol range coder
  * 32 or 64 bits range coder. Big + Little endian
  * Renormalization output 8,16 or 32 bits 
  * Easy connection to bit, nibble or byte predictors. 
  * Several built-in predictors: simple, dual speed, :new:fsm
  * Built-in order0, order1, order2, Sliding Context, :new:Context mixing,<br/>
            - Run Length Encoding, Gamma Coding, :new:Rice Coding,<br/>
            - :new:Bit entropy coding,<br/>
            - :new:Turbo VLC: novel Variable Length Coding for large integers, <br/>
            - MTF (Move-To-Front) / QLFC (Quantized Local Frequency Coding)<br/>
  * Fast full 8/16 bits BWT: Burrows-Wheeler compression/decompression w/
    - preprocessing : lzp and :new:utf-8
    - postprocessing: QLFC (Quantized Local Frequency Coding), :new:RLE and :new:Bit entropy coder
  * BWT :new:libsais + optimized libdivsufsort + :new:optimized inverse bwt included
  * static + adaptive cdf - cumulative distribution functions
  * stdin/stdout file compressor included
  * TurboRC App for benchmarking all the functions and test allmost all byte, integer, floating point, date and timestamp file types.
    - :new:read and convert text, csv or binary files to 8/16/32 bits before processing
    - :new:set predictor and parameters at the command line

## Usage examples
        ./turborc -e0   file           " benchmark all basic functions using the default simple predictor
        ./turborc -e20  file           " byte gamma coding + rc
        ./turborc -e11,14 file
        ./turborc -e0 -pss -r47 file   " use dual speed predictor with parameters 4 and 7
        ./turborc -e0 -psf -r1 file    " use FSM predictor with filename "FSM1.txt"
        ./turborc -e0 file -Os         " raw 16 bits input
        ./turborc -e0 file -Ou         " raw 32 bits input
        ./turborc -e0 file -Ft         " text file (one integer/line) 
        ./turborc -e0 file -Fc         " text file with multiple integer entries (separated by non-digits characters ex. 456,32,54)
        ./turborc -e0 file -Fc -v5     " like prev., display the first 100 values read
        ./turborc -e0 file -Fcf        " text file with multiple floating-point entries (separated by non-digits characters ex. 456.56,32.1,54)
        ./turborc -e0 file -Fru -Ob    " convert raw 32 bits input to bytes before processing possibly truncating large values
        ./turborc -e0 file -Ft -K3 -Ou " convert column 3 of a csv text file to 32 bits integers
        ./turborc -e0 file -pss -r47   " benchmark all basic functions using the dual speed predictor with paramters 4 and 7
        ./turborc -e0 file -psf -r1    " benchmark all basic functions using the fsm predictor with the paramter file FSM1.txt

## Benchmark
   see also [Entropy Coder Benchmark](https://sites.google.com/site/powturbo/entropy-coder) 


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
        ./turborc -1 inputfile outputfile         "order 0 simple
        ./turborc -2 inputfile outputfile         "order 1 simple

#### Range Coder + RLE
        ./turborc -12 inputfile outputfile         "order 0 simple
        ./turborc -14 inputfile outputfile         "order 1 simple
        ./turborc -d inputfile outputfile          "decompress

#### BWT (Burrows-Wheeler) + QLFC (Quantized Local Frequency Coding) + TurboRC

        ./turborc -20 inputfile outputfile -l# [-Os]  "bwt compression 
		                                              #:level  1:Bit EC, 2:RC simple, 3:RC dual speed, 4:RLE O0
													  -Os : optional 16-bits BWT only for levels 1 and 4
        ./turborc -d inputfile outputfile             "decompress

## Compile:
        Download or clone TurboRC
		git clone --recursive git://github.com/powturbo/Turbo-Range-Coder.git
		cd Turbo-Range-Coder
        
###### Linux, MacOS, Windows (MingW), Clang,... (see also makefile)
		make
	or
		make AVX2=1                                "compile for recent architectures >= haswell
     
###### Windows visual c++
		nmake /f makefile.vs

###### Windows visual studio c++
		project files in directory vs/vs2017

## Function usage:
See examples in "turborc.c"

## Environment:
###### OS/Compiler (32 + 64 bits):
- Windows: Visual C++ (2017)
- Windows: MinGW-w64 makefile
- Linux amd/intel: GNU GCC (>=4.6)
- Linux amd/intel: Clang (>=3.2)
- Linux arm: aarch64
- MaxOS: XCode (>=9) + Apple M1
- PowerPC ppc64le
- IBM s390x

## References:

* **References:**
  * <a name="a"></a>[Entropy Coder Benchmark](https://sites.google.com/site/powturbo/entropy-coder) 

Last update:  09 JAN 2022
