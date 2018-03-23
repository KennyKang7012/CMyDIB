#include "stdafx.h"
#include "CMyDIB.h"
//#include "Windows.h"

CMyDIB::CMyDIB()
{
	m_pDib = NULL;
	m_pDibBits = NULL;
	m_pBIH = NULL;
	m_pPalette = NULL;
}

CMyDIB::~CMyDIB()
{
	if (m_pDib != NULL)
		delete [] m_pDib;
}

BOOL CMyDIB::Load(const char *pszFileName)
{
	CFile cf;
	CString cStr;
	cStr = pszFileName;
	if (!cf.Open(cStr.GetBuffer(), CFile::modeRead))
		return FALSE;

	DWORD dwDibSize;
	dwDibSize = (DWORD) cf.GetLength() - sizeof(BITMAPFILEHEADER);
	
	unsigned char *pDib = NULL;
	pDib = new unsigned char[dwDibSize];
	if (pDib == NULL)
		return FALSE;

	BITMAPFILEHEADER BFH;
	try {
		if (cf.Read(&BFH, sizeof(BITMAPFILEHEADER)) != sizeof(BITMAPFILEHEADER) || BFH.bfType != 0x4D42 || cf.Read(pDib, dwDibSize) != dwDibSize)
		{
			delete [] pDib;
			return FALSE;
		}
		//TRACE("BFH.bfType = %d / BFH.bfSize = %d / BFH.bfOffBits = %d\r\n", BFH.bfType, BFH.bfSize, BFH.bfOffBits);
	}
	catch (CFileException *e) {
		e->Delete();
		delete [] pDib;
		return FALSE;
	}

	if (m_pDib != NULL)
		delete m_pDib;

	m_pDib = pDib;
	m_dwDibSize = dwDibSize;

	//Bitmap Info Header
	m_pBIH = (BITMAPINFOHEADER *)m_pDib;
	m_pPalette = (RGBQUAD *)&m_pDib[sizeof(BITMAPINFOHEADER)];
	
	m_Width = m_pBIH->biWidth;
	m_Height = m_pBIH->biHeight;
	m_BitCount = 1 << m_pBIH->biBitCount;
	if (m_BitCount  > 256)
		m_nPaletteEntries = 0;
	else
		m_nPaletteEntries = m_BitCount;

	m_pDibBits = &m_pDib[sizeof(BITMAPINFOHEADER) + m_nPaletteEntries * sizeof(RGBQUAD)];

	if (m_Palette.GetSafeHandle() != NULL)
		m_Palette.DeleteObject();

	if (m_nPaletteEntries != 0) {
		LOGPALETTE *pLoogPal = (LOGPALETTE *) new char[sizeof(LOGPALETTE) + m_nPaletteEntries * sizeof(PALETTEENTRY)];
		if (pLoogPal != NULL) {
			pLoogPal->palVersion = 0x300;
			pLoogPal->palNumEntries = m_nPaletteEntries;
			for (int i = 0; i < m_nPaletteEntries; i++) {
				pLoogPal->palPalEntry[i].peRed = m_pPalette[i].rgbRed;
				pLoogPal->palPalEntry[i].peGreen = m_pPalette[i].rgbGreen;
				pLoogPal->palPalEntry[i].peBlue = m_pPalette[i].rgbBlue;
			}
			m_Palette.CreatePalette(pLoogPal);
			delete [] pLoogPal;
		}
	}
	return TRUE;
}

BOOL CMyDIB::Save(const char *pszFileName)
{
	if (m_pDib == NULL)
		return FALSE;

	CFile cf;
	CString cStr;
	cStr = pszFileName;
	if (!cf.Open(cStr.GetBuffer(), CFile::modeCreate | CFile::modeWrite))
		return FALSE;

	try {
		BITMAPFILEHEADER BFH;
		memset(&BFH, 0, sizeof(BITMAPFILEHEADER));
		BFH.bfType = 0x4D42;
		BFH.bfSize = sizeof(BITMAPFILEHEADER) + m_dwDibSize; //14 + 26664 = 26678
		BFH.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + m_nPaletteEntries * sizeof(RGBQUAD);//1078

#if 0
		cf.Write(&BFH, sizeof(BITMAPFILEHEADER));
		cf.Write(m_pDib, m_dwDibSize);
#else
		cf.Write(&BFH, sizeof(BITMAPFILEHEADER));
		cf.Write(m_pBIH, sizeof(BITMAPINFOHEADER));
		cf.Write(m_pPalette, m_nPaletteEntries * sizeof(RGBQUAD));
		cf.Write(m_pDibBits, BFH.bfSize - BFH.bfOffBits);
#endif
	}
	catch (CFileException *e) {
		e->Delete();
		return FALSE;
	}
	return TRUE;
}

