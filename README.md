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
  * Several built-in predictors: simple, dual speed, fsm
  * Built-in order0, order1, order2, Sliding Context, Context mixing,<br/>
            - Run Length Encoding, Gamma Coding, Rice Coding,<br/>
            - Bit entropy coding,<br/>
            - Turbo VLC: novel Variable Length Coding for large integers, <br/>
            - MTF (Move-To-Front) / QLFC (Quantized Local Frequency Coding)<br/>
  * Fast full 8/16 bits BWT: Burrows-Wheeler compression/decompression w/
    - preprocessing : lzp and :new:utf-8
    - postprocessing: QLFC (Quantized Local Frequency Coding), :new:RLE and :new:Bit entropy coder
  * BWT :libsais + optimized libdivsufsort + :new:optimized inverse bwt included
  * static + adaptive cdf - cumulative distribution functions
  * stdin/stdout file compressor included
  * TurboRC App for benchmarking all the functions and test allmost all byte, integer, floating point, date and timestamp file types.
    - read and convert text, csv or binary files to 8/16/32 bits before processing
    - set predictor and parameters at the command line

## Usage examples
        ./turborc -e0   file           " benchmark all basic functions using the default simple predictor
        ./turborc -e20  file           " byte gamma coding + rc
        ./turborc -e1,2 file
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

