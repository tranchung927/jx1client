//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:    KTabFileCtrl.cpp
// Date:    2000.08.08
// Desc:    Triển khai bộ điều khiển tệp dạng bảng (Tab/CSV-like)
// Refactor: Production C++20 (2026-03-11)
//---------------------------------------------------------------------------

#include <cstring>
#include <cstdlib>

#include "KTabFileCtrl.h"
#include "KStrBase.h"
#include "KMemClass.h"

// Mục đích (VI):
//  - KTabFileCtrl đọc/ghi tệp dạng bảng, thao tác theo hàng/cột với chuỗi/số/float.

KTabFileCtrl::KTabFileCtrl()
{
    isload = false;
}

KTabFileCtrl::~KTabFileCtrl()
{
    Clear();
}

// Đọc tệp bảng từ hệ thống tệp hoặc tạo mới nếu chưa tồn tại.
int KTabFileCtrl::Load(const char* FileName /*,char* mMemKey*/)
{
    KPakFile File;
    unsigned long dwSize = 0;
    PVOID Buffer = NULL;
    KFile cFile;

    if (FileName == NULL || FileName[0] == 0)
        return FALSE;

    if (!File.Open(FileName)) { // Mở tệp; nếu không có thì tạo rỗng rồi mở lại
        if (cFile.Create(FileName)) {
            if (!File.Open(FileName)) {
                cFile.Close();
                return FALSE;
            }
            cFile.Close();
        }
    }

    dwSize = File.Size();
    isload = true;

    KMemClass Meme;
    Buffer = Meme.Alloc(dwSize);
    if (!Buffer && dwSize > 0) {
        File.Close();
        return FALSE;
    }

    if (dwSize > 0)
        File.Read(Buffer, dwSize);
    File.Close();

    unsigned long nCurPos = 0;
    while (nCurPos < dwSize) {
        // Đọc một dòng (kết thúc bằng CRLF hoặc LF)
        char szLine[10000];
        int i = 0;
        while (nCurPos < dwSize && i < (int)sizeof(szLine) - 1) {
            char ch = ((char*)Buffer)[nCurPos++];
            if (ch == '\r') {
                // Bỏ qua \n nếu có
                if (nCurPos < dwSize && ((char*)Buffer)[nCurPos] == '\n')
                    ++nCurPos;
                break;
            }
            if (ch == '\n')
                break;
            szLine[i++] = ch;
        }
        szLine[i] = '\0';

        // Tạo node dòng và phân tách cột theo tab
        TTabLineNode* pLineNode = new TTabLineNode;
        KList* pList = new KList;
        pLineNode->pList = pList;
        m_RowList.AddTail(pLineNode);

        char* szData = szLine;
        while (true) {
            char* szFind = std::strstr(szData, "\t");
            if (szFind == NULL) {
                size_t len = std::strlen(szData);
                char* newStr = new char[len + 1];
                g_StrCpyLen(newStr, szData, (unsigned long)(len + 1));
                TTabColNode* pNode = new TTabColNode;
                pNode->m_Str = newStr;
                pLineNode->pList->AddTail(pNode);
                break;
            }
            size_t partLen = (size_t)(szFind - szData);
            char* newStr = new char[partLen + 1];
            g_StrCpyLen(newStr, szData, (unsigned long)(partLen + 1));
            TTabColNode* pNode = new TTabColNode;
            pNode->m_Str = newStr;
            pLineNode->pList->AddTail(pNode);
            szData = szFind + 1;
        }
    }
    return TRUE;
}

