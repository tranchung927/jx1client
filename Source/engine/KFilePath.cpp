// ---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:   KFilePath.cpp
// Mục đích: Triển khai các hàm tiện ích xử lý đường dẫn/tên tệp cho engine.
// ---------------------------------------------------------------------------

#include "gamecore/KCore.h"
#include "KMemBase.h"
#include "KStrBase.h"
#include "KFilePath.h"
#include "KPakList.h"
#include "dataChecksum.h"
#include "KPlatform.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
#    include <direct.h>
#    include <windows.h>
#else
#    include <unistd.h>
#    include <sys/stat.h>
#    include <sys/types.h>
#    include <dirent.h>
#endif

// Đường dẫn gốc và hiện tại (giữ nguyên biến toàn cục, tương thích hành vi cũ)
static char szRootPath[MAXPATH] = "";
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
static char szCurrPath[MAXPATH] = "\\"; // Đường dẫn hiện tại
#   define CURPOS 1
#else
static char szCurrPath[MAXPATH] = "/"; // Đường dẫn hiện tại
#   define CURPOS 1
#endif

namespace {
inline void NormalizePathSeparators(char* path)
{
    if (!path)
        return;

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
    for (int i = 0; path[i]; ++i)
    {
        if (path[i] == '/')
            path[i] = '\\';
    }
#else
    for (int i = 0; path[i]; ++i)
    {
        if (path[i] == '\\')
            path[i] = '/';
    }
#endif
}

inline void TrimTrailingSeparators(char* path)
{
    if (!path)
        return;

    int len = g_StrLen(path);
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
    while (len > 3 && (path[len - 1] == '\\' || path[len - 1] == '/'))
#else
    while (len > 1 && (path[len - 1] == '\\' || path[len - 1] == '/'))
#endif
    {
        path[len - 1] = 0;
        --len;
    }
}

inline void InitDefaultRootPath()
{
    if (szRootPath[0])
        return;

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
    if (!_getcwd(szRootPath, MAXPATH))
        szRootPath[0] = 0;
#else
    std::string runtimeRoot;
    if (!m_SDcardDirPath.empty())
        runtimeRoot = m_SDcardDirPath;
    else
        runtimeRoot = ax::FileUtils::getInstance()->getWritablePath();

    std::snprintf(szRootPath, MAXPATH, "%s", runtimeRoot.c_str());
#endif

    NormalizePathSeparators(szRootPath);
    TrimTrailingSeparators(szRootPath);
}
}

// Loại bỏ thành phần ".." trong đường dẫn, trả về độ dài mới
int RemoveTwoPointPath(char* szPath, int nLength)
{
    if (!szPath || nLength <= 0) return 0;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
    const char* lpszOld = "\\..\\";
#else
    const char* lpszOld = "/../";
#endif
    int nRemove = 0;
    char* lpszTarget = std::strstr(szPath, lpszOld);
    if (lpszTarget)
    {
        char* lpszAfter = lpszTarget + 3;
        while (lpszTarget > szPath)
        {
            --lpszTarget;
            if ((*lpszTarget) == '\\' || (*lpszTarget) == '/')
                break;
        }
        std::memmove(lpszTarget, lpszAfter, static_cast<size_t>(nLength - static_cast<int>(lpszAfter - szPath) + 1));
        nRemove = static_cast<int>(lpszAfter - lpszTarget);
        return RemoveTwoPointPath(szPath, nLength - nRemove);
    }
    return nLength - nRemove;
}

// Loại bỏ thành phần "." trong đường dẫn, trả về độ dài mới
int RemoveOnePointPath(char* szPath, int nLength)
{
    if (!szPath || nLength <= 0) return 0;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
    const char* lpszOld = "\\.\\";
#else
    const char* lpszOld = "/./";
#endif
    int nRemove = 0;
    char* lpszTarget = std::strstr(szPath, lpszOld);
    if (lpszTarget)
    {
        char* lpszAfter = lpszTarget + 2;
        std::memmove(lpszTarget, lpszAfter, static_cast<size_t>(nLength - static_cast<int>(lpszAfter - szPath) + 1));
        nRemove = static_cast<int>(lpszAfter - lpszTarget);
        return RemoveOnePointPath(szPath, nLength - nRemove);
    }
    return nLength - nRemove;
}

// Loại bỏ cả "." và ".." trong đường dẫn
int RemoveAllPointPath(char* szPath, int nLength)
{
    return RemoveOnePointPath(szPath, RemoveTwoPointPath(szPath, nLength));
}

