//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:    KTabFile.cpp
// Date:    2002.02.20
// Code:    Huyi (Spe)
// Desc:    Tab File Operation Class
// Refactor: Production C++20 (2026-03-11)
//---------------------------------------------------------------------------

#include <cstring>
#include <cstdlib>

#include "KStrBase.h"
#include "KFile.h"
#include "KFilePath.h"
#include "KPakFile.h"
#include "KCodec.h"
#include "KTabFile.h"

KTabFile::KTabFile()
{
    m_Width  = 0;
    m_Height = 0;
    dwSize   = 0;
}

KTabFile::~KTabFile()
{
    Clear();
}

bool KTabFile::CreatFile(const char* FileName)
{
    if (!FileName)
        return false;
    KFile cFile;
    if (cFile.Create(FileName)) {
        cFile.Close();
        return true;
    }
    return false;
}

// Tải tệp bảng vào bộ nhớ và xây dựng bảng offset.
int KTabFile::Load(const char* FileName)
{
    KPakFile File;
    void* Buffer = NULL;

    if (FileName == NULL || FileName[0] == 0)
        return false;

    if (!File.Open(FileName)) {
        return false;
    }

    dwSize = File.Size();
    if (dwSize <= 0) {
        File.Close();
        return false;
    }

    Buffer = m_Memory.Alloc(dwSize);
    if (!Buffer) {
        File.Close();
        return false;
    }

    File.Read(Buffer, dwSize);

    CreateTabOffset();

    File.Close();
    return true;
}

// Xây dựng bảng offset cho từng ô, đồng thời xác định kích thước bảng.
void KTabFile::CreateTabOffset()
{
    int nWidth = 1;
    int nHeight = 1;
    int nOffset = 0;

    unsigned char* Buffer = (unsigned char*)m_Memory.GetMemPtr();
    int nSize = m_Memory.GetMemLen();

    if (!Buffer || nSize <= 0) {
        m_Width = 0;
        m_Height = 0;
        return;
    }

    // Đếm số cột từ dòng đầu tiên
    unsigned char* p = Buffer;
    while (nOffset < nSize && *p != 0x0d && *p != 0x0a) {
        if (*p == 0x09) // tab
            ++nWidth;
        ++p;
        ++nOffset;
    }
    // Bỏ qua kết thúc dòng (CRLF hoặc LF)
    if (nOffset < nSize) {
        if (*p == 0x0d && (nOffset + 1 < nSize) && *(p + 1) == 0x0a) {
            p += 2;
            nOffset += 2;
        } else {
            ++p;
            ++nOffset;
        }
    }

    // Đếm số dòng còn lại
    while (nOffset < nSize) {
        while (nOffset < nSize && *p != 0x0d && *p != 0x0a) {
            ++p;
            ++nOffset;
        }
        ++nHeight;
        if (nOffset < nSize) {
            if (*p == 0x0d && (nOffset + 1 < nSize) && *(p + 1) == 0x0a) {
                p += 2;
                nOffset += 2;
            } else {
                ++p;
                ++nOffset;
            }
        }
    }

    m_Width = nWidth;
    m_Height = nHeight;

    TABOFFSET* TabBuffer = (TABOFFSET*)m_OffsetTable.Alloc(m_Width * m_Height * sizeof(TABOFFSET));
    p = (unsigned char*)m_Memory.GetMemPtr();
    nOffset = 0;

    for (int i = 0; i < nHeight; ++i) {
        for (int j = 0; j < nWidth; ++j) {
            int nLength = 0;
            if (TabBuffer)
                TabBuffer->dwOffset = nOffset;
            // Tính độ dài ô cho tới tab hoặc kết thúc dòng
            while (nOffset < nSize && *p != 0x09 && *p != 0x0d && *p != 0x0a) {
                ++p;
                ++nOffset;
                ++nLength;
            }
            // Ghi độ dài
            if (TabBuffer) {
                TabBuffer->dwLength = nLength;
                ++TabBuffer;
            }
            // Bỏ qua dấu phân tách (tab hoặc CR/LF)
            if (nOffset < nSize) {
                ++p;
                ++nOffset;
            }
            // Nếu vừa kết thúc dòng, đổ đầy các cột còn lại bằng độ dài 0
            if (*(p - 1) == 0x0a || *(p - 1) == 0x0d) {
                for (int k = j + 1; k < nWidth; ++k) {
                    if (TabBuffer) {
                        TabBuffer->dwOffset = nOffset;
                        TabBuffer->dwLength = 0;
                        ++TabBuffer;
                    }
                }
                break;
            }
        }
        // Xử lý CRLF: nếu trước đó là CR và hiện tại là LF, bỏ qua LF
        if (nOffset < nSize && *(p - 1) == 0x0d && *p == 0x0a) {
            ++p;
            ++nOffset;
        }
    }
}

