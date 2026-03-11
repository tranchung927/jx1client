/*****************************************************************************************
//    Đọc gói (pack file)
//    Bản quyền : Kingsoft 2003
//    Tác giả   : Wooy (Wu Yue)
//    Thời gian : 2003-09-16
//
*****************************************************************************************/

#include "XPackFile.h"
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include "ucl/ucl.h"

// Bộ nhớ đệm (cache) tĩnh cho các tệp con
XPackFile::XPackElemFileCache XPackFile::ms_ElemFileCache[MAX_XPACKFILE_CACHE];
int XPackFile::ms_nNumElemFileCache = 0;

XPackFile::XPackFile()
{
    m_hFile = NULL;
    m_uFileSize = 0;
    m_pIndexList = NULL;
    m_nElemFileCount = 0;  // Số lượng tệp con trong gói
    m_nSelfIndex = 0;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
    InitializeCriticalSection(&m_ReadCritical);
#else
    pthread_mutex_init(&m_ReadCritical, NULL);
#endif
    std::memset(ms_ElemFileCache, 0, sizeof(XPackElemFileCache) * MAX_XPACKFILE_CACHE);
}

XPackFile::~XPackFile()
{
    Close();
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
    DeleteCriticalSection(&m_ReadCritical);
#else
    pthread_mutex_destroy(&m_ReadCritical);
#endif
}

//-------------------------------------------------
// Chức năng: Mở tệp gói (pack)
// Trả về:    true nếu thành công
//-------------------------------------------------
bool XPackFile::Open(const char* pszPackFileName, int nSelfIndex)
{
    bool bResult = false;
    Close();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
    EnterCriticalSection(&m_ReadCritical);
#else
    pthread_mutex_lock(&m_ReadCritical);
#endif

    m_nSelfIndex = nSelfIndex;
    m_hFile = std::fopen(pszPackFileName, "rb");

    while (m_hFile)
    {
        if (std::fseek(m_hFile, 0, SEEK_END) != 0) break;
        long endPos = std::ftell(m_hFile);
        if (endPos <= 0) break;
        m_uFileSize = static_cast<unsigned int>(endPos);  // Kích thước tệp
        if (std::fseek(m_hFile, 0, SEEK_SET) != 0) break; // Trỏ về đầu tệp

        if (m_uFileSize <= sizeof(XPackFileHeader))
            break;

        XPackFileHeader Header{};
        size_t dwReaded = std::fread(&Header, 1, sizeof(Header), m_hFile);
        if (dwReaded != sizeof(Header))
            break;

        // Kiểm tra hợp lệ của header
        if (*(int*)(&Header.cSignature) != XPACKFILE_SIGNATURE_FLAG ||
            Header.uCount == 0 ||
            Header.uIndexTableOffset < sizeof(XPackFileHeader) ||
            Header.uIndexTableOffset >= m_uFileSize ||
            Header.uDataOffset < sizeof(XPackFileHeader) ||
            Header.uDataOffset >= m_uFileSize)
        {
            break;
        }

        // Đọc bảng chỉ mục
        size_t dwListSize = sizeof(XPackIndexInfo) * Header.uCount;
        m_pIndexList = static_cast<XPackIndexInfo*>(std::malloc(dwListSize));
        if (m_pIndexList == NULL)
            break;

        if (std::fseek(m_hFile, static_cast<long>(Header.uIndexTableOffset), SEEK_SET) != 0)
            break;

        dwReaded = std::fread(m_pIndexList, 1, dwListSize, m_hFile);
        if (dwReaded != dwListSize)
            break;

        m_nElemFileCount = Header.uCount;  // Số lượng tệp con
        bResult = true;
        break;
    }

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
    LeaveCriticalSection(&m_ReadCritical);
#else
    pthread_mutex_unlock(&m_ReadCritical);
#endif

    if (bResult == false)
        Close();

    fileName = std::string(pszPackFileName);
    return bResult;
}

//-------------------------------------------------
// Chức năng: Đóng gói đang mở, dọn tài nguyên liên quan
//-------------------------------------------------
void XPackFile::Close()
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
    EnterCriticalSection(&m_ReadCritical);
#else
    pthread_mutex_lock(&m_ReadCritical);
#endif

    if (m_pIndexList)
    {
        // Giải phóng các mục cache thuộc về gói này
        for (int i = ms_nNumElemFileCache - 1; i >= 0; --i)
        {
            if (ms_ElemFileCache[i].nPackIndex == m_nSelfIndex)
            {
                FreeElemCache(i);
                ms_nNumElemFileCache--;
                for (int j = i; j < ms_nNumElemFileCache; ++j)
                    ms_ElemFileCache[j] = ms_ElemFileCache[j + 1];
            }
        }
        std::free(m_pIndexList);
        m_pIndexList = NULL;
    }
    m_nElemFileCount = 0;

    if (m_hFile)
    {
        std::fclose(m_hFile);
        m_hFile = NULL;
    }
    m_uFileSize = 0;

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
    LeaveCriticalSection(&m_ReadCritical);
