//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:    KTabFile.h
// Date:    2002.02.18
// Code:    Spe
// Desc:    Header File
// Refactor: Production C++20 (2026-03-11)
//---------------------------------------------------------------------------
#ifndef KTabFile_H
#define KTabFile_H

// Mục đích (VI):
//  - KTabFile là bộ đọc/tra cứu nhanh tệp dạng bảng (Tab/CSV-like) dựa trên bảng offset.

#include "KMemClass.h"
#include "KITabFile.h"

//---------------------------------------------------------------------------
// Bảng offset cho từng dòng: vị trí bắt đầu (dwOffset) và độ dài (dwLength)
typedef struct tagTabOffset {
    unsigned int dwOffset;
    unsigned int dwLength;
} TABOFFSET;

class KTabFile {
private:
    int         m_Width;
    int         m_Height;
    KMemClass   m_Memory;       // Bộ nhớ chứa toàn bộ dữ liệu tệp
    KMemClass   m_OffsetTable;  // Bảng offset cho từng dòng
    unsigned int dwSize;        // Kích thước dữ liệu nguồn
    // unsigned int     dwFileIdx;
    // void *           Buffer;
    // const char       nMemKey[32];

private:
    void CreateTabOffset();
    int  GetValue(int nRow, int nColumn, char* lpRString, unsigned int dwSize);
    int  Str2Col(const char* szColumn);

public:
    KTabFile();
    ~KTabFile();
    // KTabFileCtrl *TabFileCtrl;

    // Tải tệp bảng; sử dụng KMemClass để lưu dữ liệu trong bộ nhớ.
    int  Load(const char* FileName /*,const char * mMemKey=NULL*/);

    // Tạo tệp mới rỗng.
    bool CreatFile(const char* FileName);

    // Lưu tệp: chưa hỗ trợ theo thiết kế cũ, luôn trả về false.
    int  Save(const char* FileName) { return false; }  // Chưa hỗ trợ lưu
    // int LoadPack(const char* FileName);
    // unsigned int GetFileDwIdx(){ return dwFileIdx; }

    unsigned int GetSize() { return dwSize; }

    int  FindRow(const char* szRow);        // Tìm vị trí hàng; trả về -1 nếu không thấy
    int  FindColumn(const char* szColumn);  // Tìm vị trí cột; trả về -1 nếu không thấy
    void Col2Str(int nCol, char* szColumn);

    int  GetWidth() { return m_Width; }
    int  GetHeight() { return m_Height; }

    int  GetString(int nRow,
                   const char* szColumn,
                   const char* lpDefault,
                   char* lpRString,
                   unsigned int dwSize,
                   int bColumnLab = TRUE);

    int  GetString(const char* szRow,
                   int szColumn,
                   const char* lpDefault,
                   char* lpRString,
                   unsigned int dwSize,
                   int bColumnLab = TRUE);

    int  GetString(int nRow, int nColumn, const char* lpDefault, char* lpRString, unsigned int dwSize);

    int  GetString(const char* szRow, const char* szColumn, const char* lpDefault, char* lpRString, unsigned int dwSize);

    int  GetInteger(int nRow, const char* szColumn, int nDefault, int* pnValue, int bColumnLab = TRUE);

    int  GetInteger6(int nRow,
                     const char* szColumna,
                     const char* szColumnb,
                     const char* szColumnc,
                     const char* szColumnd,
                     const char* szColumne,
                     const char* szColumnf,
                     int nDefault,
                     int* pnValue,
                     int bColumnLab = TRUE);

    int  GetInteger3(int nRow,
                     const char* szColumna,
                     const char* szColumnb,
                     const char* szColumnc,
                     int nDefault,
                     int* pnValue,
                     int bColumnLab = TRUE);

    int  GetInteger(int nRow, int nColumn, int nDefault, int* pnValue);

    int  GetIntegerDW(int nRow, const char* szColumn, int nDefault, unsigned int* pnValue, int bColumnLab = TRUE);

    int  GetInteger(const char* szRow, const char* szColumn, int nDefault, int* pnValue);

    int  GetFloat(int nRow, const char* szColumn, float fDefault, float* pfValue, int bColumnLab = TRUE);

    int  GetDouble(int nRow, const char* szColumn, double fDefault, double* pfValue, int bColumnLab = TRUE);

    int  GetDword(int nRow, const char* szColumn, unsigned int fDefault, unsigned int* pfValue, int bColumnLab = TRUE);

    // int GetFloat(const char * szRow, const char * szColumn, float fDefault, float *pfValue);
    int  GetFloat(int nRow, int nColumn, float fDefault, float* pfValue);
    int  GetFloat(const char* szRow, const char* szColumn, float fDefault, float* pfValue);

    void GetInt2(int nRow, int szColumn, int* pRect);
    void GetInt15(int nRow, int szColumn, int* pRect);
    void GetInt15s(int nRow, const char* szColumn, int* pRect);
    void GetInt3(int nRow, int szColumn, int* pRect);
    void GetInt2s(int nRow, const char* szColumn, int* pRect);
    void GetInt3s(int nRow, const char* szColumn, int* pRect);

    void Clear();
};

//---------------------------------------------------------------------------
#endif // KTabFile_H
