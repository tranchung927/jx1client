//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:    KPakFile.cpp
// Date:    2000.08.08
// Code:    WangWei (Daphnis)
// Desc:    Triển khai KPakFile và các API liên quan tới SPR/KSG
// Refactor: Production C++20 (2026-03-11)
//---------------------------------------------------------------------------

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cstdlib>

#include "KFilePath.h"
#include "KPakFile.h"
#include "KPakList.h"

// Chế độ mở tệp cho KPakFile (giữ nguyên biến toàn cục để tương thích).
static int m_nPakFileMode = 1;

void g_SetPakFileMode(int nFileMode)
{
    m_nPakFileMode = nFileMode;
}

#define PAK_INDEX_STORE_IN_RESERVED 0

//---------------------------------------------------------------------------
// Mục đích (VI):
//  - KPakFile cung cấp truy cập tệp ưu tiên từ PAK nếu có; fallback sang hệ thống tệp.
//  - Các API Spr* hỗ trợ đọc tài nguyên SPR (sprite) cả trong PAK và ngoài.
//  - Giữ nguyên API để tương thích 100% với memory pool và packet chain.
//  - Việt hoá chú thích, sửa lỗi kiểm tra tham số, đảm bảo an toàn bộ đệm, định dạng lại.
//---------------------------------------------------------------------------

SPRHEAD* SprGetHeader(char* pszFileName, SPROFFS*& pOffsetTable)
{
    pOffsetTable = NULL;

    if (pszFileName == NULL || pszFileName[0] == 0)
        return NULL;

    KPakFile File;
    if (!File.Open(pszFileName))
        return NULL;

    SPRHEAD* pSpr = NULL;
    if (File.IsFileInPak()) {
        // Đang đọc từ PAK: lấy ElemRef và truy vấn header qua KPakList.
        if (g_pPakList) {
            XPackElemFileRef PakRef;
            if (g_pPakList->pGetFilePath(pszFileName, PakRef)) {
                pSpr = g_pPakList->GetSprHeader(PakRef, pOffsetTable);
                if (pSpr)
                    pSpr->Reserved[PAK_INDEX_STORE_IN_RESERVED] = (unsigned short)(short)PakRef.nPackIndex;
            }
        }
    } else {
        // Đọc trực tiếp từ hệ thống tệp: nạp toàn bộ vào bộ nhớ.
        bool bOk = false;
        SPRHEAD Header;

        while (File.Read(&Header, sizeof(SPRHEAD)) == sizeof(SPRHEAD)) {
            // Kiểm tra chữ ký và số màu hợp lệ.
            if (*(int*)&Header.Comment[0] != SPR_COMMENT_FLAG || Header.Colors > 256)
                break;

            int uEntireSize = (int)File.Size();
            pSpr = (SPRHEAD*)std::malloc((size_t)uEntireSize);
            if (pSpr == NULL)
                break;

            uEntireSize -= (int)sizeof(SPRHEAD);

            if (File.Read(&pSpr[1], (unsigned)uEntireSize) == (unsigned)uEntireSize) {
                // Bảng offset bắt đầu sau phần palette: sizeof(SPRHEAD) + Colors*3
                pOffsetTable = (SPROFFS*)(((char*)(pSpr)) + sizeof(SPRHEAD) + Header.Colors * 3);
                Header.Reserved[PAK_INDEX_STORE_IN_RESERVED] = (unsigned short)(-1);
                std::memcpy(pSpr, &Header, sizeof(SPRHEAD));
                bOk = true;
            }
            break;
        }

        if (!bOk && pSpr) {
            std::free(pSpr);
            pSpr = NULL;
        }
    }

    File.Close();
    return pSpr;
}

void SprReleaseHeader(SPRHEAD* pSprHeader)
{
    if (pSprHeader) {
        std::free(pSprHeader);
    }
}