#else
    pthread_mutex_unlock(&m_ReadCritical);
#endif
}

//-------------------------------------------------
// Chức năng: Giải phóng một mục cache
// Trả về:    void
//-------------------------------------------------
void XPackFile::FreeElemCache(int nCacheIndex)
{
    if (nCacheIndex < 0 || nCacheIndex >= ms_nNumElemFileCache) return;
    if (ms_ElemFileCache[nCacheIndex].pBuffer)
    {
        // Ghi chú: Bộ nhớ của pBuffer do nơi khác quản lý (pool), không free ở đây
        ms_ElemFileCache[nCacheIndex].pBuffer = NULL;
    }
    ms_ElemFileCache[nCacheIndex].uId = 0;
    ms_ElemFileCache[nCacheIndex].lSize = 0;
    ms_ElemFileCache[nCacheIndex].uRefFlag = 0;
    ms_ElemFileCache[nCacheIndex].nPackIndex = -1;
}

//-------------------------------------------------
// Chức năng: Đọc trực tiếp dữ liệu thô tại offset/size chỉ định
// Trả về:    true nếu đọc đủ uSize byte
//-------------------------------------------------
bool XPackFile::DirectRead(void* pBuffer, unsigned int uOffset, unsigned int uSize) const
{
    bool bResult = false;
    if (pBuffer == NULL || !m_hFile) return false;

    if (uOffset > m_uFileSize || uSize > m_uFileSize || uOffset + uSize > m_uFileSize)
        return false;

    if (std::fseek(m_hFile, static_cast<long>(uOffset), SEEK_SET) != 0)
        return false;

    size_t dwReaded = std::fread(pBuffer, 1, uSize, m_hFile);
    if (dwReaded == uSize)
        bResult = true;

    return bResult;
}

//-------------------------------------------------
// Chức năng: Đọc và giải nén dữ liệu theo kiểu nén
// Tham số:
//   pBuffer       -> bộ đệm đích
//   uExtractSize  -> kích thước sau giải nén mong đợi
//   lCompressType -> kiểu nén
//   uOffset       -> offset trong gói
//   uSize         -> kích thước dữ liệu nén
// Trả về: true nếu giải nén thành công và kích thước khớp uExtractSize
//-------------------------------------------------
bool XPackFile::ExtractRead(void* pBuffer, unsigned int uExtractSize,
                            int lCompressType, unsigned int uOffset, unsigned int uSize) const
{
    if (!pBuffer) return false;
    bool bResult = false;

    if (lCompressType == TYPE_NONE)
    {
        // Không nén: đọc trực tiếp
        if (uExtractSize == uSize)
            bResult = DirectRead(pBuffer, uOffset, uSize);
    }
    else
    {
        void* pReadBuffer = std::malloc(uSize);
        if (pReadBuffer)
        {
            if ((lCompressType == TYPE_UCL || lCompressType == TYPE_UCL_OLD) &&
                DirectRead(pReadBuffer, uOffset, uSize))
            {
                unsigned int uDestLength = 0;
                ucl_nrv2b_decompress_8((BYTE*)pReadBuffer, uSize, (BYTE*)pBuffer, &uDestLength, NULL);
                bResult = (uDestLength == uExtractSize);
            }
            else if ((lCompressType == TYPE_FRAGMENT_OLD) && DirectRead(pReadBuffer, uOffset, uSize))
            {
                unsigned int uDestLength = 0;
                ucl_nrv2b_decompress_8((BYTE*)pReadBuffer, uSize, (BYTE*)pBuffer, &uDestLength, NULL);
                bResult = (uDestLength == uExtractSize);
            }
            else if ((lCompressType == TYPE_FRAGMENTA_OLD) && DirectRead(pReadBuffer, uOffset, uSize))
            {
                unsigned int uDestLength = 0;
                ucl_nrv2b_decompress_8((BYTE*)pReadBuffer, uSize, (BYTE*)pBuffer, &uDestLength, NULL);
                bResult = (uDestLength == uExtractSize);
            }
            else
            {
                if (DirectRead(pReadBuffer, uOffset, uSize))
                {
                    unsigned int uDestLength = 0;
                    // Sử dụng hàm giải nén tương thích cũ
                    ucl_decompress_8_New((BYTE*)pReadBuffer, uSize, (BYTE*)pBuffer, &uDestLength);
                    bResult = (uDestLength == uExtractSize);
                }
            }
            std::free(pReadBuffer);
        }
    }
    return bResult;
}

