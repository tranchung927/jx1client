//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:    KFile.cpp
// Date:    2000.08.08 (updated to C++20 cross-platform)
// Code:    WangWei(Daphnis) / Refactor: 2026
//---------------------------------------------------------------------------

#include "KPlatform.h"
#include "KFilePath.h"
#include "KFile.h"

#include <cstdio>
#include <cstring>
#include <cctype>
#include <string>
#include <string_view>
#include <utility>

// Trợ giúp (VI): chuyển xâu sang chữ thường an toàn (không sử dụng strlwr không chuẩn)
static void to_lower_inplace(char* s) noexcept {
    if (!s) return;
    for (char* p = s; *p; ++p) {
        unsigned char ch = static_cast<unsigned char>(*p);
        *p = static_cast<char>(std::tolower(ch));
    }
}

// Trợ giúp (VI): thay thế ký tự '\\' bằng '/'
static void backslash_to_slash_inplace(char* s) noexcept {
    if (!s) return;
    for (char* p = s; *p; ++p) {
        if (*p == '\\') *p = '/';
    }
}

//---------------------------------------------------------------------------
// Tên:     KFile
// Mô tả:   Hàm tạo
//---------------------------------------------------------------------------
KFile::KFile()
{
    m_hFile = NULL;
    m_dwLen = 0;
    m_dwPos = 0;
}

//---------------------------------------------------------------------------
// Tên:     ~KFile
// Mô tả:   Hàm hủy
//---------------------------------------------------------------------------
KFile::~KFile()
{
    Close();
}

//---------------------------------------------------------------------------
// Tên:     Open
// Mô tả:   Mở một tệp để đọc
// Tham số: FileName - tên tệp
// Trả về:  TRUE nếu thành công, FALSE nếu thất bại
//---------------------------------------------------------------------------
BOOL KFile::Open(LPCSTR FileName)
{
    char PathName[MAXPATH] = {0};

    if (m_hFile != NULL)
        Close();

    if (FileName == NULL || FileName[0] == 0)
        return FALSE;

    // Lấy đường dẫn đầy đủ
    g_GetFullPath(PathName, FileName);

#ifndef WIN32
    // Thống nhất phân tách đường dẫn và hạ chữ thường theo logic cũ trên *nix
    backslash_to_slash_inplace(PathName);

    char lcasePathName[MAXPATH] = {0};
    char szRootPath[MAXPATH] = {0};
    g_GetRootPath(szRootPath);

    std::strncpy(lcasePathName, PathName, sizeof(lcasePathName) - 1);

    size_t rootLen = std::strlen(szRootPath);
    if (rootLen > 0 && std::memcmp(lcasePathName, szRootPath, rootLen) == 0) {
        to_lower_inplace(lcasePathName + rootLen);
    } else {
        to_lower_inplace(lcasePathName);
    }

    m_hFile = std::fopen(lcasePathName, "rb");
    if (m_hFile == NULL) {
        // Thử lại với đường dẫn gốc nếu bản chữ thường thất bại
        m_hFile = std::fopen(PathName, "rb");
    }
#else
    // Windows: giữ nguyên đường dẫn, dùng chế độ nhị phân đọc
    m_hFile = std::fopen(PathName, "rb");
#endif

    if (m_hFile == NULL)
        return FALSE;

    // Đặt vị trí ban đầu về 0 và lấy độ dài file nếu có thể.
    m_dwPos = 0;
    long endPos = 0;
    if (std::fseek((FILE*)m_hFile, 0, SEEK_END) == 0) {
        endPos = std::ftell((FILE*)m_hFile);
        std::fseek((FILE*)m_hFile, 0, SEEK_SET);
    }
    m_dwLen = (endPos > 0) ? static_cast<DWORD>(endPos) : 0;
    return TRUE;
}

//---------------------------------------------------------------------------
// Tên:     Create
// Mô tả:   Tạo một tệp để ghi (ghi đè nếu tồn tại)
// Tham số: FileName - tên tệp
// Trả về:  TRUE nếu thành công, FALSE nếu thất bại
//---------------------------------------------------------------------------
BOOL KFile::Create(LPCSTR FileName)
{
    char PathName[MAXPATH] = {0};

    if (m_hFile != NULL)
        Close();

    if (FileName == NULL || FileName[0] == 0)
        return FALSE;

    g_GetFullPath(PathName, FileName);

    m_hFile = std::fopen(PathName, "wb+");
    if (m_hFile == NULL)
        return FALSE;

    m_dwPos = 0;
    m_dwLen = 0;
    return TRUE;
}

//---------------------------------------------------------------------------
// Tên:     Append
// Mô tả:   Mở tệp để ghi nối tiếp (tạo mới nếu chưa tồn tại)
// Tham số: FileName - tên tệp
// Trả về:  TRUE nếu thành công, FALSE nếu thất bại
//---------------------------------------------------------------------------
BOOL KFile::Append(LPCSTR FileName)
{
    char PathName[MAXPATH] = {0};

    if (m_hFile != NULL)
        Close();

    if (FileName == NULL || FileName[0] == 0)
        return FALSE;

    g_GetFullPath(PathName, FileName);

    // Dùng "ab+" để có thể vừa append vừa Seek/Read nếu cần
    m_hFile = std::fopen(PathName, "ab+");
    if (m_hFile == NULL)
        return FALSE;

    // Đặt con trỏ về cuối tệp để nối tiếp.
    if (std::fseek((FILE*)m_hFile, 0, SEEK_END) == 0) {
        long pos = std::ftell((FILE*)m_hFile);
        m_dwPos = (pos >= 0) ? static_cast<DWORD>(pos) : 0;
        m_dwLen = (pos >= 0) ? static_cast<DWORD>(pos) : 0;
    } else {
        m_dwPos = 0;
        m_dwLen = 0;
    }
    return TRUE;
}

