LIBRARY,'LIBRARY',\ zlib.dll
;,';',\ zlib data compression library

EXPORTS,'EXPORTS',\
;,';',\ basic functions
    zlibVersion,'zlibVersion',\
    deflate,'deflate',\
    deflateEnd,'deflateEnd',\
    inflate,'inflate',\
    inflateEnd,'inflateEnd',\
;,';',\ advanced functions
    deflateSetDictionary,'deflateSetDictionary',\
    deflateCopy,'deflateCopy',\
    deflateReset,'deflateReset',\
    deflateParams,'deflateParams',\
    deflateTune,'deflateTune',\
    deflateBound,'deflateBound',\
    deflatePrime,'deflatePrime',\
    deflateSetHeader,'deflateSetHeader',\
    inflateSetDictionary,'inflateSetDictionary',\
    inflateSync,'inflateSync',\
    inflateCopy,'inflateCopy',\
    inflateReset,'inflateReset',\
    inflateReset2,'inflateReset2',\
    inflatePrime,'inflatePrime',\
    inflateMark,'inflateMark',\
    inflateGetHeader,'inflateGetHeader',\
    inflateBack,'inflateBack',\
    inflateBackEnd,'inflateBackEnd',\
    zlibCompileFlags,'zlibCompileFlags',\
;,';',\ utility functions
    compress,'compress',\
    compress2,'compress2',\
    compressBound,'compressBound',\
    uncompress,'uncompress',\
    gzopen,'gzopen',\
    gzdopen,'gzdopen',\
    gzbuffer,'gzbuffer',\
    gzsetparams,'gzsetparams',\
    gzread,'gzread',\
    gzwrite,'gzwrite',\
    gzprintf,'gzprintf',\
    gzputs,'gzputs',\
    gzgets,'gzgets',\
    gzputc,'gzputc',\
    gzgetc,'gzgetc',\
    gzungetc,'gzungetc',\
    gzflush,'gzflush',\
    gzseek,'gzseek',\
    gzrewind,'gzrewind',\
    gztell,'gztell',\
    gzoffset,'gzoffset',\
    gzeof,'gzeof',\
    gzdirect,'gzdirect',\
    gzclose,'gzclose',\
    gzclose_r,'gzclose_r',\
    gzclose_w,'gzclose_w',\
    gzerror,'gzerror',\
    gzclearerr,'gzclearerr',\
;,';',\ checksum functions
    adler32,'adler32',\
    crc32,'crc32',\
    adler32_combine,'adler32_combine',\
    crc32_combine,'crc32_combine',\
;,';',\ various hacks, don't look :)
    deflateInit_,'deflateInit_',\
    deflateInit2_,'deflateInit2_',\
    inflateInit_,'inflateInit_',\
    inflateInit2_,'inflateInit2_',\
    inflateBackInit_,'inflateBackInit_',\
    zError,'zError',\
    inflateSyncPoint,'inflateSyncPoint',\
    get_crc_table,'get_crc_table',\
    inflateUndermine,'inflateUndermine',\
