#include "engine/KPlatform.h"
#include "gamecore/KCore.h"
#include "KuiNewPlayer.h"
//#include "Klogin.h"
#include "Klogin_f.h"
#include "KTranSence.h"
#include "engine/CXmlStream.h"
#include "engine/KIniFile.h"
#include "gamecore/CoreShell.h"
#include "gameui/PopupLayer.h"
//#include "engine/KPalette.h"
#include "engine/KCodec.h"
#include "engine/KCodecLzo.h"
#include "engine/KFile.h"
#include "engine/KFilePath.h"
#include "engine/KList.h"
#include "engine/KMemBase.h"
#include "engine/KMemClass.h"
#include "engine/KMemStack.h"
#include "engine/KNode.h"
#include "engine/KPakFile.h"
#include "engine/KStrBase.h"
#include "engine/KPakList.h"
#include "engine/XPackFile.h"
#include "gamecore/KNpc.h"
#include "gamecore/KNpcSet.h"
#include "gamecore/KPlayer.h"
#include "gamecore/KPlayerSet.h"
#include "gamecore/KSubWorldSet.h"
#include "gamescene/KUpdateLayer.h"
#include "gameui/KuiSelNativePlace.h"
#include <iostream>
#include <fstream>
extern iCoreShell*	g_pCoreShell; //ȫ�ֵ���
extern KImageStore2 m_ImageStore; //ȫ�ֵ���
//extern KPakList     g_pPakList;
//extern KPakList * g_pPakList;
#include <list>
USING_NS_AX;
KuiNewPlayer::KuiNewPlayer()
{
  m_Status = LL_S_ROLE_LIST_READY; //�ȴ���ɫ������Ϸ������
  m_Result = LL_R_NOTHING;
  memset(&m_Info, 0, sizeof(m_Info));
  m_Info.Attribute = series_metal;
  m_Info.Gender    = OBJ_G_MALE;
  m_Info.NativePlaceId = nNativePlaceId;
  memset(&m_propTypeInfoTable, 0, sizeof(m_propTypeInfoTable));
  pLoginItem=NULL;
}

KuiNewPlayer::~KuiNewPlayer()
{
}
Scene* KuiNewPlayer::scene()
{
	// 'scene' is an autorelease object
	Scene *scene = Scene::create();

	// 'layer' is an autorelease object
	KuiNewPlayer *
    layer = KuiNewPlayer::create();

	// add layer as a child to scene
	scene->addChild(layer);

	// return the scene
	return scene;
}

