//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:    KTabFileCtrl.h
// Date:    2000.08.08
// Desc:    Bảng điều khiển đọc/ghi tệp dạng bảng (Tab/CSV-like)
// Refactor: Production C++20 (2026-03-11)
//---------------------------------------------------------------------------

// Mục đích:
//  - KTabFileCtrl cung cấp API thao tác dữ liệu bảng: đọc/ghi chuỗi, số, float theo hàng/cột.

#pragma once

#include "KEngine.h"
#include "KNode.h"
#include "KITabFile.h"

class TTabColNode : public KNode {
public:
    char* m_Str;

    ~TTabColNode()
    {
        if (m_Str) {
            // Sửa lỗi: m_Str được cấp phát như mảng ký tự => dùng delete[]
            delete[] m_Str;
            m_Str = NULL;
        }
    }
};

class TTabLineNode : public KNode {
public:
    KList* pList;

    ~TTabLineNode()
    {
        // Dọn sạch toàn bộ node cột trong dòng
        while (pList && pList->GetHead()) {
            TTabColNode* pDelNode = (TTabColNode*)pList->GetHead();
            pList->RemoveHead();
            if (pDelNode) {
                delete pDelNode;
                pDelNode = NULL;
            }
        }
        // Không huỷ pList ở đây để tránh double-free, quyền sở hữu thuộc nơi khác.
    }
};

class KTabFileCtrl {
private:
    int SetValue(int nRow, int nColumn, char* lpRString, unsigned long dwSize, int bCanCreateNew = TRUE);
    int GetValue(int nRow, int nColumn, char*& lpRString, unsigned long dwSize);
    int GetValue(char* RowName, char* ColumnName, char* szValue, unsigned long dwSize)
    {
        return GetValue(FindRow(RowName), FindColumn(ColumnName), szValue, dwSize);
    }
    int Str2Col(const char*);
    // KMemClass   Meme;

public:
    KTabFileCtrl();
    ~KTabFileCtrl();

    KList m_RowList;

    int Load(const char* FileName /*,char* mMemKey=NULL*/);  // Tải tệp bảng từ hệ thống tệp
    int openfile(char* FileName);
    int Save(const char* FileName);                           // Lưu tệp bảng ra hệ thống tệp
    bool CreatFile(const char* FileName);                     // Tạo tệp bảng mới rỗng
    int LoadPack(char* FileName);                             // Tải tệp bảng từ PAK

    int FindRow(char* szRow);              // Tìm vị trí hàng, trả về -1 nếu không thấy
    int FindColumn(const char* szColumn);  // Tìm vị trí cột, trả về -1 nếu không thấy

    char* GetRowName(int nRow);
    char* GetColName(int nCol);

    int InsertNewCol(char* strNewCol);  // Chèn cột mới, trả về chỉ số; lỗi trả về -1

    int InsertAfter(int nRow);

    int InsertAfter(char* szRow, int bColumnLab = TRUE)
    {
        if (!bColumnLab)
            return InsertAfter(Str2Col(szRow));
        else
            return InsertAfter(FindRow(szRow));
    }

    int InsertBefore(int nRow);

    int InsertBefore(char* szRow, int bColumnLab = TRUE)
    {
        if (!bColumnLab)
            return InsertBefore(Str2Col(szRow));
        else
            return InsertBefore(FindRow(szRow));
    }

    int GetWidth()  // Lấy số cột của dòng đầu tiên
    {
        TTabLineNode* pLineNode = (TTabLineNode*)m_RowList.GetHead();
        if (!pLineNode)
            return 0;
        return pLineNode->pList ? pLineNode->pList->GetNodeCount() : 0;
    }

    int GetHeight() { return m_RowList.GetNodeCount(); }  // Lấy tổng số dòng

    int GetString(int nRow, int nColumn, char* lpDefault, char* lpRString, unsigned long dwSize);