BOOL CMyDIB::Save(const char *pszFileName, unsigned char *pData)
{
	if (m_pDib == NULL)
		return FALSE;

	CFile cf;
	CString cStr;
	cStr = pszFileName;
	if (!cf.Open(cStr.GetBuffer(), CFile::modeCreate | CFile::modeWrite))
		return FALSE;

	try {
		BITMAPFILEHEADER BFH;
		memset(&BFH, 0, sizeof(BITMAPFILEHEADER));
		BFH.bfType = 0x4D42;
		BFH.bfSize = sizeof(BITMAPFILEHEADER) + m_dwDibSize; //14 + 26664 = 26678
		BFH.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + m_nPaletteEntries * sizeof(RGBQUAD);//1078

		cf.Write(&BFH, sizeof(BITMAPFILEHEADER));
		cf.Write(m_pBIH, sizeof(BITMAPINFOHEADER));
		cf.Write(m_pPalette, m_nPaletteEntries * sizeof(RGBQUAD));
		cf.Write(pData, BFH.bfSize - BFH.bfOffBits);
	}
	catch (CFileException *e) {
		e->Delete();
		return FALSE;
	}
	return TRUE;
}

BOOL CMyDIB::Draw(CDC *pDC, int nX, int nY, int nWidth, int nHeight, int mode)
{
	if (m_pDib == NULL)
		return FALSE;

	if (nWidth == -1)
		nWidth = m_pBIH->biWidth;
	if (nHeight == -1)
		nHeight = m_pBIH->biHeight;

	StretchDIBits(pDC->m_hDC, nX, nY, nWidth, nHeight, 0, 0, m_pBIH->biWidth, m_pBIH->biHeight, m_pDibBits, (BITMAPINFO *)m_pBIH, BI_RGB, mode);
	return TRUE;
}

BOOL CMyDIB::Draw(CDC *pDC, unsigned char *pDibBits, BITMAPINFO *pBIH, int nX, int nY, int nWidth, int nHeight, int mode)
{
	if (m_pDib == NULL || pDibBits == NULL || pBIH == NULL)
		return FALSE;

	if (nWidth == -1)
		nWidth = pBIH->bmiHeader.biWidth;
	if (nHeight == -1)
		nHeight = pBIH->bmiHeader.biHeight;

	StretchDIBits(pDC->m_hDC, nX, nY, nWidth, nHeight, 0, 0, pBIH->bmiHeader.biWidth, pBIH->bmiHeader.biHeight, pDibBits, pBIH, BI_RGB, mode);
	return TRUE;
}

BOOL CMyDIB::SetPalette(CDC *pDC)
{
	if (m_pDib == NULL)
		return FALSE;

	if (m_Palette.GetSafeHandle() == NULL)
		return TRUE;

	CPalette *pOldPalette;
	pOldPalette = pDC->SelectPalette(&m_Palette, FALSE);
	pDC->RealizePalette();
	pDC->SelectPalette(pOldPalette, FALSE);
	return TRUE;
}

BOOL CMyDIB::Getsize(long *lWidth, long *lHeight)
{
	if (m_pDib == NULL)
		return FALSE;

	*lWidth = m_pBIH->biWidth;
	*lHeight = m_pBIH->biHeight;
	return TRUE;
}

void CMyDIB::GetDibBits(unsigned char *pOutImgData, int nWidth, int nHeight, bool bExchangeBits)
{
	if (pOutImgData == NULL || nWidth == 0 || nHeight == 0)
		return;
	if (m_pDibBits == NULL)
		return;

	int j, i;
	for (j = 0; j < nHeight; j++) {
		for (i = 0; i < nWidth; i++) {
			if (bExchangeBits == false)
				pOutImgData[j * nWidth + i] = m_pDibBits[j * nWidth + i];//直接將Bitmap Image Data輸出
			else
				pOutImgData[j * nWidth + i] = m_pDibBits[(nHeight - 1 - j) * nWidth + i];//將Bitmap Image Data做上下資料的轉換
		}
	}
}

BITMAPINFO *CMyDIB::GetBITMAPINFO(void)
{
	if (m_pDib == NULL || m_pBIH == NULL)
		return NULL;

	BITMAPINFO *DibInfo;
	DibInfo = (BITMAPINFO *) m_pBIH;
	return DibInfo;
}

void CMyDIB::Save2TXT(char *filename, unsigned char *pImgData, int nWidth, int nHeight)
{
	if (filename == NULL || pImgData == NULL || nWidth == 0 || nHeight == 0)
		return;

	int i, j;
	char str[7] = { 0 };
	FILE *file = fopen(filename, "w");
	if (!file) {
		return;
	}

	for (j = 0; j < nHeight; j++) {
		for (i = 0; i < nWidth; i++) {
			if (i == nWidth - 1) {
				sprintf(str, "%3d,\n", pImgData[j * nWidth + i]);
			}
			else {
				sprintf(str, "%3d,", pImgData[j * nWidth + i]);
			}
			fwrite(str, 1, strlen(str), file);
		}
	}
	fclose(file);
}
