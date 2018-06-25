// QR_Encode.h : CQR_Encode �N���X�錾����уC���^�[�t�F�C�X��`
// Date 2006/05/17	Ver. 1.22	Psytec Inc.

#ifndef __QR_ENCODE_H__
#define __QR_ENCODE_H__

#include <stdbool.h>

typedef const char *LPCSTR;

/////////////////////////////////////////////////////////////////////////////
// �萔

// ���������x��

#define QR_LEVEL_L	0
#define QR_LEVEL_M	1
#define QR_LEVEL_Q	2
#define QR_LEVEL_H	3

// �f�[�^���[�h
#define QR_MODE_NUMERAL		0
#define QR_MODE_ALPHABET	1
#define QR_MODE_8BIT		2
#define QR_MODE_KANJI		3

// �o�[�W����(�^��)�O���[�v
#define QR_VRESION_S	0 // 1 �` 9
#define QR_VRESION_M	1 // 10 �` 26
#define QR_VRESION_L	2 // 27 �` 40

#define MAX_ALLCODEWORD	 26 // ���R�[�h���[�h���ő�l
#define MAX_DATACODEWORD 19 // �f�[�^�R�[�h���[�h�ő�l(�o�[�W����40-L)
#define MAX_CODEBLOCK	  153 // �u���b�N�f�[�^�R�[�h���[�h���ő�l(�q�r�R�[�h���[�h���܂�)
#define MAX_MODULESIZE	  21 // ��Ӄ��W���[�����ő�l

// �r�b�g�}�b�v�`�掞�}�[�W��
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
	int ncRSBlock;		// �q�r�u���b�N��
	int ncAllCodeWord;	// �u���b�N���R�[�h���[�h��
	int ncDataCodeWord;	// �f�[�^�R�[�h���[�h��(�R�[�h���[�h�� - �q�r�R�[�h���[�h��)

} RS_BLOCKINFO, *LPRS_BLOCKINFO;


/////////////////////////////////////////////////////////////////////////////
// QR�R�[�h�o�[�W����(�^��)�֘A���

typedef struct tagQR_VERSIONINFO
{
	int nVersionNo;	   // �o�[�W����(�^��)�ԍ�(1�`40)
	int ncAllCodeWord; // ���R�[�h���[�h��

	// �ȉ��z��Y���͌�������(0 = L, 1 = M, 2 = Q, 3 = H)
	int ncDataCodeWord[4];	// �f�[�^�R�[�h���[�h��(���R�[�h���[�h�� - �q�r�R�[�h���[�h��)

	int ncAlignPoint;	// �A���C�����g�p�^�[�����W��
	int nAlignPoint[6];	// �A���C�����g�p�^�[�����S���W

	RS_BLOCKINFO RS_BlockInfo1[4]; // �q�r�u���b�N���(1)
	RS_BLOCKINFO RS_BlockInfo2[4]; // �q�r�u���b�N���(2)

} QR_VERSIONINFO, *LPQR_VERSIONINFO;


/////////////////////////////////////////////////////////////////////////////
// CQR_Encode �N���X

struct CQR_Encode
{
	int m_nLevel;		// ���������x��
	int m_nVersion;		// �o�[�W����(�^��)
	bool m_bAutoExtent;	// �o�[�W����(�^��)�����g���w��t���O
	int m_nMaskingNo;	// �}�X�L���O�p�^�[���ԍ�

	int m_nSymbleSize;
	unsigned char m_byModuleData[MAX_MODULESIZE][MAX_MODULESIZE]; // [x][y]
	// bit5:�@�\���W���[���i�}�X�L���O�ΏۊO�j�t���O
	// bit4:�@�\���W���[���`��f�[�^
	// bit1:�G���R�[�h�f�[�^
	// bit0:�}�X�N��G���R�[�h�`��f�[�^
	// 20h�Ƃ̘_���a�ɂ��@�\���W���[������A11h�Ƃ̘_���a�ɂ��`��i�ŏI�I�ɂ�BOOL�l���j

	int m_ncDataCodeWordBit; // �f�[�^�R�[�h���[�h�r�b�g��
	unsigned char m_byDataCodeWord[MAX_DATACODEWORD]; // ���̓f�[�^�G���R�[�h�G���A

	int m_ncDataBlock;
	unsigned char m_byBlockMode[MAX_DATACODEWORD];
	int m_nBlockLength[MAX_DATACODEWORD];

	int m_ncAllCodeWord; // ���R�[�h���[�h��(�q�r�������f�[�^���܂�)
	unsigned char m_byAllCodeWord[MAX_ALLCODEWORD]; // ���R�[�h���[�h�Z�o�G���A
	unsigned char m_byRSWork[MAX_CODEBLOCK]; // �q�r�R�[�h���[�h�Z�o���[�N


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
