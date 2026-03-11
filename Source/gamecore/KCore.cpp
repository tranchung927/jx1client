//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:   KCore.cpp (được chuẩn hoá C++20, đa nền tảng)
// Mục đích: Triển khai các khởi tạo và tài nguyên lõi của engine, giữ tương thích
//           với memory pool, packet chain và API gốc.
// Ghi chú: Đã dịch chú thích gốc sang tiếng Việt, định dạng lại, sửa lỗi nền tảng.
//---------------------------------------------------------------------------

#include "KCore.h"
#include "engine/KEngine.h"
#include "KSortScript.h"
#include "gamescene/KScenePlaceC.h"
#include "KItemChangeRes.h"
#include "KNpcSet.h"
#include "KSkills.h"
#include "KPlayerSet.h"
#include "KPlayerTeam.h"
#include "KMissleSet.h"
#include "KFaction.h"
#include "KMath.h"
#include "KPlayerTask.h"
#include "KSubWorldSet.h"
#include "KItemGenerator.h"
#include "KObjSet.h"
#include "KItemSet.h"
#include "KNpc.h"
#include "KPlayer.h"
#include "KNpcTemplate.h"
#include "CoreUseNameDef.h"
#include "KBuySell.h"
#include "KTaskFuns.h"
#include "TaskDef.h"
#include "LuaFuns.h"

// Script cấp độ NPC
KLuaScript* g_pNpcLevelScript = nullptr;
KLuaScript  g_WorldScript;

// Client bắt đầu tiền xử lý cấu hình
#define CLIENTWEAPONSKILL_TABFILE "\\settings\\ClientWeaponSkill.txt"
#define ADJUSTCOLOR_TABFILE       "\\settings\\AdjustColor.txt"

// Dữ liệu từ các bảng cấu hình và map
_ItemDataInfo  iItemDataInfo;
_StrNpcInfo    strNpcInfo;
_StrObjInfo    strObjInfo;
_StrCoreInfo   strCoreInfo;

// Thông tin màn hình/hiển thị
Size visibleSize;   // Kích thước vùng hiển thị
Vec2  origin;       // Gốc toạ độ
Size kSize;         // Kích thước tham chiếu

// Đường dẫn và thông tin thiết bị
std::string m_SDcardDirPath;
std::string maxMapPicPath;
std::string m_MobileKey = "NYc0bUz0z_JhCNAi0WUK8JK0MOlOQlkP";

// Âm thanh và hệ thống file
AudioEngine* __pSound = nullptr;
FileUtils*   ccFileUtils = nullptr;
bool         _openSound = true;

// Phiên bản di động/khóa
unsigned int curMobileVer = 30;
unsigned int curMobileKey = 14302019;

//KSprCodec SprDecode; // Giữ chỗ nếu cần

// Bộ hẹn giờ toàn cục
KTimer   timerStart;

// Các bảng cấu hình
KTabFile g_ClientWeaponSkillTabFile, g_ObjChangeName;
KTabFile g_OrdinSkillsSetting, g_MisslesSetting, g_PlayerTitle;
KTabFile g_SkillLevelSetting, g_FsJinMai, g_ForbitMap, g_CompEquip;
KTabFile g_NpcSetting, g_NpcKindFile, g_RankTabSetting;
KTabFile cTabFile, g_CompAtlas, nToolItem;
KIniFile g_GameSetTing, g_NpcMapDropRate, g_MagicBaoshiDesc;
KIniFile g_MapTraffic;
KTabFile g_GetMsg;

// Kỹ năng vũ khí
int g_nMeleeWeaponSkill[MAX_MELEEWEAPON_PARTICULARTYPE_NUM];
int g_nRangeWeaponSkill[MAX_RANGEWEAPON_PARTICULARTYPE_NUM];
int g_nHandSkill = 0;

// Cấu hình game
int  nGameVersion = 0; // Phiên bản game (GS)
int  nExpSkillIdx = 0;
int  nLuckySkillIdx = 0;
int  nKangkillIdx = 0;
int  nYaoPinSkillIdx = 0;
int  nIsGaldInit = false;
bool nIsShowShadow = true;
bool nIsShowTitle = true;

#if defined(_WIN32)
bool nisgetinfo = true;
#else
bool nisgetinfo = false;
#endif

// Gói dữ liệu và cấu hình đầu vào
KPakList    g_pcPakList;
Kgetinidata _getinidata;

