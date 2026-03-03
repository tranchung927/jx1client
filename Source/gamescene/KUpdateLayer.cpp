#include "KUpdateLayer.h"
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
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#    include <jni.h>
#endif

#if (CC_TARGET_PLATFORM != CC_PLATFORM_WIN32)
#    include <memory.h>
#    include <sys/stat.h>
#    include <sys/types.h>
#    include <dirent.h>
#endif
#ifdef WIN32
#    include <process.h>
#endif

#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;

USING_NS_AX;

iCoreShell* g_pCoreShell = NULL;
extern KImageStore2 m_ImageStore;  // ȫ�ֵ���
// extern KPakList * g_pPakList;
#define ERROR_TIME_OUT 300
KUpdateLayer::KUpdateLayer()
{
    CCLOG("start KUpdateLayer");
    nDownState   = STATE_NONE_UP;
    downPtr      = NULL;
    isOpenDown   = true;
    errorLoop    = 0;
    nPakCount    = 0;
    pakDownCount = 0;
    // pakreload    = 0;
    m_WritablePath       = "";
    pListData            = NULL;
    isHaveClient         = false;
    m_IsInPutIpPadIsOpen = false;
    ZeroMemory(downUrlmask, sizeof(downUrlmask));
    isOver = false;
    CCLOG("end KUpdateLayer");
}

KUpdateLayer::~KUpdateLayer()
{
    CCLOG("start ~KUpdateLayer");
    if (downPtr)
        downPtr->release();
    if (pListData)
    {
        free(pListData);
        pListData = NULL;
    }
    isOver               = false;
    m_IsInPutIpPadIsOpen = false;
    CCLOG("end ~KUpdateLayer");
}

