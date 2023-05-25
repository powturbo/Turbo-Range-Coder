## TurboRC: Turbo Range Coder + rANS Asymmetric Numeral Systems
[![Build ubuntu](https://github.com/powturbo/Turbo-Range-Coder/actions/workflows/build.yaml/badge.svg)](https://github.com/powturbo/Turbo-Range-Coder/actions/workflows/build.yaml)

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
    - preprocessing : lzp and utf-8
    - postprocessing: QLFC (Quantized Local Frequency Coding),<br/>
    RLE and Bit entropy coder
  * BWT :libsais + optimized libdivsufsort + optimized inverse bwt included
  * static + adaptive CDF - cumulative distribution functions
  * stdin/stdout file compressor included
  * TurboRC App for benchmarking all the functions and test allmost all byte, integer, floating point, date and timestamp file types.
    - read and convert text, csv or binary files to 8/16/32 bits before processing
    - set predictor and parameters at the command line
* **Asymmetric Numeral Systems**
  * :new:(2023.04) Adaptive CDF rANS Asymmetric Numeral Systems
  * :new:(2023.05) bitwise ANS
###  LICENSE
- GPL 3.0
- A commercial license is available. Contact us at powturbo [AT] gmail.com for more information.

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


###### File: enwik8bwt generated BWT (wikipedia XML 100MB )
        > turborc -e0 enwik8bwt
        
|C Size   |ratio% |C MB/s  |D MB/s  |Name       |Description                  |
|--------:|------:|-------:|-------:|-----------|-----------------------------|
| 23334248| 23.33%|   88.20|   88.54| 1:rc      |  o0                         |         
| 22394444| 22.39%|   82.46|   86.35| 2:rcc     |  o1                         |         
| 23116048| 23.12%|   74.11|   79.26| 3:rcc2    |  o2                         |         
| 22500640| 22.50%|   64.96|   67.81| 4:rcx     |  o8b =o1 context slide      |         
| 23213968| 23.21%|   55.70|   61.45| 5:rcx2    |  o16b=o2 context slide      |         
| 21605020| 21.61%|   25.48|   26.98| 9:rcms    |  o1 mixer/sse               |         
| 21550184| 21.55%|   21.82|   22.74|10:rcm2    |  o2 mixer/sse               |         
| 20814372| 20.81%|   23.06|   24.77|11:rcmr    |  o2 8b mixer/sse run        |         
| 20789560| 20.79%|   22.68|   24.70|12:rcmrr   |  o2 8b mixer/sse run > 2    |         
| 23170048| 23.17%|  156.61|  129.94|13:rcrle   |  RLE o0                     |         
| 22004856| 22.00%|  128.43|  114.98|14:rcrle1  |  RLE o1                     |         
| 23412436| 23.41%|   73.08|   70.74|17:rcu3    |  varint8 3/5/8 bits         |         
| 21088368| 21.09%|   79.78|   93.89|18:rcqlfc  |  QLFC                       |         
| 22275484| 22.28%|   91.81|   96.60|19:bec     |  Bit EC                     |         
| 32703468| 32.70%|   54.48|   58.27|26:rcg-8   |  gamma                      |         
| 32271396| 32.27%|  124.84|  110.15|27:rcgz-8  |  gamma zigzag               |         
| 34195068| 34.20%|   66.13|   65.23|28:rcr-8   |  rice                       |         
| 36864024| 36.86%|   78.31|   70.00|29:rcrz-8  |  rice zigzag                |         
| 63541712| 63.54%|  552.28|   87.84|42:cdfsb   |  static/decode search       |         
| 63541712| 63.54%|  552.38|  115.42|43:cdfsv   |  static/decode division     |         
| 63976686| 63.98%|  479.38|  104.09|44:cdfsm   |  static/decode division lut |         
| 63541720| 63.54%|  628.18|   92.24|45:cdfsb   |  static interlv/dec. search |         
| 24811052| 24.81%|  177.39|  104.30|46:cdf     |  byte   adaptive            |         
| 24811060| 24.81%|  191.80|   98.96|47:cdfi    |  byte   adaptive interleaved|         
| 31004892| 31.00%|  158.06|   72.18|48:cdf-8   |  vnibble                    |         
| 31004896| 31.00%|  159.56|   73.53|49:cdfi-8  |  vnibble interleaved        |         
| 24848864| 24.85%|  116.76|  202.27|56:ans auto|                             |         
| 24848864| 24.85%|  126.57|  175.43|57:ans sse |                             |         
| 23068372| 23.07%|  128.06|   83.57|64:ans auto|  o1                         |         
| 23521656| 23.52%|   50.43|   82.32|66:ansb    |  bitwise ans                |         
|100000012|100.00%|16495.29|16050.82|79:memcpy  |                             |         

## BWT Benchmark: TurboRC vs the best BWT compressors (2023.04)
- [bsc](https://github.com/IlyaGrebnov/libbsc)
- [bzip3](https://github.com/kspalaiologos/bzip3)
- [bzip2](https://github.com/asimonov-im/bzip2)

#### - [enwik8](http://mattmahoney.net/dc/text.html) - 100.000.000 bytes EN Wikipedia
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

#### - [Silesia - Compression Corpus](https://sun.aei.polsl.pl//~sdeor/index.php?page=silesia) (211 MB mixed binary + text)
|C Size|ratio%|C MB/s|D MB/s|Name|
|--------:|-----:|--------:|--------:|----------------|
|48400486| 22.8|**9.63**|**16.08**|**TurboRC 20e9**|
|48621296| 22.9|**14.51**|**18.05**|**bsc 0e2**|
|48754005| 23.0|12.49|11.73|bzip3|
|49142246| 23.2|**18.47**|**28.62**|**bsc 0e1**|
|49589166| 23.4|**18.64**|**34.69**|**TurboRC 20e8**|
|50110576| 23.6|**20.98**|**35.99**|**bsc 0e3**|
|54592210| 25.8|18.22|**52.14**|**bzip2**|

#### - [English.100mb text files from Gutenberg Project](http://pizzachili.dcc.uchile.cl/texts.html)
|C Size|ratio%|C MB/s|D MB/s|Name|
|--------:|-----:|--------:|--------:|----------------|
|18720206| 17.9|**10.89**|**19.07**|**TurboRC 20e9**|
|18739661| 17.9|**12.69**|11.12|**bzip3**|
|19080950| 18.2|**20.59**|**40.81**|**TurboRC 20e8**|
|19255056| 18.4|15.83|21.37|bsc 0e2|
|19371264| 18.5|19.68|33.01|bsc 0e1|
|19614180| 18.7|**22.22**|**41.67**|**bsc 0e0**|
|19673386| 18.8|20.91|**42.48**|**TurboRC 20e6**|
|19809790| 18.9|**22.90**|**45.62**|**TurboRC 20e5**|
|29433182| 28.1|19.65|41.49|bzip2|

#### - html8 : 100MB random html pages from Alexa 1m Top sites
|C Size|ratio%|C MB/s|D MB/s|Name|
|--------:|-----:|--------:|--------:|----------------|
|13203250| 13.2|**15.85**|**26.16**|**TurboRC 20e9**|
|13301850| 13.3|**17.61**|16.32|**bzip3**|
|13442610| 13.4|**30.55**|**56.65**|**TurboRC 20e8**|
|13601922| 13.6|19.12|26.78|bsc 0e2|
|13688478| 13.7|22.77|39.10|bsc 0e1|
|13918372| 13.9|25.63|46.95|bsc 0e0|
|18162609| 18.2|21.16|**67.90**|**bzip2**|

#### - [enwik9](http://mattmahoney.net/dc/text.html) - 1GB EN Wikipedia
|C Size|ratio%|C MB/s|D MB/s|Name|
|--------:|-----:|--------:|--------:|----------------|
|163656130| 16.4|**8.03**|**15.75**|**TurboRC 20e9**|
|163883906| 16.4|**12.92**|**22.30**|**bsc 0e2**|
|164960746| 16.5|**15.18**|**34.07**|**bsc 0e1**|
|165206106| 16.5|15.06|**37.90**|**TurboRC 20e8**|
|167071950| 16.7|**16.54**|**42.30**|**bsc 0e0**|
|169984250| 17.0|11.27|9.89|bzip3|
|253977891| 25.4|**19.90**|**46.46**|**bzip2**|

#### - test1.txt - 1GB ZH (chineese) Wikipedia from [GDCC2021](https://www.facebook.com/gdccompetition) 
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

#### - Text log file:[NASA access log](https://ita.ee.lbl.gov/html/contrib/NASA-HTTP.html) 200MB  
|C Size|ratio%|C MB/s|D MB/s|Name|
|--------:|-----:|--------:|--------:|----------------|
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

Last update: 23 MAY 2023
