#pragma once
#include "KEngine.h"
// Giao diện thao tác TabFile (đọc/ghi, truy vấn theo dòng/cột, lấy chuỗi/số/thực)
class KITabFile
{
public:
    KITabFile() = default;
    virtual ~KITabFile() = default;

    // Tải dữ liệu từ tập tin Tab
    virtual BOOL Load(LPCSTR FileName) = 0;
    // Lưu dữ liệu ra tập tin Tab
    virtual BOOL Save(LPCSTR FileName) = 0;

    // Tìm dòng theo nhãn dòng (tên dòng)
    virtual int  FindRow(LPCSTR szRow) = 0;
    // Tìm cột theo nhãn cột (tên cột)
    virtual int  FindColumn(LPCSTR szColumn) = 0;

    // Lấy số cột (chiều rộng)
    virtual int  GetWidth() = 0;
    // Lấy số dòng (chiều cao)
    virtual int  GetHeight() = 0;

    // Đọc chuỗi theo (dòng, nhãn cột)
    virtual BOOL GetString(int nRow, LPCSTR szColumn, LPCSTR lpDefault, LPSTR lpRString, DWORD dwSize, BOOL bColumnLab = TRUE) = 0;
    // Đọc chuỗi theo (dòng, chỉ số cột)
    virtual BOOL GetString(int nRow, int nColumn, LPCSTR lpDefault, LPSTR lpRString, DWORD dwSize) = 0;
    // Đọc chuỗi theo (nhãn dòng, nhãn cột)
    virtual BOOL GetString(LPCSTR szRow, LPCSTR szColumn, LPCSTR lpDefault, LPSTR lpRString, DWORD dwSize) = 0;

    // Đọc số nguyên theo (dòng, nhãn cột)
    virtual BOOL GetInteger(int nRow, LPCSTR szColumn, int nDefault, int* pnValue, BOOL bColumnLab = TRUE) = 0;
    // Đọc số nguyên theo (dòng, chỉ số cột)
    virtual BOOL GetInteger(int nRow, int nColumn, int nDefault, int* pnValue) = 0;
    // Đọc số nguyên theo (nhãn dòng, nhãn cột)
    virtual BOOL GetInteger(LPCSTR szRow, LPCSTR szColumn, int nDefault, int* pnValue) = 0;

    // Đọc số thực theo (dòng, nhãn cột)
    virtual BOOL GetFloat(int nRow, LPCSTR szColumn, float fDefault, float* pfValue, BOOL bColumnLab = TRUE) = 0;
    // Đọc số thực theo (dòng, chỉ số cột)
    virtual BOOL GetFloat(int nRow, int nColumn, float fDefault, float* pfValue) = 0;
    // Đọc số thực theo (nhãn dòng, nhãn cột)
    virtual BOOL GetFloat(LPCSTR szRow, LPCSTR szColumn, float fDefault, float* pfValue) = 0;

    // Xóa/giải phóng dữ liệu đang giữ trong đối tượng
    virtual void Clear() = 0;
};