// on "init" you need to initialize your instance
bool KuiNewPlayer::init()
{
	//////////////////////////////
	// 1. super init first
	if ( !Layer::init() )
	{
		return false;
	}
	/*visibleSize        = ax::Director::getInstance()->getVisibleSize();//�ɼ���
	origin             = ax::Director::getInstance()->getVisibleOrigin();
	Size size        = ax::Director::getInstance()->getWinSize();
	*/
	m_sprCount         =0;
	//nTimeCount = 0;
	m_nWidth   = 0;
	m_nHeight  = 0;
	clicked    = false;
	//pWW        =NULL;
	Animate* animate         =NULL;

    auto touchListener = EventListenerTouchAllAtOnce::create();
    touchListener->onTouchesEnded = AX_CALLBACK_2(KuiNewPlayer::ccTouchesEnded, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);

	this->scheduleUpdate();  //ң�˸��µ��ú���
	 //��ֹ��㴥��
	char nSprName[128]={0};
	int nFrams=0;
	m_CurSelIndex = 0;
	MenuItemImage *pCancelItem=NULL;
	SelbgSprite = NULL;
	//Layer *mainlayer = Layer::create();
	actionManager = ax::Director::getInstance()->getActionManager();
	/*pRoela = Label::createWithTTF("", UI_GAME_FONT_DEFAULT, 12);
	pRoelb = Label::createWithTTF("", UI_GAME_FONT_DEFAULT, 12);
	pRoelc = Label::createWithTTF("", UI_GAME_FONT_DEFAULT, 12);
	*/
	//float m_Scalex,m_Scaley; //�뱳�� ��Ļ��С�����ű���
	//��½ѡ��� \Spr\Ui3\�������\ѡ�浵����\��ɫ
	sprintf(m_szPlayerImgPrefix,"\\Spr\\Ui3\\登入界面\\选存档人物\\角色");
	sprintf(nSprName,"\\Spr\\Ui3\\登入界面\\登陆界面_7_角色名.spr");
	g_StrLower(nSprName);
	Texture2D *SelCur = _getinidata.getinidata_new(nSprName,0,&m_nWidth,&m_nHeight,&nFrams);
	if (SelCur)
	{
		SelbgSprite=Sprite::createWithTexture(SelCur);
		SelbgSprite->setPosition(ax::Vec2(visibleSize.width/2 + origin.x,50));
		m_sprCount++;
		this->addChild(SelbgSprite,1,m_sprCount);
	}

	//ȷ�ϰ�ť
	nFrams=0; //"\\spr\\Ui3\\loginui\\saverole\\ok_9_btn.spr"
	sprintf(nSprName,"\\Spr\\Ui3\\登入界面\\新建角色按钮\\登陆界面_7_确定.spr");//"\\Spr\\Ui3\\�������\\ѡ�浵��ɫ��ť\\��½����_9_ȷ��.spr");
	g_StrLower(nSprName);
	Texture2D *Login = _getinidata.getinidata_new(nSprName,0,&m_nWidth,&m_nHeight,&nFrams);
	if  (Login)
	{
		char nSprFilePath[64]={0};
		unsigned long nFielpahtdwid = g_FileName2Id(nSprName);
		sprintf(nSprFilePath,"%u-%d",nFielpahtdwid,0);
		Texture2D *pNormalTexture = ax::Director::getInstance()->getTextureCache()->getTextureForKey(nSprFilePath);
		sprintf(nSprFilePath,"%u-%d",nFielpahtdwid,1);
		Texture2D *pSelectedTexture = ax::Director::getInstance()->getTextureCache()->getTextureForKey(nSprFilePath);
		sprintf(nSprFilePath,"%u-%d",nFielpahtdwid,2);
		Texture2D *pDisabledTexture = ax::Director::getInstance()->getTextureCache()->getTextureForKey(nSprFilePath);

        pLoginItem = MenuItemImage::create();
        pLoginItem->setNormalImage(Sprite::createWithTexture(pNormalTexture));
        pLoginItem->setSelectedImage(Sprite::createWithTexture(pSelectedTexture));
        pLoginItem->setDisabledImage(Sprite::createWithTexture(pDisabledTexture));
        pLoginItem->setCallback(CC_CALLBACK_1(KuiNewPlayer::mainEnterCallback, this));
		//pLoginItem->setPosition(ax::Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));//һ����ť
		//pLoginItem->setOrderOfArrival(1);
		//pLoginItem->setScaleX(m_Scalex); //���þ��������ű���
		//pLoginItem->setScaleY(m_Scaley);
	}
	//ȡ�� �˵���½����
	nFrams=0;
	sprintf(nSprName,"\\Spr\\Ui3\\登入界面\\新建角色按钮\\登陆界面_7_取消.spr");//"\\spr\\Ui3\\loginui\\saverole\\cancel_9_btn.spr");//"\\Spr\\Ui3\\�������\\ѡ�浵��ɫ��ť\\��½����_9_ȡ��.spr");
	g_StrLower(nSprName);
	Texture2D *Cancel = _getinidata.getinidata_new(nSprName,0,&m_nWidth,&m_nHeight,&nFrams);
	if (Cancel)
	{
		char nSprFilePath[64]={0};
		unsigned long nFielpahtdwid = g_FileName2Id(nSprName);
		sprintf(nSprFilePath,"%u-%d",nFielpahtdwid,0);
		Texture2D *pNormalTexture = ax::Director::getInstance()->getTextureCache()->getTextureForKey(nSprFilePath);
		sprintf(nSprFilePath,"%u-%d",nFielpahtdwid,1);
		Texture2D *pSelectedTexture = ax::Director::getInstance()->getTextureCache()->getTextureForKey(nSprFilePath);
		sprintf(nSprFilePath,"%u-%d",nFielpahtdwid,2);
		Texture2D *pDisabledTexture = ax::Director::getInstance()->getTextureCache()->getTextureForKey(nSprFilePath);

        pCancelItem = MenuItemImage::create();
        pCancelItem->setNormalImage(Sprite::createWithTexture(pNormalTexture));
        pCancelItem->setSelectedImage(Sprite::createWithTexture(pSelectedTexture));
        pCancelItem->setDisabledImage(Sprite::createWithTexture(pDisabledTexture));
        pCancelItem->setCallback(CC_CALLBACK_1(KuiNewPlayer::mExit, this));
		//pCancelItem->setPosition(ax::Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));//һ����ť
		//pCancelItem->setScaleX(m_Scalex); //���þ��������ű���
		//pCancelItem->setScaleY(m_Scaley);
		//pCancelItem->setOrderOfArrival(1);
	}
	Sprite *bgSprite=Sprite::create("loginbg_2-9.jpg");//��½����
	bgSprite->setPosition(ax::Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));
	float winw = visibleSize.width;   //��ȡ��Ļ���
	float winh = visibleSize.height;  //��ȡ��Ļ�߶�
	float spx  = bgSprite->getTextureRect().getMaxX();
	float spy  = bgSprite->getTextureRect().getMaxY();
	bgSprite->setScaleX(winw/spx); //���þ��������ű���
	bgSprite->setScaleY(winh/spy);
	m_sprCount++;
	this->addChild(bgSprite,0,m_sprCount);
	//���涯��Ч��
		 sprintf(nSprName,"\\spr\\Ui3\\loginui\\uidonghua\\yezi.spr");
		 g_StrLower(nSprName);
		 Texture2D *bgCur = _getinidata.getinidata_new(nSprName,0,&m_nWidth,&m_nHeight,&nFrams);
		 if  (bgCur)
		 {
			Sprite *ySprite=Sprite::createWithTexture(bgCur);
			ySprite->setPosition(ax::Vec2(visibleSize.width/2 + origin.x-100, visibleSize.height/2 + origin.y));

			/*float winw = visibleSize.width;   //��ȡ��Ļ���
			float winh = visibleSize.height;  //��ȡ��Ļ�߶�
			float spx  = bgSprite->getTextureRect().getMaxX();
			float spy  = bgSprite->getTextureRect().getMaxY();
			ySprite->setScaleX(winw/spx); //���þ��������ű���
			ySprite->setScaleY(winh/spy);
			*/
			m_sprCount++;
			this->addChild(ySprite,2,m_sprCount);
			char nySprFilePath[64]={0};
			Animation*animation = Animation::create();//����һ������
			unsigned long nFielpahtdwid = g_FileName2Id(nSprName);
			for (int i=0;i<nFrams;i++)
			{
				sprintf(nySprFilePath,"%u-%d",nFielpahtdwid,i);
				//messageBox(nSprFilePath,"animation");
                auto texture = ax::Director::getInstance()->getTextureCache()->getTextureForKey(nySprFilePath);
                Rect rect = Rect::ZERO;
                rect.size = texture->getContentSize();
                animation->addSpriteFrameWithTexture(texture, rect);
//				animation->addSpriteFrameWithFile(nySprFilePath);//�����������������֡
			}
			//���ö���֡��ʱ����
			animation->setDelayPerUnit(0.10f);
			//���ò���ѭ�� һֱ���� Ϊ-1
		    animation->setLoops(-1);
			//���ö���������ָ�����һ֡ setRestoreOriginalFrame
			animation->setRestoreOriginalFrame(TRUE);
			// ����һ����Զѭ���Ķ���
			animate = Animate::create(animation);
			//RepeatForever* forever = RepeatForever::create(animate);
			if (ySprite) {
                auto callback = [this](ax::Node* sender) {
                    KuiNewPlayer::finish();
                };
                ySprite->runAction(Sequence::create(animate,callback,NULL));
            }

		 }
		Menu* pMenu=NULL;
		if (Login && Cancel)
		{
			pMenu = Menu::create(pLoginItem,pCancelItem,NULL);
			//pMenu->alignItemsInRows(1,2,3,4,NULL);     //����һ��
			//pMenu->alignItemsVertically();             //�������� alignItemsHorizontally
			pMenu->alignItemsHorizontally();             //�������� Ĭ�ϼ����kDefaultPadding=5??
			pMenu->alignItemsHorizontallyWithPadding(7);//ָ������??
			//pMenu->alignItemsVertically();             //Ĭ�ϼ��Ҳ��kDefaultPadding=5?
			//pMenu->alignItemsInRows(3,2,1,NULL);  ?
			//pMenu->alignItemsVerticallyWithPadding(20);//ָ�����
			pMenu->setPosition(ax::Vec2(371,40));//ax::Vec2::ZERO
			SelbgSprite->addChild(pMenu,2);
		}
		 MenuItemImage* pjinItem=NULL;
		 MenuItemImage* pmuItem=NULL;
		 MenuItemImage* pshuiItem=NULL;
         MenuItemImage* phuoItem=NULL;
		 MenuItemImage* ptuItem=NULL;
		//��
		nFrams=0;
		sprintf(nSprName,"\\Spr\\Ui3\\登入界面\\新建角色按钮\\登陆界面_7_金.spr");//"\\spr\\Ui3\\loginui\\saverole\\cancel_9_btn.spr");//"\\Spr\\Ui3\\�������\\ѡ�浵��ɫ��ť\\��½����_9_ȡ��.spr");
		g_StrLower(nSprName);
		Texture2D *jinTexture = _getinidata.getinidata_new(nSprName,0,&m_nWidth,&m_nHeight,&nFrams);
		if (jinTexture)
		{
			char nSprFilePath[64]={0};
			unsigned long nFielpahtdwid = g_FileName2Id(nSprName);
			sprintf(nSprFilePath,"%u-%d",nFielpahtdwid,0);
			Texture2D *pNormalTexture = ax::Director::getInstance()->getTextureCache()->getTextureForKey(nSprFilePath);
			sprintf(nSprFilePath,"%u-%d",nFielpahtdwid,1);
			Texture2D *pSelectedTexture = ax::Director::getInstance()->getTextureCache()->getTextureForKey(nSprFilePath);

            pjinItem = MenuItemImage::create();
            pjinItem->setNormalImage(Sprite::createWithTexture(pNormalTexture));
            pjinItem->setSelectedImage(Sprite::createWithTexture(pSelectedTexture));
            pjinItem->setDisabledImage(Sprite::createWithTexture(pSelectedTexture));
            pjinItem->setCallback(CC_CALLBACK_1(KuiNewPlayer::mSellistCallBack, this));
			pjinItem->setTag(1);
		}
		//ľ
		nFrams=0;
		sprintf(nSprName,"\\Spr\\Ui3\\登入界面\\新建角色按钮\\登陆界面_7_木.spr");//"\\spr\\Ui3\\loginui\\saverole\\cancel_9_btn.spr");//"\\Spr\\Ui3\\�������\\ѡ�浵��ɫ��ť\\��½����_9_ȡ��.spr");
		g_StrLower(nSprName);
		Texture2D *muTexture = _getinidata.getinidata_new(nSprName,0,&m_nWidth,&m_nHeight,&nFrams);
		if (muTexture)
		{
			char nSprFilePath[64]={0};
			unsigned long nFielpahtdwid = g_FileName2Id(nSprName);
			sprintf(nSprFilePath,"%u-%d",nFielpahtdwid,0);
			Texture2D *pNormalTexture = ax::Director::getInstance()->getTextureCache()->getTextureForKey(nSprFilePath);
			sprintf(nSprFilePath,"%u-%d",nFielpahtdwid,1);
			Texture2D *pSelectedTexture = ax::Director::getInstance()->getTextureCache()->getTextureForKey(nSprFilePath);

            pmuItem = MenuItemImage::create();
            pmuItem->setNormalImage(Sprite::createWithTexture(pNormalTexture));
            pmuItem->setSelectedImage(Sprite::createWithTexture(pSelectedTexture));
            pmuItem->setDisabledImage(Sprite::createWithTexture(pSelectedTexture));
            pmuItem->setCallback(CC_CALLBACK_1(KuiNewPlayer::mSellistCallBack, this));
			pmuItem->setTag(2);
		}
		//ˮ
		nFrams=0;
		sprintf(nSprName,"\\Spr\\Ui3\\登入界面\\新建角色按钮\\登陆界面_7_水.spr");//"\\spr\\Ui3\\loginui\\saverole\\cancel_9_btn.spr");//"\\Spr\\Ui3\\�������\\ѡ�浵��ɫ��ť\\��½����_9_ȡ��.spr");
		g_StrLower(nSprName);
		Texture2D *shuiTexture = _getinidata.getinidata_new(nSprName,0,&m_nWidth,&m_nHeight,&nFrams);
		if (shuiTexture)
		{
			char nSprFilePath[64]={0};
			unsigned long nFielpahtdwid = g_FileName2Id(nSprName);
			sprintf(nSprFilePath,"%u-%d",nFielpahtdwid,0);
			Texture2D *pNormalTexture = ax::Director::getInstance()->getTextureCache()->getTextureForKey(nSprFilePath);
			sprintf(nSprFilePath,"%u-%d",nFielpahtdwid,1);
			Texture2D *pSelectedTexture = ax::Director::getInstance()->getTextureCache()->getTextureForKey(nSprFilePath);

            pshuiItem = MenuItemImage::create();
            pshuiItem->setNormalImage(Sprite::createWithTexture(pNormalTexture));
            pshuiItem->setSelectedImage(Sprite::createWithTexture(pSelectedTexture));
            pshuiItem->setDisabledImage(Sprite::createWithTexture(pSelectedTexture));
            pshuiItem->setCallback(CC_CALLBACK_1(KuiNewPlayer::mSellistCallBack, this));

			pshuiItem->setTag(3);
		}
		//��
		nFrams=0;
		sprintf(nSprName,"\\Spr\\Ui3\\登入界面\\新建角色按钮\\登陆界面_7_火.spr");//"\\spr\\Ui3\\loginui\\saverole\\cancel_9_btn.spr");//"\\Spr\\Ui3\\�������\\ѡ�浵��ɫ��ť\\��½����_9_ȡ��.spr");
		g_StrLower(nSprName);
		Texture2D *huoTexture = _getinidata.getinidata_new(nSprName,0,&m_nWidth,&m_nHeight,&nFrams);
		if (huoTexture)
		{
			char nSprFilePath[64]={0};
			unsigned long nFielpahtdwid = g_FileName2Id(nSprName);
			sprintf(nSprFilePath,"%u-%d",nFielpahtdwid,0);
			Texture2D *pNormalTexture = ax::Director::getInstance()->getTextureCache()->getTextureForKey(nSprFilePath);
			sprintf(nSprFilePath,"%u-%d",nFielpahtdwid,1);
			Texture2D *pSelectedTexture = ax::Director::getInstance()->getTextureCache()->getTextureForKey(nSprFilePath);

            phuoItem = MenuItemImage::create();
            phuoItem->setNormalImage(Sprite::createWithTexture(pNormalTexture));
            phuoItem->setSelectedImage(Sprite::createWithTexture(pSelectedTexture));
            phuoItem->setDisabledImage(Sprite::createWithTexture(pSelectedTexture));
            phuoItem->setCallback(CC_CALLBACK_1(KuiNewPlayer::mSellistCallBack, this));

			phuoItem->setTag(4);
		}
		//��
		nFrams=0;
		sprintf(nSprName,"\\Spr\\Ui3\\登入界面\\新建角色按钮\\登陆界面_7_土.spr");//"\\spr\\Ui3\\loginui\\saverole\\cancel_9_btn.spr");//"\\Spr\\Ui3\\�������\\ѡ�浵��ɫ��ť\\��½����_9_ȡ��.spr");
		g_StrLower(nSprName);
		Texture2D *tuTexture = _getinidata.getinidata_new(nSprName,0,&m_nWidth,&m_nHeight,&nFrams);
		if (tuTexture)
		{
			char nSprFilePath[64]={0};
			unsigned long nFielpahtdwid = g_FileName2Id(nSprName);
			sprintf(nSprFilePath,"%u-%d",nFielpahtdwid,0);
			Texture2D *pNormalTexture = ax::Director::getInstance()->getTextureCache()->getTextureForKey(nSprFilePath);
			sprintf(nSprFilePath,"%u-%d",nFielpahtdwid,1);
			Texture2D *pSelectedTexture = ax::Director::getInstance()->getTextureCache()->getTextureForKey(nSprFilePath);

            ptuItem = MenuItemImage::create();
            ptuItem->setNormalImage(Sprite::createWithTexture(pNormalTexture));
            ptuItem->setSelectedImage(Sprite::createWithTexture(pSelectedTexture));
            ptuItem->setDisabledImage(Sprite::createWithTexture(pSelectedTexture));
            ptuItem->setCallback(CC_CALLBACK_1(KuiNewPlayer::mSellistCallBack, this));

			ptuItem->setTag(5);
		}
		//ѡ������˵�����
		Sprite *nMenuBgSpr = NULL;
		nFrams=0;
		sprintf(nSprName,"\\Spr\\Ui3\\登入界面\\新建角色按钮\\登陆界面_7_选择人物.spr");//"\\spr\\Ui3\\loginui\\saverole\\cancel_9_btn.spr");//"\\Spr\\Ui3\\�������\\ѡ�浵��ɫ��ť\\��½����_9_ȡ��.spr");
		g_StrLower(nSprName);
		SPRFRAMSINFO nSprInfo;
		ZeroMemory(&nSprInfo,sizeof(nSprInfo));
		Texture2D *muBgTexture = _getinidata.getinidata_one(nSprName,0,&m_nWidth,&m_nHeight,&nFrams,&nSprInfo);
		Size m_selSize = SelbgSprite->getTextureRect().size;
		float offsetY=0;
		if (muBgTexture)
		{
			offsetY = m_selSize.height/2+muBgTexture->getContentSize().height/2+50;
			nMenuBgSpr = Sprite::createWithTexture(muBgTexture);
			nMenuBgSpr->setPosition(ax::Vec2(m_selSize.width/2,offsetY));
			SelbgSprite->addChild(nMenuBgSpr,1);
		}
		Menu *pselMenu = NULL;
		pselMenu = Menu::create(pjinItem,pmuItem,pshuiItem,phuoItem,ptuItem,NULL);
		//pMenu->alignItemsInRows(1,2,3,4,NULL);     //����һ��
		//             //�������� alignItemsHorizontally
		//pMenu->alignItemsHorizontally();             //�������� Ĭ�ϼ����kDefaultPadding=5??
		//pMenu->alignItemsHorizontallyWithPadding(7);//ָ������??
		//pMenu->alignItemsVertically();               //Ĭ�ϼ��Ҳ��kDefaultPadding=5?
		//pMenu->alignItemsInRows(3,2,1,NULL);
		pselMenu->alignItemsVertically();
		pselMenu->alignItemsVerticallyWithPadding(30); //ָ�����
		pselMenu->setPosition(ax::Vec2(m_selSize.width/2,offsetY-10));           //ax::Vec2::ZERO
		SelbgSprite->addChild(pselMenu,2);

		//SpriteFrameCache
	    //TextureCache ������
		/*

		void addSpriteFramesWithFile(const char *pszPlist)
		����һ��.plist�ļ���Ӷ������֡�� һ���������Զ����ء��������ƽ���.plist��׺���滻Ϊ.png����ɡ�
		void addSpriteFramesWithFile(const char* plist, const char* textureFileName)
		��ͨ��һ��.plist�ļ���Ӷ������֡�������뱻�����ľ���֡��ϡ�
		void addSpriteFramesWithFile(const char *pszPlist, Texture2D *pobTexture)
		��ͨ��һ��.plist�ļ���Ӷ������֡�������뱻�����ľ���֡��ϡ�
		void addSpriteFrame(SpriteFrame *pobFrame, const char *pszFrameName)
		��ͨ���������������һ������֡�� ��������Ѿ����ڣ���ôԭ�����Ƶ����ݽ����µ������
		*/
		/*
		float winw = visibleSize.width;   //��ȡ��Ļ���
		float winh = visibleSize.height;  //��ȡ��Ļ�߶�
		float spx = testSprite->getTextureRect().getMaxX();
		float spy = testSprite->getTextureRect().getMaxY();
		testSprite->setScaleX(winw/spx);  //���þ��������ű���
		testSprite->setScaleY(winh/spy);
		*/
	//this->draw();

		//if (m_nNumRole>0)
		{//�����ɫ������� �Ϳ�ʼ��������
			    static const char* pszAttribute[series_num] = {"金","木","水","火","土"};
			    static const char* pszGender[2] = { "男", "女" };
			//for (int i = 0; i < m_nNumRole; ++i)
			{
				char nInfoKey[64]={0};
				char szFileName[128]={0};
				GetRoleImageName(szFileName,m_szPlayerImgPrefix,false,m_Info.Attribute,0);//2 ��ɫ���� 0ѡ�� 1 Ϊһ�Թ��ɶ���
//				GetRoleImageName(szFileName,m_szPlayerImgPrefix,true,m_Info.Attribute,2);//2 ��ɫ���� 0ѡ�� 1 Ϊһ�Թ��ɶ���
				nFrams=0;
				g_StrLower(szFileName);
				//��
				//if (i == 0)
				{
					Texture2D *RoleCura = _getinidata.getinidata_new(szFileName,0,&m_nWidth,&m_nHeight,&nFrams);
					Sprite *RoleSpritea = NULL;
					if (RoleCura)
					{
						RoleSpritea=Sprite::createWithTexture(RoleCura);
						RoleSpritea->setPosition(ax::Vec2(visibleSize.width/2 + origin.x-210, visibleSize.height/2 + origin.y+50));
						m_sprCount++;
						this->addChild(RoleSpritea,2,1000);
					}
					Animation* nTempAni;
				    nTempAni  = creatSpranimation(0);
					if (RoleSpritea) {
                        //RoleSpritea->runAction(Sequence::create(Roleanimatea,callback,NULL));
                        auto callback = [this]() {
                            KuiNewPlayer::finish();
                        };

                        RoleSpritea->runAction(Sequence::create(Animate::create(nTempAni),callback,NULL));
                    }
				}
				//Ů
				//if (i == 1)
				{
					GetRoleImageName(szFileName, m_szPlayerImgPrefix,true,m_Info.Attribute,2);
					g_StrLower(szFileName);
					Texture2D *RoleCurb = _getinidata.getinidata_new(szFileName,0,&m_nWidth,&m_nHeight,&nFrams);
					Sprite *RoleSpriteb = NULL;
					if (RoleCurb)
					{
						RoleSpriteb=Sprite::createWithTexture(RoleCurb);
						RoleSpriteb->setPosition(ax::Vec2(visibleSize.width/2 + origin.x+210, visibleSize.height/2 + origin.y+50));
						m_sprCount++;
						this->addChild(RoleSpriteb,2,999);
					}
					m_Info.Gender = OBJ_G_FEMALE;
					Animation* nTempAni;
					nTempAni  = creatSpranimation(2);
					if (RoleSpriteb)
                    {
                        auto callback = [this]() {
                            KuiNewPlayer::finish();
                        };
                        RoleSpriteb->runAction(Sequence::create(Animate::create(nTempAni),callback,NULL));
                        //RoleSpriteb->runAction(Sequence::create(Roleanimateb,callback,NULL));
                    }

					m_Info.Gender = OBJ_G_MALE;
				}
			}
		}
		//����һ�������
		ax::ui::Scale9Sprite *pBg = ax::ui::Scale9Sprite::create();
		proleNameEditBox = EditBox::create(Size(112,24),pBg);
		const ax::Color3B nfcolor ={255,255,255};
		if (proleNameEditBox)
		{
			proleNameEditBox->setPlaceHolder("role name");
			proleNameEditBox->setFontColor(nfcolor);
			proleNameEditBox->setFontSize(24);
			proleNameEditBox->setMaxLength(32);
			proleNameEditBox->setPosition(ax::Vec2(180,40));
			proleNameEditBox->setInputMode(ax::ui::EditBox::InputMode::ANY);
			proleNameEditBox->setReturnType(ax::ui::EditBox::KeyboardReturnType::DONE);
            proleNameEditBox->setText("duongtammmmm");
			//pMoneyEditBox->setDelegate(this);
			//pMoneyEditBox->setDefaultTouchPriority(-128);
			SelbgSprite->addChild(proleNameEditBox,2,500);
		}
	//DrawNode* drawNode = ccDrawNode::create();
	return true;
}