// Các typedef phục vụ các thuật toán giải nén giữ nguyên như cũ
typedef unsigned char       uint8;
#define _BYTE  uint8
#define BYTEn(x, n)   (*((_BYTE*)&(x)+n))
#define BYTE1(x)      BYTEn(x,  1)

// Các hàm giải nén sau đây giữ nguyên thuật toán/chi tiết để đảm bảo tương thích nhị phân
// Đồng thời bổ sung chú thích tiếng Việt để dễ hiểu.

// Giải nén UCL (biến thể New) - giữ nguyên thuật toán gốc
int XPackFile::ucl_decompress_8_New(unsigned char* a1, unsigned int a2, unsigned char* a3, unsigned int* a4) const
{
    BYTE* v4; int v5; int v6; int v7; int v8; int v9; int v10; int v11; unsigned int v12; int v13; int v14; int v15; int v16; int v17; int v18; int v19; int v20; int v21; int v22; int v23; BYTE* v24; int v25; int result; int v27;
    v4 = a1; v5 = 0; v27 = 1; v6 = 0;
    while (1) {
        while (1) {
            if (v5 & 0x7F) { v5 *= 2; }
            else { v7 = *v4++; v5 = 2 * v7 + 1; }
            if (!(v5 & 0x0100)) break;
            *(v6++ + a3) = *v4++;
        }
        v8 = 1;
        do {
            if (v5 & 0x7F) { v9 = 2 * v5; }
            else { v10 = *v4++; v9 = 2 * v10 + 1; }
            v8 = ((v9 & 0x0100) >> 8) + 2 * v8;
            if (v9 & 0x7F) { v5 = 2 * v9; }
            else { v11 = *v4++; v5 = 2 * v11 + 1; }
        } while (!(v5 & 0x0100));
        if (v8 == 2) { v12 = v27; goto LABEL_19; }
        v13 = *v4++ + ((v8 + 0x00fffffd) << 8);
        if (v13 == 0xffffffff) break;
        v12 = v13 + 1; v27 = v12;
LABEL_19:
        if (v5 & 0x7F) { v14 = 2 * v5; }
        else { v15 = *v4++; v14 = 2 * v15 + 1; }
        v16 = (v14 & 0x0100) >> 8;
        if (v14 & 0x7F) { v5 = 2 * v14; }
        else { v17 = *v4++; v5 = 2 * v17 + 1; }
        v18 = ((v5 & 0x0100) >> 8) + 2 * v16;
        if (!v18) {
            v19 = 1;
            do {
                if (v5 & 0x7F) { v20 = 2 * v5; }
                else { v21 = *v4++; v20 = 2 * v21 + 1; }
                v19 = ((v20 & 0x0100) >> 8) + 2 * v19;
                if (v20 & 0x7F) { v5 = 2 * v20; }
                else { v22 = *v4++; v5 = 2 * v22 + 1; }
            } while (!(v5 & 0x0100));
            v18 = v19 + 2;
        }
        if (v12 > 0x0D00) ++v18;
        v23 = v6++ - v12; *(v6 + a3 - 1) = *(v23 + a3); v24 = (v23 + a3 + 1);
        do { *(v6++ + a3) = *v24++; --v18; } while (v18);
    }
    v25 = v4 - a1; *a4 = v6; if (a2 <= (unsigned)v25) { if (a2 == (unsigned)v25) result = 0; else result = (a2 >= (unsigned)v25) - 1; } else { result = 1; }
    return result;
}

