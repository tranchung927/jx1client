//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:    KPakFile.h
// Date:    2000.08.08
// Code:    WangWei (Daphnis)
// Desc:    Header File
// Refactor: Production C++20 (2026-03-11)
//---------------------------------------------------------------------------

#pragma once

// Mục đích (VI):
//  - Cung cấp lớp KPakFile để truy cập tệp: ưu tiên đọc từ PAK nếu có, ngược lại từ hệ thống tệp.

#include <cstddef>
#include <cstdint>

#include "KPlatform.h"
#include "KFile.h"
#include "XPackFile.h"

//---------------------------------------------------------------------------
// Thiết lập chế độ mở tệp cho KPakFile (theo quy ước của engine).
void g_SetPakFileMode(int nFileMode);

//---------------------------------------------------------------------------
class KPakFile {
public:
    KPakFile();
    ~KPakFile();

    // Mở tệp theo tên. Nếu tệp tồn tại trong PAK, ưu tiên mở từ PAK.
    int                 Open(const char* pszFileName);

    // Đóng tệp đang mở (trong PAK hoặc hệ thống tệp).
    void                Close();

    // Kiểm tra xem tệp hiện tại có đang được đọc từ PAK không.
    bool                IsFileInPak();

    // Đọc dữ liệu từ vị trí hiện tại vào pBuffer, kích thước uSize byte.
    unsigned long       Read(void* pBuffer, unsigned int uSize);

    // Di chuyển vị trí đọc/ghi.
    unsigned long       Seek(int nOffset, unsigned int uMethod);

    // Lấy vị trí hiện tại trong tệp.
    unsigned long       Tell();

    // Lấy kích thước tệp.
    unsigned long       Size();

    // Lưu tệp ra hệ thống tệp với tên mới (nếu cần).
    int                 Save(const char* pszFileName);

    // Truy cập đối tượng KFile nền tảng (trường hợp tệp không nằm trong PAK).
    KFile               getFilePtr() { return m_File; };

private:
    KFile               m_File;      // Tệp thực trên hệ thống tệp (khi không ở trong PAK)
    XPackElemFileRef    m_PackRef;   // Tham chiếu tới tệp con trong PAK (nếu có)
};

//---------------------------------------------------------------------------
// Cấu trúc dữ liệu ảnh KSG (định dạng lịch sử của engine)
struct KSGImageContent {
    int             nWidth;
    int             nHeight;
    unsigned short  Data[1];
};

// Tính kích thước bộ đệm cần thiết cho KSGImageContent với (w,h)
#define KSG_IMAGE_CONTENT_SIZE(w, h) ((unsigned)((&((KSGImageContent *)0)->Data[0])) + (unsigned)(w) * (unsigned)(h) * 2)

// Các API liên quan tới tài nguyên SPR (sprite) - giữ nguyên giao diện cũ.
SPRHEAD*    SprGetHeader(char* pszFileName, SPROFFS*& pOffsetTable);
void        SprReleaseHeader(SPRHEAD* pSprHeader);
SPRFRAME*   SprGetFrame(SPRHEAD* pSprHeader, int nFrame, unsigned int& nSingFrameSize);
void        SprReleaseFrame(SPRFRAME* pFrame);

// Tải ảnh JPG theo định dạng KSG, trả về bộ đệm ảnh 16-bit. Caller chịu trách nhiệm giải phóng.
KSGImageContent*   get_jpg_image(const char cszName[], unsigned uRGBMask16 = ((unsigned)-1));
void               release_image(KSGImageContent* pImage);

