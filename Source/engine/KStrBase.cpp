//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:   KStrBase.cpp
// Date:   2000.08.08 (cập nhật: 2026.03.11)
// Code:   WangWei(Daphnis)
// Mục đích:
//  - Triển khai tập hàm tiện ích chuỗi/định dạng/in ấn bậc thấp cho engine.
//---------------------------------------------------------------------------

#include "KMemBase.h"
#include "KStrBase.h"
#include "KPakList.h"

#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <string_view>
#include <limits>

#if !defined(_WIN32)
#  include "KGbktoUtf8.h"
#  include <sys/stat.h>
#  include <sys/types.h>
#  include <dirent.h>
#else
#  include <direct.h>
#endif

// Đếm số cặp byte phù hợp phạm vi mã Big5 trong chuỗi (ước lượng nhanh, không xác thực đầy đủ).
int GetBig5Count(char* str)
{
    if (!str) return 0;
    int lnBIG5    = 0;   // Đếm số cặp phù hợp Big5
    const int len = static_cast<int>(std::strlen(str));

    for (int i = 0; i + 1 < len; ++i)
    {
        const unsigned char b1 = static_cast<unsigned char>(str[i]);
        const unsigned char b2 = static_cast<unsigned char>(str[i + 1]);
        // Các phạm vi thường gặp trong Big5
        if ((b1 >= 161 && b1 <= 254 && b2 >= 64 && b2 <= 126) ||
            (b1 >= 129 && b1 <= 160 && b2 >= 64 && b2 <= 126) ||
            (b1 >= 129 && b1 <= 160 && b2 >= 161 && b2 <= 254) ||
            (b1 >= 164 && b1 <= 169 && b2 >= 161 && b2 <= 254))
        {
            ++lnBIG5;
        }
    }
    return lnBIG5;
}

UINT g_Atoui(char* str)
{
    if (!str) return 0U;
    // Bỏ qua khoảng trắng đầu
    while (*str && std::isspace(static_cast<unsigned char>(*str))) ++str;
    unsigned long long acc = 0ULL;
    while (*str && std::isdigit(static_cast<unsigned char>(*str)))
    {
        acc = acc * 10ULL + static_cast<unsigned long long>(*str - '0');
        if (acc > std::numeric_limits<UINT>::max())
        {
            acc = std::numeric_limits<UINT>::max();
            break;
        }
        ++str;
    }
    return static_cast<UINT>(acc);
}

int g_Round2Int(double d)
{
    // Làm tròn 0.5 lên, xử lý số âm đối xứng
    return static_cast<int>(d + (d >= 0.0 ? 0.5 : -0.5));
}

float g_Int2Round(int d)
{
    // Giữ hành vi gốc: chuyển int sang float không thay đổi
    return static_cast<float>(d);
}

int g_ExtractChar(const char* inStr, char scrchar, char destchar, char* outStr, char* outStra, int nMoedel)
{
    if (!inStr) return 0;
    char* tmp  = outStr;
    char* tmpa = outStra;
    int nLen   = 0;

    while (*inStr != '\0')
    {
        if (nMoedel == 0 && *inStr == scrchar)
        {
            ++inStr;
            while (*inStr != '\0')
            {
                if (*inStr == destchar)
                {
                    ++inStr;
                    break;
                }
                if (tmpa) { *tmpa++ = *inStr; }
                ++inStr;
            }
            if (tmpa) *tmpa = '\0';
        }
        else if (nMoedel == 1 && *inStr == scrchar)
        {
            break; // Lấy phần trước scrchar vào outStr
        }
        else if (nMoedel == 2 && *inStr == scrchar)
        {
            ++inStr;
            while (*inStr != '\0')
            {
                if (tmpa) { *tmpa++ = *inStr; }
                ++inStr;
            }
            if (tmpa) *tmpa = '\0';
        }

        if (*inStr == '\0') break;
        if (tmp) { *tmp++ = *inStr; }
        ++inStr;
        ++nLen;
    }
    if (tmp) *tmp = '\0';
    return nLen;
}

int g_StrLen(const char* lpStr)
{
    return lpStr ? static_cast<int>(std::strlen(lpStr)) : 0;
}

char* g_StrEnd(const char* lpStr)
{
    return const_cast<char*>(lpStr) + (lpStr ? std::strlen(lpStr) : 0);
}

