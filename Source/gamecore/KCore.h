//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:   KCore.h
// Tác giả gốc: WangWei (Daphnis)
// Mục đích: Tập trung các định nghĩa lõi dùng chung trong engine
//---------------------------------------------------------------------------

#pragma once

// Phụ thuộc vào engine/third‑party
#include "cocos2d.h"
#include "AudioEngine.h"
USING_NS_AX;

// C++ chuẩn và hệ thống
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <ctime>
#include <cassert>
#include <map>
#include <string>

// Nền tảng & cảnh báo
#if defined(_MSC_VER)
  // Vô hiệu hóa một số cảnh báo cũ chỉ dành cho MSVC
  #pragma warning(disable : 4512) // không tạo được operator= mặc định (không còn phù hợp)
  #pragma warning(disable : 4786) // tên ký hiệu quá dài (VC6 trở xuống)
  #pragma warning(disable : 4244) // chuyển kiểu có thể mất dữ liệu (giữ nguyên hành vi cũ)
#endif

//// Giữ tương thích với mã cũ: INT
//// Lưu ý: Trên các nền tảng không phải Windows, một số mã cũ dùng INT = int.
//#ifndef _WIN32
//  #ifndef INT
//    #define INT int
//  #endif
//#endif

// Hằng số/định hướng
#define WEATHERID_NOTHING 255
#define DIR_DOWN          0
#define DIR_LEFTDOWN      1
#define DIR_LEFT          2
#define DIR_LEFTUP        3
#define DIR_UP            4
#define DIR_RIGHTUP       5
#define DIR_RIGHT         6
#define DIR_RIGHTDOWN     7

// Đường dẫn font dùng trong UI trò chơi
#define UI_GAME_FONT_SONG          "fonts/common_zh_vet.ttf"
#define UI_GAME_FONT_DEFAULT       "fonts/VNARIAL.ttf"
#define UI_GAME_FONT_NUMBER        "fonts/number_16_w.fnt"

#define UI_GAME_FONT_SONG_VN       "fonts/syzt-e.ttf"
#define UI_GAME_FONT_VN            "fonts/common_zh_vet.ttf"
#define UI_GAME_FONT_DEFAULT_VN    "fonts/common_zh_vet.ttf"
#define UI_GAME_FONT_DEFAULT_VN_BTN "fonts/btn_hydl_vet.ttf"
//#define UI_GAME_FONT_NUMBER_VN   "fonts/number_16_w.fnt"

// ZeroStruct: đảm bảo đa nền tảng, không phụ thuộc vào ZeroMemory của Windows
#ifndef ZeroStruct
  #define ZeroStruct(buf) std::memset(&(buf), 0, sizeof(buf))
#endif

// Giới hạn số loại vũ khí (giữ nguyên giá trị gốc)
#define MAX_MELEEWEAPON_PARTICULARTYPE_NUM 100   // Số loại vũ khí cận chiến tối đa
#define MAX_RANGEWEAPON_PARTICULARTYPE_NUM 100   // Số loại vũ khí tầm xa tối đa

// Thông tin ứng dụng (giữ nguyên)
#define APP_STRINFO_1 "vvlmXQy0KkYOvN9gEi0IW4kkp602Lym0"
#define APP_STRINFO_2 "IzOINKf_Rz1T0Lvdf106klw0HxpZ4pWh"

// Các header nội bộ engine
#include "engine/KPlatform.h"
#include "engine/KEngine.h"
#include "engine/KTimer.h"
#include "engine/KGbktoUtf8.h"
#include "KProtocol.h"
#include "KProtocolProcess.h"
#include "KMagicDesc.h"
#include "network/ClientStage.h"
#include "gamescene/LoginDef.h"
#include "gamescene/KgameWorld.h"

#ifndef _WIN32
  #include <unistd.h>
  #include <ctype.h>
#endif




//---------------------------------------------------------------------------
// Bộ sinh số ngẫu nhiên cũ
// Lưu ý: Thuật toán tuyến tính (LCG) cũ, không dùng cho bảo mật.
//---------------------------------------------------------------------------
static unsigned long gs_holdrand = static_cast<unsigned long>(std::time(nullptr));

static inline unsigned long _Rand()
{
    // Giữ nguyên công thức LCG gốc để tương thích chuỗi gói (packet chain)
    gs_holdrand = gs_holdrand * 244213UL + 1541021UL;
    return gs_holdrand;
}

