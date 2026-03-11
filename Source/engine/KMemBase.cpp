//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KMemBase.cpp
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Memory base functions
//---------------------------------------------------------------------------

#include "KNode.h"
#include "KList.h"
#include "KMemBase.h"
#include <cstring>
#include <mutex>
#include <cstdio>

//---------------------------------------------------------------------------
// Memory tracking node
//---------------------------------------------------------------------------

class KMemNode : public KNode
{
public:
    DWORD m_dwMemSize;
    DWORD m_dwMemSign;
};

//---------------------------------------------------------------------------

class KMemList : public KList
{
public:
    ~KMemList()
    {
        KMemNode* pNode = (KMemNode*)GetHead();
        while (pNode)
        {
            std::fprintf(stderr, "KMemList::Leak Detected, Size = %u\n", pNode->m_dwMemSize);
            pNode = (KMemNode*)pNode->GetNext();
        }
    }

    void ShowUsage()
    {
        KMemNode* pNode = (KMemNode*)GetHead();
        DWORD total = 0;

        while (pNode)
        {
            total += pNode->m_dwMemSize;
            pNode = (KMemNode*)pNode->GetNext();
        }

        std::fprintf(stderr, "Memory Usage Size = %u KB\n", total / 1024);
    }
};

//---------------------------------------------------------------------------

static KMemList m_MemList;
static std::mutex g_mem_mutex;

#define MEMSIGN 1234567890

//---------------------------------------------------------------------------

void g_MemInfo()
{
    // Reserved for future platform memory stats
}

//---------------------------------------------------------------------------

LPVOID g_MemAlloc(DWORD dwSize)
{
    if (dwSize == 0)
        return nullptr;

    DWORD totalSize = dwSize + sizeof(KMemNode);

    auto* raw = new(std::nothrow) unsigned char[totalSize];

    if (!raw)
    {
        std::fprintf(stderr, "g_MemAlloc() Failed, Size = %u\n", dwSize);
        return nullptr;
    }

    auto* node = reinterpret_cast<KMemNode*>(raw);

    node->m_pPrev = nullptr;
    node->m_pNext = nullptr;
    node->m_dwMemSize = dwSize;
    node->m_dwMemSign = MEMSIGN;

    {
        std::lock_guard<std::mutex> lock(g_mem_mutex);
        m_MemList.AddHead(node);
    }

    return raw + sizeof(KMemNode);
}

//---------------------------------------------------------------------------

void g_MemFree(LPVOID lpMem)
{
    if (!lpMem)
        return;

    auto* raw = reinterpret_cast<unsigned char*>(lpMem) - sizeof(KMemNode);
    auto* node = reinterpret_cast<KMemNode*>(raw);

    if (node->m_dwMemSign != MEMSIGN)
    {
        std::fprintf(stderr, "g_MemFree() Failed, Corrupted Memory Block\n");
        return;
    }

    {
        std::lock_guard<std::mutex> lock(g_mem_mutex);
        node->Remove();
    }

    delete[] raw;
}

//---------------------------------------------------------------------------

void g_MemCopy(PVOID lpDest, PVOID lpSrc, DWORD dwLen)
{
    if (!lpDest || !lpSrc || dwLen == 0)
        return;

    std::memcpy(lpDest, lpSrc, dwLen);
}

//---------------------------------------------------------------------------

void g_MemCopyMmx(PVOID lpDest, PVOID lpSrc, DWORD dwLen)
{
    // Modern compilers already optimize memcpy
    g_MemCopy(lpDest, lpSrc, dwLen);
}

//---------------------------------------------------------------------------

BOOL g_MemComp(PVOID lpDest, PVOID lpSrc, DWORD dwLen)
{
    if (!lpDest || !lpSrc)
        return FALSE;

    return (std::memcmp(lpDest, lpSrc, dwLen) == 0) ? TRUE : FALSE;
}

//---------------------------------------------------------------------------

void g_MemFill(PVOID lpDest, DWORD dwLen, BYTE byFill)
{
    if (!lpDest || dwLen == 0)
        return;

    std::memset(lpDest, byFill, dwLen);
}

//---------------------------------------------------------------------------

void g_MemFill(PVOID lpDest, DWORD dwLen, WORD wFill)
{
    if (!lpDest)
        return;

    WORD* ptr = reinterpret_cast<WORD*>(lpDest);

    DWORD count = dwLen / sizeof(WORD);

    for (DWORD i = 0; i < count; ++i)
        ptr[i] = wFill;
}

//---------------------------------------------------------------------------

void g_MemFill(PVOID lpDest, DWORD dwLen, DWORD dwFill)
{
    if (!lpDest)
        return;

    DWORD* ptr = reinterpret_cast<DWORD*>(lpDest);

    DWORD count = dwLen / sizeof(DWORD);

    for (DWORD i = 0; i < count; ++i)
        ptr[i] = dwFill;
}

//---------------------------------------------------------------------------

void g_MemZero(PVOID lpDest, DWORD dwLen)
{
    if (!lpDest || dwLen == 0)
        return;

    std::memset(lpDest, 0, dwLen);
}

//---------------------------------------------------------------------------

void g_MemXore(PVOID lpDest, DWORD dwLen, DWORD dwXor)
{
    if (!lpDest)
        return;

    DWORD* ptr = reinterpret_cast<DWORD*>(lpDest);

    DWORD count = dwLen / sizeof(DWORD);

    for (DWORD i = 0; i < count; ++i)
        ptr[i] ^= dwXor;
}

//---------------------------------------------------------------------------