int KuiNewPlayer::GetRoleCount(int& nAdviceChoice)
{
	nAdviceChoice = 0;
	if (m_ClientChoices.szProcessingRoleName[0])
	{
		for (int i = 0; i < m_nNumRole; ++i)
		{
			if (strcmp(m_ClientChoices.szProcessingRoleName, m_RoleList[i].Name) == 0)
			{
				nAdviceChoice = i;
				break;
			}
		}
	}
	return m_nNumRole;
}
//����˺�������ֵ������,�����������ַ����������Ƿ���
long KuiNewPlayer::_GetBig5Count(char *str)
{
	int lnBIG5 = 0;//���ͳ�ƿ����Ƿ����ֵĺ��ָ���
	int lnGB = 0;//���ͳ�ƿ����Ǽ����ֵĺ��ָ���
	int liTranLen = strlen(str);

	for(int liT = 0; liT<liTranLen-1; liT++)
	{
		//β�ֽ�40-7E��BGI5�����е�,���ɨ�赽���ֱ���˵������Ԫ���Ƿ���(������:������,�����Ǻ��ֵ�����һ��������Ӣ�ı�����϶��ɵ�)
		if((BYTE)(BYTE)str[liT]>=161 && (BYTE)(BYTE)str[liT]<=254 && (BYTE)(BYTE)str[liT+1]>=64 && (BYTE)(BYTE)str[liT+1]<=126)
		{
			//messageBox("1","TEST");
			lnBIG5++;
		}

		if((BYTE)(BYTE)str[liT]>=129 && (BYTE)(BYTE)str[liT]<=160 && (BYTE)(BYTE)str[liT+1]>=64 && (BYTE)(BYTE)str[liT+1]<=126)
		{
			//messageBox("2","TEST");
			lnBIG5++;
		}

		/*if((BYTE)(BYTE)str[liT]>=129 && (BYTE)(BYTE)str[liT]<=160 && (BYTE)(BYTE)str[liT+1]>=161 && (BYTE)(BYTE)str[liT+1]<=254)
		{
			messageBox("3","TEST");
			lnBIG5++;
		}*/

		//���ֽ�A4-A9��GB��Ϊ���ļ���,ϣ����ĸ,������ĸ���Ʊ��,�����ı��к��ٳ���,�������Χ��BIG5�ĳ��ú���,������Ϊ����BIG5��
		if((BYTE)(BYTE)str[liT]>=164 && (BYTE)(BYTE)str[liT]<=169 && (BYTE)(BYTE)str[liT+1]>=161 && (BYTE)(BYTE)str[liT+1]<=254)
		{
			//messageBox("4","TEST");
			lnBIG5++;
		}

		//GB�����ֽ�AA-AFû�ж���,�������ֽ�λ�AA-AF֮��,β�ֽ�λ�A1-FE�ı��뼸��100%��BIG5(������:û��100%),��Ϊ��BIG5��
		/*if((BYTE)(BYTE)str[liT]>=170 && (BYTE)(BYTE)str[liT]<=175 && (BYTE)(BYTE)str[liT+1]>=161 && (BYTE)(BYTE)str[liT+1]<=254)
			lnBIG5 ++;
		*/
		//���ֽ�C6-D7,β�ֽ�A1-FE��GB�����һ���ֿ�,�ǳ��ú���,����BIG5��,C6-C7û����ȷ����,��ͨ�����������ļ��������,C8-D7��춺��ú�����,���Կ���Ϊ��GB��
		//if((BYTE)(BYTE)str[liT]>=196 && (BYTE)(BYTE)str[liT]<=215 && (BYTE)(BYTE)str[liT+1]>=161 && (BYTE)(BYTE)str[liT+1]<=254)
		//	lnGB ++;
	}

	//���ɨ����������Ԫ��,�����Ǽ����ֵ���Ŀ�ȿ����Ƿ����ֵ���Ŀ�����Ϊ�Ǽ����ֲ�ת��(��һ��׼ȷ)
	//return lnBIG5-lnGB;
	return lnBIG5;
}