// Đặt đường dẫn gốc (root) cho trò chơi
void g_SetRootPath(char* lpPathName, bool isChange)
{
    if (lpPathName && lpPathName[0])
    {
        g_StrCpy(szRootPath, lpPathName);
    }
    else
    {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
        if (isChange || !szRootPath[0])
        {
            if (!_getcwd(szRootPath, MAXPATH))
                szRootPath[0] = 0;
        }
#else
        std::string runtimeRoot;
        if (!m_SDcardDirPath.empty())
            runtimeRoot = m_SDcardDirPath;
        else
            runtimeRoot = ax::FileUtils::getInstance()->getWritablePath();

        std::snprintf(szRootPath, MAXPATH, "%s", runtimeRoot.c_str());
#endif
    }

    NormalizePathSeparators(szRootPath);
    TrimTrailingSeparators(szRootPath);
}

// Lấy đường dẫn gốc hiện tại
void g_GetRootPath(char* lpPathName)
{
    InitDefaultRootPath();
    g_StrCpy(lpPathName, szRootPath);
}

// Đặt đường dẫn thư mục hiện tại
void g_SetFilePath(const char* lpPathName)
{
    if (!lpPathName) return;

    // Bỏ ký tự phân cách đầu nếu có
    if (lpPathName[0] == '\\' || lpPathName[0] == '/')
        g_StrCpy(szCurrPath, lpPathName + 1);
    else
        g_StrCpy(szCurrPath, lpPathName);

    // Đảm bảo có ký tự phân cách ở cuối
    int len = g_StrLen(szCurrPath);
    if (len > 0 && szCurrPath[len - 1] != '\\' && szCurrPath[len - 1] != '/')
    {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
        szCurrPath[len] = '\\';
#else
        szCurrPath[len] = '/';
#endif
        szCurrPath[len + 1] = 0;
    }

    RemoveAllPointPath(szCurrPath, len + 1);
    NormalizePathSeparators(szCurrPath);

#if !(CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
    // Chuẩn hóa '\\' -> '/'
    for (len = 0; szCurrPath[len]; ++len)
    {
        if (szCurrPath[len] == '\\')
            szCurrPath[len] = '/';
    }
#endif
}

// Lấy đường dẫn thư mục hiện tại
void g_GetFilePath(char* lpPathName)
{
    g_StrCpy(lpPathName, szCurrPath);
}

// Lấy đường dẫn đầy đủ từ root/curr và tên tệp
void g_GetFullPath(char* lpPathName, const char* lpFileName)
{
    if (!lpPathName || !lpFileName) return;

    InitDefaultRootPath();

    char szNormalizedFileName[MAXPATH];
    std::snprintf(szNormalizedFileName, sizeof(szNormalizedFileName), "%s", lpFileName);
    NormalizePathSeparators(szNormalizedFileName);

    int len = g_StrLen(szRootPath);
    if (len > 0 && (szRootPath[len - 1] == '\\' || szRootPath[len - 1] == '/'))
        szRootPath[len - 1] = 0;

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
    if (szNormalizedFileName[CURPOS] == ':')
    {
        g_StrCpy(lpPathName, szNormalizedFileName);
        return;
    }

    if (szNormalizedFileName[0] == '\\' || szNormalizedFileName[0] == '/')
    {
        g_StrCpy(lpPathName, szRootPath);
        g_StrCat(lpPathName, szNormalizedFileName);
        return;
    }
#else
    if (szNormalizedFileName[0] == '/')
    {
        g_StrCpy(lpPathName, szNormalizedFileName);
        return;
    }
#endif

    g_StrCpy(lpPathName, szRootPath);

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
    if (szCurrPath[CURPOS] == ':')
    {
        g_StrCpy(lpPathName, szCurrPath);
        g_StrCat(lpPathName, szNormalizedFileName);
        return;
    }
#endif

    if (szCurrPath[0] != '\\' && szCurrPath[0] != '/')
    {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
        g_StrCat(lpPathName, "\\");
#else
        g_StrCat(lpPathName, "/");
#endif
    }

    g_StrCat(lpPathName, szCurrPath);

    if (szNormalizedFileName[0] == '.' && (szNormalizedFileName[1] == '\\' || szNormalizedFileName[1] == '/'))
        g_StrCat(lpPathName, szNormalizedFileName + 2);
    else
        g_StrCat(lpPathName, szNormalizedFileName);
}

// Lấy nửa đường dẫn (tương đối từ curr) + file
void g_GetHalfPath(char* lpPathName, char* lpFileName)
{
    if (!lpPathName || !lpFileName) return;

    char szNormalizedFileName[MAXPATH];
    std::snprintf(szNormalizedFileName, sizeof(szNormalizedFileName), "%s", lpFileName);
    NormalizePathSeparators(szNormalizedFileName);

    if (szNormalizedFileName[0] == '\\' || szNormalizedFileName[0] == '/')
    {
        g_StrCpy(lpPathName, szNormalizedFileName);
    }
    else
    {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
        g_StrCpy(lpPathName, "\\");
#else
        g_StrCpy(lpPathName, "/");
#endif
        g_StrCat(lpPathName, szCurrPath);
        g_StrCat(lpPathName, szNormalizedFileName);
    }
}

