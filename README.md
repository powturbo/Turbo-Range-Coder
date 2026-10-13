## TurboRC: Turbo Range Coder + rANS Asymmetric Numeral Systems
[![Build ubuntu](https://github.com/powturbo/Turbo-Range-Coder/actions/workflows/build.yaml/badge.svg)](https://github.com/powturbo/Turbo-Range-Coder/actions/workflows/build.yaml)

======================================
* **Fastest (Branchless) Range Coder / Arithmetic Coder**
  * 100% C (C++ headers). 
  * OS/Arch: Linux amd/intel, arm, RISC-V, PowerPC, s390x, Apple M1-M5 MacOs. Windows: MSYS2, Mingw, visual c++
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
  * BWT :libsais + optimized libdivsufsort + optimized inverse bwt included + Multithreading
  * static + adaptive CDF - cumulative distribution functions
  * stdin/stdout file compressor included
  * TurboRC App for benchmarking all the functions and test allmost all byte, integer, floating point, date and timestamp file types.
    - read and convert text, csv or binary files to 8/16/32 bits before processing
    - set predictor and parameters at the command line
  * Preprocessing:
    - Linear Quantization for 16/32/64 floating point to 8/16/32/64 integer 
* **Asymmetric Numeral Systems**
  * Fastest Adaptive CDF rANS Asymmetric Numeral Systems / SSE, AVX2 / Arm NEON / RISC-V RVV 
  * bitwise ANS
  * No other adaptive ANS encode or decode faster with better compression
###  LICENSE
- GPL 3.0
- **A commercial license is available. Contact us at powturbo [AT] gmail.com for more information.**

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
| 21075240| 21.09%|   79.78|   93.89|18:rcqlfc  |  QLFC                       |
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

## BWT Benchmark: TurboRC/BwtSatan vs the best BWT compressors (2026-10)
- [bsc](https://github.com/IlyaGrebnov/libbsc)
- [bzip3](https://github.com/kspalaiologos/bzip3)
- [bzip2](https://github.com/asimonov-im/bzip2)

#### - [Enceladus_Cassini_mosaic_global_100m_schenk2024.tif](https://astrogeology.usgs.gov/search/map/enceladus-cassini-global-mosaic-100m-schenk) - 518.340.175 bytes
|C Size|ratio%|C MB/s|D MB/s|Rank|2026/10|
|--------:|-----:|--------:|--------:|----------------|----------------|
|320482718| 61.83%   |$\color{green}{\textbf{8.14}}$|$\color{green}{\textbf{11.41}}$|5🥉|$\color{green}{\textbf{bwtsatan 9}}$|
|320722119| 61.87%   |8.02|6.57|7 |bzip3 512|
|321845846| 62.09%   |7.82|8.06|9 |bsc 0e2|
|324465458| 62.60%   |$\color{green}{\textbf{10.90}}$|$\color{green}{\textbf{17.30}}$|2🏆|$\color{green}{\textbf{bwtsatan 8}}$|
|324569678| 62.62%   |10.53|15.32|6 |bsc 0|
|329404800| 63.55%   |$\color{green}{\textbf{12.69}}$|$\color{green}{\textbf{18.42}}$|1👑|$\color{green}{\textbf{bsc 0e0}}$|
|330833640| 63.83%   |8.36|9.70|8 |kanzi 7|
|356284409| 68.74%   |6.85|16.78|10 |kanzi 6|
|377603323| 72.85%   |11.16|$\color{green}{\textbf{21.37}}$|4🥈|$\color{green}{\textbf{kanzi 5}}$|
|385167317| 74.31%   |$\color{green}{\textbf{12.97}}$|$\color{green}{\textbf{30.39}}$|3🥇|$\color{green}{\textbf{bzip2}}$|
|395061965| 76.22%   |2.01|8.73|11 |pulsar|
|518340175|100.00%   |$\color{green}{\textbf{17926.96}}$|$\color{green}{\textbf{18431.84}}$|12 |$\color{green}{\textbf{memcpy}}$|


#### - [enwik8](http://mattmahoney.net/dc/text.html) - 100.000.000 bytes EN Wikipedia
 (bold = pareto)  MB=1.000.000
|C Size|ratio%|C MB/s|D MB/s|2026-10|
|--------:|-----:|--------:|--------:|----------------|
|20684938| 20.68|**9.02**|**16.04**|**TurboRC 20e9**|
|20749619| 20.7|**10.70**|9.19|**bzip3 512**|
|20786596| 20.8|**13.72**|**19.26**|**bsc 0e2**|
|20920306| 20.9|**16.92**|**29.48**|**bsc 0**|
|20982094| 20.98|**17.42**|**36.21**|**TurboRC 20e7**|
|21224212| 21.2|**19.03**|**37.14**|**bsc 0e0**|
|22011302| 22.0|**19.39**|**40.86**|**TurboRC 20e6**|
|29008758| 29.0|**20.72**|**43.39**|**bzip2**|

#### - [Silesia - Compression Corpus](https://sun.aei.polsl.pl//~sdeor/index.php?page=silesia) (211 MB mixed binary + text)
|C Size|ratio%|C MB/s|D MB/s|2026-10|
|--------:|-----:|--------:|--------:|----------------|
|47178114| 22.26|**10.59**|**16.79**|**TurboRC 20e9x -b68m**|
|48391030| 22.83|**9.63**|**16.08**|**TurboRC 20e9**|
|48400266| 22.90|**20.10**|**34.69**|**TurboRC 20e7x -b68m**|
|48621296| 22.9|**14.51**|**18.05**|**bsc 0e2**|
|48754005| 23.0|12.49|11.73|bzip3 512|
|49142246| 23.2|**18.47**|**28.62**|**bsc 0**|
|49560370| 23.41|**18.64**|**34.69**|**TurboRC 20e7**|
|50110576| 23.6|**20.98**|**35.99**|**bsc 0e0**|
|54592210| 25.8|18.22|**52.14**|**bzip2**|

#### - [English.100mb text files from Gutenberg Project](http://pizzachili.dcc.uchile.cl/texts.html)
|C Size|ratio%|C MB/s|D MB/s|2026-10|
|--------:|-----:|--------:|--------:|----------------|
|18703190| 17.84|**10.89**|**19.07**|**TurboRC 20e9**|
|18739661| 17.9|**12.69**|11.12|**bzip3 512**|
|19058890| 18.18|**20.59**|**40.81**|**TurboRC 20e7**|
|19255056| 18.4|15.83|21.37|bsc 0e2|
|19371264| 18.5|19.68|33.01|bsc 0|
|19614180| 18.7|**22.22**|**41.67**|**bsc 0e0**|
|19809790| 18.9|**22.90**|**45.62**|**TurboRC 20e6**|
|29433182| 28.1|19.65|41.49|bzip2|

#### - html8 : 100MB random html pages from Alexa 1m Top sites
|C Size|ratio%|C MB/s|D MB/s|2026-10|
|--------:|-----:|--------:|--------:|----------------|
|13118618| 13.12|**15.85**|**26.16**|**TurboRC 20e9**|
|13301850| 13.3|**17.61**|16.32|**bzip3 512**|
|13397178| 13.40|**30.55**|**56.65**|**TurboRC 20e7**|
|13601922| 13.6|19.12|26.78|bsc 0e2|
|13688478| 13.7|22.77|39.10|bsc 0|
|13918372| 13.9|25.63|46.95|bsc 0e0|
|18162609| 18.2|21.16|**67.90**|**bzip2**|

#### - [enwik9](http://mattmahoney.net/dc/text.html) - 1GB EN Wikipedia
|C Size|ratio%|C MB/s|D MB/s|2026-10|
|--------:|-----:|--------:|--------:|----------------|
|163621162| 16.36|**8.03**|**15.75**|**TurboRC 20e9**|
|163883906| 16.4|**12.92**|**22.30**|**bsc 0e2**|
|164960746| 16.5|**15.18**|**34.07**|**bsc 0**|
|165114202| 16.51|15.06|**37.90**|**TurboRC 20e7**|
|167071950| 16.7|**16.54**|**42.30**|**bsc 0e0**|
|169984250| 17.0|11.27|9.89|bzip3 512|
|253977891| 25.4|**19.90**|**46.46**|**bzip2**|

#### - test1.txt - 1GB ZH (chineese) Wikipedia from [GDCC2021](https://www.facebook.com/gdccompetition) 
|C Size|ratio%|C MB/s|D MB/s|2026-10|
|--------:|-----:|--------:|--------:|----------------|
|234873322| 23.5|**11.82**|**18.08**|**bsc 0e2**|
|235560478| 23.58|**14.88**|**35.27**|**TurboRC 20e7**|
|236077752| 23.6|13.79|28.76|bsc 0|
|236200122| 23.6|8.94|17.03|TurboRC 20e9|
|239463880| 23.9|**15.37**|**37.21**|**bsc 0e0**|
|245841481| 24.6|8.85|8.51|bzip3 512|
|257419802| 25.7|**18.03**|**41.98**|**TurboRC 20e4**|
|359610522| 36.0|**21.51**|**42.38**|**bzip2**|

#### - Text log file:[NASA access log](https://ita.ee.lbl.gov/html/contrib/NASA-HTTP.html) 200MB  
|C Size|ratio%|C MB/s|D MB/s|Rank|Name|2026/10|
|--------:|-----:|--------:|--------:|----------------|----------------|------------------------------|
|8243238|  4.02%   |$\color{green}{\textbf{79.92}}$|$\color{green}{\textbf{108.49}}$|2🏆|$\color{green}{\textbf{TurboRC 20e8}}$|access_log_Jul95|
|8244226|  4.02%   |68.75|100.40|4🥈|TurboRC 20e9|access_log_Jul95|
|8425042|  4.10%   |$\color{green}{\textbf{126.27}}$|$\color{green}{\textbf{246.64}}$|1👑|$\color{green}{\textbf{TurboRC 20e7t4}}$|access_log_Jul95|
|8425042|  4.10%   |97.02|179.81|3🥇|TurboRC 20e7|access_log_Jul95|
|9138568|  4.45%   |5.26|5.46|8 |bzip3 512|access_log_Jul95|
|9449012|  4.60%   |20.29|41.56|5🥉|bsc 0e2|access_log_Jul95|
|9717048|  4.73%   |12.01|7.45|9 |bcm|access_log_Jul95|
|9930919|  4.84%   |18.91|36.77|6 |kanzi 7|access_log_Jul95|
|11960479|  5.83%   |14.85|80.88|7 |bzip2|access_log_Jul95|
|205242368|100.00%   |$\color{green}{\textbf{17405.22}}$|$\color{green}{\textbf{18395.84}}$|10 |$\color{green}{\textbf{memcpy}}$|access_log_Jul95|

#### - [CLP log/json compression](https://docs.yscope.com/clp/main/user-docs/resources-datasets) using [TurboBench](https://github.com/powturbo/TurboBench) (BwtSatan = TurboRC -e20)
|C Size|ratio%|C MB/s|D MB/s|Rank|Name|2026/10|
|--------:|-----:|--------:|--------:|----------------|----------------|------------------------------|
|211743448|  2.02%   |$\color{green}{\textbf{104.96}}$|$\color{green}{\textbf{166.49}}$|3🥇|$\color{green}{\textbf{bwtsatan 9}}$|cockroach.node1.log|
|211767848|  2.02%   |$\color{green}{\textbf{121.27}}$|$\color{green}{\textbf{177.21}}$|1👑|$\color{green}{\textbf{bwtsatan 8}}$|cockroach.node1.log|
|214648516|  2.04%   |23.25|25.24|4🥈|bzip3 512|cockroach.node1.log|
|216660396|  2.06%   |$\color{green}{\textbf{156.26}}$|$\color{green}{\textbf{318.82}}$|2🏆|$\color{green}{\textbf{bwtsatan 7}}$|cockroach.node1.log|
|267697716|  2.55%   |31.65|67.63|5🥉|bsc 0e2|cockroach.node1.log|

|C Size|ratio%|C MB/s|D MB/s|Rank|Name|2026/10|
|--------:|-----:|--------:|--------:|----------------|----------------|------------------------------|
|21712748|  0.25%   |$\color{green}{\textbf{406.85}}$|$\color{green}{\textbf{772.12}}$|1👑|$\color{green}{\textbf{bwtsatan 8}}$|elasticsearch.log|
|21735160|  0.25%   |358.65|723.76|3🥇|bwtsatan 9|elasticsearch.log|
|22436580|  0.26%   |$\color{green}{\textbf{532.46}}$|$\color{green}{\textbf{1647.23}}$|2🏆|$\color{green}{\textbf{bwtsatan 7}}$|elasticsearch.log|
|30904714|  0.36%   |46.85|62.38|6 |bzip3 512|elasticsearch.log|
|31122700|  0.36%   |100.81|236.72|5🥉|bsc 0e2|elasticsearch.log|
|39703881|  0.46%   |267.47|521.73|4🥈|kanzi 7|elasticsearch.log|
|49049600|  0.57%   |11.39 |      |   |CLP 19|elasticsearch.log|


|C Size|ratio%|C MB/s|D MB/s|Rank|Name|2026/10|
|--------:|-----:|--------:|--------:|----------------|----------------|------------------------------|
|30123628|  1.40%   |$\color{green}{\textbf{112.97}}$|$\color{green}{\textbf{200.93}}$|3🥇|$\color{green}{\textbf{bwtsatan 9}}$|hive-24hr.tar|
|30144608|  1.40%   |$\color{green}{\textbf{141.85}}$|$\color{green}{\textbf{213.82}}$|2🏆|$\color{green}{\textbf{bwtsatan 8}}$|hive-24hr.tar|
|30857040|  1.43%   |$\color{green}{\textbf{178.61}}$|$\color{green}{\textbf{389.29}}$|1👑|$\color{green}{\textbf{bwtsatan 7}}$|hive-24hr.tar|
|31426560|  1.46%   |19.78|     |  |CLP level 19|hive-24hr.tar|
|33146648|  1.54%   |18.41|19.97|8 |bzip3 512|hive-24hr.tar|
|36435456|  1.69%   |37.34|83.41|7 |bsc 0e2|hive-24hr.tar|
|42084147|  1.95%   |83.48|133.05|4🥈|kanzi 7|hive-24hr.tar|

|C Size|ratio%|C MB/s|D MB/s|Rank|Name|2026/10|
|--------:|-----:|--------:|--------:|----------------|----------------|------------------------------|
|274554794|  0.39%   |$\color{green}{\textbf{284.81}}$|$\color{green}{\textbf{542.61}}$|3🥇|$\color{green}{\textbf{bwtsatan 9}}$|mongodb.tar|
|274610450|  0.39%   |$\color{green}{\textbf{320.66}}$|$\color{green}{\textbf{576.68}}$|1👑|$\color{green}{\textbf{bwtsatan 8}}$|mongodb.tar|
|275093394|  0.40%   |40.55|46.68|6 |bzip3 512|mongodb.tar|
|279664350|  0.40%   |$\color{green}{\textbf{412.07}}$|$\color{green}{\textbf{1095.35}}$|2🏆|$\color{green}{\textbf{bwtsatan 7}}$|mongodb.tar|
|465340600|  0.67%   |74.33|181.88|7 |bsc 0e2|mongodb.tar|
|485053806|  0.70%   |78.30|215.54|5🥉|bsc 0|mongodb.tar|
|502791034|  0.72%   |10.50|||CLP level 19|mongodb.tar|

|C Size|ratio%|C MB/s|D MB/s|Rank|Name|2026/10|
|--------:|-----:|--------:|--------:|----------------|----------------|------------------------------|
|656628102|  1.85%   |$\color{green}{\textbf{151.99}}$|$\color{green}{\textbf{222.84}}$|1👑|$\color{green}{\textbf{bwtsatan 8x -b2047m}}$|openstack-24hr.tar|
|656669010|  1.85%   |132.35|210.32|3🥇|bwtsatan 9x -b2047m|openstack-24hr.tar|
|666507010|  1.88%   |$\color{green}{\textbf{152.21}}$|$\color{green}{\textbf{337.53}}$|2🏆|$\color{green}{\textbf{bwtsatan 7x -b2047m}}$|openstack-24hr.tar|
|801468704|  2.26%   |42.39|84.54|4🥈|bsc 0e2|openstack-24hr.tar|
|826207693|  2.33%   |23.13|25.43|5🥉|bzip3 512|openstack-24hr.tar|
|843991040|  2.38%   |10|||CLP 19|openstack-24hr.tar|
|972564480|  2.75    |264| | |CLP 3|openstack-24hr.tar|

|C Size|ratio%|C MB/s|D MB/s|Rank|Name|2026/10|
|--------:|-----:|--------:|--------:|----------------|----------------|------------------------------|
|4549138|  1.10%   |$\color{green}{\textbf{182.79}}$|$\color{green}{\textbf{274.52}}$|2🏆|$\color{green}{\textbf{bwtsatan 8}}$|postgresql.tar|
|4552798|  1.11%   |153.00|252.35|4🥈|bwtsatan 9|postgresql.tar|
|4686638|  1.14%   |$\color{green}{\textbf{301.26}}$|$\color{green}{\textbf{679.05}}$|1👑|$\color{green}{\textbf{bwtsatan 7t4}}$|postgresql.tar|
|4686638|  1.14%   |233.74|503.87|3🥇|bwtsatan 7|postgresql.tar|
|4909031|  1.19%   |27.34|30.10|7 |bzip3 512|postgresql.tar|
|6785858|  1.65%   |33.91|84.32|6 |bsc 0e2|postgresql.tar|
|6862874|  1.67%   |76.62|148.41|5🥉|kanzi 7|postgresql.tar|
|7472304|  1.81%   |11.70|12.47|11 |bcm|postgresql.tar|
|8082307|  1.96%   |11.17|113.64|10 |bzip2|postgresql.tar|
|8089600|  1.96%   |16.24| | |CLP 19|postgresql.tar|
|11017480|  2.67%   |0.38|$\color{green}{\textbf{5385.13}}$|8 |$\color{green}{\textbf{zstd 22}}$|postgresql.tar|
|13137953|  3.19%   |8.86|871.75|9 |lzma 9|postgresql.tar|
|13363200|  3.24%   |164|| |CLP 3|postgresql.tar|
|411924480|100.00%   |$\color{green}{\textbf{19112.16}}$|$\color{green}{\textbf{19610.78}}$|12 |$\color{green}{\textbf{memcpy}}$|postgresql.tar|

13363200

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

Last update: 10 OCT 2026