// Chuyển tên cột kiểu Excel (A, B, AA, ...) sang chỉ số (bắt đầu từ 1 theo API cũ)
int KTabFile::Str2Col(const char* szColumn)
{
    int nStrLen = g_StrLen(szColumn);
    char szTemp[4];

    g_StrCpy(szTemp, szColumn);
    g_StrUpper(szTemp);
    if (nStrLen == 1) {
        return (szTemp[0] - 'A');
    }
    return ((szTemp[0] - 'A' + 1) * 26 + szTemp[1] - 'A') + 1;
}

// Lấy chuỗi theo hàng/cột (tên cột)
int KTabFile::GetString(int nRow, const char* szColumn, const char* lpDefault, char* lpRString, unsigned int dwSize, int bColumnLab)
{
    int nColumn = bColumnLab ? FindColumn(szColumn) : Str2Col(szColumn);
    if (GetValue(nRow - 1, nColumn - 1, lpRString, dwSize))
        return true;
    g_StrCpyLen(lpRString, lpDefault, dwSize);
    return false;
}

// Lấy chuỗi theo tên hàng và số cột
int KTabFile::GetString(const char* szRow, int szColumn, const char* lpDefault, char* lpRString, unsigned int dwSize, int bColumnLab)
{
    int nRow = FindRow(szRow);
    int nColumn = szColumn; // bColumnLab không ảnh hưởng vì tham số đã là số cột
    if (GetValue(nRow - 1, nColumn - 1, lpRString, dwSize))
        return true;
    g_StrCpyLen(lpRString, lpDefault, dwSize);
    return false;
}

// Lấy chuỗi theo tên hàng và tên cột
int KTabFile::GetString(const char* szRow, const char* szColumn, const char* lpDefault, char* lpRString, unsigned int dwSize)
{
    int nRow = FindRow(szRow);
    int nColumn = FindColumn(szColumn);
    if (GetValue(nRow - 1, nColumn - 1, lpRString, dwSize))
        return true;
    g_StrCpyLen(lpRString, lpDefault, dwSize);
    return false;
}

// Lấy chuỗi theo chỉ số hàng/cột (1-based theo API cũ)
int KTabFile::GetString(int nRow, int nColumn, const char* lpDefault, char* lpRString, unsigned int dwSize)
{
    if (GetValue(nRow - 1, nColumn - 1, lpRString, dwSize))
        return true;
    g_StrCpyLen(lpRString, lpDefault, dwSize);
    return false;
}

int KTabFile::GetInteger(int nRow, const char* szColumn, int nDefault, int* pnValue, int bColumnLab)
{
    char Buffer[64];
    int nColumn = bColumnLab ? FindColumn(szColumn) : Str2Col(szColumn);
    if (GetValue(nRow - 1, nColumn - 1, Buffer, sizeof(Buffer))) {
        *pnValue = std::atoi(Buffer);
        return true;
    } else {
        *pnValue = nDefault;
        return false;
    }
}