// Lấy đường dẫn bên trong gói nén (pack)
void g_GetPackPath(char* lpPathName, char* lpFileName)
{
    if (!lpPathName || !lpFileName) return;

    char szNormalizedFileName[MAXPATH];
    std::snprintf(szNormalizedFileName, sizeof(szNormalizedFileName), "%s", lpFileName);
    NormalizePathSeparators(szNormalizedFileName);

    if (szNormalizedFileName[0] == '\\' || szNormalizedFileName[0] == '/')
    {
        // Bỏ ký tự phân cách đầu
        g_StrCpy(lpPathName, szNormalizedFileName + 1);
    }
    else
    {
        g_StrCat(lpPathName, szNormalizedFileName);
    }

    int len = g_StrLen(lpPathName);
    RemoveAllPointPath(lpPathName, len + 1);
    NormalizePathSeparators(lpPathName);
    g_StrLower(lpPathName); // Chuyển về chữ thường toàn bộ
}

// Lấy đường dẫn trên đĩa (CDROM). Logic cũ: thử ổ từ C:
void g_GetDiskPath(char* lpPathName, char* lpFileName)
{
    if (!lpPathName || !lpFileName) return;

    g_StrCpy(lpPathName, "C:");
    for (int i = 0; i < 24; lpPathName[0]++, ++i)
    {
        // Giữ nguyên logic lịch sử, không kiểm tra GetDriveType
    }

    if (lpFileName[0] == '\\' || lpPathName[0] == '/')
    {
        g_StrCat(lpPathName, lpFileName);
    }
    else
    {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
        g_StrCat(lpPathName, "\\");
#else
        g_StrCat(lpPathName, "/");
#endif
        g_StrCat(lpPathName, szCurrPath);
        g_StrCat(lpPathName, lpFileName);
    }
}

// Tạo đường dẫn (thư mục) bên trong thư mục trò chơi
void g_CreatePath(char* lpPathName)
{
    if (!lpPathName || !lpPathName[0])
        return;

    char szFullPath[MAXPATH];
    int  i = 0;

    // Nếu là đường dẫn tuyệt đối kiểu "C:"
    if (lpPathName[CURPOS] == ':')
    {
        if (g_StrLen(lpPathName) < 4)
            return;
        g_StrCpy(szFullPath, lpPathName);
        i = 4;
    }
    else if (lpPathName[0] == '\\' || lpPathName[0] == '/')
    {
        g_StrCpy(szFullPath, szRootPath);
        g_StrCat(szFullPath, lpPathName);
        i = g_StrLen(szRootPath) + 1;
    }
    else
    {
        g_StrCpy(szFullPath, szRootPath);
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
        g_StrCat(szFullPath, "\\");
#else
        g_StrCat(szFullPath, "/");
#endif
        g_StrCat(szFullPath, lpPathName);
        i = g_StrLen(szRootPath) + 1;
    }

    const int fullLen = g_StrLen(szFullPath);
    NormalizePathSeparators(szFullPath);
    for (; i < fullLen; ++i)
    {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
        if (szFullPath[i] == '\\')
        {
            szFullPath[i] = 0;
            CreateDirectoryA(szFullPath, NULL);
            szFullPath[i] = '\\';
        }
#else
        if (szFullPath[i] == '/')
        {
            szFullPath[i] = 0;
            ::mkdir(szFullPath, 0755);
            szFullPath[i] = '/';
        }
#endif
    }
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
    CreateDirectoryA(szFullPath, NULL);
#else
    ::mkdir(szFullPath, 0755);
#endif
}

// Hợp nhất đường dẫn và tên file thành đường dẫn đầy đủ sử dụng '\\'
void g_UnitePathAndName(const char* lpPath, const char* lpFile, char* lpGet)
{
    if (!lpPath || !lpFile || !lpGet) return;

    std::strcpy(lpGet, lpPath);
    NormalizePathSeparators(lpGet);
    int nSize = static_cast<int>(std::strlen(lpGet));
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
    const char kSep = '\\';
#else
    const char kSep = '/';
#endif
    if (nSize > 0 && lpGet[nSize - 1] != kSep)
    {
        lpGet[nSize] = kSep;
        lpGet[nSize + 1] = 0;
    }

    char szNormalizedFile[MAXPATH];
    std::snprintf(szNormalizedFile, sizeof(szNormalizedFile), "%s", lpFile);
    NormalizePathSeparators(szNormalizedFile);
    if (szNormalizedFile[0] != kSep)
        std::strcat(lpGet, szNormalizedFile);
    else
        std::strcat(lpGet, &szNormalizedFile[1]);
}