// Đọc từ hệ thống tệp (không qua PAK)
int KTabFileCtrl::openfile(char* FileName)
{
    KFile File;
    unsigned long dwSize = 0;
    PVOID Buffer = NULL;

    if (FileName == NULL || FileName[0] == 0)
        return FALSE;

    if (!File.Open(FileName)) {
        if (File.Create(FileName)) {
            if (!File.Open(FileName)) {
                File.Close();
                return FALSE;
            }
        }
    }

    dwSize = File.Size();
    isload = true;

    KMemClass Meme;
    Buffer = Meme.Alloc(dwSize);
    if (!Buffer && dwSize > 0) {
        File.Close();
        return FALSE;
    }

    if (dwSize > 0)
        File.Read(Buffer, dwSize);
    File.Close();

    unsigned long nCurPos = 0;
    while (nCurPos < dwSize) {
        char szLine[10000];
        int i = 0;
        while (nCurPos < dwSize && i < (int)sizeof(szLine) - 1) {
            char ch = ((char*)Buffer)[nCurPos++];
            if (ch == '\r') {
                if (nCurPos < dwSize && ((char*)Buffer)[nCurPos] == '\n')
                    ++nCurPos;
                break;
            }
            if (ch == '\n')
                break;
            szLine[i++] = ch;
        }
        szLine[i] = '\0';

        TTabLineNode* pLineNode = new TTabLineNode;
        KList* pList = new KList;
        pLineNode->pList = pList;
        m_RowList.AddTail(pLineNode);

        char* szData = szLine;
        while (true) {
            char* szFind = std::strstr(szData, "\t");
            if (szFind == NULL) {
                size_t len = std::strlen(szData);
                char* newStr = new char[len + 1];
                g_StrCpyLen(newStr, szData, (unsigned long)(len + 1));
                TTabColNode* pNode = new TTabColNode;
                pNode->m_Str = newStr;
                pLineNode->pList->AddTail(pNode);
                break;
            }
            size_t partLen = (size_t)(szFind - szData);
            char* newStr = new char[partLen + 1];
            g_StrCpyLen(newStr, szData, (unsigned long)(partLen + 1));
            TTabColNode* pNode = new TTabColNode;
            pNode->m_Str = newStr;
            pLineNode->pList->AddTail(pNode);
            szData = szFind + 1;
        }
    }
    return TRUE;
}

int KTabFileCtrl::LoadPack(char* FileName)
{
    (void)FileName;
    // Chưa triển khai: giữ nguyên hành vi cũ trả về TRUE.
    return TRUE;
}

bool KTabFileCtrl::CreatFile(const char* FileName)
{
    KFile File;
    if (FileName == NULL || FileName[0] == 0)
        return false;

    if (!File.Create(FileName))
        return false;

    File.Close();
    return true;
}

int KTabFileCtrl::Save(const char* FileName)
{
    KFile File;
    if (FileName == NULL || FileName[0] == 0)
        return FALSE;

    if (!File.Create(FileName)) {
        return FALSE;
    }

    char szCol[1000];
    TTabLineNode* pLineNode = (TTabLineNode*)m_RowList.GetHead();
    while (pLineNode) {
        TTabColNode* pNode = (TTabColNode*)pLineNode->pList->GetHead();
        while (pNode) {
            TTabColNode* pNextNode = (TTabColNode*)pNode->GetNext();
            if (pNextNode)
                std::snprintf(szCol, sizeof(szCol), "%s\t", pNode->m_Str);
            else
                std::snprintf(szCol, sizeof(szCol), "%s", pNode->m_Str);

            File.Write(szCol, g_StrLen(szCol));
            pNode = (TTabColNode*)pNode->GetNext();
        }
        // Kết thúc dòng theo CRLF để tương thích lịch sử
        File.Write((void*)"\r", 1);
        File.Write((void*)"\n", 1);

        pLineNode = (TTabLineNode*)pLineNode->GetNext();
    }

    File.Close();
    return TRUE;
}