/*void KuiNewPlayer::isSimpleOrComplex(std::string str, std::string encode)
{

if (str.equals(new std::string(str.getBytes(encode), encode))) {
	//System.out.println(str + "---�Ǽ���");

}else{
	//System.out.println(str + "---�Ƿ���");
}
*/
//������Ϸ
void KuiNewPlayer::mainEnterCallback(Ref* pSender)
{//
	if (nAcclient/* && g_pClient*/)
	{
		//��ʼ������ɫ
		m_Info.Gender = m_CurSelIndex;
	//��ʼ��ȡ�༭������ݺ� ����ַ���
		if  (!proleNameEditBox->getText())
		{
#ifdef WIN32
			messageBox("��ɫ������Ϊ��!","����");
#else
			messageBox("the name is Null","Warning");
#endif
		     return;
		}
		//char msg[64];
		//sprintf(msg,"��ͼ:%d",m_Info.NativePlaceId);
		//messageBox(msg,"��ʾ:");

		char nRoleName[32];
		sprintf(nRoleName,"%s", proleNameEditBox->getText());

		int len=0;
		    len = strlen(nRoleName);
			if (len<12 || len>24)
			{
if (_clientlanguage==1)
{
#ifdef WIN32
	messageBox("��ɫ����С����12,��󳤶�24!","����");
#else
	messageBox(G2U("��ɫ����С����12,��󳤶�24!").c_str(),G2U("����").c_str());
#endif
}
else
{
#ifdef WIN32
	messageBox("��ɫ����С����12,��󳤶�24!","����");
#else
	messageBox("the Name length:less than 12 or more than 24","Warning");
#endif
}
				return;
			}

		//��ʼ����ֹ���ַ���
		//��ʼ����ַ��ĺϷ���
		std::string strcheck="check";
		strcheck+=nRoleName;
		int count = 0,spaceNum=0,chineseNum=0,other=0,letter=0,xiaoxie=0,biaodian=0;
		for(int i = 0; i <strcheck.length();i++)
		{
			if(isdigit(strcheck[i])) //�ж��Ƿ�������
				count++;
			else if(isspace(strcheck[i]))//�ж��Ƿ��ǿո�
				++spaceNum;
			else if(isalpha(strcheck[i])==1)//�ж��Ƿ�����ĸ	 1 Ϊ��д 2 Ϊ Сд  0 Ϊ������ĸ
			{
				++letter;
			}
			else if(isalpha(strcheck[i])==2)//�ж��Ƿ�����ĸ	 1 Ϊ��д 2 Ϊ Сд  0 Ϊ������ĸ
			{
				++xiaoxie;
			}
			//else if(ispunct(strcheck[i])) //�Ƿ��б�����
			//	++biaodian;
			else if(strcheck[i]&0x80)//if(strcheck[0]>=0xB0) 0x80
				++chineseNum;        //�ж��Ƿ�������(��������������)
			else
			{//���ź���ע�� ������������
				++other;
			}
		}
if (_clientlanguage==1)
{
		if (spaceNum>0 || count >0 || other>0 || letter>0)
		{ 	//system("pause");
#ifdef WIN32
			messageBox("���зǷ��ַ�,��������\n���ܺ��пո�,����,��д��ĸ���������!","����");
#else
			messageBox(G2U("���зǷ��ַ�,��������\n���ܺ��пո�,����,��д��ĸ���������!").c_str(),G2U("����").c_str());
#endif
			return;
		}
}
else
{
	if (spaceNum>0 || count >0 || other>0)
	{ 	//system("pause");
#ifdef WIN32
		messageBox("���зǷ��ַ�,��������\n���ܺ��пո�,����,��д��ĸ���������!","����");
#else
		messageBox("Invalid character,please check your name!\n Cannot contain special symbols such as NumBer, spaces, etc.!","Warning");
#endif
		return;
	}
}
		/*if  (_GetBig5Count(nRoleName))
		{
#ifdef WIN32
			messageBox("���зǷ��ַ�,��������\n���ܺ��пո�,����,��д��ĸ,������������!","����");
#else
			messageBox(G2U("���зǷ��ַ�,��������\n���ܺ��пո�,����,��д��ĸ,������������!").c_str(),G2U("����").c_str());
#endif
			return;
		}*/

		sprintf(m_Info.Name,"%s", U2G(nRoleName).c_str());//��uf8 ר�� ���ı���Ȼ����
		if (CreateRole())
		{
			if (pLoginItem)
			    pLoginItem->setEnabled(false);
		}
	}
}

