// ---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:   KFilePath.h
// Mục đích: Khai báo các hàm tiện ích thao tác đường dẫn/tên tệp cho engine.
// ---------------------------------------------------------------------------
#pragma once

#include <cstddef>

// Hằng số độ dài đường dẫn/tên tệp.
#ifndef MAXPATH
#    define MAXPATH  260
#    define MAXDIR   256
#    define MAXFILE  256
#    define MAXEXT   256
#    define MAXDRIVE 3
#endif

// Lưu ý tương thích:
// - Giữ nguyên chữ ký hàm (tên, tham số, kiểu trả về) để không phá vỡ ABI.
// - Không thay đổi layout của bất kỳ POD/struct nào (file này chỉ khai báo hàm).
// - Không đưa thêm namespace gây thay đổi liên kết (linkage) của các hàm C-style.

// Đặt đường dẫn gốc của trò chơi.
// lpPathName: nếu NULL, tự suy ra theo nền tảng; isChange (Windows) quyết định lấy CWD.
void g_SetRootPath(char* lpPathName = NULL, bool isChange = false);

// Lấy đường dẫn gốc hiện tại vào lpPathName.
void g_GetRootPath(char* lpPathName);

// Đặt đường dẫn thư mục hiện tại (để ghép đường dẫn tương đối).
void g_SetFilePath(const char* lpPathName);

// Lấy đường dẫn thư mục hiện tại vào lpPathName.
void g_GetFilePath(char* lpPathName);

// Lấy đường dẫn đầy đủ (full path) từ root/curr và tên tệp.
void g_GetFullPath(char* lpPathName, const char* lpFileName);

// Lấy nửa đường dẫn (half path) từ thư mục hiện tại và tên tệp.
void g_GetHalfPath(char* lpPathName, char* lpFileName);

// Lấy đường dẫn nội bộ trong gói nén (pack) từ tên tệp.
void g_GetPackPath(char* lpPathName, char* lpFileName);

// Lấy đường dẫn trên đĩa (CDROM) theo logic cũ.
void g_GetDiskPath(char* lpPathName, char* lpFileName);

// Tạo đường dẫn (thư mục) bên trong thư mục trò chơi.
void g_CreatePath(char* lpPathName);

// Hợp nhất một đường dẫn và một tên tệp vào lpGet để tạo thành đường dẫn đầy đủ.
void g_UnitePathAndName(const char* lpPath, const char* lpFile, char* lpGet);

// Kiểm tra tệp tồn tại (trong pak hoặc trên đĩa). Trả về TRUE/FALSE.
int g_FileExists(char* lpPathName);

// Hash tên tệp thành ID 32-bit (giữ nguyên thuật toán cũ).
unsigned int g_FileName2Id(char* lpFileName);

// Thay đổi phần mở rộng của tệp.
void g_ChangeFileExt(char* lpFileName, char* lpNewExt);

// Tách tên tệp từ đường dẫn đầy đủ.
void g_ExtractFileName(char* lpFileName, char* lpFilePath);

// Tách đường dẫn thư mục từ đường dẫn đầy đủ.
void g_ExtractFilePath(char* lpPathName, char* lpFilePath);

// Kiểm tra tồn tại (hash) theo thuật toán đặc thù, trả về mã 32-bit; -1u nếu lỗi.
unsigned int g_CheckFileExist(const char* lpFileName);

