// QR_Encode.h : CQR_Encode クラス宣言およびインターフェイス定義
// Date 2006/05/17	Ver. 1.22	Psytec Inc.

#ifndef __QR_ENCODE_H__
#define __QR_ENCODE_H__

#include <stdbool.h>

typedef const char *LPCSTR;

/////////////////////////////////////////////////////////////////////////////
// 定数

// 誤り訂正レベル

#define QR_LEVEL_L	0
#define QR_LEVEL_M	1
#define QR_LEVEL_Q	2
#define QR_LEVEL_H	3

// データモード
#define QR_MODE_NUMERAL		0
#define QR_MODE_ALPHABET	1
#define QR_MODE_8BIT		2
#define QR_MODE_KANJI		3

// バージョン(型番)グループ
#define QR_VRESION_S	0 // 1 ～ 9
#define QR_VRESION_M	1 // 10 ～ 26
#define QR_VRESION_L	2 // 27 ～ 40

#define MAX_ALLCODEWORD	 26 // 総コードワード数最大値
#define MAX_DATACODEWORD 19 // データコードワード最大値(バージョン40-L)
#define MAX_CODEBLOCK	  153 // ブロックデータコードワード数最大値(ＲＳコードワードを含む)
#define MAX_MODULESIZE	  21 // 一辺モジュール数最大値

// ビットマップ描画時マージン
#define QR_MARGIN	4


#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

/////////////////////////////////////////////////////////////////////////////
typedef struct tagRS_BLOCKINFO
{
	int ncRSBlock;		// ＲＳブロック数
	int ncAllCodeWord;	// ブロック内コードワード数
	int ncDataCodeWord;	// データコードワード数(コードワード数 - ＲＳコードワード数)

} RS_BLOCKINFO, *LPRS_BLOCKINFO;


/////////////////////////////////////////////////////////////////////////////
// QRコードバージョン(型番)関連情報

typedef struct tagQR_VERSIONINFO
{
	int nVersionNo;	   // バージョン(型番)番号(1～40)
	int ncAllCodeWord; // 総コードワード数

	// 以下配列添字は誤り訂正率(0 = L, 1 = M, 2 = Q, 3 = H)
	int ncDataCodeWord[4];	// データコードワード数(総コードワード数 - ＲＳコードワード数)

	int ncAlignPoint;	// アライメントパターン座標数
	int nAlignPoint[6];	// アライメントパターン中心座標

	RS_BLOCKINFO RS_BlockInfo1[4]; // ＲＳブロック情報(1)
	RS_BLOCKINFO RS_BlockInfo2[4]; // ＲＳブロック情報(2)

} QR_VERSIONINFO, *LPQR_VERSIONINFO;


/////////////////////////////////////////////////////////////////////////////
// CQR_Encode クラス

struct CQR_Encode
{
	int m_nLevel;		// 誤り訂正レベル
	int m_nVersion;		// バージョン(型番)
	bool m_bAutoExtent;	// バージョン(型番)自動拡張指定フラグ
	int m_nMaskingNo;	// マスキングパターン番号

	int m_nSymbleSize;
	unsigned char m_byModuleData[MAX_MODULESIZE][MAX_MODULESIZE]; // [x][y]
	// bit5:機能モジュール（マスキング対象外）フラグ
	// bit4:機能モジュール描画データ
	// bit1:エンコードデータ
	// bit0:マスク後エンコード描画データ
	// 20hとの論理和により機能モジュール判定、11hとの論理和により描画（最終的にはBOOL値化）

	int m_ncDataCodeWordBit; // データコードワードビット長
	unsigned char m_byDataCodeWord[MAX_DATACODEWORD]; // 入力データエンコードエリア

	int m_ncDataBlock;
	unsigned char m_byBlockMode[MAX_DATACODEWORD];
	int m_nBlockLength[MAX_DATACODEWORD];

	int m_ncAllCodeWord; // 総コードワード数(ＲＳ誤り訂正データを含む)
	unsigned char m_byAllCodeWord[MAX_ALLCODEWORD]; // 総コードワード算出エリア
	unsigned char m_byRSWork[MAX_CODEBLOCK]; // ＲＳコードワード算出ワーク


	bool (*EncodeData)(struct CQR_Encode *instance, int nLevel, int nVersion, bool bAutoExtent, int nMaskingNo, const unsigned char lpsSource[], int ncSource);

	int (*GetEncodeVersion)(struct CQR_Encode *instance, int nVersion, const unsigned char lpsSource[], int ncLength);
	bool (*EncodeSourceData)(struct CQR_Encode *instance, const unsigned char lpsSource[], int ncLength, int nVerGroup);

	int (*GetBitLength)(struct CQR_Encode *instance, unsigned char nMode, int ncData, int nVerGroup);

	int (*SetBitStream)(struct CQR_Encode *instance, int nIndex,  int wData, int ncData);

	bool (*IsNumeralData)(  unsigned char c);
	bool (*IsAlphabetData)(  unsigned char c);
	bool (*IsKanjiData)( unsigned char c1, unsigned char c2);

	unsigned char (*AlphabetToBinaly)( unsigned char c);
    int (*KanjiToBinaly)(int wc);

	void (*GetRSCodeWord)(unsigned char * lpbyRSWork, int ncDataCodeWord, int ncRSCodeWord);

	void (*FormatModule)(struct CQR_Encode *instance);

	void (*SetFunctionModule)(struct CQR_Encode *instance);
	void (*SetFinderPattern)(struct CQR_Encode *instance, int x, int y);
	void (*SetAlignmentPattern)(struct CQR_Encode *instance, int x, int y);
	void (*SetVersionPattern)(struct CQR_Encode *instance);
	void (*SetCodeWordPattern)(struct CQR_Encode *instance);
	void (*SetMaskingPattern)(struct CQR_Encode *instance, int nPatternNo);
	void (*SetFormatInfoPattern)(struct CQR_Encode *instance, int nPatternNo);
	int (*CountPenalty)(struct CQR_Encode *instance);
};

/////////////////////////////////////////////////////////////////////////////

extern struct CQR_Encode encoder;

#endif
