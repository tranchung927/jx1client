#pragma once
//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:   KStrBase.h
// Date:   2000.08.08 (cập nhật: 2026.03.11)
// Code:   WangWei(Daphnis) — chỉnh sửa hiện đại hoá C++20
// Mục đích:
//  - Cung cấp tập hàm tiện ích xử lý chuỗi/định dạng/in ấn bậc thấp cho engine.
//---------------------------------------------------------------------------

#include <cstdint>
#include <cstddef>
#include <string>
#include "KPlatform.h"

//---------------------------------------------------------------------------
// API xử lý chuỗi bậc thấp (giữ nguyên chữ ký). Các hàm này hoạt động tương tự
// như strlen/strcpy/strcat/strcmp nhưng được tuỳ biến cho engine.
//---------------------------------------------------------------------------
int     g_StrLen(const char* lpStr);                                  // Độ dài chuỗi (không tính '\0')
char*   g_StrEnd(const char* lpStr);                                   // Con trỏ tới ký tự '\0' kết thúc
void    g_StrCpy(char* lpDest, const char* lpSrc);                     // Sao chép chuỗi (KHÔNG kiểm tra kích thước)
void    g_StrCpyLen(char* lpDest, const char* lpSrc, int nMaxLen);     // Sao chép tối đa nMaxLen ký tự, luôn kết thúc '\0' nếu có thể
void    g_StrCat(char* lpDest, const char* lpSrc);                     // Nối chuỗi (KHÔNG kiểm tra kích thước)
void    g_StrCatLen(char* lpDest, const char* lpSrc, int nMaxLen);     // Nối tối đa nMaxLen ký tự, đảm bảo kết thúc đúng
int     g_StrCmp(const char* lpDest, const char* lpSrc);               // So sánh chuỗi (giống strcmp)
int     g_StrCmpLen(const char* lpDest, const char* lpSrc, int nMaxLen); // So sánh tối đa nMaxLen ký tự
void    g_StrUpper(char* lpDest);                                      // Chuyển chuỗi sang CHỮ HOA (tại chỗ)
void    g_StrLower(char* lpDest);                                      // Chuyển chuỗi sang chữ thường (tại chỗ)
void    g_StrRep(char* lpDest, const char* lpSrc, const char* lpRep);  // Thay thế chuỗi con: src -> rep vào dest

// "Chuyển chuỗi số sang UINT chuyên dụng".
// Giải nghĩa: chuyển đổi chuỗi số unsigned (thập phân) sang UINT, bỏ qua khoảng trắng đầu/cuối.
UINT    g_Atoui(char* str);

// Sao chép đường dẫn thẻ nhớ và tên nhạc vào chuỗi đích (giữ nguyên API gốc).
// Lưu ý: tham số std::string truyền theo giá trị trong API gốc; để tương thích, không đổi.
void    g_StrCopy(std::string nSdcardPath, char* pMusicName);

// Trích/ký tự thay thế trong chuỗi:
//  - inStr: chuỗi đầu vào
//  - scrchar: ký tự nguồn cần tìm
//  - destchar: ký tự đích thay thế
//  - outStr/outStra: tuỳ chọn, nhận chuỗi kết quả/phụ (có thể NULL)
//  - nMoedel (giữ nguyên chính tả theo API cũ): chế độ xử lý
int     g_ExtractChar(const char* inStr, char scrchar, char destchar,
                      char* outStr = NULL, char* outStra = NULL, int nMoedel = 0);

// Làm tròn số:
int     g_Round2Int(double d);    // Làm tròn về int theo quy tắc 0.5 lên
float   g_Int2Round(int d);       // Chuyển int sang float sau khi làm tròn/chuẩn hoá (giữ hành vi gốc)

// Đếm số ký tự Big5 trong chuỗi (mã hoá Big5). Trả về số lượng cặp byte hợp lệ.
int     GetBig5Count(char* str);

// Định dạng an toàn tương tự snprintf/sprintf/printf. Trên mọi nền tảng:
//  - t_snprintf: ghi tối đa count byte (kể cả '\0'), trả về số ký tự sẽ ghi (không kể '\0').
//  - t_sprintf: như sprintf nhưng có kiểm tra và đảm bảo kết thúc chuỗi trong triển khai.
//  - t_printf: in ra stdout; trả về số ký tự đã in.
int     t_snprintf(char* buf, unsigned int count, const char* format, ...);
int     t_sprintf(char* out, const char* format, ...);
int     t_printf(const char* format, ...);

// Tạo thư mục (đệ quy nếu cần) theo đường dẫn nPath bên trong nSdcardPath.
// Trả về true nếu tạo thành công hoặc đã tồn tại.
bool    __CreateFolder(std::string nSdcardPath, char* nPath);

// Băm chuỗi kiểu djb2/fnv (tuỳ triển khai) cho nhu cầu tra cứu nhanh.
//  - Trả về giá trị băm 32-bit ổn định giữa các nền tảng.
unsigned int _ccHash(const char* key);