inline void RandMemSet(int nSize, unsigned char* pbyBuffer)
{
    // Điền bộ đệm bằng byte ngẫu nhiên từ LCG (giữ nguyên hành vi)
    if (nSize <= 0 || pbyBuffer == nullptr) return;
    while (nSize--) {
        *pbyBuffer++ = static_cast<unsigned char>(_Rand());
    }
}

// Ghi chú: GetRandomNumber giữ nguyên API và logic xử lý âm
// Phụ thuộc vào g_Random (được khai báo ở nơi khác) nên không thay đổi.
inline int GetRandomNumber(int nMin, int nMax)
{
    int nIsThis = 0;
    if (nMin < 0 && nMax < 0) {
        nMin = -nMin;
        nMax = -nMax;
        nIsThis = 1;
    } else if (nMin < 0 && nMax > 0) {
        // Trường hợp giao nhau qua 0: giữ nguyên hành vi cũ (không chỉnh nIsThis)
    }

    // Bảo vệ tràn khi nMax - nMin + 1 < 0 (giữ nguyên điều kiện trả 0)
    if (nMax - nMin + 1 < 0)
        return 0;

    if (nIsThis) {
        // Trả về số âm theo quy tắc cũ
        int nReg = g_Random(nMax - nMin + 1) + nMin;
        nReg = -nReg;
        return nReg;
    } else {
        return g_Random(nMax - nMin + 1) + nMin;
    }
}

// Khai báo các hàm khởi tạo dữ liệu gameplay (giữ nguyên API)
bool InitSkillSetting();
bool InitMissleSetting();
bool InitNpcSetting();
void g_ReleaseCore();

//---------------------------------------------------------------------------
// Phân loại đối tượng/game UI (đã dịch chú thích sang tiếng Việt)
//---------------------------------------------------------------------------

enum GAME_OBJ_KIND
{   // Các node chính trong UI/trò chơi
    OBJ_NODE_UI = 1,
    OBJ_NODE_MAP,
    OBJ_NODE_MISS,
    OBJ_NODE_NPC,
    OBJ_NODE_ITEM,
    OBJ_NODE_OBJSET,
    OBJ_NODE_SHORTKILL,
    OBJ_NODE_MINMAP,
    OBJ_NODE_BIGMAP,
    OBJ_NODE_ROLESTATE,
    OBJ_NODE_CHAT,
    OBJ_NODE_STATE,
    OBJ_NODE_SKILL,
    OBJ_NODE_ITEMBOX,
    OBJ_NODE_TEAM,
    OBJ_NODE_STORE,
    OBJ_NODE_ITEMBOX_EX,
    OBJ_NODE_ITEMBOX_A,
    OBJ_NODE_ITEMBOX_B,
    OBJ_NODE_ITEMBOX_C,
    OBJ_NODE_NPCDIALOG,
    OBJ_NODE_TALKDIALOG,
    OBJ_NODE_NPCSHOP,
    OBJ_NODE_STRINGBOX,
    OBJ_NODE_SKILL_STATE,
    OBJ_NODE_NPC_OTHER,
    OBJ_NODE_NPC_BLOOD,
    OBJ_NODE_TALKDIALOG_NEW,
    OBJ_NODE_PROGRESS,
    OBJ_NODE_MOVEMAP,
    OBJ_NODE_SHORTKILL_R,
    OBJ_NODE_SHORTKILL_L,
    OBJ_NODE_LAOTOU,
    OBJ_NODE_GIVE,
    OBJ_NODE_CHATLIST,
    OBJ_NODE_AUTOPLAY,
    OBJ_NODE_PLAYFUN,
    OBJ_NODE_VIEW,
    OBJ_NODE_ITEM_SELL,
    OBJ_NODE_PLAYERSHOP,
    OBJ_NODE_TRADE,
    OBJ_NODE_DYSHOP,
    OBJ_NODE_MAXMAP,
    OBJ_NODE_ITEMLINK,
    OBJ_NODE_MOVECAVE,
    OBJ_NODE_TASK,
    OBJ_NODE_TONG,
    OBJ_NODE_ESC,
    OBJ_NODE_MISS_SPECIAL,
    OBJ_NODE_TONG_CREAT,
    OBJ_NODE_COUNT,
};