// Lấy tên hàng đầu tiên (ô đầu) của dòng nRow
char* KTabFileCtrl::GetRowName(int nRow)
{
    TTabLineNode* pLineNode = (TTabLineNode*)m_RowList.GetHead();
    for (int i = 0; i < nRow - 1; i++) {
        if (!pLineNode)
            return NULL;
        pLineNode = (TTabLineNode*)pLineNode->GetNext();
    }
    if (!pLineNode || !pLineNode->pList)
        return NULL;
    TTabColNode* head = (TTabColNode*)(pLineNode->pList->GetHead());
    return head ? head->m_Str : NULL;
}

// Lấy tên cột ở dòng tiêu đề (dòng đầu)
char* KTabFileCtrl::GetColName(int nCol)
{
    TTabLineNode* pLineNode = (TTabLineNode*)m_RowList.GetHead();
    if (!pLineNode || !pLineNode->pList)
        return NULL;
    TTabColNode* pColNode = (TTabColNode*)pLineNode->pList->GetHead();
    for (int i = 0; i < nCol - 1; i++) {
        if (!pColNode)
            return NULL;
        pColNode = (TTabColNode*)pColNode->GetNext();
    }
    return pColNode ? pColNode->m_Str : NULL;
}

int KTabFileCtrl::FindRow(char* szRow)
{
    TTabLineNode* pLineNode = (TTabLineNode*)m_RowList.GetHead();
    int nRow = 1;
    while (pLineNode) {
        KList* pList = pLineNode->pList;
        TTabColNode* pCol = (TTabColNode*)pList->GetHead();
        if (!pCol)
            return -1;
        if (std::strcmp(pCol->m_Str, szRow) == 0) {
            return nRow;
        }
        pLineNode = (TTabLineNode*)pLineNode->GetNext();
        ++nRow;
    }
    return -1;
}

int KTabFileCtrl::FindColumn(const char* szColumn)
{
    TTabLineNode* pLine = (TTabLineNode*)m_RowList.GetHead();
    if (!pLine || !pLine->pList)
        return -1;

    TTabColNode* pColNode = (TTabColNode*)pLine->pList->GetHead();
    int nCol = 1;
    while (pColNode) {
        if (g_StrCmp(pColNode->m_Str, szColumn))
            return nCol;
        pColNode = (TTabColNode*)pColNode->GetNext();
        ++nCol;
    }
    return -1;
}

// Lấy giá trị ở toạ độ (nRow,nColumn), chỉ mục bắt đầu từ 0 ở nội bộ
int KTabFileCtrl::GetValue(int nRow, int nColumn, char*& lpRString, unsigned long dwSize)
{
    (void)dwSize; // giữ nguyên chữ ký, không dùng ở đây
    if (nRow < 0 || nColumn < 0)
        return FALSE;

    TTabLineNode* pLineNode = (TTabLineNode*)m_RowList.GetHead();
    for (int i = 0; i < nRow; i++) {
        if (!pLineNode)
            return FALSE;
        pLineNode = (TTabLineNode*)pLineNode->GetNext();
    }
    if (!pLineNode || !pLineNode->pList)
        return FALSE;

    TTabColNode* pColNode = (TTabColNode*)pLineNode->pList->GetHead();
    for (int j = 0; j < nColumn; j++) {
        if (!pColNode)
            return FALSE;
        pColNode = (TTabColNode*)pColNode->GetNext();
    }
    if (!pColNode)
        return FALSE;

    lpRString = pColNode->m_Str;
    return TRUE;
}