int  KuiNewPlayer::CreateRole()
{
	if (!m_bIsClientConnecting || !nAcclient || m_ClientChoices.nServerRegionIndex<=0)
		return false;

	int nRet = false;
	m_Result = LL_R_NOTHING;

	if (m_Status == LL_S_ROLE_LIST_READY && m_nNumRole < MAX_PLAYER_PER_ACCOUNT &&
		m_Info.Attribute >= 0 && m_Info.Attribute < series_num)
	{
		int nNameLen = strlen(m_Info.Name);
		if (nNameLen >= 1 && nNameLen < sizeof(m_Info.Name))
		{
			char	Data[sizeof(TProcessData) + sizeof(NEW_PLAYER_COMMAND)];
			TProcessData*	pNetCommand = (TProcessData*)&Data;
			NEW_PLAYER_COMMAND* pInfo = (NEW_PLAYER_COMMAND*)pNetCommand->pDataBuffer;
			pInfo->m_btRoleNo = m_Info.Gender;    //��Ů
			pInfo->m_btSeries = m_Info.Attribute; //����
			pInfo->m_NativePlaceId = m_Info.NativePlaceId; //������ͼID

			memcpy(pInfo->m_szName, m_Info.Name, nNameLen);
			pInfo->m_szName[nNameLen] = '\0';
            //PathFileExists();
			pNetCommand->nProtoId = c2s_newplayer;     //����bishop
			pNetCommand->nDataLen = sizeof(NEW_PLAYER_COMMAND) - sizeof(pInfo->m_szName) + nNameLen + 1/* sizeof( '\0' ) */;
			pNetCommand->ulIdentity = 0;
			//�ڼ���������
			pNetCommand->uSelServer = m_ClientChoices.nServerRegionIndex;//KUiSelServer::GetCurServer();
			//�ʺŷ�����
			nAcclient->SendPackToServer(&Data, sizeof(TProcessData) - sizeof(pNetCommand->pDataBuffer) + pNetCommand->nDataLen);
            //��ֵ�����ش���Ľ�ɫ��
			memcpy(m_ClientChoices.szProcessingRoleName, m_Info.Name, nNameLen);
			m_ClientChoices.szProcessingRoleName[nNameLen] =0;

			m_Status = LL_S_CREATING_ROLE;
			m_Result = LL_R_NOTHING;
			nRet = true;
			//messageBox("�����ɹ�","�����ɹ�");
		}
	}
	return nRet;
}