int KTabFile::GetInteger6(int nRow, const char* szColumna, const char* szColumnb, const char* szColumnc, const char* szColumnd, const char* szColumne, const char* szColumnf, int nDefault, int* pnValue, int bColumnLab)
{
    char Buffer[64];
    int nColumna, nColumnb, nColumnc, nColumnd, nColumne, nColumnf;
    if (bColumnLab) {
        nColumna = FindColumn(szColumna);
        nColumnb = FindColumn(szColumnb);
        nColumnc = FindColumn(szColumnc);
        nColumnd = FindColumn(szColumnd);
        nColumne = FindColumn(szColumne);
        nColumnf = FindColumn(szColumnf);
    } else {
        nColumna = Str2Col(szColumna);
        nColumnb = Str2Col(szColumnb);
        nColumnc = Str2Col(szColumnc);
        nColumnd = Str2Col(szColumnd);
        nColumne = Str2Col(szColumne);
        nColumnf = Str2Col(szColumnf);
    }

    if (GetValue(nRow - 1, nColumna - 1, Buffer, sizeof(Buffer)))
        *pnValue = std::atoi(Buffer);
    else
        *pnValue = nDefault;
    ++pnValue;

    if (GetValue(nRow - 1, nColumnb - 1, Buffer, sizeof(Buffer)))
        *pnValue = std::atoi(Buffer);
    else
        *pnValue = nDefault;
    ++pnValue;

    if (GetValue(nRow - 1, nColumnc - 1, Buffer, sizeof(Buffer)))
        *pnValue = std::atoi(Buffer);
    else
        *pnValue = nDefault;
    ++pnValue;

    if (GetValue(nRow - 1, nColumnd - 1, Buffer, sizeof(Buffer)))
        *pnValue = std::atoi(Buffer);
    else
        *pnValue = nDefault;
    ++pnValue;

    if (GetValue(nRow - 1, nColumne - 1, Buffer, sizeof(Buffer)))
        *pnValue = std::atoi(Buffer);
    else
        *pnValue = nDefault;
    ++pnValue;

    if (GetValue(nRow - 1, nColumnf - 1, Buffer, sizeof(Buffer)))
        *pnValue = std::atoi(Buffer);
    else
        *pnValue = nDefault;

    return true;
}

int KTabFile::GetInteger3(int nRow, const char* szColumna, const char* szColumnb, const char* szColumnc, int nDefault, int* pnValue, int bColumnLab)
{
    char Buffer[64];
    int nColumna, nColumnb, nColumnc;
    if (bColumnLab) {
        nColumna = FindColumn(szColumna);
        nColumnb = FindColumn(szColumnb);
        nColumnc = FindColumn(szColumnc);
    } else {
        nColumna = Str2Col(szColumna);
        nColumnb = Str2Col(szColumnb);
        nColumnc = Str2Col(szColumnc);
    }

    if (GetValue(nRow - 1, nColumna - 1, Buffer, sizeof(Buffer)))
        *pnValue = std::atoi(Buffer);
    else
        *pnValue = nDefault;
    ++pnValue;

    if (GetValue(nRow - 1, nColumnb - 1, Buffer, sizeof(Buffer)))
        *pnValue = std::atoi(Buffer);
    else
        *pnValue = nDefault;
    ++pnValue;

    if (GetValue(nRow - 1, nColumnc - 1, Buffer, sizeof(Buffer)))
        *pnValue = std::atoi(Buffer);
    else
        *pnValue = nDefault;

    return true;
}

void KTabFile::GetInt2(int nRow, int szColumn, int* pRect)
{
    char Buffer[256];
    if (GetValue(nRow - 1, szColumn - 1, Buffer, sizeof(Buffer))) {
        std::sscanf(Buffer, "%d|%d", &pRect[0], &pRect[1]);
    }
}

void KTabFile::GetInt15(int nRow, int szColumn, int* pRect)
{
    char Buffer[256];
    if (GetValue(nRow - 1, szColumn - 1, Buffer, sizeof(Buffer))) {
        std::sscanf(Buffer, "%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d",
                    &pRect[0], &pRect[1], &pRect[2], &pRect[3], &pRect[4],
                    &pRect[5], &pRect[6], &pRect[7], &pRect[8], &pRect[9],
                    &pRect[10], &pRect[11], &pRect[12], &pRect[13], &pRect[14]);
    }
}