//---------------------------------------------------------------------------
// Tên:     Close
// Mô tả:   Đóng tệp đang mở
//---------------------------------------------------------------------------
void KFile::Close()
{
    if (m_hFile) {
        std::fclose((FILE*)m_hFile);
    }
    m_hFile = NULL;
    m_dwLen = 0;
    m_dwPos = 0;
}

//---------------------------------------------------------------------------
// Tên:     Read
// Mô tả:   Đọc dữ liệu từ tệp
// Tham số: lpBuffer - bộ đệm nhận dữ liệu
//          dwReadBytes - số byte cần đọc
// Trả về:  Số byte đã đọc, 0 nếu lỗi
//---------------------------------------------------------------------------
DWORD KFile::Read(LPVOID lpBuffer, DWORD dwReadBytes)
{
    if (m_hFile == NULL || lpBuffer == NULL || dwReadBytes == 0)
        return 0;

    size_t n = std::fread(lpBuffer, 1, dwReadBytes, (FILE*)m_hFile);
    m_dwPos += static_cast<DWORD>(n);
    return static_cast<DWORD>(n);
}

//---------------------------------------------------------------------------
// Tên:     Write
// Mô tả:   Ghi dữ liệu ra tệp
// Tham số: lpBuffer - bộ đệm nguồn
//          dwWriteBytes - số byte cần ghi
// Trả về:  Số byte đã ghi, 0 nếu lỗi
//---------------------------------------------------------------------------
DWORD KFile::Write(LPCVOID lpBuffer, DWORD dwWriteBytes)
{
    if (m_hFile == NULL || lpBuffer == NULL || dwWriteBytes == 0)
        return 0;

    size_t n = std::fwrite(lpBuffer, 1, dwWriteBytes, (FILE*)m_hFile);
    m_dwPos += static_cast<DWORD>(n);
    if (m_dwPos > m_dwLen)
        m_dwLen = m_dwPos;
    // Có thể vô hiệu hoá bộ đệm nếu cần đảm bảo flush tức thì: std::fflush((FILE*)m_hFile);
    return static_cast<DWORD>(n);
}

//---------------------------------------------------------------------------
// Tên:     Seek
// Mô tả:   Di chuyển vị trí con trỏ tệp
// Tham số: lDistance - khoảng dịch
//          dwMoveMethod - FILE_BEGIN/FILE_CURRENT/FILE_END (tương ứng SEEK_SET/CUR/END)
// Trả về:  Vị trí mới nếu thành công, SEEK_ERROR nếu lỗi
//---------------------------------------------------------------------------
DWORD KFile::Seek(LONG lDistance, DWORD dwMoveMethod)
{
    if (m_hFile == NULL)
        return SEEK_ERROR;

    int origin;
    switch (dwMoveMethod) {
        case FILE_BEGIN:   origin = SEEK_SET; break;
        case FILE_CURRENT: origin = SEEK_CUR; break;
        case FILE_END:     origin = SEEK_END; break;
        default:           origin = SEEK_SET; break;
    }

    if (std::fseek((FILE*)m_hFile, static_cast<long>(lDistance), origin) != 0) {
        return SEEK_ERROR;
    }

    long pos = std::ftell((FILE*)m_hFile);
    if (pos < 0)
        return SEEK_ERROR;

    m_dwPos = static_cast<DWORD>(pos);
    return m_dwPos;
}

//---------------------------------------------------------------------------
// Tên:     Tell
// Mô tả:   Lấy vị trí hiện tại của con trỏ tệp
// Trả về:  Vị trí hiện tại, SEEK_ERROR nếu lỗi
//---------------------------------------------------------------------------
DWORD KFile::Tell()
{
    if (m_hFile == NULL)
        return SEEK_ERROR;

    // Đồng bộ hoá m_dwPos với ftell để chắc chắn
    long pos = std::ftell((FILE*)m_hFile);
    if (pos < 0) return SEEK_ERROR;
    m_dwPos = static_cast<DWORD>(pos);
    return m_dwPos;
}

//---------------------------------------------------------------------------
// Tên:     Size
// Mô tả:   Lấy kích thước tệp (byte)
// Trả về:  Độ dài tệp, 0 nếu lỗi
//---------------------------------------------------------------------------
DWORD KFile::Size()
{
    if (m_hFile == NULL)
        return 0;

    // Lưu lại vị trí hiện tại.
    long cur = std::ftell((FILE*)m_hFile);
    if (cur < 0)
        cur = 0;

    if (std::fseek((FILE*)m_hFile, 0, SEEK_END) == 0) {
        long endPos = std::ftell((FILE*)m_hFile);
        if (endPos >= 0)
            m_dwLen = static_cast<DWORD>(endPos);
    }

    // Trả lại vị trí cũ.
    std::fseek((FILE*)m_hFile, cur, SEEK_SET);
    m_dwPos = static_cast<DWORD>(cur);
    return m_dwLen;
}

//---------------------------------------------------------------------------