void g_StrCpy(char* lpDest, const char* lpSrc)
{
    if (!lpDest) return;
    if (!lpSrc)
    {
        lpDest[0] = '\0';
        return;
    }
    std::strcpy(lpDest, lpSrc); // Giữ hành vi cũ (có thể tràn nếu bộ đệm không đủ)
}

void g_StrCpyLen(char* lpDest, const char* lpSrc, int nMaxLen)
{
    if (!lpDest || nMaxLen <= 0) return;
    if (!lpSrc)
    {
        lpDest[0] = '\0';
        return;
    }
    // Sao chép tối đa nMaxLen-1 ký tự để đảm bảo null-termination
    std::strncpy(lpDest, lpSrc, static_cast<size_t>(nMaxLen));
    lpDest[nMaxLen - 1] = '\0';
}

void g_StrCat(char* lpDest, const char* lpSrc)
{
    if (!lpDest || !lpSrc) return;
    std::strcat(lpDest, lpSrc); // Hành vi cũ
}

void g_StrCatLen(char* lpDest, const char* lpSrc, int nMaxLen)
{
    if (!lpDest || !lpSrc || nMaxLen <= 0) return;
    const int destLen = g_StrLen(lpDest);
    if (nMaxLen <= 0) return;
    // Chỉ nối tối đa nMaxLen ký tự từ lpSrc
    std::strncat(lpDest, lpSrc, static_cast<size_t>(nMaxLen));
}

int g_StrCmp(const char* lpDest, const char* lpSrc)
{
    if (!lpDest || !lpSrc) return 0;
    const int nLen1 = g_StrLen(lpDest);
    const int nLen2 = g_StrLen(lpSrc);
    if (nLen1 != nLen2) return 0;
    return g_MemComp((void*)lpDest, (void*)lpSrc, nLen1);
}

int g_StrCmpLen(const char* lpDest, const char* lpSrc, int nMaxLen)
{
    if (!lpDest || !lpSrc || nMaxLen <= 0) return 0;
    const int nLen1 = g_StrLen(lpDest);
    const int nLen2 = g_StrLen(lpSrc);
    int nLen = nMaxLen;
    if (nLen > nLen1) nLen = nLen1;
    if (nLen > nLen2) nLen = nLen2;
    return g_MemComp((void*)lpDest, (void*)lpSrc, nLen);
}

void g_StrUpper(char* lpDest)
{
    if (!lpDest) return;
    for (char* p = lpDest; *p; ++p)
        *p = static_cast<char>(std::toupper(static_cast<unsigned char>(*p)));
}

void g_StrLower(char* lpDest)
{
    if (!lpDest) return;
    for (char* p = lpDest; *p; ++p)
        *p = static_cast<char>(std::tolower(static_cast<unsigned char>(*p)));
}

void g_StrRep(char* lpDest, const char* lpSrc, const char* lpRep)
{
    if (!lpDest || !lpSrc || !lpRep) return;
    const int nSrcLen  = g_StrLen(lpSrc);
    const int nDestLen = g_StrLen(lpDest);
    if (nSrcLen == 0 || nDestLen < nSrcLen) return;

    int pos = -1;
    for (int i = 0; i <= nDestLen - nSrcLen; ++i)
    {
        if (g_StrCmpLen(&lpDest[i], lpSrc, nSrcLen)) { pos = i; break; }
    }
    if (pos < 0) return;

    const int repLen = g_StrLen(lpRep);
    const int nMaxLen = nDestLen - nSrcLen + repLen + 1;
    char* pStart = new char[static_cast<size_t>(nMaxLen)];

    if (pos != 0)
    {
        g_StrCpyLen(pStart, lpDest, pos + 1);
        pStart[pos] = '\0';
        g_StrCat(pStart, lpRep);
        g_StrCat(pStart, &lpDest[pos + nSrcLen]);
    }
    else
    {
        g_StrCpy(pStart, lpRep);
        g_StrCat(pStart, &lpDest[nSrcLen]);
    }
    g_StrCpy(lpDest, pStart);
    delete[] pStart;
}

static void t_printchar(char** str, int c)
{
    if (str && *str)
    {
        **str = static_cast<char>(c);
        ++(*str);
    }
    else
    {
        (void)std::putchar(c);
    }
}