//ѭ�������麯�� Ĭ���Զ�����
void KuiNewPlayer::update(float delta)
{
	unsigned int nSize = 0;
	const char* pBuffer = NULL;
	if (m_bIsClientConnecting)
	{ //�˺ŷ�����
		while (true)
		{
			if (!nAcclient)
				break;
			//����ѭ���ӷ�������ȡ����С��
			nSize = 0;
			pBuffer = NULL;

			pBuffer = (const char*)nAcclient->GetPackFromServer(nSize);
			if (!(pBuffer && nSize))
				break;
			PROTOCOL_MSG_TYPE*	pMsg = (PROTOCOL_MSG_TYPE*)pBuffer;
			PROTOCOL_MSG_TYPE	Msg  = *pMsg;  //Э��ͷ
			//CCAssert(Msg > s2c_multiserverbegin || Msg < s2c_end,"");
			if  (Msg == s2c_login || Msg == s2c_roleserver_getrolelist_result ||Msg == s2c_notifypl_ayerlogin_hfkqotivbhd || Msg == s2c_rolenewdelresponse)
			     AcceptNetMsg(pMsg);
		}
	}
	//----����������Ϸ������������----
	if (/*m_bIsGameSevConnecting && */g_pClient)
	{
		while (true)
		{
			if (!g_pClient)	//GS�����ݰ�
				break;
			//����ѭ���ӷ�������ȡ����С��
			nSize  = 0;
			pBuffer = NULL;
			pBuffer = (const char*)g_pClient->GetPackFromServer(nSize);

			if (!(pBuffer && nSize))
				break;
			PROTOCOL_MSG_TYPE *pMsg = (PROTOCOL_MSG_TYPE*)pBuffer; //��ǰ��

			while(pMsg < (PROTOCOL_MSG_TYPE*)(pBuffer + nSize))
			{//˧ѡ���ݰ�
				PROTOCOL_MSG_TYPE Msg = *pMsg;
				// ���������Э��
				if (Msg == s2c_notifyplayerexchange)
				{//ת����ͼ��ת��
					//ProcessSwitchGameSvrMsg(pMsg);
					break;
				}
				else if (g_pCoreShell)
				{
					if  (Msg <= s2c_clientbegin ||  Msg >= s2c_end || g_pCoreShell->GetProtocolSize(Msg)==0)
						break;

					g_pCoreShell->NetMsgCallbackFunc(pMsg);          //���ܴӷ�����������Э����Ϣ

					if (g_pCoreShell->GetProtocolSize(Msg) > 0)
						pMsg = (PROTOCOL_MSG_TYPE*)(((char*)pMsg) + g_pCoreShell->GetProtocolSize(Msg));
					else //û�����Ƴ��ȵ� -1
						pMsg = (PROTOCOL_MSG_TYPE*)(((char*)pMsg) + PROTOCOL_MSG_SIZE + (*(unsigned short*)(((char*)pMsg) + PROTOCOL_MSG_SIZE)));
				}
			}
		}
	}
}

Rect KuiNewPlayer::getRect(Node* pNode)
{
	Rect  rc;
	rc.origin     = pNode->getPosition();
	rc.size       = pNode->getContentSize();
	rc.origin.x -= 50;//rc.size.width;
	rc.origin.y -= 80;//rc.size.height;
	rc.size.width /=6;
	rc.size.height /=4;
	//Size s = pNode->getContentSize();
	//return ax::Rect(-s.width / 2, -s.height / 2, s.width, s.height);
	return rc;
}

void KuiNewPlayer::draw(Renderer* renderer, const Mat4& transform, unsigned int flags)
{
	if (m_Result != LL_R_NOTHING)
	{
		if (nAcclient)
		{
			m_bIsPlayerInGame =false;
			m_bIsClientConnecting=false;
			nAcclient->Cleanup();
		}

		if (g_pClient)
		{;
			m_bIsPlayerInGame =false;
			m_bIsGameSevConnecting=false;
			g_pClient->Cleanup();
			delete g_pClient;
			g_pClient=NULL;
		}
		isthisVer = false;
		ax::Director::getInstance()->replaceScene(Klogin_f::scene());
		return;
	}

	if (m_Status == LL_S_ENTERING_GAME)
	{//������ת����
		ax::Director::getInstance()->replaceScene(KTranSence::scene());
		//ax::Director::getInstance()->replaceScene(KgameWorld::scene());
		return;
	}
}
//
bool KuiNewPlayer::isFileExist(const char* pFileName)
{
	if( !pFileName ) return false;
	//strFilePathName is :/data/data/ + package name
	std::string filePath = ax::FileUtils::getInstance()->getWritablePath();//getWriteablePath();

	filePath += pFileName;

	FILE *fp = fopen(filePath.c_str(),"r");
	if(fp)
	{
		fclose(fp);
		return true;
	}
	return false;
}

void KuiNewPlayer::copyData(const char* pFileName)
{
    std::string strPath = ax::FileUtils::getInstance()->fullPathForFilename(pFileName);
    unsigned int len = 0;
    unsigned char * data = NULL;

    std::ifstream file(strPath, std::ios::binary | std::ios::ate);
    if (file.is_open()) {
        // L?y k��ch th??c c?a t?p
        std::streampos size = file.tellg();
        data = new unsigned char[size];
        file.seekg(0, std::ios::beg);
        file.read(reinterpret_cast<char*>(data), size);
        file.close();
    }

    if (data) {
        len = ax::FileUtils::getInstance()->getFileSize(strPath);
        std::string destPath = ax::FileUtils::getInstance()->getWritablePath();
        destPath += pFileName;
        FILE *fp = fopen(destPath.c_str(),"w+");

        if (fp) {
            fwrite(data,sizeof(unsigned char),len,fp);
            fclose(fp);
        }

        delete [] data;
        data = NULL;
    }
}

/*
void KuiNewPlayer::onEnter()
{

}

void KuiNewPlayer::onExit()
{

}
//����� �� �������
void KuiNewPlayer::onEnterTransitionDidFinish()
{

}*/

bool KuiNewPlayer::ccTouchBegan(Touch *pTouch, Event *pEvent)
{
	return true;
}
void KuiNewPlayer::ccTouchMoved(Touch *pTouch, Event *pEvent)
{

}

void KuiNewPlayer::ccTouchCancelled(Touch *pTouch, Event *pEvent)
{

}
// default implements are used to call script callback if exist
void KuiNewPlayer::ccTouchesBegan(const std::vector<ax::Touch*>&pTouches, Event *pEvent)
{
}

void KuiNewPlayer::CheckClick(float tt)
{
}

void KuiNewPlayer::ccTouchesMoved(const std::vector<ax::Touch*>&pTouches, Event *pEvent)
{

}

void KuiNewPlayer::ccTouchesEnded(const std::vector<ax::Touch*>&pTouches, Event *pEvent)
{
	auto it = pTouches.begin();
	Touch* touch = (Touch*)(*it);
	//containsPoint(convertTouchToNodeSpaceAR(touch))
	Point location = touch->getLocation();//Ŀ�ĵ�����?��Ļ�����
	RoleNodeman = (Sprite *)this->getChildByTag(1000);
	RoleNodewoman = (Sprite *)this->getChildByTag(999);

	char nRoleInfo[128]={0};
	if (RoleNodeman)
	{
		if (getRect(RoleNodeman).containsPoint(location))
		{
			m_CurSelIndex = 0;
			if (m_CurSelIndex == 0)
			{//ԭ���͵���0 ��ɫ��
				m_Info.Attribute;
				m_Info.Gender = OBJ_G_MALE;
				Animation* mananimation = creatSpranimation(1);//���ɶ���
				actionManager->removeAllActionsFromTarget(RoleNodeman);
//				RoleNodea->runAction(RepeatForever::create(Animate::create(ax::AnimationCache::getInstance()->getAnimation("0_1"))));
                auto callback = [this]() {
                    KuiNewPlayer::finish();
                };

                RoleNodeman->runAction(Sequence::create(Animate::create(mananimation),callback,NULL));
				if (RoleNodewoman)
				{
					m_Info.Gender = OBJ_G_FEMALE;
					Animation*  womenanimation = creatSpranimation(2);//��ɫ
					actionManager->removeAllActionsFromTarget(RoleNodewoman);
					RoleNodewoman->runAction(Sequence::create(Animate::create(womenanimation),NULL,NULL));
				}
			}
			//EnterGame();
		}
	}

	if (RoleNodewoman)
	{
		if (getRect(RoleNodewoman).containsPoint(location))
		{
			m_CurSelIndex = 1;
			if (m_CurSelIndex == 1)
			{//ԭ���͵���0 ��ɫ��
				m_Info.Attribute;
				m_Info.Gender = OBJ_G_FEMALE;
				Animation* womanimation = creatSpranimation(1);//���ɶ���
				actionManager->removeAllActionsFromTarget(RoleNodewoman);
				//RoleNodea->runAction(RepeatForever::create(Animate::create(ax::AnimationCache::getInstance()->getAnimation("0_1"))));
                auto callback = [this]() {
                    KuiNewPlayer::finish();
                };
				RoleNodewoman->runAction(Sequence::create(Animate::create(womanimation),callback,NULL));
				if (RoleNodeman)
				{
					m_Info.Gender = OBJ_G_MALE;
					Animation* manimation = creatSpranimation(2);//��ɫ
					actionManager->removeAllActionsFromTarget(RoleNodeman);
					RoleNodeman->runAction(Sequence::create(Animate::create(manimation),NULL,NULL));
				}
			}

			//EnterGame();
		}
	}
}