## BWT Benchmark: TurboRC vs the best BWT compressors
- [bsc](https://github.com/IlyaGrebnov/libbsc)
- [bzip3](https://github.com/kspalaiologos/bzip3)
- [bzip2](https://github.com/asimonov-im/bzip2)

#### [enwik8](http://mattmahoney.net/dc/text.html) - 100.000.000 bytes EN Wikipedia
 (bold = pareto)  MB=1.000.000
|C Size|ratio%|C MB/s|D MB/s|Name|
|--------:|-----:|--------:|--------:|----------------|
|20698282| 20.7|**9.02**|**16.04**|**TurboRC 20e9**|
|20749619| 20.7|**10.70**|9.19|**bzip3**|
|20786596| 20.8|**13.72**|**19.26**|**bsc 0e2**|
|20920306| 20.9|**16.92**|**29.48**|**bsc 0e1**|
|21002082| 21.0|**17.42**|**36.21**|**TurboRC 20e8**|
|21224212| 21.2|**19.03**|**37.14**|**bsc 0e0**|
|21824818| 21.8|17.89|**38.13**|**TurboRC 20e6**|
|22011302| 22.0|**19.39**|**40.86**|**TurboRC 20e5**|
|29008758| 29.0|**20.72**|**43.39**|**bzip2**|

#### [Silesia - Compression Corpus](https://sun.aei.polsl.pl//~sdeor/index.php?page=silesia) (mixed binary + text)
|C Size|ratio%|C MB/s|D MB/s|Name|
|--------:|-----:|--------:|--------:|----------------|
|48400486| 22.8|**9.63**|**16.08**|**TurboRC 20e9**|
|48621296| 22.9|**14.51**|**18.05**|**bsc 0e2**|
|48754005| 23.0|12.49|11.73|bzip3|
|49142246| 23.2|**18.47**|**28.62**|**bsc 0e1**|
|49589166| 23.4|**18.64**|**34.69**|**TurboRC 20e8**|
|50110576| 23.6|**20.98**|**35.99**|**bsc 0e3**|
|54592210| 25.8|18.22|**52.14**|**bzip2**|

#### [English.100mb text files from Gutenberg Project](http://pizzachili.dcc.uchile.cl/texts.html)
|C Size|ratio%|C MB/s|D MB/s|Name|
|--------:|-----:|--------:|--------:|----------------|
|18720206| 17.9|**10.89**|**19.07**|**TurboRC 20e9**|
|18720206| 17.9|10.46|18.72|TurboRC 20e9x|
|18739661| 17.9|**12.69**|11.12|**bzip3**|
|19080950| 18.2|**20.41**|**41.03**|**TurboRC 20e8x**|
|19080950| 18.2|**20.59**|40.81|**TurboRC 20e8**|
|19255056| 18.4|15.83|21.37|bsc 0e2|
|19371264| 18.5|19.68|33.01|bsc 0e1|
|19614180| 18.7|**22.22**|**41.67**|**bsc 0e0**|
|19673386| 18.8|20.91|**42.48**|**TurboRC 20e6**|
|19809790| 18.9|**22.90**|**45.62**|**TurboRC 20e5**|
|29433182| 28.1|19.65|41.49|bzip2|

#### [html8 : 100MB random html pages from 1m Alexa Top sites]
|C Size|ratio%|C MB/s|D MB/s|Name|
|--------:|-----:|--------:|--------:|----------------|
|13203250| 13.2|**15.85**|**26.16**|**TurboRC 20e9**|
|13301850| 13.3|**17.61**|16.32|**bzip3**|h
|13442610| 13.4|**30.55**|**56.65**|**TurboRC 20e8**|
|13601922| 13.6|19.12|26.78|bsc 0e2|
|13688478| 13.7|22.77|39.10|bsc 0e1|
|13918372| 13.9|25.63|46.95|bsc 0e0|
|18162609| 18.2|21.16|**67.90**|**bzip2**|

#### [enwik9](http://mattmahoney.net/dc/text.html) - 1GB EN Wikipedia
|C Size|ratio%|C MB/s|D MB/s|Name|
|--------:|-----:|--------:|--------:|----------------|
|163656130| 16.4|**8.03**|**15.75**|**TurboRC 20e9**|
|163656170| 16.4|6.99|12.04|TurboRC 20e9m120|
|163667694| 16.4|7.92|**15.84**|**TurboRC 20e9m104**|
|163669458| 16.4|7.28|12.45|TurboRC 20e9x|
|163883906| 16.4|**12.92**|**22.30**|**bsc 0e2**|
|164960746| 16.5|**15.18**|**34.07**|**bsc 0e1**|
|165206106| 16.5|15.06|**37.90**|**TurboRC 20e8**|
|167071950| 16.7|**16.54**|**42.30**|**bsc 0e0**|
|169984250| 17.0|11.27|9.89|bzip3|
|253977891| 25.4|**19.90**|**46.46**|**bzip2**|

#### test1.txt - 1GB ZH (chineese) Wikipedia from GDCC2021
|C Size|ratio%|C MB/s|D MB/s|Name|
|--------:|-----:|--------:|--------:|----------------|
|234873322| 23.5|**11.82**|**18.08**|**bsc 0e2**|
|235628874| 23.6|**14.88**|**35.27**|**TurboRC 20e8**|
|236077752| 23.6|13.79|28.76|bsc 0e1|
|236200554| 23.6|8.94|17.03|TurboRC 20e9|
|239463880| 23.9|**15.37**|**37.21**|**bsc 0e3**|
|245841481| 24.6|8.85|8.51|bzip3|
|254748922| 25.5|**15.69**|**39.18**|**TurboRC 20e6**|
|257419802| 25.7|**18.03**|**41.98**|**TurboRC 20e5**|
|359610522| 36.0|**21.51**|**42.38**|**bzip2**|

#### Text log file:[NASA access log](https://ita.ee.lbl.gov/html/contrib/NASA-HTTP.html) 200MB  
|C Size|ratio%|C MB/s|D MB/s|Name|
|--------:|-----:|--------:|--------:|
|9082122|  4.4|**38.69**|**106.37**|**TurboRC 20e8m32**|
|9138588|  4.5|14.52|13.66|bzip3|
|9438810|  4.6|19.41|43.66|bsc 0e2|
|9503102|  4.6|20.74|53.42|bsc 0e1|
|9529266|  4.6|20.24|65.88|TurboRC 20e8|
|9639310|  4.7|20.81|38.94|TurboRC 20e9m32|
|9647322|  4.7|21.45|58.95|bsc 0e3|
|9710206|  4.7|8.73|18.14|TurboRC 20e9|
|9812018|  4.8|20.35|66.12|TurboRC 20e6|
|9817630|  4.8|20.87|68.17|TurboRC 20e5|
|11960479|  5.8|15.71|83.16|bzip2|

## File Compression

#### Range Coder
        ./turborc -1 inputfile outputfile         "order 0 simple
        ./turborc -2 inputfile outputfile         "order 1 simple

#### Range Coder + RLE
        ./turborc -1 inputfile outputfile         "order 0 simple
        ./turborc -2 inputfile outputfile         "order 1 simple
        ./turborc -d inputfile outputfile          "decompress

#### BWT (Burrows-Wheeler) + QLFC (Quantized Local Frequency Coding) + TurboRC

        ./turborc -20e# inputfile outputfile -l# [-Os]  "bwt compression 
		           #:0:store, 2:bit ec, 3/4:RLE, 5/6:RLE o1, 7/8:QLFC, 9:Max
        ./turborc -d inputfile outputfile             "decompress

## Compile:
        Download or clone TurboRC
		git clone --recursive https://github.com/powturbo/Turbo-Range-Coder.git
		cd Turbo-Range-Coder
        
###### Linux, MacOS, Windows (MingW), Clang,... (see also makefile)
		make
	or
		make AVX2=1                                "compile for recent architectures >= haswell
     
###### Windows visual c++
		nmake /f makefile.vs

###### Windows visual studio c++
		project files in directory vs/vs2022

## Function usage:
See examples in "turborc.c"

## Environment:
###### OS/Compiler (32 + 64 bits):
- Windows: Visual C++ (2022)
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

Last update:  27 MAR 2023