void KTabFile::GetInt15s(int nRow, const char* szColumn, int* pRect)
{
    char Buffer[256];
    int nColumn = FindColumn(szColumn);
    if (GetValue(nRow - 1, nColumn - 1, Buffer, sizeof(Buffer))) {
        std::sscanf(Buffer, "%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d",
                    &pRect[0], &pRect[1], &pRect[2], &pRect[3], &pRect[4],
                    &pRect[5], &pRect[6], &pRect[7], &pRect[8], &pRect[9],
                    &pRect[10], &pRect[11], &pRect[12], &pRect[13], &pRect[14]);
    }
}

void KTabFile::GetInt3(int nRow, int szColumn, int* pRect)
{
    char Buffer[256];
    if (GetValue(nRow - 1, szColumn - 1, Buffer, sizeof(Buffer))) {
        std::sscanf(Buffer, "%d|%d|%d", &pRect[0], &pRect[1], &pRect[2]);
    }
}

void KTabFile::GetInt2s(int nRow, const char* szColumn, int* pRect)
{
    char Buffer[256];
    int nColumn = FindColumn(szColumn);
    if (GetValue(nRow - 1, nColumn - 1, Buffer, sizeof(Buffer))) {
        std::sscanf(Buffer, "%d|%d", &pRect[0], &pRect[1]);
    }
}

void KTabFile::GetInt3s(int nRow, const char* szColumn, int* pRect)
{
    char Buffer[256];
    int nColumn = FindColumn(szColumn);
    if (GetValue(nRow - 1, nColumn - 1, Buffer, sizeof(Buffer))) {
        std::sscanf(Buffer, "%d|%d|%d", &pRect[0], &pRect[1], &pRect[2]);
    }
}

int KTabFile::GetInteger(const char* szRow, const char* szColumn, int nDefault, int* pnValue)
{
    int nRow = FindRow(szRow);
    int nColumn = FindColumn(szColumn);
    char Buffer[64];
    if (GetValue(nRow - 1, nColumn - 1, Buffer, sizeof(Buffer))) {
        *pnValue = std::atoi(Buffer);
        return true;
    } else {
        *pnValue = nDefault;
        return false;
    }
}

int KTabFile::GetInteger(int nRow, int nColumn, int nDefault, int* pnValue)
{
    char Buffer[64] = {0};
    if (GetValue(nRow - 1, nColumn - 1, Buffer, sizeof(Buffer))) {
        *pnValue = std::atoi(Buffer);
        return true;
    } else {
        *pnValue = nDefault;
        return true; // Giữ hành vi cũ trả về true dù dùng giá trị mặc định
    }
}

int KTabFile::GetIntegerDW(int nRow, const char* szColumn, int nDefault, unsigned int* pnValue, int bColumnLab)
{
    char Buffer[64];
    int nColumn = bColumnLab ? FindColumn(szColumn) : Str2Col(szColumn);
    if (GetValue(nRow - 1, nColumn - 1, Buffer, sizeof(Buffer))) {
        *pnValue = (unsigned int)std::atoi(Buffer);
        return true;
    } else {
        *pnValue = nDefault;
        return false;
    }
}

int KTabFile::GetDword(int nRow, const char* szColumn, unsigned int fDefault, unsigned int* pfValue, int bColumnLab)
{
    char Buffer[64];
    int nColumn = bColumnLab ? FindColumn(szColumn) : Str2Col(szColumn);
    if (GetValue(nRow - 1, nColumn - 1, Buffer, sizeof(Buffer))) {
        *pfValue = (unsigned int)std::atof(Buffer);
        return true;
    } else {
        *pfValue = fDefault;
        return false;
    }
}

int KTabFile::GetDouble(int nRow, const char* szColumn, double fDefault, double* pfValue, int bColumnLab)
{
    char Buffer[64];
    int nColumn = bColumnLab ? FindColumn(szColumn) : Str2Col(szColumn);
    if (GetValue(nRow - 1, nColumn - 1, Buffer, sizeof(Buffer))) {
        *pfValue = (double)std::atof(Buffer);
        return true;
    } else {
        *pfValue = fDefault;
        return false;
    }
}

