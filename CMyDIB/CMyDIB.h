#pragma once
#ifndef __CMYDIB_H__
#define __CMYDIB_H__
#include <wingdi.h>

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
};
#endif	//!__CMYDIB_H__
