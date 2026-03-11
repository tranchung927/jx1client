#pragma once
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <stddef.h>

#include "cocos2d.h"
#pragma warning (disable: 4005)
USING_NS_AX;

// Cấu trúc thuộc tính bảo mật tương thích kiểu Win32 legacy.
typedef struct _SECURITY_ATTRIBUTES_S {
    unsigned int nLength;              // Kích thước cấu trúc.
    void* lpSecurityDescriptor;        // Con trỏ tới mô tả bảo mật.
    bool bInheritHandle;               // Có cho phép kế thừa handle hay không.
} SECURITY_ATTRIBUTES_S, *PSECURITY_ATTRIBUTES_S, *LPSECURITY_ATTRIBUTES_S;

//typedef unsigned short *PWORD;
//#define unsigned long* * unsigned long
//typedef unsigned int  *unsigned long*;
//typedef unsigned int  *LPDWORD;

// Các hằng dùng cho seek file theo kiểu Win32.
#define FILE_BEGIN           0
#define FILE_CURRENT         1
#define FILE_END             2

// Macro đánh dấu tham số đầu vào/đầu ra để giữ tương thích API cũ.
#define IN
#define OUT

// Kích thước đường dẫn tối đa dùng trong code legacy.
#define MAX_PATH 1200
//#define UINT unsigned int
//#define INT  int
//#define FALSE false
//#define TRUE  true
//
//#define PVOID void *
//
//#define int  int
//#define PBYTE unsigned char *
//#define LPBYTE PBYTE
////typedef unsigned char BYTE;
//
////UINT_PTR
//#define INVALID_HANDLE_VALUE ((HANDLE)(LONG_PTR)-1)
//#define INVALID_FILE_SIZE ((unsigned long)0xFFFFFFFF)

#define MAX_CHUNK    250        // Số phần tử khối tối đa dùng trong bộ đệm/tập dữ liệu legacy.
#define HANDLE void*

#if !defined(_WIN32)
//#if (CC_TARGET_PLATFORM != CC_PLATFORM_WIN32)
// Mã sự kiện kết nối máy chủ dùng trong tầng network legacy.
enum enumServerConnectInfo
{
    enumServerConnectCreate = 0x100, // Kết nối được tạo.
    enumServerConnectClose          // Kết nối đóng.
};
#define CP_UTF8 65001
typedef int* LPINT;
#define FLOAT float
#define LPCTSTR const char*
#define CONST const
#define SOCKET_ERROR -1
#define VOID void
#if !defined(CALLBACK)
#define CALLBACK
#endif
#define CHAR char
#define ULONG unsigned int
typedef int SOCKET;
#define SAFE_DELETE_ARRAY(p) { if (p) { delete[] (p); (p) = NULL; } }
#define RELEASE(a) if (a) { (a)->Release(); (a) = NULL; }
#define SAFE_DELETE(a) if (a) { delete (a); (a) = NULL; }
#define SAFE_CLOSEHANDLE(x) try { if (x) { close(x); (x) = NULL; } } catch (...) { }
typedef unsigned char UCHAR;
typedef unsigned short USHORT;
#define INVALID_SOCKET -1
#define ZeroMemory(x,y) memset((x), 0, (y))

typedef struct timeval TIMEVAL;
typedef struct linger LINGER;
typedef struct sockaddr_in SOCKADDR_IN;

// Cấu trúc bộ đệm gửi/nhận dữ liệu tương thích WSABUF của WinSock.
typedef struct {
    ULONG len;     // Độ dài bộ đệm.
    CHAR* buf;     // Con trỏ tới dữ liệu bộ đệm.
} WSABUF, *LPWSABUF;

//typedef void (CALLBACK *CALLBACK_CLIENT_EVENT )(void* lpParam,const unsigned int &ulnEventType);
typedef struct
{
    uintptr_t Internal;       // Trạng thái nội bộ.
    uintptr_t InternalHigh;   // Thông tin nội bộ mở rộng.
    union
    {
        struct
        {
            uint32_t Offset;      // Offset thấp trong file/stream.
            uint32_t OffsetHigh;  // Offset cao trong file/stream.
        };
        void* Pointer;            // Con trỏ thay thế cho offset.
    };
    void* hEvent;             // Sự kiện đồng bộ gắn với thao tác I/O.
} OVERLAPPED;

// Kích thước 2 chiều.
typedef struct tagSIZE
{
    int cx; // Chiều rộng.
    int cy; // Chiều cao.
} SIZE, *PSIZE, *LPSIZE;

// Điểm tọa độ 2D.
typedef struct
{
    int x; // Tọa độ X.
    int y; // Tọa độ Y.
} POINT;

