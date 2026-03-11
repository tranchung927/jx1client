#include "KUpdateLayer.h"

// ---------------------------------------------------------------------------
// File:   KUpdateLayer.cpp
// Mục đích: Triển khai lớp KUpdateLayer điều phối cập nhật tài nguyên/phiên bản
//           của trò chơi, quản lý tải xuống, hiển thị tiến trình và chuyển cảnh.
// ---------------------------------------------------------------------------

#include "gamecore/KCore.h"
#include "gamecore/CoreShell.h"
#include "gamescene/KgameWorld.h"
#include "engine/KGbktoUtf8.h"
#include "gameui/Klogin.h"
#include "gameui/Klogin_f.h"
#include "gameui/KuiInPutIp.h"
#include "engine/dataChecksum.h"
#include "audio/AudioEngine.h"
#include "MainMenu.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <filesystem>
#include <thread>
#include <chrono>

#if (CC_TARGET_PLATFORM != CC_PLATFORM_WIN32)
#    include <sys/stat.h>
#    include <sys/types.h>
#    include <dirent.h>
#endif

USING_NS_AX;

// Biến toàn cục
iCoreShell* g_pCoreShell = nullptr;
extern KImageStore2 m_ImageStore;

namespace {
// Hằng số thời gian chờ lỗi
constexpr unsigned int ERROR_TIME_OUT = 300;

// Ngủ chéo nền tảng theo mili-giây
inline void sleep_ms(unsigned int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

inline std::string joinPath(const std::string& base, const char* name) {
    std::string result = base;
    if (!result.empty() && result.back() != '/' && result.back() != '\\') {
        result.push_back('/');
    }
    if (name) {
        result += name;
    }
    return result;
}
} // namespace

KUpdateLayer::KUpdateLayer() {
    CCLOG("start KUpdateLayer");
    
    nDownState   = STATE_NONE_UP;
    downPtr      = nullptr;
    isOpenDown   = 1;
    errorLoop    = 0;
    nPakCount    = 0;
    pakDownCount = 0;
    m_WritablePath.clear();
    pListData            = nullptr;
    isHaveClient         = false;
    m_IsInPutIpPadIsOpen = false;
    ZeroMemory(downUrlmask, sizeof(downUrlmask));
    isOver = false;

    CCLOG("[KUpdateLayer] init: downloadDir(pending) state=%d", nDownState);
    
    CCLOG("end KUpdateLayer");
}

KUpdateLayer::~KUpdateLayer() {
    CCLOG("start ~KUpdateLayer");
    if (downPtr) {
        downPtr->release();
        downPtr = nullptr;
    }
    if (pListData) {
        free(pListData);
        pListData = nullptr;
    }

    CCLOG("[KUpdateLayer] destroyed. downPtr=%p, pListData=%p", (void*)downPtr, (void*)pListData);

    isOver               = false;
    m_IsInPutIpPadIsOpen = false;
    CCLOG("end ~KUpdateLayer");
}

bool KUpdateLayer::init() {
    if (!Layer::init()) {
        return false;
    }

    // Khởi tạo âm thanh và đường dẫn ghi
    auto* ccFileUtils = ax::FileUtils::getInstance();
    m_downloadDir     = ccFileUtils->getWritablePath();
    m_WritablePath    = m_downloadDir;

    // Thư mục dữ liệu tải về
    m_downloadDir += "data";
    initDownloadDir();
    creatDownloadDir("music");
    creatDownloadDir("sound");

    nCurpFileName.clear();
    writelen          = 0;

    // Cấu hình tổng số tài nguyên để hiển thị tiến trình
    m_nAllLoadResources = 50;
    m_nLoadedResources  = 0;
    m_fProgressIndex    = 0.0F;
    m_nLoadedFinish     = 0;

    // Kích thước màn hình
    const auto visibleSize = ax::Director::getInstance()->getVisibleSize();
    const auto origin      = ax::Director::getInstance()->getVisibleOrigin();
    const auto kSize       = ax::Director::getInstance()->getWinSize();
    const float winw       = visibleSize.width;
    const float winh       = visibleSize.height;

    CCLOG("[KUpdateLayer] init UI: winw=%.1f winh=%.1f writable='%s'", winw, winh, m_WritablePath.c_str());

    // Sao chép file cấu hình ban đầu
    if (_clientlanguage != 1) {
        nCurpFileName = "package_vn.ini";
        copyData("package_vn.ini");
    } else {
        nCurpFileName = "package.ini";
        copyData("package.ini");
    }
    ++m_nLoadedResources;

    // Đọc địa chỉ máy chủ cập nhật từ package.ini
    const std::string fielDir = joinPath(m_downloadDir, (_clientlanguage != 1) ? "package_vn.ini" : "package.ini");

    CCLOG("[KUpdateLayer] using package file: %s", fielDir.c_str());

    if (!_clientNoNetWork) {
        KIniFile dwmask;
        char nTempstr[256];
        std::snprintf(nTempstr, sizeof(nTempstr), "%s", fielDir.c_str());
        if (dwmask.Load(nTempstr)) {
            dwmask.GetString("Package", "down", "", downUrlmask, sizeof(downUrlmask));
            dwmask.Clear();
            if (downUrlmask[0] != '\0') {
                char nTempPaht[128];
                ZeroMemory(nTempPaht, sizeof(nTempPaht));
                std::snprintf(nTempPaht, sizeof(nTempPaht), "http://%s/", downUrlmask);
                downPtr = downloadFile::GetInst(nTempPaht, m_WritablePath);
                CCLOG("[KUpdateLayer] download host: %s", nTempPaht);
            } else {
                CCLOG("[KUpdateLayer] package loaded but [Package]/down is empty: %s", nTempstr);
            }
        } else {
            CCLOG("[KUpdateLayer] failed to load package file: %s", nTempstr);
        }

        if (downPtr) {
            // Bắt đầu tải
            startload();
            CCLOG("[KUpdateLayer] startload triggered (network=%d)", !_clientNoNetWork);
        } else {
            CCLOG("[KUpdateLayer] startload skipped because downPtr is null");
        }
    }

    // Label demo
    {
        auto label = Label::createWithSystemFont("Hello ChungTV", "Arial", 24);
        label->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
        this->addChild(label, 2);
    }

    // Nhãn trạng thái
    m_label = Label::createWithTTF("", UI_GAME_FONT_DEFAULT, 18);
    m_label->setAnchorPoint(ax::Vec2(1, 0.5F));
    m_label->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
    this->addChild(m_label, 2);

    // Khởi tạo kho ảnh
    m_ImageStore.Init();
    ++m_nLoadedResources;

    // Nhãn tiến trình
    m_pLabelLoading = Label::createWithTTF("loading...", UI_GAME_FONT_DEFAULT, 15);
    m_pLabelPercent = Label::createWithTTF("%0", UI_GAME_FONT_DEFAULT, 15);
    m_pLabelLoading->setPosition(Vec2(kSize.width / 2 + origin.x, kSize.height / 4 + origin.y));
    m_pLabelPercent->setPosition(Vec2(kSize.width / 2 + origin.x, kSize.height / 5 + origin.y));
    this->addChild(m_pLabelLoading, 2);
    this->addChild(m_pLabelPercent, 2);

    // Thanh tiến trình
    m_pLoadBarStart = Sprite::create("loadingStart.png");
    m_pLoadBarStart->setPosition(ax::Vec2(kSize.width / 2, kSize.height / 7));
    float sx = m_pLoadBarStart->getTextureRect().getMaxX();
    m_pLoadBarStart->setScaleY(2.0F);
    m_pLoadBarStart->setScaleX(winw / sx);
    this->addChild(m_pLoadBarStart, 2);

    Sprite* m_pLoadEndSpr = Sprite::create("loadingEnd.png");
    sx                    = m_pLoadEndSpr->getTextureRect().getMaxX();

    m_pLoadBarEnd = ProgressTimer::create(m_pLoadEndSpr);
    m_pLoadBarEnd->setPercentage(1.0F);
    m_pLoadBarEnd->setPosition(ax::Vec2(kSize.width / 2, kSize.height / 7));
    m_pLoadBarEnd->setType(ProgressTimer::Type::BAR);
    m_pLoadBarEnd->setBarChangeRate(ax::Vec2(1, 0));
    m_pLoadBarEnd->setMidpoint(ax::Vec2(0, 0));
    m_pLoadBarEnd->setScaleX(winw / sx);
    m_pLoadBarEnd->setScaleY(2.0F);
    this->addChild(m_pLoadBarEnd, 2);

    // Nền đăng nhập
    Sprite* sprite = Sprite::create("longin_bg.png");
    sprite->setPosition(ax::Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
    float spx = sprite->getTextureRect().getMaxX();
    float spy = sprite->getTextureRect().getMaxY();
    sprite->setScaleX(winw / spx);
    sprite->setScaleY(winh / spy);
    this->addChild(sprite, 0);

    this->scheduleUpdate();
    return true;
}

// Xác nhận và khởi động luồng nhập IP (callback UI)
void KUpdateLayer::startInPutCallback(Node* pNode) {
    if (!pNode) return;

    if (pNode->getTag() == 2) {  // Nút hủy
        m_IsInPutIpPadIsOpen = false;
        return;
    }

    m_IsInPutIpPadIsOpen = false;
    KIniFile dwmask;
    char nTempstr[256];
    const std::string tempPackagePath = joinPath(m_downloadDir, "package_temp.ini");
    std::snprintf(nTempstr, sizeof(nTempstr), "%s", tempPackagePath.c_str());
    if (dwmask.Load(nTempstr)) {
        dwmask.GetString("Package", "down", "", downUrlmask, sizeof(downUrlmask));
        dwmask.Clear();
        char nTempPaht[128];
        ZeroMemory(nTempPaht, sizeof(nTempPaht));
        std::snprintf(nTempPaht, sizeof(nTempPaht), "http://%s:%d/", downUrlmask, 5500);
        CCLOG("[KUpdateLayer] input IP: %s", nTempPaht);
        downPtr = downloadFile::GetInst(nTempPaht, m_WritablePath);
        isOver  = true;
        startload();
        CCLOG("[KUpdateLayer] startload from input IP");
        return;
    }
    showAlert("The Server Ip Error", "Error");
}

void KUpdateLayer::creatDownloadDir(std::string nTempPath) {
    // Nếu thư mục con chưa tồn tại thì tạo
    std::string m_TempDir = joinPath(m_downloadDir, nTempPath.c_str());
    CCLOG("[KUpdateLayer] ensure subdir: %s", m_TempDir.c_str());
#if (CC_TARGET_PLATFORM != CC_PLATFORM_WIN32)
    if (DIR* pDir = opendir(m_TempDir.c_str()); !pDir) {
        mkdir(m_TempDir.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
    } else {
        closedir(pDir);
    }
#else
    if ((GetFileAttributesA(m_TempDir.c_str())) == INVALID_FILE_ATTRIBUTES) {
        CreateDirectoryA(m_TempDir.c_str(), nullptr);
    }
#endif
}

void KUpdateLayer::initDownloadDir() {
    CCLOG("[KUpdateLayer] ensure base dir: %s", m_downloadDir.c_str());
#if (CC_TARGET_PLATFORM != CC_PLATFORM_WIN32)
    if (DIR* pDir = opendir(m_downloadDir.c_str()); !pDir) {
        mkdir(m_downloadDir.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
    } else {
        closedir(pDir);
    }
#else
    if ((GetFileAttributesA(m_downloadDir.c_str())) == INVALID_FILE_ATTRIBUTES) {
        CreateDirectoryA(m_downloadDir.c_str(), nullptr);
    }
#endif
}

void KUpdateLayer::deleteDir(std::string dir) {
    CCLOG("[KUpdateLayer] delete dir: %s", dir.c_str());
    // Xóa thư mục theo nền tảng
#if (CC_TARGET_PLATFORM != CC_PLATFORM_WIN32)
    std::string command = "rm -r ";
    command += "\"" + dir + "\"";
    std::system(command.c_str());
#else
    std::string command = "rd /s /q ";
    command += "\"" + dir + "\"";
    std::system(command.c_str());
#endif
}

void KUpdateLayer::updates(Ref* /*pSender*/) {
    CCLOG("[KUpdateLayer] updates(): re-init download dir");
    // Khởi tạo lại thư mục tải xuống
    initDownloadDir();
}

void KUpdateLayer::reset(Ref* /*pSender*/) {
    CCLOG("[KUpdateLayer] reset(): deleting %s", m_downloadDir.c_str());
    if (!m_downloadDir.empty()) {
        deleteDir(m_downloadDir);
    }
}

void KUpdateLayer::getClientVersion(Ref* /*pSender*/) {
    std::string msg = StringUtils::format("current client version : %s", "test");
    CCLOG("[KUpdateLayer] getClientVersion -> %s", msg.c_str());
    m_label->setString(msg);
}

void KUpdateLayer::checkUpdate(Ref* /*pSender*/) {
    // Có thể bổ sung logic kiểm tra phiên bản tại đây nếu cần.
}

void KUpdateLayer::enterScene(Ref* /*pSender*/) {
    // Chuyển cảnh vào menu chính sau khi cập nhật
    // ax::Director::getInstance()->replaceScene(Klogin_f::scene());
}

int KUpdateLayer::isFileExist(const char* pFileName) {
    if (!pFileName) return 0;

    std::string filePath = joinPath(m_downloadDir, pFileName);

    int pSize = 0;
    if (FILE* fp = std::fopen(filePath.c_str(), "rb")) {
        std::fseek(fp, 0, SEEK_END);
        pSize = static_cast<int>(std::ftell(fp));
        std::fseek(fp, 0, SEEK_SET);
        std::fclose(fp);
    }
    return pSize;
}

void KUpdateLayer::copyData(const char* pFileName) {
    CCLOG("[KUpdateLayer] copyData start: %s", pFileName ? pFileName : "(null)");
    nCurpFileName = pFileName ? pFileName : "";
    writelen      = 0;
#ifndef WIN32
    if (!ax::FileUtils::getInstance()->isFileExist(nCurpFileName)) {
        showAlert(nCurpFileName.c_str(), "file is Noexist!");
        return;
    }
#endif
    std::string strPath     = ax::FileUtils::getInstance()->fullPathForFilename(pFileName);
    std::string fileContent;

    if (FileUtils::getInstance()->isFileExist(strPath)) {
        fileContent = FileUtils::getInstance()->getStringFromFile(strPath);
    }

    const unsigned long readlen = ax::FileUtils::getInstance()->getFileSize(strPath);
    if (fileContent.empty() || readlen == 0) {
        showAlert(strPath.c_str(), "copy Error");
        return;
    }

    std::vector<char> buffer(fileContent.begin(), fileContent.end());
    buffer.push_back('\0');

    std::string destPath = joinPath(m_downloadDir, pFileName);
    CCLOG("[KUpdateLayer] copyData writing to: %s (size=%lu)", destPath.c_str(), readlen);
    if (FILE* fp = std::fopen(destPath.c_str(), "wb+")) {
        while (writelen < readlen) {
            writelen += std::fwrite(buffer.data() + writelen, 1, (readlen - writelen), fp);
            sleep_ms(10);
        }
        std::fclose(fp);
        CCLOG("[KUpdateLayer] copyData done: wrote=%zu bytes", writelen);
    }

    ++m_nLoadedResources;
}

void KUpdateLayer::update(float /*delta*/) {
    CCLOG("[KUpdateLayer] update: state=%d loaded=%d/%d finish=%d", nDownState, m_nLoadedResources, m_nAllLoadResources, m_nLoadedFinish);

    if (_clientNoNetWork && !isOver) return;

    char tmp[64] = {0};
    if (!isHaveClient) {
        CCLOG("[KUpdateLayer] waiting for downloader initialization");
        return;
    }

    if (nDownState == STATE_ON_NET_UP) {
        CCLOG("[KUpdateLayer] ERROR: network");
        errorLoop++;
        if (errorLoop >= ERROR_TIME_OUT) {
            errorLoop = 0;
            showAlert("Lỗi kết nối mạng.", "Thông báo");
        }
        return;
    }
    if (nDownState == STATE_ON_URLFIEL_UP) {
        CCLOG("[KUpdateLayer] ERROR: url fail");
        errorLoop++;
        if (errorLoop >= ERROR_TIME_OUT) {
            errorLoop = 0;
            showAlert("Lỗi tải bản cập nhật.", "Thông báo");
        }
        return;
    }

    if (nDownState == STATE_ON_FIEL_UP) {
        CCLOG("[KUpdateLayer] ERROR: file fail");
        errorLoop++;
        if (errorLoop >= ERROR_TIME_OUT) {
            errorLoop = 0;
            showAlert("Tải file thất bại.", "Thông báo");
        }
        return;
    }
    errorLoop = 0;

    if (nDownState == STATE_LOAD_LISTDATA) {
        // Bắt đầu đọc danh sách dữ liệu
        KIniFile pPakList;
        char nTempStr[256];
        ZeroMemory(nTempStr, sizeof(nTempStr));
        const std::string pakListPath = joinPath(m_downloadDir, (_clientlanguage != 1) ? "paklist_vn.ini" : "paklist.ini");
        nCurpFileName = (_clientlanguage != 1) ? "paklist_vn.ini" : "paklist.ini";
        std::snprintf(nTempStr, sizeof(nTempStr), "%s", pakListPath.c_str());

        if (pPakList.Load(nTempStr)) {
            if (pListData) {
                free(pListData);
                pListData = nullptr;
            }

            int nPakNum = 0, _isOpen = 0;
            pPakList.GetInteger("list", "count", 0, &nPakNum);
            pakDownCount = nPakNum;
            pPakList.GetInteger("list", "isopen", 1, &_isOpen);
            isOpenDown = _isOpen;
            pListData  = static_cast<Kpaklistdata*>(std::malloc(sizeof(Kpaklistdata) * static_cast<std::size_t>(nPakNum)));
            if (pListData && nPakNum > 0) {
                Kpaklistdata* pInfo = pListData;
                for (int i = 0; i < nPakNum; ++i) {
                    char nKey[32];
                    std::snprintf(nKey, sizeof(nKey), "pak_%d", i);
                    pPakList.GetString("list", nKey, "", pInfo->pakName, sizeof(pInfo->pakName));
                    std::snprintf(nKey, sizeof(nKey), "pak_%d_r", i);
                    pPakList.GetInteger("list", nKey, 0, &pInfo->_pakreload);
                    pInfo->_isbreak = (pInfo->_pakreload == 0);
                    ++pInfo;
                }
                nDownState = STATE_DWON_LISTDATA;
            }
            pPakList.Clear();
        }
    }

    if (nDownState == STATE_FLS_UP) {
        // Làm mới: mở lại các pak
        nDownState = STATE_NONE_UP;
        const std::string fielDir = joinPath(m_downloadDir, (_clientlanguage != 1) ? "package_vn.ini" : "package.ini");
        nCurpFileName = (_clientlanguage != 1) ? "package_vn.ini" : "package.ini";
        char nTempstr[256];
        std::snprintf(nTempstr, sizeof(nTempstr), "%s", fielDir.c_str());
        nPakCount = g_pcPakList.Open(nTempstr);
        CCLOG("[KUpdateLayer] refresh pak list: file=%s pakCount=%d", nTempstr, nPakCount);

        if (!nPakCount) {
            CCLOG("[KUpdateLayer] warning: g_pcPakList.Open returned 0, forcing finish after download phase");
        }

        ++m_nLoadedResources;
        if (m_nLoadedResources < m_nAllLoadResources) {
            m_nLoadedResources = m_nAllLoadResources + 1;
        }
    }
    CCLOG("[KUpdateLayer] runtime status: pakCount=%d pakDownCount=%d curPercent=%.2f", nPakCount, pakDownCount, downPtr ? downPtr->getCurpercent() : 0.0F);
    if (!nCurpFileName.empty()) {
        std::snprintf(tmp, sizeof(tmp), "%%%d,%s:%zu",
                      static_cast<int>((static_cast<float>(m_nLoadedResources) / m_nAllLoadResources) * 100.0F),
                      nCurpFileName.c_str(), writelen);
    } else {
        std::snprintf(tmp, sizeof(tmp), "%%%d",
                      static_cast<int>((static_cast<float>(m_nLoadedResources) / m_nAllLoadResources) * 100.0F));
    }

    if (m_nLoadedFinish) {
        if (!nCurpFileName.empty()) {
            std::snprintf(tmp, sizeof(tmp), "%%%d,%s:%zu", 100, nCurpFileName.c_str(), writelen);
        } else {
            std::snprintf(tmp, sizeof(tmp), "%%%d", 100);
        }
    }
    m_pLabelPercent->setString(tmp);
    CCLOG("[KUpdateLayer] progress: %s", tmp);

    if (downPtr) {
        char mstrInfo[64];
        std::snprintf(mstrInfo, sizeof(mstrInfo), "loading:%02f", downPtr->getCurpercent());
        m_pLabelLoading->setString(mstrInfo);
        CCLOG("[KUpdateLayer] loading percent: %s", mstrInfo);
    }

    m_fProgressIndex = ((static_cast<float>(m_nLoadedResources) / m_nAllLoadResources) * 100.0F);
    m_pLoadBarEnd->setPercentage(m_fProgressIndex);

    if (nDownState == STATE_NONE_UP && downPtr && downPtr->getCurpercent() >= 100.0F && m_nLoadedResources > 0 && m_nLoadedResources < m_nAllLoadResources) {
        CCLOG("[KUpdateLayer] downloader reached 100%% while state is idle but loaded=%d/%d, forcing finish", m_nLoadedResources, m_nAllLoadResources);
        m_nLoadedResources = m_nAllLoadResources + 1;
    }

    if (m_nLoadedResources >= m_nAllLoadResources && nDownState == STATE_NONE_UP) {
        CCLOG("[KUpdateLayer] load completed -> loadSuccess()");
        CCLOG("loadSuccess......");
        m_nLoadedResources = 0;
        m_nLoadedFinish    = 1;
        loadSuccess();
        return;
    }
}

void KUpdateLayer::loadSuccess() {
    nCurpFileName.clear();
    CCLOG("[KUpdateLayer] loadSuccess(): switching scene");
    Terminate(1);
    Director::getInstance()->replaceScene(MainMenu::create());
}

void KUpdateLayer::Run() {
    CCLOG("[KUpdateLayer] Run(): begin background downloading");
    // Bắt đầu tải tài nguyên từ máy chủ
    if (!downPtr) {
        CCLOG("[KUpdateLayer] Run(): no downPtr, abort background downloading");
        return;
    }

    int  ndwonstate       = 0;
    bool ret              = false;
    int  isReLoad         = 1;
    bool isBreak          = true;
    int  successFileCount = 0;
    int  failedFileCount  = 0;

    // Tải danh sách pak
    nCurpFileName = (_clientlanguage != 1) ? "paklist_vn.ini" : "paklist.ini";
    while (true) {
        CCLOG("[KUpdateLayer] downloading paklist (reload=%d)", isReLoad);
        if (_clientlanguage != 1)
            ret = downPtr->DownLoadFile(m_WritablePath, "data/paklist_vn.ini", isReLoad);
        else
            ret = downPtr->DownLoadFile(m_WritablePath, "data/paklist.ini", isReLoad);
        isReLoad = 0; // chỉ reload lần đầu

        ndwonstate = downPtr->getDownState();
        if (ndwonstate == 1) nDownState = STATE_ON_NET_UP;
        if (ndwonstate == 2) nDownState = STATE_ON_FIEL_UP;
        if (ndwonstate == 3) nDownState = STATE_ON_URLFIEL_UP;

        CCLOG("[KUpdateLayer] paklist state=%d ret=%d", ndwonstate, (int)ret);

        const bool isPakListCompleted = ret && (ndwonstate == 4 || ndwonstate == 5);
        if (isPakListCompleted) {
            ++successFileCount;
            nDownState = STATE_LOAD_LISTDATA;
            ++m_nLoadedResources;
            CCLOG("[KUpdateLayer] paklist downloaded, parsing... success=%d failed=%d", successFileCount, failedFileCount);
            break;
        }

        if (ndwonstate == 1 || ndwonstate == 2 || ndwonstate == 3) {
            ++failedFileCount;
            CCLOG("[KUpdateLayer] paklist download failed: state=%d success=%d failed=%d", ndwonstate, successFileCount, failedFileCount);
        }
        sleep_ms(500);
    }

    CCLOG("[KUpdateLayer] waiting for list data to be parsed...");
    // Chờ đọc xong danh sách
    while (true) {
        if (nDownState == STATE_DWON_LISTDATA) break;
        sleep_ms(500);
    }

    int nCurSelCount = 0;
    char strVer[128];
    Kpaklistdata* pInfo = pListData;
    if (!pInfo || pakDownCount <= 0) {
        nDownState = STATE_FLS_UP;
        CCLOG("[KUpdateLayer] no pak entries found. Refreshing...");
        ++m_nLoadedResources;
        return;
    }

    std::snprintf(strVer, sizeof(strVer), "%s", pInfo->pakName);
    isReLoad = pInfo->_pakreload;
    isBreak  = pInfo->_isbreak;

    // Tải từng pak theo danh sách
    while (pInfo) {
        nCurpFileName = strVer;
        CCLOG("[KUpdateLayer] downloading pak: %s (reload=%d break=%d)", strVer, isReLoad, (int)isBreak);
        nDownState = STATE_RUN_UP;

        if (!isOpenDown && isBreak && !isReLoad) {
            // Nếu không mở tải và gói này không cần reload thì bỏ qua
            ++nCurSelCount;
            if (nCurSelCount >= pakDownCount) {
                nDownState = STATE_FLS_UP;
                CCLOG("[KUpdateLayer] all paks downloaded. Refreshing...");
                break;
            }
            ++pInfo;
            if (!pInfo) {
                nDownState = STATE_FLS_UP;
                CCLOG("[KUpdateLayer] pak list ended unexpectedly. Refreshing...");
                break;
            }
            std::snprintf(strVer, sizeof(strVer), "%s", pInfo->pakName);
            nCurpFileName = strVer;
            isReLoad = pInfo->_pakreload;
            isBreak  = pInfo->_isbreak;
            ++m_nLoadedResources;
            continue;
        }

        ret = downPtr->DownLoadFile(m_WritablePath, strVer, static_cast<bool>(isReLoad));
        if (isReLoad) isReLoad = 0;

        ndwonstate = downPtr->getDownState();
        if (ndwonstate == 1) nDownState = STATE_ON_NET_UP;
        if (ndwonstate == 2) nDownState = STATE_ON_FIEL_UP;
        if (ndwonstate == 3) nDownState = STATE_ON_URLFIEL_UP;

        CCLOG("[KUpdateLayer] pak result: state=%d ret=%d", ndwonstate, (int)ret);

        const bool isDownloadCompleted = ret && (ndwonstate == 4 || ndwonstate == 5);
        if (isDownloadCompleted) {
            ++successFileCount;
            ++nCurSelCount;
            ++m_nLoadedResources;
            CCLOG("[KUpdateLayer] pak download success: file=%s success=%d failed=%d", strVer, successFileCount, failedFileCount);

            if (nCurSelCount >= pakDownCount) {
                nDownState = STATE_FLS_UP;
                CCLOG("[KUpdateLayer] all paks downloaded. Refreshing... success=%d failed=%d", successFileCount, failedFileCount);
                break;
            }

            ++pInfo;
            if (!pInfo) {
                nDownState = STATE_FLS_UP;
                CCLOG("[KUpdateLayer] pak list ended unexpectedly. Refreshing... success=%d failed=%d", successFileCount, failedFileCount);
                break;
            }

            std::snprintf(strVer, sizeof(strVer), "%s", pInfo->pakName);
            nCurpFileName = strVer;
            CCLOG("[KUpdateLayer] next pak: %s", strVer);
            isReLoad = pInfo->_pakreload;
            isBreak  = pInfo->_isbreak;
        } else if (ndwonstate == 1 || ndwonstate == 2 || ndwonstate == 3) {
            ++failedFileCount;
            CCLOG("[KUpdateLayer] pak download failed: file=%s state=%d success=%d failed=%d", strVer, ndwonstate, successFileCount, failedFileCount);
        }
        sleep_ms(500);
    }

    ++m_nLoadedResources;
    CCLOG("[KUpdateLayer] Run(): end background downloading success=%d failed=%d", successFileCount, failedFileCount);
}

void KUpdateLayer::startload() {
    // Khởi chạy một luồng để tải/giải nén dữ liệu
    if (!downPtr) {
        CCLOG("[KUpdateLayer] startload(): skipped because downPtr is null");
        isHaveClient = false;
        return;
    }

    isHaveClient = true;
    Start();
}
