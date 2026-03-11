//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:   KFile.h
//---------------------------------------------------------------------------
#pragma once

#include <cstdint>
#include <cstddef>
#include <cstdio>

#include "KPlatform.h"

// Lớp bao bọc FILE* theo phong cách engine legacy.
// Mục tiêu là giữ API cũ nhưng tương thích tốt hơn với C++ hiện đại.

// Giá trị lỗi khi seek
#define SEEK_ERROR 0xFFFFFFFFu

class KFile
{
private:
    FILE*        m_hFile;  // File Handle
    DWORD        m_dwLen;  // Kích thước file
    DWORD        m_dwPos;  // Vị trí con trỏ file

public:
    KFile();
    ~KFile();

    // Mở file hiện có để đọc.
    BOOL Open(LPCSTR FileName);

    // Tạo file mới hoặc ghi đè file hiện có.
    BOOL Create(LPCSTR FileName);

    // Mở file ở chế độ nối thêm dữ liệu vào cuối file.
    BOOL Append(LPCSTR FileName);

    // Đóng file
    void Close();

    // Đọc dữ liệu từ file vào bộ đệm đích.
    DWORD Read(LPVOID lpBuffer, DWORD dwReadBytes);

    // Ghi dữ liệu từ bộ đệm nguồn ra file.
    DWORD Write(LPCVOID lpBuffer, DWORD dwWriteBytes);

    // Di chuyển vị trí con trỏ file theo kiểu seek của C/Win32.
    DWORD Seek(LONG lDistance, DWORD dwMoveMethod);

    // Lấy vị trí hiện tại của con trỏ file.
    DWORD Tell();

    // Lấy kích thước file tính theo byte.
    DWORD Size();

    // Trả về FILE* nội bộ để tương thích với mã legacy.
    FILE* getFilePtr() { return m_hFile; }
};
//---------------------------------------------------------------------------
