#pragma once
#ifndef __CMYDIB_H__
#define __CMYDIB_H__
#include <wingdi.h>
#include <afxwin.h>

class CMyDIB
{
public:
	CMyDIB();
	~CMyDIB();

	BOOL Load(const char *pszFileName);
	BOOL Save(const char *pszFileName);
	BOOL Save(const char *pszFileName, unsigned char *pData);
	BOOL Draw(CDC *, int nX = 0, int nY = 0, int nWidth = -1, int nHeight = -1, int mode = SRCCOPY);
	BOOL Draw(CDC *pDC, unsigned char *pDibBits, BITMAPINFO *pBIH, int nX = 0, int nY = 0, int nWidth = -1, int nHeight = -1, int mode = SRCCOPY);
	BOOL SetPalette(CDC *);
	BOOL Getsize(long *lWidth, long *lHeight);
	void GetDibBits(unsigned char *pOutImgData, int nWidth, int nHeight, bool bExchangeBits = false);
	BITMAPINFO  *GetBITMAPINFO(void);
	void Save2TXT(char *filename, unsigned char *pImgData, int nWidth, int nHeight);
	void Smooth(unsigned char **image_in, unsigned char **image_out, int nWidth, int nHeight);

	// ���J�I�}���ɮ�
	BOOL LoadBMPFile(const char *pszFileName);
	// �ഫ�I�}��ƤW�U��m
	BOOL ExchangeBits(int iChange);
	// ��ܥi�Y���I�}����
	BOOL ShowBMPFile(CDC &cdc, CRect &crSrc, CRect &crDst);
	BOOL ShowBMPFile(CDC &cdc, CRect &crSrc, CPoint &cpDst);
	BOOL DrawBMPFile(CDC &cdc, int nX, int nY, int nWidth, int nHeight, CRect &crSrc, int mode = SRCCOPY);

public:
	CPalette m_Palette;
	unsigned char *m_pDib;
	unsigned char *m_pDibBits;
	DWORD m_dwDibSize;
	BITMAPINFOHEADER *m_pBIH;
	RGBQUAD *m_pPalette;
	int m_nPaletteEntries;
	int m_BitCount;
	int	m_Width;
	int	m_Height;

	// ���Y
	BITMAPINFO  *DibInfo;
	// �����}�C
	BYTE		*DibBits;
	// �Ȧs�}�C
	BYTE		*DibArry;
};
#endif	//!__CMYDIB_H__