bool KUpdateLayer::init()
{
    if (!Layer::init())
    {
        return false;
    }
    // ������Դ������Ŀ¼
    __pSound        = new AudioEngine;
    ccFileUtils     = ax::FileUtils::getInstance();
    m_downloadDir   = ccFileUtils->getWritablePath();
    m_SDcardDirPath = m_WritablePath = m_downloadDir;

    m_downloadDir += "data";  // download
    initDownloadDir();        // �����ļ���
    creatDownloadDir("music");
    creatDownloadDir("sound");
    m_bIsClientConnecting = false;
    nCurpFileName         = "";
    writelen              = 0;
    //----------------------------------����һ��������
    m_nAllLoadResources = 50;  // ����Դ����
    m_nLoadedResources  = 0;   // �Ѿ������˵�����
    m_fProgressIndex    = 0.0;
    m_nLoadedFinish     = false;

    visibleSize = ax::Director::getInstance()->getVisibleSize();  // �ɼ���
    origin      = ax::Director::getInstance()->getVisibleOrigin();
    kSize = ax::Director::getInstance()->getWinSize();

    // if (!isFileExist("package.ini"))
    // �����ھͿ�ʼ����
    if (_clientlanguage != 1)
        copyData("package_vn.ini");
    else
        copyData("package.ini");

    // else
    ++m_nLoadedResources;
    // ��ʼ����pak
    std::string fielDir;
    if (_clientlanguage != 1)
    {
        fielDir = "package_vn.ini";
#if (CC_TARGET_PLATFORM != CC_PLATFORM_WIN32)
        fielDir = "data\\package_vn.ini";
#endif
    }
    else
    {
        fielDir = "package.ini";
#if (CC_TARGET_PLATFORM != CC_PLATFORM_WIN32)
        fielDir = "data\\package.ini";
#endif
    }

    if (!_clientNoNetWork)
    {  // ���ǵ���
        KIniFile dwmask;
        char nTempstr[128];
        snprintf(nTempstr, sizeof(nTempstr), "%s", fielDir.c_str());
        if (dwmask.Load(nTempstr))
        {
            dwmask.GetString("Package", "down", "", downUrlmask, sizeof(downUrlmask));
            dwmask.Clear();
            //dataChecksum nurlMask;
            char donwPath[64] = {0}, nTempPaht[128];
            //ZeroMemory(donwPath, sizeof(donwPath));
            //ZeroMemory(nTempPaht, sizeof(nTempPaht));
            //nurlMask.SimplyDecrypt(donwPath, downUrlmask);
            snprintf(nTempPaht, sizeof(nTempPaht), "http://%s/", downUrlmask);

            downPtr = downloadFile::GetInst(nTempPaht, m_WritablePath);

        }
        //        if (sprite)//�����߳�
        startload();
    }
    else
    {

    }

    // Lấy kích thước của màn hình
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin      = Director::getInstance()->getVisibleOrigin();
    float winw       = visibleSize.width;   // ��ȡ��Ļ���
    float winh       = visibleSize.height;  // ��ȡ��Ļ�߶�

    // Tạo một Label
    auto label = Label::createWithSystemFont("Hello ChungTV", "Arial", 24);

    // Đặt vị trí của Label ở giữa màn hình
    label->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));

    // Thêm Label vào lớp
    this->addChild(label, 2);
    // ״̬��ʾ
    m_label = Label::createWithTTF("", UI_GAME_FONT_DEFAULT, 18);
    m_label->setAnchorPoint(ax::Vec2(1, 0.5));
    //        m_label->setPosition(ax::Vec2(465,20));
    m_label->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
    this->addChild(m_label, 2);

    m_ImageStore.Init();  // ��ʼ���������
    ++m_nLoadedResources;
    // m_fWaitTime = 0;
    m_pLabelLoading = Label::createWithTTF("loading...", UI_GAME_FONT_DEFAULT, 15);
    m_pLabelPercent = Label::createWithTTF("%0", UI_GAME_FONT_DEFAULT, 15);
    //        m_pLabelLoading->setPosition(ax::Vec2(size.width/2, size.height/4-70));
    m_pLabelLoading->setPosition(Vec2(kSize.width / 2 + origin.x, kSize.height / 4 + origin.y));
    m_pLabelPercent->setPosition(Vec2(kSize.width / 2 + origin.x, kSize.height / 5 + origin.y));
    this->addChild(m_pLabelLoading, 2);
    this->addChild(m_pLabelPercent, 2);
    // loading�Ķ���Ч��
    m_pLoadBarStart = Sprite::create("loadingStart.png");
    m_pLoadBarStart->setPosition(ax::Vec2(kSize.width / 2, kSize.height / 7));
    float sx = m_pLoadBarStart->getTextureRect().getMaxX();
    // float sy  = m_pLoadBarStart->getTextureRect().getMaxY();
    m_pLoadBarStart->setScaleY(2.0f);
    m_pLoadBarStart->setScaleX(winw / sx);
    this->addChild(m_pLoadBarStart, 2);

    Sprite* m_pLoadEndSpr = Sprite::create("loadingEnd.png");
    sx                    = m_pLoadEndSpr->getTextureRect().getMaxX();
    // m_pLoadEndSpr->setScaleX(winw/sx);

    m_pLoadBarEnd = ProgressTimer::create(m_pLoadEndSpr);
    m_pLoadBarEnd->setPercentage(1.0f);
    m_pLoadBarEnd->setPosition(ax::Vec2(kSize.width / 2, kSize.height / 7));
    m_pLoadBarEnd->setType(ProgressTimer::Type::BAR);
    m_pLoadBarEnd->setBarChangeRate(ax::Vec2(1, 0));
    m_pLoadBarEnd->setMidpoint(ax::Vec2(0, 0));
    m_pLoadBarEnd->setScaleX(winw / sx);
    m_pLoadBarEnd->setScaleY(2.0f);
    this->addChild(m_pLoadBarEnd, 2);

    // ���ر���
    Sprite* sprite = Sprite::create("longin_bg.png");
    // ax::Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y)
    sprite->setPosition(ax::Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
    float spx = sprite->getTextureRect().getMaxX();
    float spy = sprite->getTextureRect().getMaxY();
    sprite->setScaleX(winw / spx);  // ���þ��������ű���
    sprite->setScaleY(winh / spy);
    this->addChild(sprite, 0);

    this->scheduleUpdate();
    return true;
}