// Hình chữ nhật theo kiểu Win32.
typedef struct tagRECT
{
    int left;    // Biên trái.
    int top;     // Biên trên.
    int right;   // Biên phải.
    int bottom;  // Biên dưới.
} RECT;
//#define MAKELONG(a, b) ((int)(((unsigned short)(((DWORD_PTR)(a)) & 0xffff)) | ((unsigned long)((unsigned short)(((DWORD_PTR)(b)) & 0xffff))) << 16))
//#define LOWORD(l)      ((unsigned short)(((DWORD_PTR)(l)) & 0xffff))
//#define HIWORD(l)      ((unsigned short)((((DWORD_PTR)(l)) >> 16) & 0xffff))
//#define LOBYTE(w)      ((BYTE)(((DWORD_PTR)(w)) & 0xff))
//#define HIBYTE(w)      ((BYTE)((((DWORD_PTR)(w)) >> 8) & 0xff))

// Nhóm macro tách/gộp byte-word-long theo phong cách Win32.
#    define MAKEWORD(a, b) \
        ((uint16_t)(((uint8_t)((uintptr_t)(a) & 0xff)) | ((uint16_t)((uint8_t)((uintptr_t)(b) & 0xff))) << 8))
#    define MAKELONG(a, b) \
        ((int32_t)(((uint16_t)((uintptr_t)(a) & 0xffff)) | ((uint32_t)((uint16_t)((uintptr_t)(b) & 0xffff))) << 16))
#    define LOWORD(l) ((uint16_t)(((uintptr_t)(l)) & 0xffff))
#    define HIWORD(l) ((uint16_t)((((uintptr_t)(l)) >> 16) & 0xffff))
#    define LOBYTE(w) ((uint8_t)(((uintptr_t)(w)) & 0xff))
#    define HIBYTE(w) ((uint8_t)((((uintptr_t)(w)) >> 8) & 0xff))

// Đồng bộ kiểu dữ liệu theo Win32 legacy nhưng an toàn cho x64
typedef uintptr_t WPARAM;
typedef intptr_t  LPARAM;
typedef intptr_t  LRESULT;

typedef unsigned short* PWORD;
typedef uint32_t* PDWORD;
typedef uint32_t* LPDWORD;

#define BYTE   unsigned char
#define PBYTE  unsigned char*
#define UINT   uint32_t
#define INT    int32_t
#define FALSE  0
#define TRUE   1
#define DWORD  uint32_t
#define WORD   unsigned short
#define BOOL   int32_t
#define LPCSTR const char*
#define PVOID  void*
#define LPVOID PVOID
#define LPSTR  char*
#define LONG   int32_t
#define LPBYTE PBYTE
#define LPTSTR LPSTR

typedef intptr_t  LONG_PTR;
typedef uintptr_t DWORD_PTR;
typedef uintptr_t DWORD_PTRD;
typedef uintptr_t UINT_PTR;
typedef uintptr_t ULONG_PTR;
#define INVALID_HANDLE_VALUE ((HANDLE)(LONG_PTR)-1)
#define INVALID_FILE_SIZE    ((unsigned long)0xFFFFFFFF)
#endif // !defined(_WIN32)

#ifndef BYTE
#define BYTE unsigned char
#endif

#ifndef WORD
#define WORD unsigned short
#endif

#ifndef DWORD
#define DWORD uint32_t
#endif

#ifndef LONG
#define LONG int32_t
#endif

#ifndef BOOL
#define BOOL int32_t
#endif

// Cấu trúc màu 32-bit theo thứ tự BGRA.
typedef struct {
    BYTE Blue;   // Thành phần màu xanh dương.
    BYTE Green;  // Thành phần màu xanh lá.
    BYTE Red;    // Thành phần màu đỏ.
    BYTE Alpha;  // Thành phần alpha/độ trong suốt.
} KPAL32;

//---------------------------------------------------------------------------
 
// Cấu trúc màu 24-bit theo thứ tự RGB.
typedef struct {
    BYTE Red;    // Thành phần màu đỏ.
    BYTE Green;  // Thành phần màu xanh lá.
    BYTE Blue;   // Thành phần màu xanh dương.
} KPAL24;

//---------------------------------------------------------------------------
 
// Màu 16-bit đóng gói trong một từ 16-bit.
typedef unsigned short KPAL16;

// Định dạng ảnh dùng trong engine/resource loader.
enum eImageFormat
{
    IMAGE_FORMAT_NULL, // Không có định dạng.
    IMAGE_8888,        // 32-bit RGBA.
    IMAGE_888,         // 24-bit RGB.
    IMAGE_565,         // 16-bit RGB565.
    IMAGE_4444,        // 16-bit RGBA4444.
    IMAGE_256,         // 8-bit palette.
};