int XPackFile::ucl_nrv2e_decompress_8_2(unsigned char* a1, unsigned int a2, unsigned char* a3, unsigned int* a4) const
{
    BYTE* v4; int v5; unsigned int v6; int v7; int v8; int v9; int v10; int v11; int v12; int v13; int v14; int v15; int v16; int v17; unsigned int v18; int v19; int v20; int v21; int v22; int v23; int v24; int v25; BYTE* v26; int v27; char v28; BYTE* v29; int v30; int result; int v32; int v33; unsigned int v34; int v35;
    v4 = a1; v5 = 0; v6 = 0; v7 = 0; v35 = 1;
    while (1) {
        while (1) {
            if (v5 & 0x7F) { v5 *= 2; } else { v8 = *(v6++ + v4); v5 = 2 * v8 + 1; }
            if (!(v5 & 0x100)) break; *(v7++ + a3) = *(v6++ + v4);
        }
        v33 = v7; v9 = 1; while (1) {
            if (v5 & 0x7F) { v10 = 2 * v5; } else { v11 = *(v6++ + v4); v10 = 2 * v11 + 1; }
            v12 = ((v10 & 0x100) >> 8) + 2 * v9; if (v10 & 0x7F) { v13 = 2 * v10; } else { v14 = *(v6++ + v4); v13 = 2 * v14 + 1; }
            if (v13 & 0x100) break; if (v13 & 0x7F) { v5 = 2 * v13; v9 = (((unsigned int)(2 * v13) >> 8) & 1) + 2 * v12 - 2; } else { v15 = *(v6++ + v4); v5 = 2 * v15 + 1; v9 = ((v5 & 0x100) >> 8) + 2 * v12 - 2; }
        }
        if (v12 == 2) { v34 = v35; if (v13 & 0x7F) { v13 *= 2; v16 = (v13 & 0x100) >> 8; } else { v17 = *(v6++ + v4); v13 = 2 * v17 + 1; v16 = (v13 & 0x100) >> 8; } goto LABEL_24; }
        v18 = ((v12 + 0x00fffffd) << 8) + *(v6++ + v4); if (v18 == 0xffffffff) break; v16 = ~(BYTE)v18 & 0x100; v34 = (v18 >> 1) + 1; v35 = (v18 >> 1) + 1;
LABEL_24:
        if (v13 & 0x7F) { v5 = 2 * v13; } else { v19 = *(v6++ + v4); v5 = 2 * v19 + 1; }
        v20 = ((v5 & 0x100) >> 8) + 2 * v16; if (!v20) { v21 = 1; do { if (v5 & 0x7F) { v22 = 2 * v5; } else { v23 = *(v6++ + v4); v22 = 2 * v23 + 1; } v21 = ((v22 & 0x100) >> 8) + 2 * v21; if (v22 & 0x7F) { v5 = 2 * v22; } else { v24 = *(v6++ + v4); v5 = 2 * v24 + 1; } } while (!(v5 & 0x100)); v20 = v21 + 2; }
        v25 = (v34 > 0x500) + v20; v26 = (a3 + v33 - v34); v27 = v33 + 1; v28 = *v26; v29 = v26 + 1; *(a3 + v33 + 1 - 1) = v28; do { *(a3 + v27++) = *v29++; --v25; } while (v25); v30 = v27; v4 = a1; v7 = v30;
    }
    *a4 = v7; if (v6 == a2) { result = 0; } else { v32 = -(v6 < a2); result = v32; } return result;
}

int XPackFile::ucl_nrv2e_decompress_8_1(unsigned char* a1, unsigned int a2, unsigned char* a3, unsigned int* a4) const
{
    unsigned int v4; int v5; int v6; int v7; int v8; int v9; int v10; unsigned int v11; int v12; int v13; int v14; int v15; int v16; int v17; int v18; int v19; int v20; int v21; int v22; unsigned char* v23; int v24; char v25; unsigned char* v26; int result; int v28; int v29; int v30;
    v4 = 0; v5 = 0; v29 = 0; v30 = 1;
    while (1) {
        while (1) {
            if (v5 & 0x7F) { v5 *= 2; } else { v6 = *(v4++ + a1); v5 = 2 * v6 + 1; }
            if (!(v5 & 0x100)) break; *(v29++ + a3) = *(v4++ + a1);
        }
        v7 = 1; do { if (v5 & 0x7F) { v8 = 2 * v5; } else { v9 = *(v4++ + a1); v8 = 2 * v9 + 1; } v7 = ((v8 & 0x100) >> 8) + 2 * v7; if (v8 & 0x7F) { v5 = 2 * v8; } else { v10 = *(v4++ + a1); v5 = 2 * v10 + 1; } } while (!(v5 & 0x100));
        if (v7 == 2) { v11 = v30; goto LABEL_19; }
        v12 = ((v7 + 16777213) << 8) + *(v4++ + a1); if (v12 == -1) break; v11 = v12 + 1; v30 = v11;
LABEL_19:
        if (v5 & 0x7F) { v13 = 2 * v5; } else { v14 = *(v4++ + a1); v13 = 2 * v14 + 1; }
        v15 = (v13 & 0x100) >> 8; if (v13 & 0x7F) { v5 = 2 * v13; } else { v16 = *(v4++ + a1); v5 = 2 * v16 + 1; }
        v17 = ((v5 & 0x100) >> 8) + 2 * v15; if (!v17) { v18 = 1; do { if (v5 & 0x7F) { v19 = 2 * v5; } else { v20 = *(v4++ + a1); v19 = 2 * v20 + 1; } v18 = ((v19 & 0x100) >> 8) + 2 * v18; if (v19 & 0x7F) { v5 = 2 * v19; } else { v21 = *(v4++ + a1); v5 = 2 * v21 + 1; } } while (!(v5 & 0x100)); v17 = v18 + 2; }
        v22 = (v11 > 0xD00) + v17; v23 = (a3 + v29 - v11); v24 = v29 + 1; v25 = *v23; v26 = v23 + 1; *(v29 + 1 + a3 - 1) = v25; do { *(v24++ + a3) = *v26++; --v22; } while (v22); v29 = v24;
    }
    *a4 = v29; if (v4 == a2) { result = 0; } else { v28 = -(v4 < a2); result = v28; } return result;
}