int g_bUISelIntelActiveWithServer = 0; // Tuỳ chọn hiện tại có đồng bộ server hay không
int g_bUISpeakActiveWithServer = 0;
int g_bUISelLastSelCount = 0;

// Kết nối client/game server
CGameClient* nAcclient = nullptr;   // Client tài khoản
CGameClient* g_pClient = nullptr;   // Game server client
int m_bIsClientConnecting = 0;      // Trạng thái kết nối account server
int m_bIsGameSevConnecting = 0;     // Trạng thái kết nối game server
int m_bIsPlayerInGame = 0;          // Người chơi đã vào game hay chưa
int _clientlanguage = 2;            // 1 Trung, 2 Việt, 3 Anh (giá trị gốc)
bool _clientNoNetWork = false;      // Chế độ đơn (không mạng)

// Khai báo trước
void g_InitProtocol();

bool InitSkillSetting()
{
    // Khởi tạo hệ thống kỹ năng và bảng kỹ năng client
    if (!g_SkillManager.Init())
        showAlert("load failed.", "SkillManager");
    if (!g_ClientWeaponSkillTabFile.Load(CLIENTWEAPONSKILL_TABFILE))
        showAlert("load failed..", "ClientWeapon");
    return true;
}

bool InitMissleSetting()
{
    // Nạp cấu hình đạn (missile)
    const int nMissleNum = g_MisslesSetting.GetHeight() - 1;
    for (int i = 0; i < nMissleNum; ++i)
    {
        int nMissleId = 0;
        g_MisslesSetting.GetInteger(i + 2, "MissleId", -1, &nMissleId);
        if (nMissleId >= MAX_MISSLESTYLE)
            continue;
        if (nMissleId > 0)
        {
            g_MisslesLib[nMissleId].GetInfoFromTabFile(i + 2);
            g_MisslesLib[nMissleId].m_nMissleId = nMissleId;
        }
    }
    return true;
}

bool InitTaskSetting()
{
    // Khởi tạo nhiệm vụ: bảng chuỗi và bảng hạng
    if (!g_StringResourseTabFile.Load(STRINGRESOURSE_TABFILE))
        showAlert("[TASK]CAN NOT LOAD.", "TASK");
    if (!g_RankTabSetting.Load(PLAYER_RANK_SETTING_TABFILE))
        showAlert("[RANK]CAN NOT LOAD.", "RANK");
    return true;
}

bool InitNpcSetting()
{
    // Khởi tạo dữ liệu NPC/Template/Nguồn tài nguyên NPC
    const int nNpcTemplateNum = g_NpcSetting.GetHeight() - 1;
    (void)nNpcTemplateNum; // tránh cảnh báo biến không dùng

    if (!g_NpcKindFile.Load(NPC_RES_KIND_FILE_NAME))
        showAlert("[NpcKind]CAN NOT LOAD.", "NpcKind");

    // Nạp script cấp độ NPC để áp dụng khi khởi tạo NPC
    g_pNpcLevelScript = new KLuaScript;
    if (!g_pNpcLevelScript)
    {
        showAlert("g_pNpcLevelScript is Error", "InitNpcSetting");
    }
    else
    {
        g_pNpcLevelScript->Init();
        if (!g_pNpcLevelScript->Load(NPC_LEVELSCRIPT_FILENAME))
        {
            delete g_pNpcLevelScript;
            g_pNpcLevelScript = nullptr;
            showAlert("[NpcLevel]CAN NOT LOAD.", "InitNpcSetting");
        }
    }

    if (!g_NpcResList.Init()) // Khởi tạo danh sách tài nguyên NPC ở client
        showAlert("[NpcResList]CAN NOT LOAD.", "NpcResList");

    return true;
}

void g_ReleaseCore()
{
    // Giải phóng tài nguyên lõi
    g_SubWorldSet.Close();
    g_ScenePlace.ClosePlace();

    // g_UnInitMath(); // nếu có cần huỷ toán học toàn cục
}