// Thông tin ảnh đã nạp vào bộ nhớ.
struct stImageInfo
{
    unsigned int nDirections;  // Số hướng hiển thị.
    unsigned int nFrames;      // Số frame.
    int bytes_per_pixel;       // Số byte trên mỗi pixel.
    int size;                  // Tổng kích thước bộ đệm ảnh.
    BYTE* buffer;              // Dữ liệu ảnh.
    int width, height;         // Kích thước ảnh.
    int x, y;                  // Tọa độ/offset gốc.
};

// Header file BMP theo layout legacy của project.
typedef struct
{
    unsigned short bfType;       // Loại file BMP, thường là 'BM'.
    unsigned long bfSize;        // Kích thước toàn bộ file tính theo byte.
    unsigned short bfReserved1;  // Trường dự phòng, thường bằng 0.
    unsigned short bfReserved2;  // Trường dự phòng, thường bằng 0.
    unsigned long bfOffBits;     // Offset tới vùng dữ liệu pixel.
    // File header offset tính theo byte.
} BITMAPFILEHEADERS;

// Header thông tin ảnh BMP theo layout legacy của project.
typedef struct
{
    unsigned long biSize;          // Kích thước cấu trúc header.
    int biWidth;                   // Chiều rộng ảnh.
    int biHeight;                  // Chiều cao ảnh.
    unsigned short biPlanes;       // Số mặt phẳng màu, thường bằng 1.
    unsigned short biBitCount;     // Số bit trên mỗi pixel.
    unsigned long biCompression;   // Kiểu nén ảnh BMP.
    unsigned long biSizeImage;     // Kích thước vùng dữ liệu ảnh.
    int biXPelsPerMeter;           // Độ phân giải ngang theo mét.
    int biYPelsPerMeter;           // Độ phân giải dọc theo mét.
    unsigned long biClrUsed;       // Số màu thực sự được sử dụng.
    unsigned long biClrImportant;  // Số màu quan trọng.
} BITMAPINFOHEADERS;

// Cấu trúc màu 24-bit dùng trong một số layout BMP/palette cũ.
typedef struct {
    BYTE Red;    // Thành phần màu đỏ.
    BYTE Green;  // Thành phần màu xanh lá.
    BYTE Blue;   // Thành phần màu xanh dương.
} KPAL244;

// Một phần tử màu trong bảng màu BMP.
typedef struct
{
    BYTE rgbBlue;      // Cường độ màu xanh dương.
    BYTE rgbGreen;     // Cường độ màu xanh lá.
    BYTE rgbRed;       // Cường độ màu đỏ.
    BYTE rgbReserved;  // Byte dự phòng, thường bằng 0.
} RGBQUADS;

// Cấu trúc thông tin BMP gồm header và bảng màu mở rộng.
typedef struct
{
    BITMAPINFOHEADERS bmiHeader; // Header thông tin BMP.
    KPAL244 bmiColors[1];        // Phần tử đầu tiên của bảng màu.
} BITMAPINFOS;

// Header của file sprite định dạng SPR.
typedef struct
{
    BYTE Comment[4];           // Chữ ký định dạng, thường là "SPR\0".
    unsigned short Width;      // Chiều rộng ảnh nguồn.
    unsigned short Height;     // Chiều cao ảnh nguồn.
    unsigned short CenterX;    // Tâm theo trục X.
    unsigned short CenterY;    // Tâm theo trục Y.
    unsigned short Frames;     // Số frame.
    unsigned short Colors;     // Số màu/palette.
    unsigned short Directions; // Số hướng hiển thị.
    unsigned short nInterval;  // Khoảng thời gian giữa các frame.
    unsigned short Reserved[6];// Vùng dự phòng cho mở rộng về sau.
} SPRHEAD;

#define SPR_COMMENT_FLAG 0x525053 // Chữ ký 'SPR'

// Offset và độ dài của từng frame trong file SPR.
typedef struct
{
    unsigned int Offset; // Vị trí bắt đầu của frame.
    unsigned int Length; // Kích thước dữ liệu frame.
} SPROFFS;

// Dữ liệu một frame sprite sau khi giải mã header.
typedef struct
{
    unsigned short Width;    // Chiều rộng frame.
    unsigned short Height;   // Chiều cao frame.
    unsigned short OffsetX;  // Độ lệch X so với gốc ảnh.
    unsigned short OffsetY;  // Độ lệch Y so với gốc ảnh.
    BYTE Sprite[1];          // Dữ liệu sprite nén RLE.
} SPRFRAME;

// Danh mục thư mục con dùng trong resource/package system.
enum enumSubDir { sdFont = 0, sdMaps, sdScript, sdResource, sdSetting, sdSound, sdSpr, sdUi, sdLastDir };