int KTabFile::GetFloat(int nRow, const char* szColumn, float fDefault, float* pfValue, int bColumnLab)
{
    char Buffer[64];
    int nColumn = bColumnLab ? FindColumn(szColumn) : Str2Col(szColumn);
    if (GetValue(nRow - 1, nColumn - 1, Buffer, sizeof(Buffer))) {
        *pfValue = (float)std::atof(Buffer);
        return true;
    } else {
        *pfValue = fDefault;
        return false;
    }
}

int KTabFile::GetFloat(const char* szRow, const char* szColumn, float fDefault, float* pfValue)
{
    int nRow = FindRow(szRow);
    int nColumn = FindColumn(szColumn);
    char Buffer[64];
    if (GetValue(nRow - 1, nColumn - 1, Buffer, sizeof(Buffer))) {
        *pfValue = (float)std::atof(Buffer);
        return true;
    } else {
        *pfValue = fDefault;
        return false;
    }
}

int KTabFile::GetFloat(int nRow, int nColumn, float fDefault, float* pfValue)
{
    char Buffer[64];
    if (GetValue(nRow - 1, nColumn - 1, Buffer, sizeof(Buffer))) {
        *pfValue = (float)std::atof(Buffer);
        return true;
    } else {
        *pfValue = fDefault;
        return false;
    }
}

// Trả về chuỗi tại (nRow,nColumn) vào lpRString với kích thước tối đa dwSize
int KTabFile::GetValue(int nRow, int nColumn, char* lpRString, unsigned int dwSize)
{
    if (nRow >= m_Height || nColumn >= m_Width || nRow < 0 || nColumn < 0)
        return false;

    const char* Buffer = (const char*)m_Memory.GetMemPtr();
    TABOFFSET* TempOffset = (TABOFFSET*)m_OffsetTable.GetMemPtr();
    if (!Buffer || !TempOffset || !lpRString || dwSize == 0)
        return false;

    TempOffset += nRow * m_Width + nColumn;

    std::memset(lpRString, 0, dwSize);
    Buffer += TempOffset->dwOffset;
    if (TempOffset->dwLength == 0)
        return false;

    if (dwSize > TempOffset->dwLength) {
        std::memcpy(lpRString, Buffer, TempOffset->dwLength);
        lpRString[TempOffset->dwLength] = '\0';
    } else {
        std::memcpy(lpRString, Buffer, dwSize - 1);
        lpRString[dwSize - 1] = '\0';
    }
    return true;
}

// Giải phóng bộ nhớ bảng và offset
void KTabFile::Clear()
{
    m_OffsetTable.Free();
    m_Memory.Free();
}

// Tìm hàng theo từ khoá ở cột đầu tiên, trả về 1-based index; không thấy trả về -1
int KTabFile::FindRow(const char* szRow)
{
    char szTemp[128] = {0};
    for (int i = 0; i < m_Height; ++i) {
        GetValue(i, 0, szTemp, g_StrLen(szRow));
        if (g_StrCmp(szTemp, szRow))
            return i + 1; // API trả về 1-based
    }
    return -1;
}

// Tìm cột theo từ khoá ở dòng đầu tiên, trả về 1-based index; không thấy trả về -1
int KTabFile::FindColumn(const char* szColumn)
{
    char szTemp[128] = {0};
    for (int i = 0; i < m_Width; ++i) {
        GetValue(0, i, szTemp, g_StrLen(szColumn));
        if (g_StrCmp(szTemp, szColumn))
            return i + 1; // API trả về 1-based
    }
    return -1;
}

// Chuyển chỉ số cột sang tên cột (A..Z, AA..)
void KTabFile::Col2Str(int nCol, char* szColumn)
{
    if (nCol < 26) {
        szColumn[0] = 'A' + nCol;
        szColumn[1] = 0;
    } else {
        szColumn[0] = 'A' + (nCol / 26 - 1);
        szColumn[1] = 'A' + nCol % 26;
        szColumn[2] = 0;
    }
}