// Giải nén UCL biến thể fs (giữ nguyên thuật toán)
int XPackFile::ucl_nrv2e_decompress_8_fs(unsigned char* s_buf, int s_size, unsigned char* d_buf, unsigned int* d_size) const
{
    unsigned int ch = 0;
    unsigned int repeat_pri = 1;   // Giá trị REPEAT lần trước
    unsigned int repeat_cur;       // Giá trị REPEAT hiện tại
    unsigned int repeat_cnt;       // Số lần lặp
    unsigned int d_cnt = 0;        // Byte đã ghi vào đích
    unsigned char* bak_s_buf;      // Sao lưu con trỏ nguồn

    bak_s_buf = s_buf;
    do {
        do {
            if ((ch & 0x7f) != 0) ch <<= 1; else ch = ((*(s_buf++)) << 1) + 1;
            if ((ch & 0x100) == 0) break; *(d_buf + d_cnt++) = *(s_buf++);
        } while (1);
        repeat_cur = 1;
        do {
            if ((ch & 0x7f) != 0) ch <<= 1; else ch = ((*(s_buf++)) << 1) + 1;
            repeat_cur <<= 1; repeat_cur += (ch & 0x100) >> 8;
            if ((ch & 0x7f) != 0) ch <<= 1; else ch = ((*(s_buf++)) << 1) + 1;
        } while ((ch & 0x100) == 0);
        if (repeat_cur == 2) repeat_cur = repeat_pri; else {
            repeat_cur += 0x00fffffd; repeat_cur <<= 8; repeat_cur += *(s_buf++);
            if (repeat_cur == 0xffffffff) break; repeat_cur++; repeat_pri = repeat_cur;
        }
        if ((ch & 0x7f) != 0) ch <<= 1; else ch = ((*(s_buf++)) << 1) + 1;
        repeat_cnt = (ch & 0x100) >> 8;
        if ((ch & 0x7f) != 0) ch <<= 1; else ch = ((*(s_buf++)) << 1) + 1;
        repeat_cnt <<= 1; repeat_cnt += (ch & 0x100) >> 8;
        if (repeat_cnt == 0) {
            repeat_cnt = 1;
            do {
                if ((ch & 0x7f) != 0) ch <<= 1; else ch = ((*(s_buf++)) << 1) + 1;
                repeat_cnt <<= 1; repeat_cnt += (ch & 0x100) >> 8;
                if ((ch & 0x7f) != 0) ch <<= 1; else ch = ((*(s_buf++)) << 1) + 1;
            } while ((ch & 0x100) == 0);
            repeat_cnt += 2;
        }
        if (repeat_cur > 0x0d00) repeat_cnt++;
        repeat_cur = d_cnt - repeat_cur;
        *(d_buf + d_cnt++) = *(d_buf + repeat_cur++);
        do { *(d_buf + d_cnt++) = *(d_buf + repeat_cur++); repeat_cnt--; } while (repeat_cnt != 0);
    } while (1);
    *d_size = d_cnt;
    if (s_size > (int)(s_buf - bak_s_buf)) return 1;
    if (s_size == (int)(s_buf - bak_s_buf)) return 0;
    if (s_size < (int)(s_buf - bak_s_buf)) return -1;
    return 0;
}

//-------------------------------------------------
// Chức năng: Tìm tệp con theo ID (nhị phân)
// Trả về:   vị trí (>=0) nếu thấy, -1 nếu không
//-------------------------------------------------
int XPackFile::XFindElemFileA(DWORD ulId) const
{
    int nBegin = 0, nEnd = m_nElemFileCount - 1, nMid = -1;
    while (nBegin <= nEnd)
    {
        nMid = (nBegin + nEnd) / 2;
        if (ulId < m_pIndexList[nMid].uId) nEnd = nMid - 1;
        else if (ulId > m_pIndexList[nMid].uId) nBegin = nMid + 1;
        else break;
    }
    return ((nBegin <= nEnd) ? nMid : -1);
}

