#ifdef __cplusplus
extern "C" {
#endif

#define MP3_ERROR_UNKNOWN				1
#define MP3_ERROR_INVALID_PARAMETER		2
#define MP3_ERROR_INVALID_SYNC			3
#define MP3_ERROR_INVALID_HEADER		4
#define MP3_ERROR_OUT_OF_BUFFER			5

typedef struct {
	int			version;		//1:MPEG-1, 2:MPEG-2, 3:MPEG-2.5
	int			layer;			//1:Layer1, 2:Layer2, 3:Layer3
	int			error_prot;		//1:CRC on, 0:CRC off
	int			br_index;
	int			fr_index;
	int			padding;
	int			extension;
	int			mode;
	int			mode_ext;
	int			copyright;
	int			original;
	int			emphasis;
} MPEG_HEADER;

typedef struct {
	int			reduction;
	int			convert;
	int			freqLimit;
} MPEG_DECODE_OPTION;

typedef struct {
	MPEG_HEADER	header;
	int			channels;		//出力チャネル
	int			bitsPerSample;	//
	int			frequency;		//サンプリングレート（Hz）
	int			bitRate;		//ビットレート（bps）

	int			frames;			//フレーム数（VBR only）
	int			skipSize;		//（VBR only）
	int			dataSize;		//データサイズ（VBR only）

	int			minInputSize;	//1フレームの最小入力サイズ
	int			maxInputSize;	//1フレームの最大入力サイズ
	int			outputSize;		//1フレームの出力サイズ
} MPEG_DECODE_INFO;

typedef struct {
	MPEG_HEADER	header;
	int			bitRate;		//ビットレート（bps）

	void*		inputBuf;
	int			inputSize;
	void*		outputBuf;
	int			outputSize;
} MPEG_DECODE_PARAM;

void mp3DecodeInit();
int mp3GetLastError();
int mp3SetDecodeOption(MPEG_DECODE_OPTION* option);
void mp3GetDecodeOption(MPEG_DECODE_OPTION* option);
int mp3SetEqualizer(int* value);

int mp3FindSync(void* buf, int size, int* sync);
int mp3GetDecodeInfo(void* mpeg, int size, MPEG_DECODE_INFO* info, int decFlag);
int mp3DecodeStart(void* buf, int size);
int mp3DecodeFrame(MPEG_DECODE_PARAM* param);

void mp3MuteStart(MPEG_DECODE_PARAM* param);
void mp3MuteEnd(MPEG_DECODE_PARAM* param);

	double pow_test(double, double);

#ifdef __cplusplus
}
#endif