void KuiNewPlayer::finish()
{
	if (!nisgetinfo)
		mExit(NULL);
	/*if  (m_CurSelIndex==0)
	{
		m_Info.Attribute;
		m_Info.Gender = OBJ_G_MALE;
		actionManager->removeAllActionsFromTarget(RoleNodeman);
		RoleNodeman->runAction(Sequence::create(Animate::create(ax::AnimationCache::getInstance()->getAnimation("0_0")),NULL,NULL));
	}

	if  (m_CurSelIndex==1)
	{
		actionManager->removeAllActionsFromTarget(RoleNodewoman);
		RoleNodewoman->runAction(Sequence::create(Animate::create(ax::AnimationCache::getInstance()->getAnimation("1_0")),NULL,NULL));
	}*/

}

void KuiNewPlayer::ccTouchesCancelled(const std::vector<ax::Touch*>&pTouches, Event *pEvent)
{

}
void KuiNewPlayer::didAccelerate(Acceleration* pAccelerationValue)
{

}
void KuiNewPlayer::ccTouchEnded(Touch *pTouch, Event *pEvent)
{//convertTouchToNodeSpace
   /*Point touchLocation = convertTouchToNodeSpace(pTouch);
   if(Rect::CCRectContainsPoint(getRect(pSprite), touchLocation))
	{
	   printf("�ұ�������!\n");
	}*/
}


void KuiNewPlayer::mSellistCallBack(Ref* pSender){
	if (!pSender) return;
	MenuItemImage* nCurSelItem = (MenuItemImage*)pSender;
	int nSelSeries = nCurSelItem->getTag();
	m_Info.Attribute = nSelSeries-1;
	Animation * mananimation=NULL;
	RoleNodeman   = (Sprite *)this->getChildByTag(1000);//
	RoleNodewoman = (Sprite *)this->getChildByTag(999);
	if (m_CurSelIndex==0) //�о���
	{
		m_Info.Gender    = OBJ_G_MALE;
		mananimation = creatSpranimation(0);
		if  (mananimation)
		{
			actionManager->removeAllActionsFromTarget(RoleNodeman);
            auto callback = [this]() {
                KuiNewPlayer::finish();
            };
//			RoleNodeman->runAction(Sequence::create(Animate::create(mananimation),callback,NULL));
		}
		m_Info.Gender    = OBJ_G_FEMALE;
		mananimation = creatSpranimation(2);
		if  (mananimation)
		{
			actionManager->removeAllActionsFromTarget(RoleNodewoman);
            auto callback = [this]() {
                KuiNewPlayer::finish();
            };
//			RoleNodewoman->runAction(Sequence::create(Animate::create(mananimation),callback,NULL));
		}
	}
	else
	{//==1
		//Ů
		Animation * womananimation=NULL;
		m_Info.Gender  = OBJ_G_FEMALE;
		womananimation = creatSpranimation(0);
		if  (womananimation)
		{
			actionManager->removeAllActionsFromTarget(RoleNodewoman);
            auto callback = [this]() {
                KuiNewPlayer::finish();
            };
			RoleNodewoman->runAction(Sequence::create(Animate::create(womananimation),callback,NULL));
		}

		m_Info.Gender    = OBJ_G_MALE;
		womananimation   = creatSpranimation(2);
		if  (womananimation)
		{
			actionManager->removeAllActionsFromTarget(RoleNodeman);
            auto callback = [this]() {
                KuiNewPlayer::finish();
            };
			RoleNodeman->runAction(Sequence::create(Animate::create(womananimation),callback,NULL));
		}
	}
	//char msg[64];
	//sprintf(msg,"ѡ��:%d",nSelSeries);
	//messageBox(msg,"ѡ��");
}

void KuiNewPlayer::mExit(Ref* pSender){
	nPlaceId=-1;
	nNativePlaceId =-1;
	isthisVer = false;
	if (nAcclient)
	{//�Ͽ�����
		m_bIsClientConnecting=false;
		m_bIsPlayerInGame  = false;
		nAcclient->Cleanup();
	}

	if (g_pClient)
	{
		m_bIsPlayerInGame  = false;
		m_bIsGameSevConnecting=false;
		g_pClient->Cleanup();
		delete g_pClient;
		g_pClient=NULL;
	}
	//�˵� ��ʼ��½����
	ax::Director::getInstance()->replaceScene(Klogin_f::scene());
}

void KuiNewPlayer::GetRoleImageName(char* pszName, const char* pszPrefix, unsigned char bGender, unsigned char bAttribute, int nIndex)
{
	//CCAssert(pszName && bAttribute >= series_metal && bAttribute < series_num,"");
	if  (!pszName || bAttribute < series_metal || bAttribute >= series_num)
		return;

	static const char* pszAttribute[series_num] = {"金","木","水","火","土"};
	static const char* pszGender[2] = { "男", "女" };
	sprintf(pszName,"%s_%s_%s_%d.spr", pszPrefix, pszAttribute[bAttribute],
		(bGender ? pszGender[1] : pszGender[0]), nIndex);  //spr��ʽ ���� ����spr  ��ɫ_ϵ_�Ա�_�Ա����
}

void KuiNewPlayer::AcceptNetMsg(void* pMsgData) //ѭ�����ܴ�����Ϣ
{
	if (pMsgData == NULL)  //�Ϊ�� �򷵻�
		return;
	//messageBox("�ص��ɹ�","GetPackFromServer");
	switch(m_Status)       //�������ݰ���ʶ����
	{
	case LL_S_ACCOUNT_CONFIRMING:  //�˺���֤
		//ProcessAccountLoginResponse((KLoginStructHead*) (((char*)pMsgData) + PROTOCOL_MSG_SIZE));
		break;
	case LL_S_WAIT_ROLE_LIST:      //�ȴ���ɫ�嵥
		//ProcessRoleListResponse((TProcessData*)pMsgData);   //����Э���ɫ��Ϣ
		break;
	case LL_S_CREATING_ROLE:       //������ɫ
		ProcessCreateRoleResponse((tagNewDelRoleResponse*)pMsgData); //������ɫ
		break;
	case LL_S_DELETING_ROLE:       //ɾ����ɫ
		//ProcessDeleteRoleResponse((tagNewDelRoleResponse*)pMsgData);
		break;
	case LL_S_WAIT_TO_DEL:         //�Ѿ�ȡ���ȴ� ɾ����ɫ
		//ProcessGetRoleResponse((tagNotifyPlayerLogin*)pMsgData);
		break;
	case LL_S_WAIT_TO_LOGIN_GAMESERVER:  //�ȴ����ӵ�������
		ProcessToLoginGameServResponse((tagNotifyPlayerLogin*)pMsgData); //�������ӵ��������ĺ���
		break;
	}
}

//--------------------------------------------------------------------------
//	���ܣ��½���ɫ�б����Ӧ  ��BISHOP ���صĽ��
//	״̬�л���LL_S_CREATING_ROLE -> LL_S_ROLE_LIST_READY
//--------------------------------------------------------------------------
void KuiNewPlayer::ProcessCreateRoleResponse(tagNewDelRoleResponse* pResponse)
{
	//_ASSERT(m_Status == LL_S_DELETING_ROLE && pResponse != NULL);
	if (pResponse->cProtocol == s2c_rolenewdelresponse)
	{
		//	if (strcmp(pResponse->szRoleName, m_Choices.szProcessingRoleName) == 0)	//to be check/* �������԰� */
		{//����ͱ��صĽ�ɫ����ͬ ��ֱ���˳�
			if (pResponse->bSucceeded)
			{//�ȴ�������Ϸ
				//g_NetConnectAgent.UpdateClientRequestTime(false);
				m_ClientChoices.bIsRoleNewCreated = true;
				m_Status = LL_S_WAIT_TO_LOGIN_GAMESERVER;
				m_Result = LL_R_NOTHING;
			}
			else
			{//����ʧ�� ���ؽ�ɫ�б� ����ֱ���˳���
				//g_NetConnectAgent.UpdateClientRequestTime(true);	//ʱ�����
				if (pLoginItem)
					pLoginItem->setEnabled(true);

				m_Status = LL_S_ROLE_LIST_READY;
				m_Result = LL_R_INVALID_ROLENAME;
#ifdef WIN32
				messageBox("������ɫʧ��","��ʾ:");
#else
				messageBox("Kh?ng t?o ???c nh?n v?t",UTEXT("��ʾ:",1).c_str());
#endif
			}
		}
	}
}