    int GetString(int nRow,
                  char* szColumn,
                  char* lpDefault,
                  char* lpRString,
                  unsigned long dwSize,
                  int bColumnLab = TRUE)
    {
        if (!bColumnLab)  // Dùng nhãn kiểu Excel: a1, b1, c1
            return GetString(nRow, Str2Col(szColumn), lpDefault, lpRString, dwSize);
        else
            return GetString(nRow, FindColumn(szColumn), lpDefault, lpRString, dwSize);
    }

    int GetString(char* szRow, char* szColumn, char* lpDefault, char* lpRString, unsigned long dwSize)
    {
        return GetString(FindRow(szRow), FindColumn(szColumn), lpDefault, lpRString, dwSize);
    }

    int GetInteger(int nRow, int nColumn, int nDefault, int* pnValue);

    int GetInteger(int nRow, const char* szColumn, int nDefault, int* pnValue, int bColumnLab = TRUE)
    {
        if (!bColumnLab)
            return GetInteger(nRow, Str2Col(szColumn), nDefault, pnValue);
        else
            return GetInteger(nRow, FindColumn(szColumn), nDefault, pnValue);
    }

    int GetInteger(char* szRow, char* szColumn, int nDefault, int* pnValue)
    {
        return GetInteger(FindRow(szRow), FindColumn(szColumn), nDefault, pnValue);
    }

    int GetFloat(int nRow, int nColumn, float fDefault, float* pfValue);

    int GetFloat(int nRow, char* szColumn, float fDefault, float* pfValue, int bColumnLab = TRUE)
    {
        if (!bColumnLab)
            return GetFloat(nRow, Str2Col(szColumn), fDefault, pfValue);
        else
            return GetFloat(nRow, FindColumn(szColumn), fDefault, pfValue);
    }

    int GetFloat(char* szRow, char* szColumn, float fDefault, float* pfValue)
    {
        return GetFloat(FindRow(szRow), FindColumn(szColumn), fDefault, pfValue);
    }

    int WriteString(int nRow, int nColumn, char* lpString, unsigned long dwSize = 0);

    int WriteString(int nRow, char* szColumn, char* lpString, int bColumnLab = TRUE)
    {
        if (!bColumnLab)  // Dùng nhãn kiểu Excel: a1, b1, c1
            return WriteString(nRow, Str2Col(szColumn), lpString);
        else
            return WriteString(nRow, FindColumn(szColumn), lpString);
    }

    int WriteString(char* szRow, char* szColumn, char* lpRString)
    {
        return WriteString(FindRow(szRow), FindColumn(szColumn), lpRString);
    }

    int WriteInteger(int nRow, int nColumn, int nValue);

    int WriteInteger(int nRow, char* szColumn, int nValue, int bColumnLab = TRUE)
    {
        if (!bColumnLab)
            return WriteInteger(nRow, Str2Col(szColumn), nValue);
        else
            return WriteInteger(nRow, FindColumn(szColumn), nValue);
    }

    int WriteInteger(char* szRow, char* szColumn, int nValue)
    {
        return WriteInteger(FindRow(szRow), FindColumn(szColumn), nValue);
    }

    int WriteFloat(int nRow, int nColumn, float fValue);

    int WriteFloat(int nRow, char* szColumn, float fValue, int bColumnLab = TRUE)
    {
        if (!bColumnLab)
            return WriteFloat(nRow, Str2Col(szColumn), fValue);
        else
            return WriteFloat(nRow, FindColumn(szColumn), fValue);
    }

    int WriteFloat(char* szRow, char* szColumn, float fValue)
    {
        return WriteFloat(FindRow(szRow), FindColumn(szColumn), fValue);
    }

    int Remove(int nRow);

    int Remove(char* szRow, int bColumnLab = TRUE)
    {
        if (!bColumnLab)
            return Remove(Str2Col(szRow));
        else
            return Remove(FindRow(szRow));
    }

    void Clear();
    bool isload;
};