// Ghi giá trị ở (nRow,nColumn), có thể tự tạo hàng/cột nếu cần
int KTabFileCtrl::SetValue(int nRow, int nColumn, char* lpString, unsigned long dwSize, int bCanCreateNew)
{
    if (nRow < 0 || nColumn < 0)
        return FALSE;

    TTabLineNode* pLineNode = (TTabLineNode*)m_RowList.GetHead();
    TTabLineNode* pTempNode = pLineNode;
    for (int i = 0; i < nRow + 1; i++) {
        pLineNode = pTempNode;
        if (!pLineNode) {
            if (!bCanCreateNew)
                return FALSE;
            for (int j = 0; j < nRow + 1 - i; j++) {
                TTabLineNode* pLine = new TTabLineNode;
                KList* pList = new KList;
                pLine->pList = pList;
                m_RowList.AddTail(pLine);
            }
            pLineNode = (TTabLineNode*)m_RowList.GetTail();
            break;
        }
        pTempNode = (TTabLineNode*)pLineNode->GetNext();
    }

    KList* pColList = pLineNode->pList;
    TTabColNode* pColNode = (TTabColNode*)pColList->GetHead();
    TTabColNode* pTempColNode = pColNode;

    for (int j = 0; j < nColumn + 1; j++) {
        pColNode = pTempColNode;
        if (!pColNode) {
            if (!bCanCreateNew)
                return FALSE;
            for (int k = 0; k < nColumn + 1 - j - 1; k++) {
                TTabColNode* pNode = new TTabColNode;
                char* newStr = new char[1];
                newStr[0] = '\0';
                pNode->m_Str = newStr;
                pColList->AddTail(pNode);
            }
            TTabColNode* pNode = new TTabColNode;
            char* newStr = new char[dwSize + 1];
            g_StrCpyLen(newStr, lpString, dwSize + 1);
            pNode->m_Str = newStr;
            pColList->AddTail(pNode);
            return TRUE;
        }
        pTempColNode = (TTabColNode*)pColNode->GetNext();
    }

    if (pColNode->m_Str) {
        delete[] pColNode->m_Str;
        pColNode->m_Str = NULL;
    }

    char* pNewStr = new char[dwSize + 1];
    g_StrCpyLen(pNewStr, lpString, dwSize + 1);
    pColNode->m_Str = pNewStr;
    return TRUE;
}

int KTabFileCtrl::GetString(int nRow, int nColumn, char* lpDefault, char* lpRString, unsigned long dwSize)
{
    char* pData = NULL;
    if (!GetValue(nRow - 1, nColumn - 1, pData, dwSize) || pData == NULL) {
        g_StrCpy(lpRString, lpDefault);
        return FALSE;
    } else {
        g_StrCpyLen(lpRString, pData, dwSize);
    }
    return TRUE;
}

int KTabFileCtrl::GetInteger(int nRow, int nColumn, int nDefault, int* pnValue)
{
    char* pData = NULL;
    if (!GetValue(nRow - 1, nColumn - 1, pData, 100) || pData == NULL) {
        *pnValue = nDefault;
        return FALSE;
    } else {
        *pnValue = std::atoi(pData);
    }
    return TRUE;
}

int KTabFileCtrl::GetFloat(int nRow, int nColumn, float fDefault, float* pfValue)
{
    char* pData = NULL;
    if (!GetValue(nRow - 1, nColumn - 1, pData, 100) || pData == NULL) {
        *pfValue = fDefault;
        return FALSE;
    } else {
        *pfValue = (float)std::atof(pData);
    }
    return TRUE;
}

void KTabFileCtrl::Clear()
{
    while (m_RowList.GetTail()) {
        TTabLineNode* pLineNode = (TTabLineNode*)m_RowList.GetTail();
        KList* pList = pLineNode ? pLineNode->pList : NULL;

        while (pList && pList->GetHead()) {
            TTabColNode* pNode = (TTabColNode*)pList->GetHead();
            if (pNode) {
                if (pNode->m_Str) {
                    delete[] pNode->m_Str;
                    pNode->m_Str = NULL;
                }
                pList->RemoveHead();
                delete pNode;
            }
        }

        if (pList) {
            delete pList;
            pList = NULL;
        }

        pLineNode->Remove();
        delete pLineNode;
    }
    isload = false;
}

int KTabFileCtrl::WriteString(int nRow, int nColumn, char* lpString, unsigned long dwSize)
{
    if (dwSize == 0)
        dwSize = g_StrLen(lpString);
    return SetValue(nRow - 1, nColumn - 1, lpString, dwSize);
}