// ��ȷ���� ��ʼ�߳�
void KUpdateLayer::startInPutCallback(Node* pNode)
{
    if (!pNode)
        return;

    if (pNode->getTag() == 2)
    {  // ȡ����ť
        m_IsInPutIpPadIsOpen = false;
        return;
    }

    m_IsInPutIpPadIsOpen = false;
    KIniFile dwmask;
    char nTempstr[128];
    snprintf(nTempstr, sizeof(nTempstr), "data\\package_temp.ini");
    if (dwmask.Load(nTempstr))
    {
        dwmask.GetString("Package", "down", "", downUrlmask, sizeof(downUrlmask));
        dwmask.Clear();
        char nTempPaht[128];
        ZeroMemory(nTempPaht, sizeof(nTempPaht));
        snprintf(nTempPaht, sizeof(nTempPaht), "http://%s:%d/", downUrlmask, 8080);
        // std::string nTempPaht ="http://"+donwPath;
        // ����һ������ָ��
        downPtr = downloadFile::GetInst(nTempPaht, m_WritablePath);
        isOver  = true;
        startload();
        return;
    }
    showAlert("The Server Ip Error", "Error");
}

void KUpdateLayer::creatDownloadDir(std::string nTempPath)
{

    // �������Ŀ¼�����ڣ��򴴽�����Ŀ¼
    std::string m_TempDir = ccFileUtils->getWritablePath();
    // showAlert(m_downloadDir.c_str(),"m_downloadDir");
    m_TempDir += nTempPath;  // download
#if (CC_TARGET_PLATFORM != CC_PLATFORM_WIN32)
    DIR* pDir = NULL;
    pDir      = opendir(m_TempDir.c_str());
    if (!pDir)
    {
        mkdir(m_TempDir.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
    }
    else
        closedir(pDir);
#else
    if ((GetFileAttributesA(m_TempDir.c_str())) == INVALID_FILE_ATTRIBUTES)
    {
        CreateDirectoryA(m_TempDir.c_str(), 0);
    }
#endif
}

void KUpdateLayer::initDownloadDir()
{

// �������Ŀ¼�����ڣ��򴴽�����Ŀ¼
#if (CC_TARGET_PLATFORM != CC_PLATFORM_WIN32)
    DIR* pDir = NULL;

    pDir = opendir(m_downloadDir.c_str());
    if (!pDir)
    {
        mkdir(m_downloadDir.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
    }
#else
    if ((GetFileAttributesA(m_downloadDir.c_str())) == INVALID_FILE_ATTRIBUTES)
    {
        CreateDirectoryA(m_downloadDir.c_str(), 0);
    }
#endif
}
// ɾ��Ŀ¼
void KUpdateLayer::deleteDir(std::string dir)
{
    // Remove downloaded files
#if (CC_TARGET_PLATFORM != CC_PLATFORM_WIN32)
    std::string command = "rm -r ";
    // Path may include space.
    command += "\"" + dir + "\"";
    system(command.c_str());
#else
    std::string command = "rd /s /q ";
    // Path may include space.
    command += "\"" + dir + "\"";
    system(command.c_str());
#endif
}

// ������Դ
void KUpdateLayer::updates(Ref* pSender)
{
    // ��ʼ������Ŀ¼
    initDownloadDir();
}
// ������ť
void KUpdateLayer::reset(Ref* pSender)
{
    if ("" != m_downloadDir)
    {
        // ɾ������Ŀ¼
        deleteDir(m_downloadDir);
    }
}
// ��ȡ�汾
void KUpdateLayer::getClientVersion(Ref* pSender)
{
    std::string msg = StringUtils::format("current client version : %s", "test");
    // CCLOG("%s",msg->getCString());
    m_label->setString(msg);
}
// ������
void KUpdateLayer::checkUpdate(Ref* pSender) {}
// ���볡��
void KUpdateLayer::enterScene(Ref* pSender)
{

    /*if (nAcclient)
    {
            if (!nAcclient->FsGameServerConnectTo("192.168.100.8",56722))
            {
                    m_bIsClientConnecting = true;
                    nAcclient->Startup();     //������Ϸ���������߳�
                    showAlert("���ӷ�����ʧ�ܣ�","��ʾ:");
                    return;
            }
    }
    else
            return;*/
    // ax::Director::getInstance()->replaceScene(Klogin_f::scene());
}
// ���볡���� ������Դ �ͳ�ʼ���ͻ���

int KUpdateLayer::isFileExist(const char* pFileName)
{
    if (!pFileName)
        return false;
    // strFilePathName is :/data/data/ + package name
    // std::string filePath = ax::FileUtils::getInstance()->getWritablePath();//getWriteablePath();

    std::string filePath = m_downloadDir;
    filePath += "/";
    filePath += pFileName;
    // filePath +=pFileName;
    int pSize = 0;
    FILE* fp  = fopen(filePath.c_str(), "r");
    if (fp)
    {
        fseek(fp, 0, SEEK_END);
        pSize = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        fclose(fp);
    }
    return pSize;
}

void KUpdateLayer::copyData(const char* pFileName)
{
    nCurpFileName = pFileName;
    writelen      = 0;
#ifndef WIN32
    if (!ccFileUtils->isFileExist(nCurpFileName))
    {
        showAlert(nCurpFileName.c_str(), "file is Noexist!");
        return;
    }
#endif
    std::string strPath     = ccFileUtils->fullPathForFilename(pFileName);
    unsigned long readlen   = 0;
    std::string fileContent = "";

    if (FileUtils::getInstance()->isFileExist(strPath))
    {
        fileContent = FileUtils::getInstance()->getStringFromFile(strPath);
    }

    std::vector<char> buffer(fileContent.size() + 1);  // +1 for null terminator
    std::strcpy(buffer.data(), fileContent.c_str());
    readlen = ax::FileUtils::getInstance()->getFileSize(strPath);

    if (buffer.empty())
    {
        showAlert(strPath.c_str(), "copy Error");
    }
    // ax::FileUtils::getInstance()->addSearchPath();
    // ax::FileUtils::getInstance()->SearchPath();
    nCurpFileName        = pFileName;
    std::string destPath = m_downloadDir;
    destPath += "/";
    destPath += pFileName;
    FILE* fp = fopen(destPath.c_str(), "wb+");  // ·��
    if (fp)
    {

        while (writelen < readlen)
        {  // ѭ��д��
            writelen += fwrite(buffer.data() + writelen, 1, (readlen - writelen),
                               fp);  // 1�ֽ�1�ֽ�д��dstFile�ļ���buffer + writelen��ʾ����һ�δ�����д

#ifdef WIN32
            Sleep(10);
#else
            sleep(1);
#endif
            // loadingCallBack_new(pFileName,0,writelen);
        }
        // fwrite(data,sizeof(unsigned char),len,fp);
        fclose(fp);
    }

    ++m_nLoadedResources;

}

void KUpdateLayer::update(float delta)
{
    if (_clientNoNetWork && !isOver)
        return;

    char tmp[64] = {0};
    if (!isHaveClient)
    {
        errorLoop++;
        if (errorLoop >= ERROR_TIME_OUT)
        {
            errorLoop = 0;
#ifdef WIN32
            showAlert("Không tìm thấy client.\nVui lòng tải lại từ trang chủ.", "Thông báo");
#else
            showAlert("Không tìm thấy client.\nVui lòng tải lại từ trang chủ.", "Thông báo");
#endif
        }
        return;
    }

    if (nDownState == STATE_ON_NET_UP)
    {  // �����ϰ�
        errorLoop++;
        if (errorLoop >= ERROR_TIME_OUT)
        {
            errorLoop = 0;
#ifdef WIN32
            showAlert("Lỗi kết nối mạng.", "Thông báo");
#else
            showAlert("Lỗi kết nối mạng.", "Thông báo");
#endif
        }
        return;
    }
    if (nDownState == STATE_ON_URLFIEL_UP)
    {  // �����ϰ���STATE_ON_FIEL_UP
        errorLoop++;
        if (errorLoop >= ERROR_TIME_OUT)
        {
            errorLoop = 0;
#ifdef WIN32
            showAlert("Lỗi tải bản cập nhật.", "Thông báo");
#else
            showAlert("Lỗi tải bản cập nhật.", "Thông báo");
#endif
        }
        return;
    }

    if (nDownState == STATE_ON_FIEL_UP)
    {  // �����ϰ���
        errorLoop++;
        if (errorLoop >= ERROR_TIME_OUT)
        {
            errorLoop = 0;
#ifdef WIN32
            showAlert("Tải file thất bại.", "Thông báo");
#else
            showAlert("Tải file thất bại.", "Thông báo");
#endif
        }
        return;
    }
    errorLoop = 0;
    if (nDownState == STATE_LOAD_LISTDATA)
    {  // ��ʼ�����������
        KIniFile pPakList;
        char nTempStr[128];
        ZeroMemory(nTempStr, sizeof(nTempStr));

        if (_clientlanguage != 1)
            snprintf(nTempStr, sizeof(nTempStr), "data/paklist_vn.ini");
        else
            snprintf(nTempStr, sizeof(nTempStr), "data/paklist.ini");

        if (pPakList.Load(nTempStr))
        {  // �����ȡ�б�ɹ�
            if (pListData)
            {
                free(pListData);
                pListData = NULL;
            }

            int nPakNum = 0, reload = 0, _isOpen = 0;
            pPakList.GetInteger("list", "count", 0, &nPakNum);
            // pPakList.GetInteger("list","reload",0,&reload);//�Ƿ���������
            pakDownCount = nPakNum;
            pPakList.GetInteger("list", "isopen", 1, &_isOpen);
            isOpenDown = _isOpen;
            // pakreload    = reload;
            pListData = (Kpaklistdata*)malloc(sizeof(Kpaklistdata) * nPakNum);
            if (pListData && nPakNum > 0)
            {
                Kpaklistdata* pInfo = (Kpaklistdata*)pListData;
                for (int i = 0; i < nPakNum; i++)
                {
                    char nKey[32];
                    snprintf(nKey, sizeof(nKey), "pak_%d", i);
                    pPakList.GetString("list", nKey, "", pInfo->pakName, sizeof(pInfo->pakName));
                    snprintf(nKey, sizeof(nKey), "pak_%d_r", i);
                    pPakList.GetInteger("list", nKey, 0, &pInfo->_pakreload);

                    if (pInfo->_pakreload)
                        pInfo->_isbreak = false;
                    else
                        pInfo->_isbreak = true;

                    pInfo++;
                }
                // ��ʼ����
                nDownState = STATE_DWON_LISTDATA;
            }
            pPakList.Clear();
        }
    }

    if (nDownState == STATE_FLS_UP)
    {  // ���������ɾͼ���pak
        nDownState = STATE_NONE_UP;
        std::string fielDir;
        if (_clientlanguage != 1)
        {
            fielDir = "package_vn.ini";
#if (CC_TARGET_PLATFORM != CC_PLATFORM_WIN32)
            fielDir = "data\\package_vn.ini";
#endif
        }
        else
        {
            fielDir = "package.ini";
#if (CC_TARGET_PLATFORM != CC_PLATFORM_WIN32)
            fielDir = "data\\package.ini";
#endif
        }
        char nTempstr[128];
        snprintf(nTempstr, sizeof(nTempstr), "%s", fielDir.c_str());
        nPakCount = g_pcPakList.Open(nTempstr);  // ����PAK
        if (nPakCount)
        {
            ++m_nLoadedResources;
            if (m_nLoadedResources < m_nAllLoadResources)
                m_nLoadedResources = m_nAllLoadResources + 1;

            // char msg[64];
            // sprintf(msg,"%d",nPakCount);
            // showAlert(msg,"pak");
        }
    }

    if (nCurpFileName.length() > 0)
    {
        snprintf(tmp, sizeof(tmp), "%%%d,%s:%zu", (int)(((float)m_nLoadedResources / m_nAllLoadResources) * 100),
                nCurpFileName.c_str(), writelen);
    }
    else
        snprintf(tmp, sizeof(tmp), "%%%d", (int)(((float)m_nLoadedResources / m_nAllLoadResources) * 100));

    if (m_nLoadedFinish)
    {
        if (nCurpFileName.length() > 0)
        {
            snprintf(tmp, sizeof(tmp), "%%%d,%s:%zu", 100, nCurpFileName.c_str(), writelen);
        }
        else
            snprintf(tmp, sizeof(tmp), "%%%d", 100);
    }
    m_pLabelPercent->setString(tmp);

    if (downPtr)
    {
        char mstrInfo[64];
        snprintf(mstrInfo, sizeof(mstrInfo), "loading:%02f", downPtr->getCurpercent());
        m_pLabelLoading->setString(mstrInfo);
    }

    m_fProgressIndex = (((float)m_nLoadedResources / m_nAllLoadResources) * 100);
    if (m_nLoadedFinish)
    {
        m_fProgressIndex = (float)100;
        m_pLoadBarEnd->setPercentage(m_fProgressIndex);
    }
    else
        m_pLoadBarEnd->setPercentage(m_fProgressIndex);
    // m_pLoadBarEnd->setPercentage(m_fProgressIndex);

    if (m_nLoadedResources >= m_nAllLoadResources && nDownState == STATE_NONE_UP)
    {  // �������
        CCLOG("loadSuccess......");
        m_nLoadedResources = 0;
        m_nLoadedFinish    = true;
        // ��ʼ ��ת
        loadSuccess();
        return;
    }
}
void KUpdateLayer::loadSuccess()
{
    Terminate(1);
//    Director::getInstance()->replaceScene(Klogin_f::scene());
    Director::getInstance()->replaceScene(MainMenu::create());
    //Director::getInstance()->pushScene(loginSuccessScene);
}

void KUpdateLayer::loadingCallBack(Ref* obj)
{  // ���ý������Ľ���
    ++m_nLoadedResources;
    char tmp[10];
    snprintf(tmp, sizeof(tmp), "%%%d", (int)(((float)m_nLoadedResources / m_nAllLoadResources) * 100));
    m_pLabelPercent->setString(tmp);
    m_fProgressIndex = (((float)m_nLoadedResources / m_nAllLoadResources) * 100);
    m_pLoadBarEnd->setPercentage(m_fProgressIndex);
}

void KUpdateLayer::loadingCallBack_new(const char* nFileNane, int nTalSize, int nCurSize)
{  // ���ý������Ľ���
    ++m_nLoadedResources;
    char tmp[64] = {0};
    if (nFileNane)
    {
        snprintf(tmp, sizeof(tmp), "%%%d,%s:%u", (int)(((float)m_nLoadedResources / m_nAllLoadResources) * 100), nFileNane, nCurSize);
    }
    else
        snprintf(tmp, sizeof(tmp), "%%%d", (int)(((float)m_nLoadedResources / m_nAllLoadResources) * 100));

    m_pLabelPercent->setString(tmp);
    m_fProgressIndex = (((float)m_nLoadedResources / m_nAllLoadResources) * 100);
    m_pLoadBarEnd->setPercentage(m_fProgressIndex);
}

void KUpdateLayer::Run()
{
    // if (nPakCount)
    {  // ��ʼ����Զ����Դ
        if (downPtr)
        {
            int ndwonstate = 0;
            bool ret       = false;
            // std::string nFullPath = m_WritablePath;
            // nFullPath +="";
            int isReLoad = 1;
            bool isBreak = true;
            while (true)  // �������б�
            {             // ѭ������ ÿ30��������� ����������
                if (_clientlanguage != 1)
                    ret = downPtr->DownLoadFile(m_WritablePath, "data/paklist_vn.ini", isReLoad);  // ֱ������ ���ж����߳�
                else
                    ret = downPtr->DownLoadFile(m_WritablePath, "data/paklist.ini", isReLoad);  // ֱ������ ���ж����߳�
                // ֻ���һ�Ρ�ʣ�µ�Ϊ�ϵ�����
                isReLoad = 0;
                if (downPtr->getDownState() == 1)
                {
                    nDownState = STATE_ON_NET_UP;
                    //++m_nLoadedResources;
                }
                if (downPtr->getDownState() == 2)
                {
                    nDownState = STATE_ON_FIEL_UP;
                    //++m_nLoadedResources;
                }
                if (downPtr->getDownState() == 3)
                {
                    nDownState = STATE_ON_URLFIEL_UP;
                    //++m_nLoadedResources;
                }

                if (ret)
                {  // �������
                    nDownState = STATE_LOAD_LISTDATA;
                    ++m_nLoadedResources;
                    break;
                }
#ifndef WIN32
                sleep(0.5);  // ÿ�������м���0.5��
#else
                Sleep(500);
#endif
            }

            while (true)  // �������б�
            {             // ѭ���������ȴ���ȡ�����������
                if (nDownState == STATE_DWON_LISTDATA)
                    break;
#ifndef WIN32
                sleep(0.5);  // ÿ�������м���0.5��
#else
                Sleep(500);
#endif
            }

            /*if  (!isOpenDown && nDownState==STATE_DWON_LISTDATA)
            {//���û�п������ص�
                    ++m_nLoadedResources;
                    nDownState = STATE_FLS_UP;
                    return;
            }*/

            int nCurSelCount = 0;
            char strVer[128];
            Kpaklistdata* pInfo = (Kpaklistdata*)pListData;
            snprintf(strVer, sizeof(strVer), "%s", pInfo->pakName);
            isReLoad = pInfo->_pakreload;
            isBreak  = pInfo->_isbreak;
            while (true)
            {  // ѭ������ ÿ30��������� ���������� foldername + "/"
                nDownState = STATE_RUN_UP;

                if (!isOpenDown && isBreak && !isReLoad)
                {  // ���û�п������ص� �����ǲ���Ҫ���ص� ��ֱ�ӷ��� ������һ��
                    nCurSelCount++;
                    if (nCurSelCount >= pakDownCount)
                    {
                        nDownState = STATE_FLS_UP;
                        break;
                    }
                    else
                        pInfo++;
                    // ��һ��
                    snprintf(strVer, sizeof(strVer), "%s", pInfo->pakName);
                    isReLoad = pInfo->_pakreload;
                    isBreak  = pInfo->_isbreak;
                    ++m_nLoadedResources;
                    continue;
                }

                ret = downPtr->DownLoadFile(m_WritablePath, strVer, (bool)isReLoad);  // ֱ������ ���ж����߳�
                if (isReLoad)
                    isReLoad = 0;
                if (downPtr->getDownState() == 1)
                {
                    nDownState = STATE_ON_NET_UP;
                    ;
                    //++m_nLoadedResources;
                }
                if (downPtr->getDownState() == 2)
                {
                    nDownState = STATE_ON_FIEL_UP;
                    //++m_nLoadedResources;
                }
                if (downPtr->getDownState() == 3)
                {
                    nDownState = STATE_ON_URLFIEL_UP;
                    //++m_nLoadedResources;
                }

                if (ret && downPtr->getDownState() == 5)
                {  // �������
                    nCurSelCount++;
                    if (nCurSelCount >= pakDownCount)
                    {
                        nDownState = STATE_FLS_UP;
                        break;
                    }
                    else
                        pInfo++;

                    snprintf(strVer, sizeof(strVer), "%s", pInfo->pakName);
                    CCLOG("%s",strVer);
                    isReLoad = pInfo->_pakreload;
                    ++m_nLoadedResources;
                    // continue;
                }
#ifndef WIN32
                sleep(0.5);  // ÿ�������м���0.5��
#else
                Sleep(500);
#endif
            }
        }
        ++m_nLoadedResources;
    }  // end pakcount
}

void KUpdateLayer::startload()
{  // ����һ���̸߳��ƽ�ѹ����

    isHaveClient = true;
    Start();
    return;
}