//-------------------------------------------------
// Chức năng: Tìm tệp con trong gói
// Tham số:   uId -> ID tệp con; ElemRef -> điền thông tin nếu thấy
// Trả về:    true nếu tìm thấy
//-------------------------------------------------
bool XPackFile::XFindElemFile(DWORD uId, XPackElemFileRef& ElemRef)
{
    ElemRef.nElemIndex = -1;
    if (uId)
    {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
        EnterCriticalSection(&m_ReadCritical);
#else
        pthread_mutex_lock(&m_ReadCritical);
#endif
        ElemRef.nCacheIndex = FindElemFileInCache(uId, -1);  // Kiểm tra cache trước
        if (ElemRef.nCacheIndex >= 0)
        {
            ElemRef.uId = uId;
            ElemRef.nPackIndex = ms_ElemFileCache[ElemRef.nCacheIndex].nPackIndex;
            ElemRef.nElemIndex = ms_ElemFileCache[ElemRef.nCacheIndex].nElemIndex;
            ElemRef.nSize = ms_ElemFileCache[ElemRef.nCacheIndex].lSize;
            ElemRef.nOffset = 0;
        }
        else
        {
            ElemRef.nElemIndex = XFindElemFileA(uId);
            if (ElemRef.nElemIndex >= 0)
            {
                ElemRef.uId = uId;
                ElemRef.nPackIndex = m_nSelfIndex;
                ElemRef.nOffset = 0;
                ElemRef.nSize = m_pIndexList[ElemRef.nElemIndex].lSize;
            }
        }
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
        LeaveCriticalSection(&m_ReadCritical);
#else
        pthread_mutex_unlock(&m_ReadCritical);
#endif
    }
    return (ElemRef.nElemIndex >= 0);
}

//-------------------------------------------------
// Chức năng: Đọc toàn bộ dữ liệu của một tệp con (trả về buffer)
// Trả về:    con trỏ dữ liệu hoặc NULL nếu lỗi
//-------------------------------------------------
void* XPackFile::ReadElemFile(int nElemIndex) const
{
    if (nElemIndex < 0 || nElemIndex >= m_nElemFileCount)
        return NULL;

    void* pDataBuffer = std::malloc(m_pIndexList[nElemIndex].lSize);
    if (pDataBuffer)
    {
        // Lấy kiểu nén và kích thước nén từ cờ
        int lCompressType = m_pIndexList[nElemIndex].lCompressSizeFlag & TYPE_FILTER_OLD;
        unsigned int uSize = m_pIndexList[nElemIndex].lCompressSizeFlag & (~TYPE_FILTER_OLD);

        if (ExtractRead(pDataBuffer,
                        m_pIndexList[nElemIndex].lSize,
                        lCompressType,
                        m_pIndexList[nElemIndex].uOffset,
                        uSize) == false)
        {
            std::free(pDataBuffer);
            pDataBuffer = NULL;
        }
    }
    return pDataBuffer;
}

//-------------------------------------------------
// Chức năng: Tìm một tệp con trong cache theo ID
// Trả về:    vị trí trong cache (>=0) hoặc -1 nếu không thấy
//-------------------------------------------------
int XPackFile::FindElemFileInCache(unsigned int uId, int nDesireIndex)
{
    if (nDesireIndex >= 0 && nDesireIndex < ms_nNumElemFileCache &&
        uId == ms_ElemFileCache[nDesireIndex].uId)
    {
        ms_ElemFileCache[nDesireIndex].uRefFlag = 0xffffffff;
        return nDesireIndex;
    }

    nDesireIndex = -1;
    for (int i = 0; i < ms_nNumElemFileCache; ++i)
    {
        if (uId == ms_ElemFileCache[i].uId)
        {
            ms_ElemFileCache[i].uRefFlag = 0xffffffff;
            nDesireIndex = i;
            break;
        }
    }
    return nDesireIndex;
}

