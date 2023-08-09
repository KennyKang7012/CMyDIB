#include "stdafx.h"
#include "CMyDIB.h"

CMyDIB::CMyDIB()
{
	m_pDib = NULL;
	m_pDibBits = NULL;
	m_pBIH = NULL;
	m_pPalette = NULL;

	DibInfo = NULL;
	DibBits = NULL;
	DibArry = NULL;
}

CMyDIB::~CMyDIB()
{
	if (m_pDib != NULL)
		delete[] m_pDib;

	if(DibInfo != NULL)
		delete DibInfo;

	if(DibBits != NULL)
		delete[] DibBits;

	if(DibArry != NULL)
		delete[] DibArry;
}

BOOL CMyDIB::Load(const char *pszFileName)
{
	CFile cf;
	CString cStr;
	cStr = pszFileName;
	if (!cf.Open(cStr.GetBuffer(), CFile::modeRead))
		return FALSE;

	DWORD dwDibSize;
	dwDibSize = (DWORD)cf.GetLength() - sizeof(BITMAPFILEHEADER);

	unsigned char *pDib = NULL;
	pDib = new unsigned char[dwDibSize];
	if (pDib == NULL)
		return FALSE;

	BITMAPFILEHEADER BFH;
	try {
		if (cf.Read(&BFH, sizeof(BITMAPFILEHEADER)) != sizeof(BITMAPFILEHEADER) || BFH.bfType != 0x4D42 || cf.Read(pDib, dwDibSize) != dwDibSize)
		{
			delete[] pDib;
			return FALSE;
		}
		//TRACE("BFH.bfType = %d / BFH.bfSize = %d / BFH.bfOffBits = %d\r\n", BFH.bfType, BFH.bfSize, BFH.bfOffBits);
	}
	catch (CFileException *e) {
		e->Delete();
		delete[] pDib;
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
			delete[] pLoogPal;
		}
	}
	cf.Close();
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
	cf.Close();
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
	cf.Close();
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
	DibInfo = (BITMAPINFO *)m_pBIH;
	return DibInfo;
}