void g_InitCore()
{
    // Khởi tạo ngẫu nhiên, đồng hồ, và client
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    timerStart.Start(); // Bắt đầu đếm thời gian game

    nAcclient = nullptr;
    g_pClient = nullptr;

    // Cấu hình bộ đệm cho socket và cache
    const size_t bufferSize = 1024;              // Kích thước bộ đệm socket
    const size_t bufferSize_Cache = 1024 * 512;  // Kích thước bộ đệm cache gói
    const size_t maxFreeBuffers = 2;             // Số buffer rỗi cho socket
    const size_t maxFreeBuffers_Cache = 2;       // Số buffer rỗi cho cache

    nAcclient = new CGameClient(maxFreeBuffers, maxFreeBuffers_Cache, bufferSize_Cache, bufferSize, 0);
    // g_pClient = new CGameClient(maxFreeBuffers, maxFreeBuffers_Cache, bufferSize_Cache, bufferSize, 1);

    std::srand(static_cast<unsigned>(std::time(nullptr)));

    if (g_GameSetTing.Load(GAME_SETTING_FILE))
    {
        g_GameSetTing.GetInteger("SkillsIcon", "ExpSkillIdx", 0, &nExpSkillIdx);
        g_GameSetTing.GetInteger("SkillsIcon", "LuckySkillIdx", 0, &nLuckySkillIdx);
        g_GameSetTing.GetInteger("SkillsIcon", "KangkillIdx", 0, &nKangkillIdx);
        g_GameSetTing.GetInteger("SkillsIcon", "YaoPinSkillIdx", 0, &nYaoPinSkillIdx);
        g_GameSetTing.GetInteger("Gameconfig", "GameVersion", 1, &nGameVersion); // 1 nội địa, 2 quốc tế
    }

    if (!g_ObjChangeName.Load("\\settings\\vn\\ObjData.txt")) {
        showAlert("load vn-ObjData.txt Error", "Warning");
        return;
    }
    KTabFile nNpcInfo; // Tạm dùng để nạp bảng thay thế chuỗi

    char nTempStr[256]{};
    ZeroMemory(nTempStr, sizeof(nTempStr));

    if (_clientlanguage != 1)
        std::snprintf(nTempStr, sizeof(nTempStr), "\\settings\\lang\\vn\\stringtable_core.txt");
    else
        std::snprintf(nTempStr, sizeof(nTempStr), "\\settings\\lang\\zh\\stringtable_core.txt");

    if (nNpcInfo.Load(nTempStr))
    {
        const int nRows = nNpcInfo.GetHeight() + 1;
        for (int i = 2; i < nRows; ++i)
        {
            char nyNpcName[64]{};
            char ndNpcName[256]{};
            nNpcInfo.GetString(i, "key", "no npc name", nyNpcName, sizeof(nyNpcName));
            nNpcInfo.GetString(i, "value", "no npc name", ndNpcName, sizeof(ndNpcName));
            strCoreInfo[nyNpcName] = ndNpcName;
        }
        nNpcInfo.Clear();
    }

    // Cấp phát các mảng tài nguyên lõi. Các block ngẫu nhiên nhằm cố định layout bộ nhớ (giữ tương thích pool)
    char* szRandomMem1 = nullptr;
    char* szRandomMem2 = nullptr;
    char* szRandomMem3 = nullptr;

    SubWorld = new KSubWorld[MAX_SUBWORLD];
    szRandomMem1 = new (std::nothrow) char[((std::rand() % 64) + 6) * 1024];
    Player = new KPlayer[MAX_PLAYER];
    szRandomMem2 = new (std::nothrow) char[((std::rand() % 64) + 6) * 1024];
    Npc = new KNpc[MAX_NPC];
    szRandomMem3 = new (std::nothrow) char[((std::rand() % 64) + 6) * 1024];
    Item = new KItem[MAX_ITEM];
    KObject = new KObj[MAX_OBJECT];
    Missle = new KMissle[MAX_MISSLE];

    delete[] szRandomMem1; szRandomMem1 = nullptr;
    delete[] szRandomMem2; szRandomMem2 = nullptr;
    delete[] szRandomMem3; szRandomMem3 = nullptr;

    // Khởi tạo giao thức và seed ngẫu nhiên (giữ nguyên để không phá packet chain)
    g_InitProtocol();
    g_RandomSeed(42);

    g_InitSeries();

    if (!g_InitMath())
        showAlert("load InitMath Error", "InitMath");

    ItemSet.Init();
    ItemGen.Init();

    g_MagicDesc.Init();          // Khởi tạo mô tả ma pháp phía client
    g_ItemChangeRes.Init();      // Khởi tạo tài nguyên thay đổi item

    if (!g_ForbitMap.Load(GAME_FORBITMAP_FILE))
        showAlert("load ForbitMap Error", "ForbitMap");
    if (!g_FsJinMai.Load(GAME_FSJINMAI_FILE))
        showAlert("load JinMai Error", "JinMai");

    g_PlayerTitle.Load("\\Settings\\playertitle.txt");
    g_NpcMapDropRate.Load(MAP_DROPRATE_FILE);

    NpcSet.m_cGoldTemplate.Init(); // Khởi tạo template vàng
    NpcSet.Init();
    ObjSet.Init();
    MissleSet.Init();

    g_IniScriptEngine("\\Ui", FALSE); // Nạp script client

    if (!g_OrdinSkillsSetting.Load(SKILL_SETTING_FILE))
        showAlert("load skills error.", "skills");

    g_MisslesSetting.Load(MISSLES_SETTING_FILE); // Nạp cấu hình đạn

    g_NpcSetting.Load(NPC_SETTING_FILE);         // Nạp template NPC
    InitSkillSetting();                          // Nạp kỹ năng
    InitMissleSetting();                         // Nạp đạn
    InitNpcSetting();                            // Nạp NPC
    InitTaskSetting();                           // Nạp nhiệm vụ

    // Các thành phần phụ thuộc kỹ năng nên khởi tạo sau phần kỹ năng
    if (!PlayerSet.Init())
        showAlert("Init PlayerSet Error", "PlayerSet");

    g_CompAtlas.Load(GAME_ATLAS_FILE);
    g_GetMsg.Load(GAME_MSG_FILE);
    g_CompEquip.Load(GAME_COMP_FILE);
    g_MapTraffic.Load("\\settings\\MapTraffic.ini");

    if (!g_ScenePlace.Initialize())
        showAlert("Scene Load Error", "Initialize");
    g_Faction.Init();

    std::memset(g_nMeleeWeaponSkill, 0, sizeof(g_nMeleeWeaponSkill));
    std::memset(g_nRangeWeaponSkill, 0, sizeof(g_nRangeWeaponSkill));

    KTabFile Weapon_PhysicsSkillIdFile;
    if (Weapon_PhysicsSkillIdFile.Load(WEAPON_PHYSICSSKILLFILE))
    {
        const int nHeight = Weapon_PhysicsSkillIdFile.GetHeight() - 1;
        for (int i = 0; i < nHeight; ++i)
        {
            int nDetail = 0, nParticular = 0, nPhysicsSkill = 0;
            Weapon_PhysicsSkillIdFile.GetInteger(i + 2, 1, -1, &nDetail);       // WEAPON_DETAILTYPE
            Weapon_PhysicsSkillIdFile.GetInteger(i + 2, 2, -1, &nParticular);   // WEAPON_PARTICULARTYPE
            Weapon_PhysicsSkillIdFile.GetInteger(i + 2, 3, -1, &nPhysicsSkill); // WEAPON_SKILLID

            // Gán kỹ năng theo loại vũ khí
            if (nDetail == 0)
            {
                if (nParticular >= 0 && nParticular < MAX_MELEEWEAPON_PARTICULARTYPE_NUM &&
                    nPhysicsSkill > 0 && nPhysicsSkill < MAX_SKILL)
                {
                    g_nMeleeWeaponSkill[nParticular] = nPhysicsSkill;
                }
            }
            else if (nDetail == 1)
            {
                if (nParticular >= 0 && nParticular < MAX_RANGEWEAPON_PARTICULARTYPE_NUM &&
                    nPhysicsSkill > 0 && nPhysicsSkill < MAX_SKILL)
                {
                    g_nRangeWeaponSkill[nParticular] = nPhysicsSkill;
                }
            }
            else if (nDetail == -1) // Tay không
            {
                if (nPhysicsSkill > 0 && nPhysicsSkill < MAX_SKILL)
                {
                    g_nHandSkill = nPhysicsSkill;
                }
            }
        }
    }
    else
    {
        showAlert("PhysicsSkillIdFile load Error", "PhysicsSkillIdFile");
    }

    Weapon_PhysicsSkillIdFile.Clear();

    if (!BuySell.Init()) // Khởi tạo cửa hàng
        showAlert("Init BuySell Error", "BuySell");
    // Hoàn tất khởi tạo lõi
}