//-------------------------------------------------
// Chức năng: Thêm dữ liệu tệp con vào cache
// Trả về:    vị trí cache được dùng
//-------------------------------------------------
int XPackFile::AddElemFileToCache(void* pBuffer, int nElemIndex)
{
    if (!pBuffer || nElemIndex < 0 || nElemIndex >= m_nElemFileCount) return 0;

    int nCacheIndex;
    if (ms_nNumElemFileCache < MAX_XPACKFILE_CACHE)
    {
        // Lấy một ô trống
        nCacheIndex = ms_nNumElemFileCache++;
    }
    else
    {
        // Giải phóng một mục ít được tham chiếu nhất
        nCacheIndex = 0;
        if (ms_ElemFileCache[0].uRefFlag) ms_ElemFileCache[0].uRefFlag--;
        for (int i = 1; i < MAX_XPACKFILE_CACHE; ++i)
        {
            if (ms_ElemFileCache[i].uRefFlag) ms_ElemFileCache[i].uRefFlag--;
            if (ms_ElemFileCache[i].uRefFlag < ms_ElemFileCache[nCacheIndex].uRefFlag)
                nCacheIndex = i;
        }
        FreeElemCache(nCacheIndex);
    }

    ms_ElemFileCache[nCacheIndex].pBuffer   = pBuffer;
    ms_ElemFileCache[nCacheIndex].uId       = m_pIndexList[nElemIndex].uId;
    ms_ElemFileCache[nCacheIndex].lSize     = m_pIndexList[nElemIndex].lSize;
    ms_ElemFileCache[nCacheIndex].nPackIndex= m_nSelfIndex;
    ms_ElemFileCache[nCacheIndex].nElemIndex= nElemIndex;
    ms_ElemFileCache[nCacheIndex].uRefFlag  = 0xffffffff;
    return nCacheIndex;
}

//-------------------------------------------------
// Chức năng: Đọc dữ liệu từ tệp con theo kích thước yêu cầu
// Trả về:    số byte đọc được
//-------------------------------------------------
int XPackFile::XElemFileRead(XPackElemFileRef& ElemRef, void* pBuffer, unsigned uSize)
{
    int nResult = 0;
    if (pBuffer && ElemRef.uId && ElemRef.nElemIndex >= 0)
    {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
        EnterCriticalSection(&m_ReadCritical);
#else
        pthread_mutex_lock(&m_ReadCritical);
#endif
        // Kiểm tra cache trước
        ElemRef.nCacheIndex = FindElemFileInCache(ElemRef.uId, ElemRef.nCacheIndex);

        if (ElemRef.nCacheIndex < 0 &&
            ElemRef.nElemIndex < m_nElemFileCount &&
            m_pIndexList[ElemRef.nElemIndex].uId == ElemRef.uId)
        {
            void* pDataBuffer = ReadElemFile(ElemRef.nElemIndex);
            if (pDataBuffer)
            {
                ElemRef.nCacheIndex = AddElemFileToCache(pDataBuffer, ElemRef.nElemIndex);
            }
        }

        if (ElemRef.nCacheIndex >= 0 &&
            ElemRef.nPackIndex == ms_ElemFileCache[ElemRef.nCacheIndex].nPackIndex &&
            ElemRef.nElemIndex == ms_ElemFileCache[ElemRef.nCacheIndex].nElemIndex &&
            ElemRef.nSize == ms_ElemFileCache[ElemRef.nCacheIndex].lSize)
        {
            if (ElemRef.nOffset < 0) ElemRef.nOffset = 0;
            if (ElemRef.nOffset < ElemRef.nSize)
            {
                if (ElemRef.nOffset + static_cast<int>(uSize) <= ElemRef.nSize)
                    nResult = static_cast<int>(uSize);
                else
                    nResult = ElemRef.nSize - ElemRef.nOffset;
                std::memcpy(pBuffer, (char*)ms_ElemFileCache[ElemRef.nCacheIndex].pBuffer + ElemRef.nOffset, nResult);
                ElemRef.nOffset += nResult;
            }
            else
            {
                ElemRef.nOffset = ElemRef.nSize;
            }
        }
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
        LeaveCriticalSection(&m_ReadCritical);
#else
        pthread_mutex_unlock(&m_ReadCritical);
#endif
    }
    return nResult;
}

#define NODE_INDEX_STORE_IN_RESERVED 2