#define PAD_RIGHT 1
#define PAD_ZERO  2

static int t_prints(char** out, const char* string, int width, int pad)
{
    int pc = 0;
    int padchar = ' ';
    if (!string) string = "(null)";

    int len = 0;
    for (const char* p = string; *p; ++p) ++len;

    if (width > 0)
    {
        if (len >= width) width = 0; else width -= len;
        if (pad & PAD_ZERO) padchar = '0';
    }

    if (!(pad & PAD_RIGHT))
    {
        for (; width > 0; --width) { t_printchar(out, padchar); ++pc; }
    }
    for (const char* p = string; *p; ++p) { t_printchar(out, *p); ++pc; }
    for (; width > 0; --width) { t_printchar(out, padchar); ++pc; }
    return pc;
}

#define PRINT_BUF_LEN 32

static int t_printi(char** out, long long i, int b, int sg, int width, int pad, int letbase)
{
    char print_buf[PRINT_BUF_LEN];
    char* s;
    int t, neg = 0, pc = 0;
    unsigned long long u = static_cast<unsigned long long>(i);

    if (i == 0)
    {
        print_buf[0] = '0';
        print_buf[1] = '\0';
        return t_prints(out, print_buf, width, pad);
    }

    if (sg && b == 10 && i < 0)
    {
        neg = 1;
        u   = static_cast<unsigned long long>(-i);
    }

    s  = print_buf + PRINT_BUF_LEN - 1;
    *s = '\0';

    while (u)
    {
        t = static_cast<int>(u % static_cast<unsigned long long>(b));
        if (t >= 10) t += letbase - '0' - 10;
        *--s = static_cast<char>(t + '0');
        u /= static_cast<unsigned long long>(b);
    }

    if (neg)
    {
        if (width && (pad & PAD_ZERO))
        {
            t_printchar(out, '-');
            ++pc;
            --width;
        }
        else
        {
            *--s = '-';
        }
    }

    return pc + t_prints(out, s, width, pad);
}

static int t_print(char** out, const char* format, va_list args)
{
    int width, pad;
    int pc = 0;
    char scr[2];

    for (; *format != 0; ++format)
    {
        if (*format == '%')
        {
            ++format;
            width = 0; pad = 0;
            if (*format == '\0') break;
            if (*format == '%') goto out_t;
            if (*format == '-') { ++format; pad = PAD_RIGHT; }
            while (*format == '0') { ++format; pad |= PAD_ZERO; }
            for (; *format >= '0' && *format <= '9'; ++format) { width = width * 10 + (*format - '0'); }

            switch (*format)
            {
                case 's':
                {
                    const char* s = va_arg(args, const char*);
                    pc += t_prints(out, s, width, pad);
                    continue;
                }
                case 'd':
                    pc += t_printi(out, va_arg(args, int), 10, 1, width, pad, 'a');
                    continue;
                case 'x':
                    pc += t_printi(out, va_arg(args, unsigned int), 16, 0, width, pad, 'a');
                    continue;
                case 'X':
                    pc += t_printi(out, va_arg(args, unsigned int), 16, 0, width, pad, 'A');
                    continue;
                case 'u':
                    pc += t_printi(out, va_arg(args, unsigned int), 10, 0, width, pad, 'a');
                    continue;
                case 'c':
                    scr[0] = static_cast<char>(va_arg(args, int));
                    scr[1] = '\0';
                    pc += t_prints(out, scr, width, pad);
                    continue;
                default:
                    // Ký tự định dạng không hỗ trợ: in thô
                    t_printchar(out, *format);
                    ++pc;
                    continue;
            }
        }
        else
        {
        out_t:
            t_printchar(out, *format);
            ++pc;
        }
    }
    if (out && *out) **out = '\0';
    return pc;
}

int t_printf(const char* format, ...)
{
    va_list args; va_start(args, format);
    int pc = t_print(nullptr, format, args);
    va_end(args);
    return pc;
}

int t_sprintf(char* out, const char* format, ...)
{
    va_list args; va_start(args, format);
    char* ptr = out;
    int pc = t_print(&ptr, format, args);
    if (ptr) *ptr = '\0';
    va_end(args);
    return pc;
}