//--------------------------------------------------------------------------
//	���ܣ�׼����½��Ϸ����������Ӧ---�����߳̽Ӱ�-������ �հ�
//	״̬�л���LL_S_WAIT_TO_LOGIN_GAMESERVER -> LL_S_ENTERING_GAME
//--------------------------------------------------------------------------
void KuiNewPlayer::ProcessToLoginGameServResponse(tagNotifyPlayerLogin* pResponse)
{

	if (pResponse->cProtocol == s2c_notifypl_ayerlogin_hfkqotivbhd)  ///bishop ��������Э��ͷ
	{
		char pzAc[32]={0};
		for (int k = 0; k < 32;k++)
		{
			pzAc[k] = ~m_ClientChoices.Account[k];
		}

		if (!pResponse->bPermit &&(pResponse->nIPAddr==0 || pResponse->nPort==0))
		{
			m_Result = LL_R_OPNE_ACCOUNT; //���� ���ڳ��Դ��˺�
#ifdef WIN32
			messageBox("��ǰ����������Ԥ������ɫ״̬,���ܵ�½","��ʾ:");
#else
			messageBox(UTEXT("��ǰ����������Ԥ������ɫ״̬,���ܵ�½ kull 1",1).c_str(),UTEXT("��ʾ:",1).c_str());
#endif
			return;
		}

		if (!pResponse->bPermit)
		{
			//ReturnToIdle();
			m_Result = LL_R_OPNE_ACCOUNT; //���� ���ڳ��Դ��˺�
#ifdef WIN32
			messageBox("���ڳ��Դ��˺�","��ʾ:");
#else
			messageBox(UTEXT("���ڳ��Դ��˺� kull 2",1).c_str(),UTEXT("��ʾ:",1).c_str());
#endif
			return;
		}

		if (!isthisVer)
		{
			m_Result = LL_R_INVALID_PROTOCOLVERSION; //���� ���ڳ��Դ��˺�
#ifdef WIN32
			messageBox("�汾����","��ʾ:");
#else
			messageBox(UTEXT("�汾���� kull 3",1).c_str(),UTEXT("��ʾ:",1).c_str());
#endif
			return;
		}

		if (strcmp((const char*)pResponse->szRoleName, m_ClientChoices.szProcessingRoleName) == 0
			&& strcmp((const char*)pResponse->szAccountName, pzAc) == 0)
		{
			//g_NetConnectAgent.UpdateClientRequestTime(true); //ʱ�����
			if (NULL==g_pClient)
			{
				const unsigned int bufferSize           = 1024;   //Scoket���������Ĵ�С ������ڴ�(m_bufferSize > 0) ? m_bufferSize : (1024*64);
				const unsigned int bufferSize_Cache     = 1024*512; //������ڴ� ���� �Ӱ��Ļ����С
				const unsigned int maxFreeBuffers	      = 2;        //Scoket����������
				const unsigned int maxFreeBuffers_Cache = 2;        //���� �Ӱ��Ļ��� ����������
				g_pClient     = new CGameClient(maxFreeBuffers,maxFreeBuffers_Cache,bufferSize_Cache,bufferSize,1); //2,2   8
			}

		    if (g_pClient)
			{//(const unsigned char*)&pResponse->nIPAddr
				const unsigned char* pIpAddress = (const unsigned char*)&pResponse->nIPAddr;
				char Address[128]={0}; //IP��ַ
				sprintf(Address, "%d.%d.%d.%d", pIpAddress[0], pIpAddress[1],pIpAddress[2], pIpAddress[3]);
			    if (g_pClient->FsGameServerConnectTo(Address,pResponse->nPort))
				{
					g_pClient->Startup();
					m_bIsGameSevConnecting = true;	             //�������Ѿ�����

					tagLogicLogin ll;
					ll.cProtocol = c2s__loginfs_kfjghtueodnchsf; //֪ͨGS��������ʼ��½	  ��ֵ������źŸ���ɫ

					GUID* pGuid = &pResponse->guid;//��������Ψһ�ı�ʾ
					memcpy(&ll.guid,pGuid,sizeof(GUID));

					if (!g_pClient->SendPackToServer(&ll,sizeof(tagLogicLogin)))
					{
						m_Result = LL_R_CONNECT_SERV_BUSY; //���� ���ڳ��Դ��˺�
#ifdef WIN32
						messageBox("���ӷ�����ʧ��","��ʾ:");
#else
                        messageBox(UTEXT("���ӷ�����ʧ�� kull 4",1).c_str(),UTEXT("��ʾ:",1).c_str());
#endif
						return;
					}
					//��ʼ�Ͽ����ص�����
					if (nAcclient)
					{
						m_bIsClientConnecting =false;
						nAcclient->Cleanup(true); //ֻɱ���̣߳����Ͽ�����
						nAcclient->Shutdown();
						//nAcclient->Release();
						//delete nAcclient;
						//nAcclient = NULL;
					}

					/*if (nAcclient)
					{
						nAcclient->Cleanup();
						nAcclient->Release();
						nAcclient = NULL;
					}*/
					m_Status = LL_S_ENTERING_GAME;//���ڽ�����Ϸ��
					m_Result = LL_R_NOTHING;  //���ӳɹ���
				}
				else
				{//����ʧ��
					m_Result = LL_R_CONNECT_FAILED;
#ifdef WIN32
					messageBox("���ӷ�����ʧ��","��ʾ:");
#else
                    messageBox(UTEXT("���ӷ�����ʧ�� kull 5",1).c_str(),UTEXT("��ʾ:",1).c_str());
#endif
				}
			}
			// �Ͽ������ص�����
			//g_NetConnectAgent.DisconnectClient();
		}
		else
		{
			//ReturnToIdle();
			m_Result = LL_R_SERVER_SHUTDOWN; //��ʾ��������Ա��������
#ifdef WIN32
			messageBox("���ӷ�����ʧ��","��ʾ:");
#else
            messageBox(UTEXT("���ӷ�����ʧ�� kull 6",1).c_str(),UTEXT("��ʾ:",1).c_str());
#endif
		}
	}
}

Animation* KuiNewPlayer::creatSpranimation(int nindex,int isLoop,bool isforever)
{
	char szFileName[128];
	int nFrams=0;
	char nInfoKey[32];
	sprintf(nInfoKey,"%d_%d_%d",m_Info.Attribute,m_Info.Gender,nindex);//����,��Ů���Ǹ�����
	Animation*animation=NULL;
	animation = ax::AnimationCache::getInstance()->getAnimation(nInfoKey);
	if (animation)
	{
		//messageBox("����","����");
		return animation;
	}

	GetRoleImageName(szFileName, m_szPlayerImgPrefix,m_Info.Gender,m_Info.Attribute,nindex);
	g_StrLower(szFileName);
	_getinidata.getinidata_new(szFileName,0,&m_nWidth,&m_nHeight,&nFrams);
	char nySprFilePath[64]={0};
    animation = Animation::create();//����һ������
	unsigned long nFielpahtdwid = g_FileName2Id(szFileName);
	int j;
	for (j=0;j<nFrams;j++)
	{
		sprintf(nySprFilePath,"%u-%d",nFielpahtdwid,j);
        auto texture = ax::Director::getInstance()->getTextureCache()->getTextureForKey(nySprFilePath);
        Rect rect = Rect::ZERO;
        rect.size = texture->getContentSize();
        animation->addSpriteFrameWithTexture(texture, rect);
//		animation->addSpriteFrameWithFile(nySprFilePath);//�����������������֡
	}
	//���ö���֡��ʱ����
	animation->setDelayPerUnit(0.10f);
	//���ò���ѭ�� һֱ���� Ϊ-1
	animation->setLoops(isLoop);
	//���ö���������ָ�����һ֡ setRestoreOriginalFrame
	animation->setRestoreOriginalFrame(isforever);
	// ����һ����Զѭ���Ķ���
	//Roleanimatea = Animate::create(animation);
	//���붯������
	ax::AnimationCache::getInstance()->addAnimation(animation,nInfoKey); //���뻺��
	return animation;
}