// Kiểm tra tệp tồn tại (trong pak hoặc trên đĩa)
int g_FileExists(char* FileName)
{
    int bExist = FALSE;
    char szFullName[MAX_PATH];
    if (FileName && FileName[0])
    {
        // Kiểm tra trong pak trước
        if (g_pPakList)
        {
            XPackElemFileRef PackRef;
            bExist = g_pPakList->pGetFilePath(FileName, PackRef);
        }
        // Nếu chưa có, kiểm tra hệ thống tệp
        if (bExist == FALSE)
        {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
            g_GetFullPath(szFullName, FileName);
            DWORD attr = GetFileAttributesA(szFullName);
            bExist = (attr != INVALID_FILE_ATTRIBUTES) && !(attr & FILE_ATTRIBUTE_DIRECTORY);
#else
            g_GetFullPath(szFullName, FileName);
            bExist = ax::FileUtils::getInstance()->isFileExist(szFullName);
#endif
        }
    }
    return bExist;
}

// Hash tên tệp thành ID 32-bit
unsigned int g_FileName2Id(char* lpFileName)
{
    if (lpFileName)
    {
        const int len = static_cast<int>(std::strlen(lpFileName));
        unsigned int dwID = 0;
        for (int i = 0; i < len; ++i)
        {
            dwID = (dwID + static_cast<unsigned int>((i + 1) * static_cast<unsigned char>(lpFileName[i]))) % 0x8000000B * 0xFFFFFFEF;
        }
        return static_cast<unsigned int>(dwID ^ 0x12345678);
    }
    return static_cast<unsigned int>(-1);
}

// Thay đổi phần mở rộng tệp
void g_ChangeFileExt(char* lpFileName, char* lpNewExt)
{
    if (!lpFileName || !lpNewExt) return;

    int i = 0;
    for (; lpFileName[i]; ++i)
    {
        if (lpFileName[i] == '.')
            break;
    }

    if (lpFileName[i] == '.')
    {
        g_StrCpy(&lpFileName[i + 1], lpNewExt);
    }
    else
    {
        g_StrCat(lpFileName, ".");
        g_StrCat(lpFileName, lpNewExt);
    }
}

// Tách tên tệp từ đường dẫn đầy đủ
void g_ExtractFileName(char* lpFileName, char* lpFilePath)
{
    if (!lpFileName || !lpFilePath) return;

    int nLen = g_StrLen(lpFilePath);
    if (nLen < 1) return;
    int nPos = nLen;
    while (--nPos > 0)
    {
        if (lpFilePath[nPos] == '\\' || lpFilePath[nPos] == '/')
            break;
    }
    g_StrCpy(lpFileName, &lpFilePath[nPos + 1]);
}

// Tách đường dẫn thư mục từ đường dẫn đầy đủ
void g_ExtractFilePath(char* lpPathName, char* lpFilePath)
{
    if (!lpPathName || !lpFilePath) return;

    int nLen = g_StrLen(lpFilePath);
    if (nLen < 1) return;
    int nPos = nLen;
    while (--nPos > 0)
    {
        if (lpFilePath[nPos] == '\\' || lpFilePath[nPos] == '/')
            break;
    }
    g_StrCpyLen(lpPathName, lpFilePath, nPos + 1);
}

// Kiểm tra tồn tại (hash) theo thuật toán đặc thù
unsigned int g_CheckFileExist(const char* lpFileName)
{
    if (!lpFileName) return static_cast<unsigned int>(-1);

#ifndef WIN32
    dataChecksum nappInfo;
    char nstrps[64] = {0}, destStr[128] = {0};
    // Giải mã hai đoạn khóa ứng dụng (APP_STRINFO_1/2) để xác thực
    nappInfo.SimplyDecrypt(nstrps, APP_STRINFO_1);
    std::strncat(destStr, nstrps, sizeof(destStr) - 1);
    std::memset(nstrps, 0, sizeof(nstrps));
    nappInfo.SimplyDecrypt(nstrps, APP_STRINFO_2);
    std::strncat(destStr, nstrps, sizeof(destStr) - 1);
    // Nếu cần, có thể so sánh với m_MobileKey ở nơi khác
#endif

    unsigned int dwID = 0;
    const int len = static_cast<int>(std::strlen(lpFileName));
    for (int i = 0; i < len; ++i)
    {
#if defined(WIN32)
        dwID = (dwID + static_cast<unsigned int>((i + 1) * static_cast<unsigned char>(lpFileName[i]))) % 0x8000000B * 0xFFFFFFEF;
#else
        dwID = (dwID + static_cast<unsigned int>((i + 1) * static_cast<unsigned char>(lpFileName[i]))) % 0x8000000B * 0xFFFFFFEF;
#endif
    }
    return (dwID ^ 305419896U); // 0x12345678
}
// ---------------------------------------------------------------------------