enum GAME_MAP_KIND
{   // Các node bản đồ trong UI/trò chơi
    MAP_NODE_CAODI = 1,      // Cỏ/đồng cỏ
    MAP_NODE_LUMIAN,         // Mặt đường
    MAP_NODE_LUMIAN_A,       // Mặt đường (biến thể)
    MAP_NODE_PART,           // Mảnh bản đồ
    MAP_NODE_TREES,          // Cây/cụm cây
    MAP_NODE_HOUSE,          // Nhà/tường thành
    MAP_NODE_ABOVE,          // Lớp trên (vách, dốc, ...)
    MAP_NODE_OTHER,
};

// Above -> Baove (giữ nguyên tên enum cũ nhưng dịch chú thích)
enum GAME_BAOVE_KIND
{   // Lớp che phủ/phần trên
    BAOVE_NODE_PART = 1,     // Che phủ từng phần
    BAOVE_NODE_FULL,         // Che phủ toàn phần
    BAOVE_NODE_OTHER,
};


enum GAME_MINIMAP_KIND
{   // Các node của minimap
    MINIMAP_NODE_BGSPR = 1,  // Hình nền
    MINIMAP_NODE_NAME,
    MINIMAP_NODE_POINT,
    MINIMAP_NODE_ROLE,
    MINIMAP_NODE_MAXMAP,
    MINIMAP_NODE_COUNT,
};


enum GAME_ROLESTATE_KIND
{   // Các node trạng thái nhân vật
    ROLESTATE_NODE_BGSPR = 1,
    ROLESTATE_NODE_LIFE,     // Máu
    ROLESTATE_NODE_LAN,      // Năng lượng
    ROLESTATE_NODE_NAME,     // Tên
    ROLESTATE_NODE_SAITAMA,  // Thuộc tính/biểu tượng đặc biệt
};


enum GAME_CHAT_KIND
{   // Các node khung chat
    CHAT_NODE_BGSPR = 1,     // Hình nền chat
    CHAT_NODE_TILI,          // Thể lực
    CHAT_NODE_EDIT,          // Ô nhập
    CHAT_NODE_SEND,          // Nút gửi
    CHAT_NODE_EXP,           // Kinh nghiệm/biểu cảm
    CHAT_NODE_PINDAO,        // Kênh
    CHAT_NODE_CHATLIST,      // Danh sách chat
};


enum GAME_EQ_PART
{   // Các phần trang bị
    EQ_PART_HEAD = 1,        // Đầu
    EQ_PART_BODY,            // Thân
    EQ_PART_HAND_W,          // Vũ khí tay
    EQ_PART_HORSE,           // Ngựa
    EQ_PART_PIFENG,          // Phi phong/áo choàng
    EQ_PART_COUNT,
};


enum GAME_STATE_PART
{   // Các phần trạng thái hiển thị
    STATE_PART_HEAD = 1,     // Đầu
    STATE_PART_BODY_Q,       // Thân trước
    STATE_PART_BODY_H,       // Thân sau
    STATE_PART_FOOT,         // Chân
};

// Thứ tự lớp hiển thị trong game
enum GAME_LAYER_ORDER
{   // Thứ tự node UI/trò chơi
    ORDER_MAP_UNDER = 0,     // Lớp bản đồ dưới
    ORDER_TREE_HOUSE,        // Cây/nhà
    ORDER_OBJ_MIDDLE,        // Vật thể giữa
    ORDER_NPC_PLAYER,        // NPC/người chơi
    ORDER_OBJ_ABOVE,         // Vật thể trên cao
    ORDER_UI_DIALOD,         // Hộp thoại
    ORDER_COUNT,
};