int KTabFileCtrl::WriteInteger(int nRow, int nColumn, int nValue)
{
    char IntNum[256] = {0};
    std::snprintf(IntNum, sizeof(IntNum), "%d", nValue);
    return SetValue(nRow - 1, nColumn - 1, IntNum, g_StrLen(IntNum));
}

int KTabFileCtrl::WriteFloat(int nRow, int nColumn, float fValue)
{
    char FloatNum[256] = {0};
    std::snprintf(FloatNum, sizeof(FloatNum), "%f", fValue);
    return SetValue(nRow - 1, nColumn - 1, FloatNum, g_StrLen(FloatNum));
}

int KTabFileCtrl::Str2Col(const char* szColumn)
{
    int nStrLen = g_StrLen(szColumn);
    char szTemp[4] = {0};

    g_StrCpy(szTemp, szColumn);
    g_StrUpper(szTemp);
    if (nStrLen == 1) {
        return (szTemp[0] - 'A');
    }
    return ((szTemp[0] - 'A' + 1) * 26 + szTemp[1] - 'A');
}

int KTabFileCtrl::Remove(int nRow)
{
    TTabLineNode* pNode = (TTabLineNode*)m_RowList.GetHead();
    if (!pNode)
        return FALSE;

    for (int i = 0; i < nRow - 1; i++) {
        if (!pNode)
            return FALSE;
        pNode = (TTabLineNode*)pNode->GetNext();
    }

    TTabLineNode* pDelNode = pNode;
    if (!pNode)
        return FALSE;

    pNode->Remove();

    if (pDelNode) {
        // Dọn các cột
        if (pDelNode->pList) {
            while (pDelNode->pList->GetHead()) {
                TTabColNode* pCol = (TTabColNode*)pDelNode->pList->GetHead();
                if (pCol) {
                    if (pCol->m_Str) {
                        delete[] pCol->m_Str;
                        pCol->m_Str = NULL;
                    }
                    pDelNode->pList->RemoveHead();
                    delete pCol;
                }
            }
            delete pDelNode->pList;
            pDelNode->pList = NULL;
        }
        delete pDelNode;
        pDelNode = NULL;
    }
    return TRUE;
}

int KTabFileCtrl::InsertAfter(int nRow)
{
    TTabLineNode* pNode = (TTabLineNode*)m_RowList.GetHead();
    if (!pNode)
        return FALSE;

    for (int i = 0; i < nRow - 1; i++) {
        if (!pNode)
            return FALSE;
        pNode = (TTabLineNode*)pNode->GetNext();
    }

    if (!pNode)
        return FALSE;

    TTabLineNode* pNewNode = new TTabLineNode;
    KList* pList = new KList;
    pNewNode->pList = pList;
    pNode->InsertAfter(pNewNode);
    return TRUE;
}

int KTabFileCtrl::InsertBefore(int nRow)
{
    TTabLineNode* pNode = (TTabLineNode*)m_RowList.GetHead();
    if (!pNode)
        return FALSE;

    for (int i = 0; i < nRow - 1; i++) {
        if (!pNode)
            return FALSE;
        pNode = (TTabLineNode*)pNode->GetNext();
    }

    if (!pNode)
        return FALSE;

    TTabLineNode* pNewNode = new TTabLineNode;
    KList* pList = new KList;
    pNewNode->pList = pList;
    pNode->InsertBefore(pNewNode);
    return TRUE;
}

int KTabFileCtrl::InsertNewCol(char* strNewCol)
{
    if (!strNewCol || !strNewCol[0])
        return -1;

    int nResult = -1;

    // Nếu đã có thì trả về vị trí hiện có
    if ((nResult = FindColumn(strNewCol)) > 0)
        return nResult;

    nResult = GetWidth() + 1;
    WriteString(1, nResult, strNewCol);
    return nResult;
}

