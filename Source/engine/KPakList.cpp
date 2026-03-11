//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:    KPakList.cpp
// Date:    2000.08.08
// Code:    WangWei (Daphnis)
// Desc:    Pack Data List Class
// Refactor: Production C++20 (2026-03-11)
//---------------------------------------------------------------------------

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>
#include <utility>

#include "KStrBase.h"
#include "KFilePath.h"
#include "KIniFile.h"
#include "KPakList.h"
#include "KGbktoUtf8.h"

// Biến toàn cục giữ thể hiện quản lý PAK (giữ nguyên để tương thích).
KPakList* g_pPakList = NULL;

KPakList::KPakList()
{
    g_pPakList   = this;
    m_nPakNumber = 0;
    // Đảm bảo mảng con trỏ được khởi tạo NULL an toàn.
    for (int i = 0; i < MAX_PAK; ++i) {
        m_PakFilePtrList[i] = NULL;
    }
}

// Huỷ: đóng tài nguyên.
KPakList::~KPakList()
{
    Close();
}

// Đóng toàn bộ PAK và dọn dẹp tài nguyên.
void KPakList::Close()
{
    for (int i = 0; i < m_nPakNumber; ++i) {
        if (m_PakFilePtrList[i]) {
            delete m_PakFilePtrList[i];
            m_PakFilePtrList[i] = NULL;
        }
    }
    m_nPakNumber = 0;
}

// Tìm kiếm tệp con theo ID trên toàn bộ PAK.
bool KPakList::pFindElemFileA(unsigned int uId, XPackElemFileRef& ElemRef)
{
    bool bFounded = false;
    for (int i = 0; i < m_nPakNumber; ++i) {
        if (m_PakFilePtrList[i] && m_PakFilePtrList[i]->XFindElemFile(uId, ElemRef)) {
            bFounded = true;
            break;
        }
    }
    return bFounded;
}

// Chuyển tên file thành ID (hàm băm) theo quy ước lịch sử của engine.
unsigned int KPakList::FileNameToId(const char* pszFileName)
{
    // Sửa lỗi: điều kiện kiểm tra null/empty sai trong bản cũ.
    if (!pszFileName || !pszFileName[0])
        return 0;

    unsigned int id = 0;
    const char* ptr = pszFileName;
    int index = 0;
    while (*ptr) {
        if (*ptr >= 'A' && *ptr <= 'Z')
            id = (id + (++index) * (*ptr + 'a' - 'A')) % 0x8000000b * 0xffffffef;
        else
            id = (id + (++index) * (*ptr)) % 0x8000000b * 0xffffffef;
        ++ptr;
    }
    return static_cast<unsigned int>(id ^ 0x12345678);
}

// Tìm kiếm tệp con theo tên.
bool KPakList::pFindElemFile(const char* pszFileName, XPackElemFileRef& ElemRef)
{
    bool bFounded = false;
    if (pszFileName && pszFileName[0]) {
        char szPackName[256];
        std::memset(szPackName, 0, sizeof(szPackName));

        // Dùng ký tự phân tách chuẩn theo engine: phiên bản cũ luôn '\\' ở đầu.
        szPackName[0] = '\\';

        // Hợp nhất đường dẫn, loại bỏ "..", chuyển về chữ thường (do g_GetPackPath xử lý).
        g_GetPackPath(szPackName + 1, const_cast<char*>(pszFileName));

        // Tạo ID từ tên chuẩn hoá và tìm.
        unsigned int uId = g_FileName2Id(szPackName);
        bFounded = pFindElemFileA(uId, ElemRef);
    }
    return bFounded;
}