// Dữ liệu item của shop (dịch chú thích sang tiếng Việt, giữ tên trường)
struct TDBShopItemData
{
    int  iid;           // Chỉ số item (bắt đầu từ 1)
    int  iequipgen;     // Thế hệ/nhóm trang bị
    int  idetailtype;   // Loại chi tiết
    int  iparticulartype; // Loại cụ thể
    char itemName[64];  // Tên sản phẩm
    int  ilevel;        // Cấp độ
    int  iseries;       // Dòng/series
    unsigned int irandseed; // Hạt giống ngẫu nhiên
    int  iparam2;       // Tham số tuỳ biến
    int  iparam3;
    int  iparam5;
    int  iparam4;
    int  iparam6;
    int  iparam1;
    int  iparamr2;      // Thuộc tính ngẫu nhiên
    int  iparamr3;
    int  iparamr5;
    int  iparamr4;
    int  iparamr6;
    int  iparamr1;
    int  ilucky;        // Giá trị may mắn của trang bị
    int  igoldid;       // Vàng/tiền tệ id
    int  istacknum;     // Số lượng stack
    int  ienchance;     // Tăng cường
    unsigned char ipoint; // Phân loại trang bị: 7 đồ cam, 8 đồ bình thường, 1-6 đồ trang bị
    unsigned int addtime; // Thời gian bắt đầu
    unsigned int endtime; // Thời gian kết thúc
    unsigned char iPriceKind; // Loại giá
    int  iSellPrice;    // Giá bán
    unsigned char iRongpiont; // Điểm nóng chảy (nấu chảy?)
    char iWonName[32];  // Người thắng/đoạt
    char iaccName[32];  // Tài khoản
    int  iswhere;       // Có phải đồ tẩy/đồ đặc biệt
    int  isplatina;     // Có phải bạch kim
    unsigned char isSellOut; // Đã bán hết hay chưa
    int  iparamj1;      // (Dành cho sử dụng sau)
    int  iparamj2;
    unsigned int iparamj3;
    int  iparamj5;
    int  iparamj4;
    int  iparamj6;
    int  iparamj7;
};

// Bản đồ kiểu dữ liệu (giữ nguyên typedef)
typedef std::map<int, TDBShopItemData> _ItemDataInfo;
typedef std::map<std::string, std::string> _StrNpcInfo;
typedef std::map<int, std::string> _StrObjInfo;
typedef std::map<std::string, std::string> _StrCoreInfo;

// Externs giữ nguyên tên và kiểu để không phá API
extern KTimer timerStart;
extern char* g_GetStringRes(int nStringID, char* szString, int nMaxLen);
extern CGameClient* nAcclient;
extern CGameClient* g_pClient;
extern int m_bIsClientConnecting;
extern int m_bIsGameSevConnecting;
extern int m_bIsPlayerInGame;
extern int _clientlanguage;
extern bool _clientNoNetWork;
extern int  g_bUISelIntelActiveWithServer; // Tuỳ chọn hiện tại có đồng bộ server hay không
extern int  g_bUISpeakActiveWithServer;
extern int  g_bUISelLastSelCount;
extern int  nGameVersion;
extern int  nExpSkillIdx;
extern int  nLuckySkillIdx;
extern int  nKangkillIdx;
extern int  nYaoPinSkillIdx;
extern bool nisgetinfo;
extern KPakList g_pcPakList;
extern Kgetinidata _getinidata;
extern int  nIsGaldInit;
extern bool nIsShowShadow;
extern bool nIsShowTitle;
extern KgameWorld* g_GameWorld;

extern _ItemDataInfo  iItemDataInfo;
extern _StrNpcInfo    strNpcInfo;
extern _StrObjInfo    strObjInfo;
extern _StrCoreInfo   strCoreInfo;

extern KTabFile g_OrdinSkillsSetting, g_MisslesSetting, g_PlayerTitle, g_ObjChangeName;
extern KTabFile g_SkillLevelSetting, g_FsJinMai, g_ForbitMap, g_CompEquip;
extern KTabFile g_NpcSetting, g_NpcKindFile, g_RankTabSetting;
extern KIniFile g_GameSetTing, g_NpcMapDropRate, g_MagicBaoshiDesc;
extern KTabFile cTabFile, nToolItem;
extern KIniFile g_MapTraffic;
extern KTabFile g_StringResourseTabFile;
extern KTabFile g_CompAtlas, g_GetMsg;

extern Size visibleSize;
extern Vec2 origin;
extern Size kSize;
extern std::string m_SDcardDirPath;
extern std::string m_MobileKey;
extern std::string maxMapPicPath;
extern AudioEngine* __pSound;
extern FileUtils* ccFileUtils;
extern bool _openSound;
extern unsigned int curMobileVer;
extern unsigned int curMobileKey;

// extern KSprCodec SprDecode;

// Ghi chú: Không có định nghĩa SetCallDataChangedNofify trong file này; nếu tồn tại ở nơi khác,
// việc refactor không làm thay đổi API của nó.