void CMyDIB::Smooth(unsigned char **image_in, unsigned char **image_out, int nWidth, int nHeight)
{
	int i, j, buf;
	for (j = 1; j < nHeight - 1; j++) {
		for (i = 1; i < nWidth - 1; i++) {
			buf = (int)image_in[j - 1][i - 1] +
				  (int)image_in[j - 1][i] +
				  (int)image_in[j - 1][i + 1] +
			      (int)image_in[j][i - 1] +
				  (int)image_in[j][i] +
				  (int)image_in[j][i + 1] +
				  (int)image_in[j + 1][i - 1] +
				  (int)image_in[j + 1][i] +
				  (int)image_in[j + 1][i + 1];
				  image_out[j][i] = buf / 9;
		}
	}
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

BOOL CMyDIB::LoadBMPFile(const char *pszFileName)
{
	BITMAPFILEHEADER bmBFH;
	BITMAPINFOHEADER bmBIH;
	long      lSize;
	ULONGLONG ulHeadPos;
	int       iPaletteSize = 0;
	int       iRet;
	int       iBMPHeaderSize;

	CFile file;
	CString cStr;
	cStr = pszFileName;
	if (!file.Open(cStr.GetBuffer(), CFile::modeRead))
		return FALSE;

	// 讀取檔案指標位置
	ulHeadPos = file.GetPosition();
	
	// 讀取點陣圖 Bitmap File Header
	iRet = file.Read(&bmBFH, sizeof(BITMAPFILEHEADER));
	// 檢查點陣圖檔頭大小、點陣圖識別字串 
	if (iRet != sizeof(BITMAPFILEHEADER) ||
		bmBFH.bfType != 0x4d42)
	{
		// 傳回錯誤值
		return FALSE;
	}

	// 讀取點陣圖 Bitmap Info Header
	iRet = file.Read(&bmBIH, sizeof(BITMAPINFOHEADER));
	// 檢查點陣圖資訊檔頭大小
	if (iRet
		!= sizeof(BITMAPINFOHEADER))
	{
		// 傳回錯誤值
		return FALSE;
	}

	// 判斷 RGBQUAD 大小
	switch (bmBIH.biBitCount)
	{
	case 1:
		iPaletteSize = 2;
		break;
	case 4:
		iPaletteSize = 16;
		break;
	case 8:
		iPaletteSize = 256;
		break;
	}

	// 計算 BITMAPINFO 大小
	iBMPHeaderSize = sizeof(BITMAPINFOHEADER) + (iPaletteSize * sizeof(RGBQUAD));

	// 分配 BITMAPINFO 記憶體
	DibInfo = (BITMAPINFO*) new BYTE[iBMPHeaderSize];

	// 設定 Bitmap Info Header
	DibInfo->bmiHeader = bmBIH;

	// 倘若有使用調色盤
	if (iPaletteSize > 0)
	{
		// 讀取調色盤
		iRet = file.Read(&(DibInfo->bmiColors[0]),
			iPaletteSize * sizeof(RGBQUAD));
		// 檢查調色盤大小
		if (iRet != int(iPaletteSize * sizeof(RGBQUAD)))
		{
			// 釋放點陣圖資訊檔頭與調色盤
			delete[] DibInfo;
			// 清空點陣圖資訊檔頭與調色盤
			DibInfo = NULL;
			// 傳回錯誤值
			return FALSE;
		}
	}

	// 因為Bitmaip每一列(直列橫行)都必須被4整除，因此在這裡做正規化
	DWORD dwBytesPerLine;
	dwBytesPerLine = (DibInfo->bmiHeader.biWidth * DibInfo->bmiHeader.biBitCount + 7) / 8;
	dwBytesPerLine = (dwBytesPerLine + 3) / 4;
	dwBytesPerLine = dwBytesPerLine * 4;

	// 計算像素陣列大小
	lSize = dwBytesPerLine * DibInfo->bmiHeader.biHeight;

	// 分配像素陣列記憶體
	DibBits = (BYTE*) new BYTE[lSize];
	DibArry = (BYTE*) new BYTE[lSize];

	// 移動檔案指標到像素陣列
	file.Seek(ulHeadPos + bmBFH.bfOffBits, CFile::begin);

	// 由點陣圖檔案讀取像素資料到記憶體
	iRet = file.Read(DibBits, lSize);
	// 檢查讀取資料大小
	if (iRet != int(lSize))
	{
		// 倘若資料大小不正確
		delete[] DibInfo;
		delete[] DibBits;
		DibInfo = NULL;
		DibBits = NULL;
		// 傳回錯誤值
		return FALSE;
	}
	// 傳回正確值

	// 關閉檔案
	file.Close();
	return TRUE;
}

// 轉換點陣資料上下位置
BOOL CMyDIB::ExchangeBits(int iChange)
{
	long lIndex1 = 0;
	long lX1 = 0;
	long lY1 = 0;
	long lIndex2 = 0;
	long lX2 = 0;
	long lY2 = 0;

	// 處理
	try
	{
		// 將像素資料轉到暫存陣列中
		if (iChange == 1)
		{
			// 變數初始歸零
			lIndex1 = 0;
			lIndex2 = 0;
			lX1 = 0;
			lX2 = 0;
			lY1 = 0;
			lY2 = 0;
			// Y 軸由下往上
			for (lY1 = DibInfo->bmiHeader.biHeight - 1; lY1 >= 0; lY1--)
			{
				// X 軸由左往右
				for (lX1 = 0; lX1 < DibInfo->bmiHeader.biWidth; lX1++)
				{
					// 索引1由下往上
					lIndex1 = (lX1 * 3) + (lY1 * DibInfo->bmiHeader.biWidth * 3);
					// 索引2由上往下
					lIndex2 = (lX2 * 3) + (lY2 * DibInfo->bmiHeader.biWidth * 3);
					// 將資料轉為由上往下到暫存陣列中
					DibArry[lIndex2 + 0] = DibBits[lIndex1 + 0];
					DibArry[lIndex2 + 1] = DibBits[lIndex1 + 1];
					DibArry[lIndex2 + 2] = DibBits[lIndex1 + 2];
					// 暫存 X 軸由左往右
					lX2++;
				}
				// 暫存 X 軸歸零
				lX2 = 0;
				// 暫存 Y 軸由上往下
				lY2++;
			}
		}
		// 將暫存資料轉到像素陣列中
		else if (iChange == 2)
		{
			// 變數初始歸零
			lIndex1 = 0;
			lIndex2 = 0;
			lX1 = 0;
			lX2 = 0;
			lY1 = 0;
			lY2 = 0;
			// Y 軸由下往上
			for (lY1 = DibInfo->bmiHeader.biHeight - 1; lY1 >= 0; lY1--)
			{
				// X 軸由左往右
				for (lX1 = 0; lX1 < DibInfo->bmiHeader.biWidth; lX1++)
				{
					// 索引1由下往上
					lIndex1 = (lX1 * 3) + (lY1 * DibInfo->bmiHeader.biWidth * 3);
					// 索引2由上往下
					lIndex2 = (lX2 * 3) + (lY2 * DibInfo->bmiHeader.biWidth * 3);
					// 將暫存資料轉入像素資料中
					DibBits[lIndex2 + 0] = DibArry[lIndex1 + 0];
					DibBits[lIndex2 + 1] = DibArry[lIndex1 + 1];
					DibBits[lIndex2 + 2] = DibArry[lIndex1 + 2];
					// 暫存 X 軸由左往右
					lX2++;
				}
				// 暫存 X 軸歸零
				lX2 = 0;
				// 暫存 Y 軸由上往下
				lY2++;
			}
		}
	}
	// 錯誤處理
	catch (CException* e)
	{
		e->Delete();
		// 傳回錯誤值
		return FALSE;
	}
	// 傳回成功值
	return TRUE;
}

// 顯示點陣圖檔
BOOL CMyDIB::ShowBMPFile(CDC &cdc, CRect &crSrc, CRect &crDst)
{
	int iRet;

	// 檢查點陣圖檔頭資訊
	if (!DibInfo)
	{
		// 若有錯，則傳回錯誤
		return FALSE;
	}

	// 顯示可縮放點陣圖檔
	iRet = ::StretchDIBits(cdc.m_hDC,
		crDst.left,
		crDst.top,
		crDst.Width(),
		crDst.Height(),
		crSrc.left,
		crSrc.top,
		crSrc.Width(),
		crSrc.Height(),
		DibBits,
		DibInfo,
		DIB_RGB_COLORS,
		SRCCOPY);

	// 檢查是否有顯示
	if (!iRet)
	{
		// 傳回錯誤值
		return FALSE;
	}

	// 傳回正確值
	return TRUE;
}


BOOL CMyDIB::ShowBMPFile(CDC &cdc, CRect &crSrc, CPoint &cpDst)
{
	int iRet;

	// 檢查點陣圖檔頭資訊
	if (!DibInfo)
	{
		// 若有錯，則傳回錯誤
		return FALSE;
	}

	// 顯示點陣圖檔
	iRet = ::SetDIBitsToDevice (
		cdc.m_hDC,
		cpDst.x,
		cpDst.y,
		crSrc.Width(),
		crSrc.Height(),
		crSrc.left,
		crSrc.top,
		0,
		DibInfo->bmiHeader.biHeight,
		DibBits,
		DibInfo,
		DIB_RGB_COLORS );

	// 檢查是否有顯示
	if (!iRet)
	{
		// 傳回錯誤值
		return FALSE;
	}

	// 傳回正確值
	return TRUE;
}

BOOL CMyDIB::DrawBMPFile(CDC &cdc, int nX, int nY, int nWidth, int nHeight, CRect &crSrc, int mode)
{
	if (DibInfo == NULL || DibBits == NULL)
		return FALSE;

	StretchDIBits(cdc.m_hDC, nX, nY, nWidth, nHeight, 0, 0, DibInfo->bmiHeader.biWidth, DibInfo->bmiHeader.biHeight, DibBits, DibInfo, DIB_RGB_COLORS, mode);
	return TRUE;
}