// Mở danh sách PAK từ file ini.
int KPakList::Open(char* pPakListFile, int nKind)
{
    (void)nKind; // Giữ tham số để tương thích API; việc sử dụng phụ thuộc triển khai XPackFile.

    Close();

    KIniFile IniFile;
    constexpr const char* SECTION = "Package";

    if (!pPakListFile || !pPakListFile[0]) {
        IniFile.Clear();
        return 0;
    }

    if (!IniFile.Load(pPakListFile)) {
        IniFile.Clear();
        return 0;
    }

    char szBuffer[32] = {0};
    char szKey[32] = {0};
    char szFile[MAX_PATH] = {0};

    if (IniFile.GetString(SECTION, "Path", "\\data", szBuffer, sizeof(szBuffer))) {
#ifndef WIN32
        // Bản cũ nối thêm "data" trên non-Windows; giữ hành vi cũ để tương thích.
        std::strcat(szBuffer, "data");
#endif
        g_GetFullPath(szFile, szBuffer);

        int nNameStartPos = static_cast<int>(std::strlen(szFile));
        // Sửa lỗi logic: điều kiện cũ dùng OR khiến luôn thêm ký tự phân tách.
        if (nNameStartPos > 0 && (szFile[nNameStartPos - 1] != '\\' && szFile[nNameStartPos - 1] != '/')) {
#ifdef WIN32
            szFile[nNameStartPos++] = '\\';
#else
            szFile[nNameStartPos++] = '/';
#endif
            szFile[nNameStartPos] = 0;
        }

        for (int i = 0; i < MAX_PAK; ++i) {
            std::snprintf(szKey, sizeof(szKey), "%d", i);

            if (!IniFile.GetString(SECTION, szKey, "", szBuffer, sizeof(szBuffer)))
                continue;

            if (szBuffer[0] == 0)
                continue;

            // Chú ý giới hạn bộ đệm
            std::strncpy(szFile + nNameStartPos, szBuffer, sizeof(szFile) - static_cast<size_t>(nNameStartPos) - 1);
            szFile[sizeof(szFile) - 1] = '\0';

            if (m_nPakNumber >= MAX_PAK)
                break; // Tránh tràn mảng

            m_PakFilePtrList[m_nPakNumber] = new XPackFile; // cấp phát theo thiết kế cũ

            if (m_PakFilePtrList[m_nPakNumber]) {
                if (m_PakFilePtrList[m_nPakNumber]->Open(szFile, m_nPakNumber)) {
                    ++m_nPakNumber;
                } else {
                    delete m_PakFilePtrList[m_nPakNumber];
                    m_PakFilePtrList[m_nPakNumber] = NULL;
                }
            }
        }
    }

    IniFile.Clear();
    return m_nPakNumber;
}

// Đọc tệp con theo tham chiếu ElemRef vào bộ đệm.
int KPakList::ElemFileRead(XPackElemFileRef& ElemRef, void* pBuffer, unsigned uSize)
{
    if (ElemRef.nPackIndex >= 0 && ElemRef.nPackIndex < m_nPakNumber && m_PakFilePtrList[ElemRef.nPackIndex])
        return m_PakFilePtrList[ElemRef.nPackIndex]->XElemFileRead(ElemRef, pBuffer, uSize);
    return 0;
}

// Lấy header SPR và bảng offset.
SPRHEAD* KPakList::GetSprHeader(XPackElemFileRef& ElemRef, SPROFFS*& pOffsetTable)
{
    if (ElemRef.nPackIndex >= 0 && ElemRef.nPackIndex < m_nPakNumber && m_PakFilePtrList[ElemRef.nPackIndex])
        return m_PakFilePtrList[ElemRef.nPackIndex]->GetSprHeader(ElemRef, pOffsetTable);
    return NULL;
}

// Lấy một frame đã giải nén từ SPR.
SPRFRAME* KPakList::GetSprFrame(int nPackIndex, SPRHEAD* pSprHeader, int nFrame, unsigned int& nSingFrameSize)
{
    if (nPackIndex >= 0 && nPackIndex < m_nPakNumber && m_PakFilePtrList[nPackIndex])
        return m_PakFilePtrList[nPackIndex]->GetSprFrame(pSprHeader, nFrame, nSingFrameSize);
    return NULL;
}

// Lấy đường dẫn thực tế của tệp nếu có, thông qua nhiều phép thử mã hoá tên.
bool KPakList::pGetFilePath(const char* pszFileName, XPackElemFileRef& ElemRef)
{
    bool bFounded = false;
    if (pszFileName && pszFileName[0]) {
        char szPackName[256];
        std::memset(szPackName, 0, sizeof(szPackName));

        szPackName[0] = '\\';
        g_GetPackPath(szPackName + 1, const_cast<char*>(pszFileName)); // hợp nhất đường dẫn

        unsigned int uId = 0;
        // Thử với các biến thể mã hoá chuỗi: GBK->UTF8 và UTF8->GBK, sau cùng là nguyên bản.
        uId = g_CheckFileExist(U2G(szPackName).c_str());
        bFounded = pFindElemFileA(uId, ElemRef);
        if (!bFounded) {
            uId = g_CheckFileExist(G2U(szPackName).c_str());
            bFounded = pFindElemFileA(uId, ElemRef);
            if (!bFounded) {
                uId = g_CheckFileExist(szPackName);
                bFounded = pFindElemFileA(uId, ElemRef);
            }
        }
    }
    return bFounded;
}