//-------------------------------------------------
// Chức năng: Lấy header của spr và bảng offset khung hình
//-------------------------------------------------
SPRHEAD* XPackFile::GetSprHeader(XPackElemFileRef& ElemRef, SPROFFS*& pOffsetTable)
{
    SPRHEAD* pSpr = NULL;
    bool bOk = false;

    pOffsetTable = NULL;
    if (ElemRef.uId == 0 || ElemRef.nElemIndex < 0)
        return NULL;

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
    EnterCriticalSection(&m_ReadCritical);
#else
    pthread_mutex_lock(&m_ReadCritical);
#endif

    if (ElemRef.nElemIndex < m_nElemFileCount && m_pIndexList[ElemRef.nElemIndex].uId == ElemRef.uId)
    {
        // Kiểm tra xem có phải nén theo khung (frame) không
        if ((m_pIndexList[ElemRef.nElemIndex].lCompressSizeFlag & TYPE_FRAME) == 0)
        {
            // Không nén theo khung: đọc toàn bộ spr
            pSpr = (SPRHEAD*)ReadElemFile(ElemRef.nElemIndex);
            if (pSpr)
            {
                if ((*(int*)&pSpr->Comment[0]) == SPR_COMMENT_FLAG)
                {
                    pOffsetTable = (SPROFFS*)(((char*)pSpr) + sizeof(SPRHEAD) + pSpr->Colors * 3);
                    bOk = true;
                }
            }
        }
        else
        {
            // Nén theo khung: đọc trực tiếp header và bảng offset
            SPRHEAD Header;
            if (DirectRead(&Header, m_pIndexList[ElemRef.nElemIndex].uOffset, sizeof(SPRHEAD)))
            {
                if (*(int*)&(Header.Comment[0]) == SPR_COMMENT_FLAG)
                {
                    unsigned int u2ListSize = Header.Colors * 3 + Header.Frames * sizeof(XPackSprFrameInfo);
                    pSpr = (SPRHEAD*)std::malloc(sizeof(SPRHEAD) + u2ListSize);
                    if (pSpr)
                    {
                        if (DirectRead((char*)(&pSpr[1]), m_pIndexList[ElemRef.nElemIndex].uOffset + sizeof(SPRHEAD), u2ListSize))
                        {
                            std::memcpy(pSpr, &Header, sizeof(SPRHEAD));
                            bOk = true;
                        }
                    }
                }
            }
        }

        if (pSpr)
        {
            if (bOk)
            {
                *((int*)&pSpr->Reserved[NODE_INDEX_STORE_IN_RESERVED]) = ElemRef.nElemIndex;
            }
            else
            {
                std::free(pSpr);
                pSpr = NULL;
            }
        }
    }

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
    LeaveCriticalSection(&m_ReadCritical);
#else
    pthread_mutex_unlock(&m_ReadCritical);
#endif

    return pSpr;
}

//-------------------------------------------------
// Chức năng: Lấy một khung hình nén từ spr theo chỉ số khung
//-------------------------------------------------
SPRFRAME* XPackFile::GetSprFrame(SPRHEAD* pSprHeader, int nFrame, unsigned int& nSingFrameSize)
{
    SPRFRAME* pFrame = NULL;
    if (pSprHeader && nFrame >= 0 && nFrame < pSprHeader->Frames)
    {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
        EnterCriticalSection(&m_ReadCritical);
#else
        pthread_mutex_lock(&m_ReadCritical);
#endif
        int nNodeIndex = *((int*)&pSprHeader->Reserved[NODE_INDEX_STORE_IN_RESERVED]);
        if (nNodeIndex >= 0 && nNodeIndex < m_nElemFileCount)
        {
            int lCompressType = m_pIndexList[nNodeIndex].lCompressSizeFlag;
            if ((lCompressType & TYPE_FRAME) != 0) // Nén theo nhiều khung (frame)
            {
                bool bOk = false;
                lCompressType &= TYPE_METHOD_FILTER_OLD;  // Giữ lại phần chỉ phương pháp nén
                int lTempValue = sizeof(SPRHEAD) + pSprHeader->Colors * 3;
                XPackSprFrameInfo* pFrameList = (XPackSprFrameInfo*)((char*)pSprHeader + lTempValue);
                unsigned int uSrcOffset = m_pIndexList[nNodeIndex].uOffset + lTempValue + pSprHeader->Frames * sizeof(XPackSprFrameInfo);
                while (nFrame > 0)
                {
                    uSrcOffset += (pFrameList++)->lCompressSize; // Cộng dồn offset đến khung cần lấy
                    nFrame--;
                }
                lTempValue = pFrameList->lSize;

                if (lTempValue < 0)
                {
                    lTempValue = -lTempValue;
                    pFrame = (SPRFRAME*)std::malloc(lTempValue);
                    if (pFrame)
                        bOk = DirectRead(pFrame, uSrcOffset, (unsigned)lTempValue);
                    if (bOk) nSingFrameSize = (unsigned)lTempValue;
                }
                else
                {
                    pFrame = (SPRFRAME*)std::malloc(lTempValue);
                    if (pFrame)
                    {
                        bOk = ExtractRead(pFrame, (unsigned)lTempValue, lCompressType, uSrcOffset, (unsigned)pFrameList->lCompressSize);
                        if (bOk) nSingFrameSize = (unsigned)lTempValue;
                    }
                }

                if (bOk == false && pFrame != NULL)
                {
                    std::free(pFrame);
                    pFrame = NULL;
                    showAlert("GetSprFrame--failed", "ExtractRead");
                }
            }
        }
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
        LeaveCriticalSection(&m_ReadCritical);
#else
        pthread_mutex_unlock(&m_ReadCritical);
#endif
    }

    if (pFrame == NULL)
        showAlert("pFrame IS NULL", "GetSprFrame");

    return pFrame;
}