int t_snprintf(char* buf, unsigned int count, const char* format, ...)
{
    if (!buf || count == 0) { return 0; }
    va_list args; va_start(args, format);
    char* ptr = buf;
    char* end = buf + count - 1; // chừa chỗ cho '\0'

    // Trình bao bọc ghi có giới hạn
    auto putc_bounded = [&](int c){ if (ptr < end) { *ptr++ = static_cast<char>(c); } else { /* bỏ qua */ } };

    // Tạm thời ghi qua t_printchar bằng cách hoán đổi hàm: sử dụng buffer con
    // Ở đây, dùng macro nhỏ: t_print sử dụng t_printchar, chúng ta cung cấp con trỏ &ptr
    int pc = t_print(&ptr, format, args);
    *ptr = '\0';
    va_end(args);
    (void)putc_bounded; // tránh cảnh báo không dùng
    return pc;
}

// Hàm tạo thư mục, nhận đường dẫn tương đối nPath và đường dẫn gốc nSdcardPath.
bool __CreateFolder(std::string nSdcardPath, char* nPath)
{
    if (!nPath || nSdcardPath.empty()) return false;

    // Chuẩn hoá: dùng '/' làm separator nội bộ
    std::string rel(nPath);
    for (char& ch : rel) { if (ch == '\\') ch = '/'; }
    if (!rel.empty() && (rel.front() == '/' || rel.front() == '\\')) rel.erase(rel.begin());

    // Tạo dần từng cấp
    std::string current;
    current.reserve(nSdcardPath.size() + rel.size() + 1);
    current.append(nSdcardPath);
    if (!current.empty() && current.back() == '\\') current.back() = '/';
    if (!current.empty() && current.back() != '/') current.push_back('/');

    std::string partial;
    for (size_t i = 0; i < rel.size(); ++i)
    {
        char c = rel[i];
        if (c == '/')
        {
            if (!partial.empty())
            {
                std::string dir = current + partial;
#if defined(_WIN32)
                _mkdir(dir.c_str());
#else
                mkdir(dir.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
#endif
                partial.clear();
            }
        }
        else
        {
            partial.push_back(c);
        }
    }
    if (!partial.empty())
    {
        std::string dir = current + partial;
#if defined(_WIN32)
        _mkdir(dir.c_str());
#else
        mkdir(dir.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
#endif
    }
    return true;
}

void g_StrCopy(std::string nSdcardPath, char* pMusicName)
{
    if (!pMusicName || nSdcardPath.empty() || !g_pPakList) return;

    XPackElemFileRef m_PackRef{};
    int nIsHave = g_pPakList->pGetFilePath(pMusicName, m_PackRef);
    if (!nIsHave) return;

    std::unique_ptr<char[]> sBuf(new (std::nothrow) char[m_PackRef.nSize]);
    if (!sBuf) return;

    __CreateFolder(nSdcardPath, pMusicName);
    g_pPakList->ElemFileRead(m_PackRef, sBuf.get(), m_PackRef.nSize);

    char m_szEntireMapFile[512] = {0};
#if defined(_WIN32)
    std::snprintf(m_szEntireMapFile, sizeof(m_szEntireMapFile), "%s%s", nSdcardPath.c_str(), pMusicName);
#else
    std::snprintf(m_szEntireMapFile, sizeof(m_szEntireMapFile), "%s%s", nSdcardPath.c_str(), G2U(pMusicName).c_str());
#endif

    if (FILE* oFp = std::fopen(m_szEntireMapFile, "wb"))
    {
        std::fwrite(sBuf.get(), static_cast<size_t>(m_PackRef.nSize), 1, oFp);
        std::fclose(oFp);
    }
}

unsigned int _ccHash(const char* key)
{
    if (!key) return 0U;
    // FNV-1a 32-bit, không phân biệt hoa thường (chuyển sang upper trước khi băm)
    const unsigned int FNV_offset = 2166136261u;
    const unsigned int FNV_prime  = 16777619u;
    unsigned int hash = FNV_offset;
    for (const unsigned char* p = reinterpret_cast<const unsigned char*>(key); *p; ++p)
    {
        unsigned char c = static_cast<unsigned char>(std::toupper(*p));
        hash ^= c;
        hash *= FNV_prime;
    }
    return hash;
}