// Cấu trúc header của một file PACK/PAK.
struct XPackFileHeader
{
    unsigned char cSignature[4]; // Chữ ký file, cố định là "PACK".
    unsigned int uCount;         // Số mục dữ liệu trong gói.
    unsigned int uIndexTableOffset; // Offset tới bảng chỉ mục.
    unsigned int uDataOffset;       // Offset tới vùng dữ liệu.
    unsigned int uCrc32;            // CRC32 của gói dữ liệu.
    unsigned int uPakTime;          // Thời điểm tạo pack theo time().
    unsigned char cReserved[8];     // Vùng dự phòng.
};

#define XPACKFILE_SIGNATURE_FLAG 0x4b434150 // Chữ ký 'PACK'

// Thông tin chỉ mục của từng file con trong pack.
struct XPackIndexInfo
{
    unsigned int uId;             // ID của file con.
    unsigned int uOffset;         // Offset của file con trong pack.
    int lSize;                    // Kích thước gốc của file con.
    int lCompressSizeFlag;        // Kích thước nén và cờ phương thức nén.
    // Byte cao biểu diễn phương thức nén theo XPACK_METHOD.
    // Các byte còn lại biểu diễn kích thước sau nén.
};

// Thông tin nén của một frame sprite lưu trong pack.
struct XPackSprFrameInfo
{
    int lCompressSize; // Kích thước sau nén.
    int lSize;         // Kích thước gốc.
};

// Dữ liệu file nén phân mảnh được lưu theo thứ tự:
// [XPackFileFragmentElemHeader] + [mảnh 1] + [mảnh 2] + ... + [mảnh n] + [bảng XPackFileFragmentInfo].

// Header mô tả vùng dữ liệu phân mảnh của một file.
struct XPackFileFragmentElemHeader
{
    int nNumFragment;         // Số lượng mảnh dữ liệu.
    int nFragmentInfoOffest;  // Offset tới bảng thông tin mảnh, tính từ đầu header này.
};

// Thông tin của một mảnh dữ liệu nén.
struct XPackFileFragmentInfo
{
    unsigned int uOffset;           // Offset của mảnh dữ liệu, tính từ đầu header phân mảnh.
    unsigned int uSize;             // Kích thước gốc của mảnh.
    unsigned int uCompressSizeFlag; // Kích thước nén và cờ phương thức nén.
};

// Các cờ/mặt nạ mô tả phương thức nén file trong pack.
enum XPACK_METHOD
{
    TYPE_NONE              = 0x00000000, // Không nén.
    TYPE_UCL               = 0x10000000, // Nén UCL kiểu mới.
    TYPE_BZIP2             = 0x20000000, // Nén BZIP2/UCL theo quy ước cũ của dự án.
    TYPE_FRAGMENT          = 0x30000000, // Dữ liệu nén theo nhiều mảnh.
    TYPE_FRAGMENTA         = 0x40000000, // Biến thể nén phân mảnh.
    TYPE_FRAME             = 0x10000000, // Dữ liệu nén theo frame, thường dùng cho SPR.
    TYPE_METHOD_FILTER     = 0xf0000000, // Mặt nạ lấy phương thức nén kiểu mới.
    TYPE_FILTER            = 0xf0000000, // Mặt nạ lọc phương thức kiểu mới.
    TYPE_FILTER_SIZE       = 0x07ffffff, // Mặt nạ lấy kích thước nén kiểu mới.
    TYPE_UCL_OLD           = 0x01000000, // Nén UCL kiểu cũ.
    TYPE_BZIP2_OLD         = 0x02000000, // Nén BZIP2/UCL kiểu cũ.
    TYPE_METHOD_FILTER_OLD = 0x0f000000, // Mặt nạ phương thức nén kiểu cũ.
    TYPE_FILTER_OLD        = 0xff000000, // Mặt nạ lọc kiểu cũ.
    TYPE_FRAGMENT_OLD      = 0x03000000, // Nén phân mảnh kiểu cũ.
    TYPE_FRAGMENTA_OLD     = 0x04000000, // Biến thể nén phân mảnh kiểu cũ.
    TYPE_FILTER_SIZE_OLD   = 0x00ffffff, // Mặt nạ kích thước nén kiểu cũ.
    XPACK_COMPRESS_SIZE_BIT = 27,        // Số bit dành cho kích thước nén.
};

//--------------------------------------------------
// Thông tin tham chiếu tới một file con khi truy cập qua XPackFile.
//--------------------------------------------------
struct XPackElemFileRef
{
    unsigned int uId;   // ID file trong PAK.
    int nPackIndex;     // Chỉ số gói PAK.
    int nElemIndex;     // Chỉ số phần tử trong gói.
    int nCacheIndex;    // Chỉ số cache.
    int nOffset;        // Offset hiện tại khi đọc file.
    int nSize;          // Kích thước file.
};
