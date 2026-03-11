//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:    KPakList.h
// Date:    2000.08.08
// Code:    WangWei (Daphnis)
// Modify:  Wooy (2003-09-17)
// Refactor: Production C++20 (2026-03-11)
//---------------------------------------------------------------------------
#pragma once

// Mục đích (VI):
//  - Quản lý danh sách các gói (PAK) và cung cấp API tra cứu/đọc tệp con bên trong gói.

#include <cstdint>
#include <cstddef>
#include <array>
#include <string_view>
#include <utility>

#include "KPlatform.h"
#include "XPackFile.h"

// Ghi chú tương thích:
//  - Giữ nguyên macro MAX_PAK như bản gốc để tránh phá vỡ mã gọi ở nơi khác.
//  - Đồng thời cung cấp hằng constexpr để dùng nội bộ/hiện đại hoá.
#ifndef MAX_PAK
    #define MAX_PAK 50 // Số lượng PAK tối đa có thể quản lý đồng thời
#endif
static constexpr int kMaxPak = MAX_PAK;

class KPakList {
public:
    KPakList();
    ~KPakList();

    // Mở file danh sách PAK.
    // pPakListFile: đường dẫn tới file danh sách PAK.
    // nKind: kiểu mở/tuỳ chọn (giữ nguyên ý nghĩa cũ).
    // Trả về: 0 khi thất bại, >0 số PAK đã mở khi thành công.
    int         Open(char* pPakListFile, int nKind = 0);

    // Đóng tất cả PAK đã mở, giải phóng tài nguyên.
    void        Close();

    // Tìm tệp con theo ID (đã băm/tạo từ tên) trong toàn bộ PAK.
    // Trả về true nếu tìm thấy và điền ElemRef.
    bool        pFindElemFileA(unsigned int uId, XPackElemFileRef& ElemRef);

    // Tìm tệp con theo tên trong toàn bộ PAK.
    // Trả về true nếu tìm thấy và điền ElemRef.
    bool        pFindElemFile(const char* pszFileName, XPackElemFileRef& ElemRef);

    // Đọc dữ liệu tệp con vào bộ đệm pBuffer với kích thước uSize.
    // Trả về số byte đã đọc, hoặc <0 khi lỗi.
    int         ElemFileRead(XPackElemFileRef& ElemRef, void* pBuffer, unsigned uSize);

    // Lấy header SPR và bảng offset khung hình (SPROFFS*) cho một SPR.
    // Trả về con trỏ SPRHEAD, đồng thời trả về bảng offset qua tham chiếu pOffsetTable.
    SPRHEAD*    GetSprHeader(XPackElemFileRef& ElemRef, SPROFFS*& pOffsetTable);

    // Lấy dữ liệu một khung hình đã giải nén từ SPR.
    // nPackIndex: chỉ số PAK tương ứng (giữ nguyên ý nghĩa cũ).
    // pSprHeader: header SPR hợp lệ.
    // nFrame: chỉ số khung hình cần lấy.
    // nSingFrameSize: trả về kích thước dữ liệu của khung hình.
    // Trả về: con trỏ SPRFRAME hợp lệ hoặc nullptr nếu lỗi.
    SPRFRAME*   GetSprFrame(int nPackIndex, SPRHEAD* pSprHeader, int nFrame, unsigned int& nSingFrameSize);

    // Lấy đường dẫn thực tế của tệp nếu tồn tại trong PAK hoặc hệ thống file.
    // Trả về true nếu lấy được và điền ElemRef tương ứng.
    bool        pGetFilePath(const char* pszFileName, XPackElemFileRef& ElemRef);

private:
    // Chuyển tên file sang ID (hàm băm) sử dụng quy ước lịch sử của engine.
    unsigned int FileNameToId(const char* pszFileName);

private:
    // Lưu danh sách con trỏ tới các PAK đã mở. Sử dụng mảng tĩnh để tương thích memory pool/packet chain.
    XPackFile*  m_PakFilePtrList[MAX_PAK];
    int         m_nPakNumber; // Số lượng PAK hiện có trong danh sách.
};

// Biến toàn cục dùng chung theo thiết kế gốc. Vẫn được giữ nguyên để tương thích.
extern KPakList* g_pPakList;