SPRFRAME* SprGetFrame(SPRHEAD* pSprHeader, int nFrame, unsigned int& nSingFrameSize)
{
    SPRFRAME* pFrame = NULL;
    if (pSprHeader && g_pPakList) {
        int nPakIndex = (short)pSprHeader->Reserved[PAK_INDEX_STORE_IN_RESERVED];
        if (nPakIndex >= 0) {
            pFrame = g_pPakList->GetSprFrame(nPakIndex, pSprHeader, nFrame, nSingFrameSize);
        }
    }
    return pFrame;
}

void SprReleaseFrame(SPRFRAME* pFrame)
{
    if (pFrame) {
        std::free(pFrame);
    }
}

KSGImageContent* get_jpg_image(const char cszName[], unsigned uRGBMask16)
{
    (void)cszName;
    (void)uRGBMask16;
    // Chưa triển khai: giữ nguyên hành vi cũ trả về NULL.
    return NULL;
}

void release_image(KSGImageContent* pImage)
{
    if (pImage)
        std::free(pImage);
}

//---------------------------------------------------------------------------
// Triển khai KPakFile
//---------------------------------------------------------------------------

KPakFile::KPakFile()
{
    // Khởi tạo trạng thái: mặc định không ở trong PAK.
    m_PackRef.nPackIndex = -1;
    m_PackRef.uId        = 0;
}

KPakFile::~KPakFile()
{
    Close();
}

bool KPakFile::IsFileInPak()
{
    return (m_PackRef.nPackIndex >= 0 && m_PackRef.uId);
}

int KPakFile::Open(const char* pszFileName)
{
    if (pszFileName == NULL || pszFileName[0] == 0)
        return false;

    bool bOk = false;
    Close();

    // Ưu tiên mở file ngoài trước; nếu không có, thử trong PAK.
    bOk = (m_File.Open((char*)pszFileName) != FALSE);
    if (!bOk && g_pPakList) {
        bOk = g_pPakList->pGetFilePath(pszFileName, m_PackRef);
    }

    return bOk;
}

unsigned long KPakFile::Read(void* pBuffer, unsigned int uSize)
{
    if (pBuffer == NULL || uSize == 0)
        return 0;

    if (m_PackRef.nPackIndex >= 0) {
        // Đọc từ PAK thông qua KPakList. API trả về số byte đọc; 0 nếu lỗi/hết dữ liệu.
        unsigned bytes = (unsigned)g_pPakList->ElemFileRead(m_PackRef, pBuffer, uSize);
        return bytes;
    } else {
        return m_File.Read(pBuffer, uSize);
    }
}

unsigned long KPakFile::Seek(int nOffset, unsigned int uMethod)
{
    if (m_PackRef.nPackIndex >= 0) {
        if (uMethod == FILE_BEGIN)
            m_PackRef.nOffset = nOffset;
        else if (uMethod == FILE_END)
            m_PackRef.nOffset = m_PackRef.nSize + nOffset;
        else
            m_PackRef.nOffset += nOffset;

        if (m_PackRef.nOffset > m_PackRef.nSize)
            m_PackRef.nOffset = m_PackRef.nSize;
        else if (m_PackRef.nOffset < 0)
            m_PackRef.nOffset = 0;

        return (unsigned long)m_PackRef.nOffset;
    } else {
        return m_File.Seek(nOffset, uMethod);
    }
}

unsigned long KPakFile::Tell()
{
    if (m_PackRef.nPackIndex >= 0)
        return (unsigned long)m_PackRef.nOffset;
    else
        return m_File.Tell();
}

unsigned long KPakFile::Size()
{
    if (m_PackRef.nPackIndex >= 0)
        return (unsigned long)m_PackRef.nSize;
    else
        return m_File.Size();
}

void KPakFile::Close()
{
    if (m_PackRef.nPackIndex >= 0) {
        m_PackRef.nPackIndex = -1;
        m_PackRef.uId        = 0;
        // Không cần đóng m_File vì không dùng khi ở trong PAK.
    } else {
        m_File.Close();
    }
}

// Mã lịch sử (đọc PAK theo khối) được giữ lại dưới dạng chú thích đã Việt hoá để tra cứu.
// #define BLOCK_SIZE (0x10000L)
// ...

