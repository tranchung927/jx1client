//    Đọc gói (pack file)
//    Bản quyền : Kingsoft 2003
//    Tác giả   : Wooy (Wu Yue)
//    Thời gian : 2003-09-16
//
#pragma once

#include "KPlatform.h"
#include <cstdint>
#include <cstddef>
#include <string>

class XPackFile
{
public:
    XPackFile();
    ~XPackFile();

    // Mở tệp gói (pack file)
    bool Open(const char* pszPackFileName, int nSelfIndex);
    // bool OpenX(const char* pszPackFileName, int nSelfIndex); // (giữ nguyên comment tắt)

    // Đóng tệp gói
    void Close();

    // Tìm tệp con trong gói theo ID, trả về tham chiếu đến tệp con
    bool XFindElemFile(unsigned int uId, XPackElemFileRef& ElemRef);

    // Đọc dữ liệu từ tệp con đã tìm được
    int XElemFileRead(XPackElemFileRef& ElemRef, void* pBuffer, unsigned uSize);

    // Lấy header của tệp spr và bảng offset khung hình
    SPRHEAD* GetSprHeader(XPackElemFileRef& ElemRef, SPROFFS*& pOffsetTable);

    // Lấy một khung hình nén từ spr theo chỉ số khung
    SPRFRAME* GetSprFrame(SPRHEAD* pSprHeader, int nFrame, unsigned int& nSingFrameSize);

    // Tên tệp gói đã mở (thuận tiện cho logging/kiểm tra)
    std::string fileName;

private:
    // Đọc trực tiếp dữ liệu thô từ gói tại offset/size chỉ định
    bool DirectRead(void* pBuffer, unsigned int uOffset, unsigned int uSize) const;

    // Đọc và giải nén dữ liệu từ gói (theo kiểu nén chỉ định)
    bool ExtractRead(void* pBuffer, unsigned int uExtractSize,
                     int lCompressType, unsigned int uOffset, unsigned int uSize) const;

    // Tìm tệp con theo ID trong bảng chỉ mục (trả về chỉ số hoặc -1 nếu không thấy)
    int XFindElemFileA(unsigned int ulId) const;

    // Các hàm giải nén (giữ nguyên chữ ký để tương thích)
    int ucl_nrv2e_decompress_8_fs(unsigned char* s_buf, int s_size, unsigned char* d_buf, unsigned int* d_size) const;
    int ucl_nrv2e_decompress_8_3(unsigned char* a1, unsigned int a2, unsigned char* a3, unsigned int* a4) const;
    int ucl_nrv2e_decompress_8_2(unsigned char* a1, unsigned int a2, unsigned char* a3, unsigned int* a4) const;
    int ucl_nrv2e_decompress_8_1(unsigned char* a1, unsigned int a2, unsigned char* a3, unsigned int* a4) const;
    int ucl_decompress_8_New(unsigned char* a1, unsigned int a2, unsigned char* a3, unsigned int* a4) const;

    // Tìm một tệp con trong cache theo ID (ưu tiên vị trí mong muốn)
    int FindElemFileInCache(unsigned int uId, int nDesireIndex);

    // Thêm dữ liệu tệp con vào cache
    int AddElemFileToCache(void* pBuffer, int nElemIndex);

    // Đọc toàn bộ dữ liệu của một tệp con chỉ định
    void* ReadElemFile(int nElemIndex) const;

    // Giải phóng một mục cache
    static void FreeElemCache(int nCacheIndex);

private:
    // Trình xử lý tệp gói (FILE* cho đa nền tảng)
    // HANDLE m_hFile; // bỏ dùng: tay nắm file kiểu Windows
    FILE* m_hFile;

    // Kích thước tệp gói (byte)
    unsigned int m_uFileSize;

    // Số lượng tệp con trong gói
    int m_nElemFileCount;

    // Chỉ số của chính gói này (khi nằm trong chuỗi gói)
    int m_nSelfIndex;

    // Danh sách chỉ mục các tệp con
    struct XPackIndexInfo* m_pIndexList;
    // struct XPackIndexInfoX* m_pIndexListX; // (giữ nguyên comment tắt)

#if (CC_TARGET_PLATFORM != CC_PLATFORM_WIN32)
    // Khóa đọc (Unix-like)
    pthread_mutex_t m_ReadCritical;
#else
    // Khóa khi thao tác với pack (Windows)
    CRITICAL_SECTION m_ReadCritical;
#endif

    //---- Cache dữ liệu file con ----
    struct XPackElemFileCache
    {
        void* pBuffer;     // Bộ đệm dữ liệu của file con
        DWORD uId;         // ID file con
        LONG  lSize;       // Kích thước file con
        INT   nPackIndex;  // Thuộc gói nào
        INT   nElemIndex;  // Vị trí trong bảng chỉ mục
        DWORD uRefFlag;    // Cờ tham chiếu gần đây
    };

    #define MAX_XPACKFILE_CACHE 10

    // Dữ liệu cache của file con
    static XPackElemFileCache ms_ElemFileCache[MAX_XPACKFILE_CACHE];

    // Số lượng file con đang được cache
    static int ms_nNumElemFileCache;
};
