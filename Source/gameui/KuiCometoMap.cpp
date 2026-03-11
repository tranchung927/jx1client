//�а�ť�ġ������롡�Ի���
//
#include "engine/KPlatform.h"
#include "gamecore/KCore.h"
#include "engine/CXmlStream.h"
#include "KuiCometoMap.h"
#include "engine/Text.h"
#include "engine/KIniFile.h"
#include "gamecore/CoreShell.h"
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
extern iCoreShell*	g_pCoreShell;
#include <list>
USING_NS_AX;
#define _CHESS_COUNT 100
#define _ROW_MAP_COUNT 7
KuiCometoMap::KuiCometoMap()
{
	isOpen = false;
	colorLayer        = NULL;
	red               = NULL;
	m_StartPos.x=0;
	m_StartPos.y=0;
    m_MaxCount = 0;
	m_MapCount = 0;

	for(int i=0;i<_MAPMAX_COUNT;i++)
	m_recommend[i] = NULL;

	m_pMainUiLayer = NULL;
	btnPageBox_1= NULL;
	btnPageBox_2= NULL;
	btnPageBox_3= NULL;
	btnPageBox_4= NULL;
	btnPageBox_5= NULL;
	m_TimeInfo  = NULL;
	m_AwardButton_yl= NULL;;//����/����
	m_AwardButton_fs= NULL;;//����/��Ե
	m_AwardButton_jb= NULL;;//���
	m_AwardButton_jf= NULL;;//����

	m_timeProcessSprite= NULL;;

	_nCurTime =0; //(��λ������)
	_nMoneyExpTime =0; //��������� 0
	_nFuYuanTime =0;   //��Ե������ 1
	_nJiFenTime =0;    //����       3
	_nJinBiTime =0;    //���       2
	_curPage =AUTOBTN_PAGE_BASE;
	nShowLayer = NULL;
}

KuiCometoMap::~KuiCometoMap()
{
	isOpen = false;
	colorLayer        = NULL;
}

//����ȷ��������
KuiCometoMap * KuiCometoMap::create(Ref * callbackListener,const std::function<void(ax::Node*)>& callfun)
{
	KuiCometoMap * popLayer = KuiCometoMap::create();
	popLayer->addDialogData();
	//���ð�ť
	popLayer->setcoloseButton(callbackListener,callfun);
	popLayer->isOpen = true;
	return popLayer;
}


//��ʼ��
bool KuiCometoMap::init()
{
	if ( !Layer::init() ){
		return false;
	}
	//����������Ӧ
	this->scheduleUpdate();                   //���ú���
    auto listener = ax::EventListenerTouchOneByOne::create();
    ax::Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, this);
	//����֡����
	//SpriteFrameCache * sfCache = SpriteFrameCache::sharedSpriteFrameCache();
	//sfCache->addSpriteFramesWithFile("p_tips.plist");
	//���ģ̬����
	Size winSize = ax::Director::getInstance()->getWinSize();
	/*Color4B color(112, 128, 144, 150);
	LayerColor * colorLayer = LayerColor::create(color);//��ɫ��
	colorLayer->setPosition(ax::Vec2::ZERO);
	//colorLayer->setContentSize(winSize);//���úʹ��ڵĴ�С
	this->addChild(colorLayer);
	*/
	char nSprName[128]={0};
	ZeroMemory(nSprName,sizeof(nSprName));
	sprintf(nSprName,"\\spr\\Ui3\\activityguide\\activityguideui.spr");
	g_StrLower(nSprName);
	int m_nWidth,m_nHeight,nFrams;
	Texture2D *bgCur = NULL;
	SPRFRAMSINFO nSprInfo;
	ZeroMemory(&nSprInfo,sizeof(nSprInfo));
	bgCur = _getinidata.getinidata_one(nSprName,0,&m_nWidth,&m_nHeight,&nFrams,&nSprInfo);
	if (bgCur==NULL)
		return false;
	Sprite * spriteBox = Sprite::createWithTexture(bgCur);
	ParentNode_ItemEx = spriteBox;
	spriteBox->setPosition(ax::Vec2(winSize.width/2,winSize.height/2));
	this->addChild(spriteBox);

	//��ñ���ͼƬ�Ĵ�С
	Size contentSize = spriteBox->getContentSize();
	m_size = contentSize;
	m_origin = spriteBox->getPosition();

	//colorLayer->setContentSize(contentSize);//���úʹ��ڵĴ�С
	return true;
}


void KuiCometoMap::addDialogData()
{
	if  (!g_pCoreShell) return;
	Size nSize;
	nSize.width =128;
	nSize.height=24;
	m_StartPos.x =10;//+ITEM_CELL_SIZE/2;
	m_StartPos.y =140-24;//+ITEM_CELL_SIZE/2;


	m_pMainUiLayer = Layer::create();
	ParentNode_ItemEx->addChild(m_pMainUiLayer);


	char nSprName[128]={0};
	int m_nWidth,m_nHeight,nFrams;
	Texture2D *bgCur = NULL;
	SPRFRAMSINFO nSprInfo;
	ZeroMemory(&nSprInfo,sizeof(nSprInfo));

	ZeroMemory(nSprName,sizeof(nSprName));
	sprintf(nSprName,"\\spr\\Ui3\\activityguide\\pagebutton.spr");
	g_StrLower(nSprName);

	bgCur = _getinidata.getinidata_one(nSprName,0,&m_nWidth,&m_nHeight,&nFrams,&nSprInfo);
	if (bgCur==NULL)
		return ;
	Texture2D * check_btn_normal = bgCur;
	bgCur = _getinidata.getinidata_one(nSprName,1,&m_nWidth,&m_nHeight,&nFrams,&nSprInfo);
	if (bgCur==NULL)
		return ;
	Texture2D * check_btn_active = bgCur;

	float nTempFontSize =14;
	float nOffsetY = 45;

	btnPageBox_1 = Button::create();
	btnPageBox_1->setTouchEnabled(true);
//	btnPageBox_1->loadTexturesByData(check_btn_normal,check_btn_active,check_btn_normal);
	btnPageBox_1->loadTextures(nSprName, nSprName, nSprName);
	btnPageBox_1->setTitleFontSize(nTempFontSize);
	btnPageBox_1->setTitleText(UTEXT("��������",1).c_str());
	//btnPageBox_1->setAnchorPoint(ax::Vec2(0,0));
	btnPageBox_1->setPosition(ax::Vec2(70,m_size.height-nOffsetY));
	btnPageBox_1->setTag(AUTOBTN_PAGE_BASE);
	btnPageBox_1->setTag(1);
	//btnPageBox_1->ignoreContentAdaptWithSize(true);
	btnPageBox_1->addTouchEventListener(CC_CALLBACK_2(KuiCometoMap::touchPageBtnEvent, this));
	m_pMainUiLayer->addChild(btnPageBox_1);

	btnPageBox_2 = Button::create();
	btnPageBox_2->setTouchEnabled(true);
	btnPageBox_2->loadTextures(nSprName, nSprName, nSprName);
	btnPageBox_2->setTitleFontSize(nTempFontSize);
	btnPageBox_2->setTitleText(UTEXT("�Ƽ�����",1).c_str());
	//btnPageBox_1->setAnchorPoint(ax::Vec2(0,0));
	btnPageBox_2->setPosition(ax::Vec2(175,m_size.height-nOffsetY));
	btnPageBox_2->setTag(AUTOBTN_PAGE_ART);
	btnPageBox_2->setTag(2);
	//btnPageBox_1->ignoreContentAdaptWithSize(true);
	btnPageBox_2->addTouchEventListener(CC_CALLBACK_2(KuiCometoMap::touchPageBtnEvent, this));
	m_pMainUiLayer->addChild(btnPageBox_2);

	btnPageBox_3 = Button::create();
	btnPageBox_3->setTouchEnabled(true);
//	btnPageBox_3->loadTexturesByData(check_btn_normal,check_btn_active,check_btn_normal);
	btnPageBox_3->loadTextures(nSprName, nSprName, nSprName);
	btnPageBox_3->setTitleFontSize(nTempFontSize);
	btnPageBox_3->setTitleText(UTEXT("�ƺŽ���",1).c_str());
	//btnPageBox_1->setAnchorPoint(ax::Vec2(0,0));
	btnPageBox_3->setPosition(ax::Vec2(280,m_size.height-nOffsetY));
	btnPageBox_3->setTag(AUTOBTN_PAGE_ITEM_1);
	btnPageBox_3->setTag(3);
	//btnPageBox_1->ignoreContentAdaptWithSize(true);
	btnPageBox_3->addTouchEventListener(CC_CALLBACK_2(KuiCometoMap::touchPageBtnEvent, this));
	m_pMainUiLayer->addChild(btnPageBox_3);

	btnPageBox_4 = Button::create();
	btnPageBox_4->setTouchEnabled(true);
//	btnPageBox_4->loadTexturesByData(check_btn_normal,check_btn_active,check_btn_normal);
	btnPageBox_4->loadTextures(nSprName, nSprName, nSprName);
	btnPageBox_4->setTitleFontSize(nTempFontSize);
	btnPageBox_4->setTitleText(UTEXT("��������",1).c_str());
	//btnPageBox_1->setAnchorPoint(ax::Vec2(0,0));
	btnPageBox_4->setPosition(ax::Vec2(385,m_size.height-nOffsetY));
	btnPageBox_4->setTag(AUTOBTN_PAGE_ITEM_2);
	btnPageBox_4->setTag(4);
	//btnPageBox_1->ignoreContentAdaptWithSize(true);
	btnPageBox_4->addTouchEventListener(CC_CALLBACK_2(KuiCometoMap::touchPageBtnEvent, this));
	m_pMainUiLayer->addChild(btnPageBox_4);

	btnPageBox_5 = Button::create();
	btnPageBox_5->setTouchEnabled(true);
//	btnPageBox_5->loadTexturesByData(check_btn_normal,check_btn_active,check_btn_normal);
	btnPageBox_5->loadTextures(nSprName, nSprName, nSprName);
	btnPageBox_5->setTitleFontSize(nTempFontSize);
	btnPageBox_5->setTitleText(UTEXT("��������",1).c_str());
	//btnPageBox_1->setAnchorPoint(ax::Vec2(0,0));
	btnPageBox_5->setPosition(ax::Vec2(490,m_size.height-nOffsetY));
	btnPageBox_5->setTag(AUTOBTN_PAGE_OTHER);
	btnPageBox_5->setTag(5);
	//btnPageBox_1->ignoreContentAdaptWithSize(true);
	btnPageBox_5->addTouchEventListener(CC_CALLBACK_2(KuiCometoMap::touchPageBtnEvent, this));
	m_pMainUiLayer->addChild(btnPageBox_5);
	//char nTempStr[128];
	//t_sprintf(nTempStr,pContent);
	/*Label *pstrLabel = Label::createWithTTF(UTEXT(nTempStr,1).c_str(),UI_GAME_FONT_DEFAULT,14,nSize,TextHAlignment::CENTER);//Arial
	pstrLabel->setColor(ax::Color3B::YELLOW);
	pstrLabel->setPosition(ax::Vec2(128/2+30,m_size.height-50));
	ParentNode_ItemEx->addChild(pstrLabel);

	Label *ptextLabel = Label::createWithTTF(UTEXT("�����Ļ�س�",1).c_str(),UI_GAME_FONT_DEFAULT,14,nSize,TextHAlignment::CENTER);//Arial
	ptextLabel->setColor(ax::Color3B::YELLOW);
	ptextLabel->setPosition(ax::Vec2(m_size.width/2,20));
	ParentNode_ItemEx->addChild(ptextLabel);
	*/
	//ͼ��
	ZeroMemory(nSprName,sizeof(nSprName));
	sprintf(nSprName,"\\spr\\Ui3\\activityguide\\gameicon.spr");
	g_StrLower(nSprName);
	bgCur = _getinidata.getinidata_one(nSprName,0,&m_nWidth,&m_nHeight,&nFrams,&nSprInfo);
	Sprite *iconSprite=NULL;
	if (bgCur)
	{
		iconSprite = Sprite::createWithTexture(bgCur);
		iconSprite->setAnchorPoint(ax::Vec2(0,0));
		iconSprite->setPosition(ax::Vec2(30,m_size.height-90));
		ParentNode_ItemEx->addChild(iconSprite,1);
	}
	//�����������
	Color4B color(112, 128, 144,150);
	Size nPageSize    = ax::Size(m_size.width-35,m_size.height-63);//���Ĵ�С
	Point CCPointPage(0,0);//��ʼλ��
	int nisOpen=0,nsaveMonye=0;
	_PageLayer_1 = Layer::create();//Layer::create();//��������
	//_PageLayer_1->setContentSize(nPageSize);
	_PageLayer_1->setPosition(CCPointPage);
	//_PageLayer_1->setVisible(false);
	ParentNode_ItemEx->addChild(_PageLayer_1);

	ZeroMemory(nSprName,sizeof(nSprName));
	//�������汳����� \spr\Ui3\activityguide\calendarpage.spr
	sprintf(nSprName,"\\spr\\Ui3\\activityguide\\calendarpage.spr");
	g_StrLower(nSprName);
	bgCur = _getinidata.getinidata_one(nSprName,0,&m_nWidth,&m_nHeight,&nFrams,&nSprInfo);
	if (bgCur==NULL)
		return ;
	Sprite *pageSprite_a = Sprite::createWithTexture(bgCur);
	pageSprite_a->setAnchorPoint(ax::Vec2(0,0));
	pageSprite_a->setPosition(ax::Vec2(19,23));
	_PageLayer_1->addChild(pageSprite_a,-1);

	/*KIniFile	Ini;
	if (Ini.Load("\\Ui\\Ui3\\uiactivityguide\\activitycalendar.ini"))
	{

	}*/
	m_TimeInfo = Label::createWithTTF(UTEXT("����ʱ��:0Сʱ0����",1).c_str(),UI_GAME_FONT_DEFAULT,14);//Arial
	m_TimeInfo->setColor(ax::Color3B::YELLOW);
	m_TimeInfo->setAnchorPoint(ax::Vec2(0,0));
	m_TimeInfo->setPosition(ax::Vec2(90,m_size.height-80));
	_PageLayer_1->addChild(m_TimeInfo,1);


	ZeroMemory(nSprName,sizeof(nSprName));
	sprintf(nSprName,"\\spr\\Ui3\\activityguide\\progressbarback.spr");
	g_StrLower(nSprName);
	bgCur = _getinidata.getinidata_one(nSprName,0,&m_nWidth,&m_nHeight,&nFrams,&nSprInfo);
	if (bgCur)
	{
		ImageView* ProcessBg=ImageView::create(nSprName);
//		ProcessBg->loadTextureByData(bgCur);
		ProcessBg->setAnchorPoint(ax::Vec2(0,0));
		ProcessBg->setPosition(ax::Vec2(270,m_size.height-75));
		_PageLayer_1->addChild(ProcessBg,1);
	}

	ZeroMemory(nSprName,sizeof(nSprName));
	sprintf(nSprName,"\\spr\\Ui3\\activityguide\\progressbar.spr");
	g_StrLower(nSprName);
	bgCur = _getinidata.getinidata_one(nSprName,0,&m_nWidth,&m_nHeight,&nFrams,&nSprInfo);

	if (bgCur)
	{
		m_timeProcessSprite=Sprite::createWithTexture(bgCur);
		m_timeProcessSprite->setAnchorPoint(ax::Vec2(0,0));
		m_timeProcessSprite->setPosition(ax::Vec2(270,m_size.height-75));
		_PageLayer_1->addChild(m_timeProcessSprite,2);
	}
	//\\spr\\npcres\\passerby\\passerby239\\passerby239.spr
	//������\spr\skill\����\������ը.spr
	 nShowLayer = KuiShowSprite::create("\\spr\\npcres\\passerby\\passerby239\\passerby239.spr",19,80,this,CC_CALLBACK_1(KuiCometoMap::closeShowPad, this));
	 nShowLayer->setRegisterWithTouchDispatcher(128);
	 _PageLayer_1->addChild(nShowLayer,1);


	KIniFile	Ini;
	if (Ini.Load("\\Ui\\Ui3\\uiactivityguide\\activitycalendar.ini"))
	{
	  char nTempInfo[128];
	  ZeroMemory(nTempInfo,sizeof(nTempInfo));
	  Ini.GetString("DescText","decsText","��ȡע��:\n\n  ����ʱ�䲻�ۼ�,�������߹���!\n���п���������ľ�ϲ��",nTempInfo,sizeof(nTempInfo));
	  Label * m_DescInfo = Label::createWithTTF(UTEXT("��ȡע��\n  ����ʱ�䲻�ۼ�,�������߹���!\n���п���������ľ�ϲ��",1).c_str(),UI_GAME_FONT_DEFAULT,14);//Arial
	  m_DescInfo->setColor(ax::Color3B::YELLOW);
	  m_DescInfo->setAnchorPoint(ax::Vec2(0,0));
	  m_DescInfo->setPosition(ax::Vec2(430,m_size.height-180));
	  _PageLayer_1->addChild(m_DescInfo,1);

	  Size temp_size =ax::Size(150,16);

	  m_AwardButton_yl = Button::create();
	  m_AwardButton_yl->setTouchEnabled(true);
	  m_AwardButton_yl->loadTextures("ui/btn_sel.png","ui/statelan.png","ui/btn_sel.png");
	  m_AwardButton_yl->setTitleFontSize(nTempFontSize);
	  m_AwardButton_yl->setScale9Enabled(true);

	  Ini.GetString("AwardKind0","Text","ÿ���ӻ�ȡ����:0",nTempInfo,sizeof(nTempInfo));

	  m_AwardButton_yl->setContentSize(temp_size);
	  m_AwardButton_yl->setTitleText(UTEXT(nTempInfo,1).c_str());
	  m_AwardButton_yl->setTitleColor(ax::Color3B::YELLOW);
	//btnPageBox_1->setAnchorPoint(ax::Vec2(0,0));
	  m_AwardButton_yl->setPosition(ax::Vec2(520,m_size.height-285));
	  m_AwardButton_yl->setTag(1);
	//btnPageBox_1->ignoreContentAdaptWithSize(true);
	  m_AwardButton_yl->addTouchEventListener(CC_CALLBACK_2(KuiCometoMap::touchAwardBtnEvent, this));
	  _PageLayer_1->addChild(m_AwardButton_yl);

	  m_AwardButton_fs = Button::create();
	  m_AwardButton_fs->setTouchEnabled(true);
	  m_AwardButton_fs->loadTextures("ui/btn_sel.png","ui/statelan.png","ui/btn_sel.png");
	  m_AwardButton_fs->setTitleFontSize(nTempFontSize);
	  m_AwardButton_fs->setScale9Enabled(true);
	  Ini.GetString("AwardKind1","Text","ÿ���ӻ�ȡ����:0",nTempInfo,sizeof(nTempInfo));
	  m_AwardButton_fs->setContentSize(temp_size);
	  m_AwardButton_fs->setTitleText(UTEXT(nTempInfo,1).c_str());
	  m_AwardButton_fs->setTitleColor(ax::Color3B::YELLOW);
	  //btnPageBox_1->setAnchorPoint(ax::Vec2(0,0));
	  m_AwardButton_fs->setPosition(ax::Vec2(520,m_size.height-310));
	  m_AwardButton_fs->setTag(2);
	  //btnPageBox_1->ignoreContentAdaptWithSize(true);
	  m_AwardButton_fs->addTouchEventListener(CC_CALLBACK_2(KuiCometoMap::touchAwardBtnEvent, this));
	  _PageLayer_1->addChild(m_AwardButton_fs);

	  m_AwardButton_jb = Button::create();
	  m_AwardButton_jb->setTouchEnabled(true);
	  m_AwardButton_jb->loadTextures("ui/btn_sel.png","ui/statelan.png","ui/btn_sel.png");
	  m_AwardButton_jb->setTitleFontSize(nTempFontSize);
	  m_AwardButton_jb->setScale9Enabled(true);
	  Ini.GetString("AwardKind2","Text","ÿ���ӻ�ȡ����:0",nTempInfo,sizeof(nTempInfo));
	  m_AwardButton_jb->setContentSize(temp_size);
	  m_AwardButton_jb->setTitleText(UTEXT(nTempInfo,1).c_str());
	  m_AwardButton_jb->setTitleColor(ax::Color3B::YELLOW);
	  //btnPageBox_1->setAnchorPoint(ax::Vec2(0,0));
	  m_AwardButton_jb->setPosition(ax::Vec2(520,m_size.height-340));
	  m_AwardButton_jb->setTag(3);
	  //btnPageBox_1->ignoreContentAdaptWithSize(true);
	  m_AwardButton_jb->addTouchEventListener(CC_CALLBACK_2(KuiCometoMap::touchAwardBtnEvent, this));
	  _PageLayer_1->addChild(m_AwardButton_jb);

	  m_AwardButton_jf = Button::create();
	  m_AwardButton_jf->setTouchEnabled(true);
	  m_AwardButton_jf->loadTextures("ui/btn_sel.png","ui/statelan.png","ui/btn_sel.png");
	  m_AwardButton_jf->setTitleFontSize(nTempFontSize);
	  m_AwardButton_jf->setScale9Enabled(true);
	  Ini.GetString("AwardKind3","Text","ÿ���ӻ�ȡ����:0",nTempInfo,sizeof(nTempInfo));
	  m_AwardButton_jf->setContentSize(temp_size);
	  m_AwardButton_jf->setTitleText(UTEXT(nTempInfo,1).c_str());
	  m_AwardButton_jf->setTitleColor(ax::Color3B::YELLOW);
	  //btnPageBox_1->setAnchorPoint(ax::Vec2(0,0));
	  m_AwardButton_jf->setPosition(ax::Vec2(520,m_size.height-365));
	  m_AwardButton_jf->setTag(4);
	  //btnPageBox_1->ignoreContentAdaptWithSize(true);
	  m_AwardButton_jf->addTouchEventListener(CC_CALLBACK_2(KuiCometoMap::touchAwardBtnEvent, this));
	  _PageLayer_1->addChild(m_AwardButton_jf);

	}
	Ini.Clear();
	//�Ƽ��������
	Sprite *pageSprite;
	_PageLayer_2 = Layer::create();//��������
	_PageLayer_2->setPosition(CCPointPage);
	ParentNode_ItemEx->addChild(_PageLayer_2);
	//�Ƽ����� �����һ���ҳ����
	pageSprite = Sprite::create("ui/calendarpage.png");
	pageSprite->setAnchorPoint(ax::Vec2(0,0));
	pageSprite->setPosition(ax::Vec2(19,23));
	_PageLayer_2->addChild(pageSprite,-1);

	//��ʼ�������
	KIniFile mapData;
	char nTmepStr[256];
	ZeroMemory(nTmepStr,sizeof(nTmepStr));
	if (_clientlanguage!=1)
		sprintf(nTmepStr,"\\Ui\\Ui3\\uiactivityguide\\recommendmap_vn.ini");
	else
		sprintf(nTmepStr,"\\Ui\\Ui3\\uiactivityguide\\recommendmap.ini");

	if (mapData.Load(nTmepStr))
	{
		mapData.GetInteger("ValueConfig","lvlCount",_ROW_MAP_COUNT+1,&m_MaxCount);
		mapData.GetInteger("ValueConfig","AllMapCount",_ROW_MAP_COUNT+1,&m_MapCount);
		char nKeyName[32];
		ZeroMemory(nKeyName,sizeof(nKeyName[32]));
		int i,j;
		int inMapIdx[_ROW_MAP_COUNT];
		ZeroMemory(inMapIdx,sizeof(inMapIdx));
		//for (i = 0; i < _ROW_MAP_COUNT; ++i)
		//inMapIdx[i]=0;

		float offsetX=19,offsetY=pageSprite->getContentSize().height+pageSprite->getPosition().y-iconSprite->getContentSize().height-20;
		float startPosY = offsetY-23;

		Menu* maplistMenu = Menu::create(NULL);
		maplistMenu->setAnchorPoint(ax::Vec2(0,0));
		maplistMenu->setPosition(ax::Vec2(19,23));
		_PageLayer_2->addChild(maplistMenu,1);

		for (i = 0; i < m_MaxCount; ++i)
		{//8���ȼ��ֶ�
			sprintf(nKeyName,"recommend_%d",i+1);
			char nstrName[32];
			mapData.GetString(nKeyName,"Text","",nstrName,sizeof(nstrName)); //ax::TextHAlignment::LEFT
			m_recommend[i] = Label::createWithTTF(UTEXT(nstrName,1).c_str(),UI_GAME_FONT_DEFAULT,14);//Arial
			m_recommend[i]->setAnchorPoint(ax::Vec2(0,0));
			m_recommend[i]->setPosition(ax::Vec2(offsetX,offsetY));
			offsetY -=30;
			_PageLayer_2->addChild(m_recommend[i]);
			ZeroMemory(inMapIdx,sizeof(inMapIdx));
			mapData.GetInt8(nKeyName,"maps",inMapIdx); //ÿ�����ж��ٸ���ͼ

			float nLeft,nWidht;
			nLeft  = m_recommend[i]->getPosition().x;
			//nTop   = m_recommend[i]->getPosition().y;
			nWidht = m_recommend[i]->getContentSize().width;
			//nHight = m_recommend[i]->getContentSize().height;
		    float backBtnW = nLeft+nWidht + 20; //������һ����ť�ĳ���
			for (j = 0; j < _ROW_MAP_COUNT; ++j)
			{//ÿ���ȼ������¡��ж��ٸ���ͼ
				if (inMapIdx[j]>0)
				{
					char nNameMame[32]={0};
					ZeroMemory(nNameMame,sizeof(nNameMame));
					int _MapId=0;
					sprintf(nKeyName,"map_%d",inMapIdx[j]);
					mapData.GetString(nKeyName,"MapName","����",nNameMame,sizeof(nNameMame));
					mapData.GetInteger(nKeyName,"MapId",0,&_MapId);
					int nFonSize = 12;
					Size tempSize;
					tempSize.width  = 80;
					tempSize.height = 26;
					Label* mapNameLabel = Label::createWithTTF(UTEXT(nNameMame,1).c_str(),UI_GAME_FONT_DEFAULT,nFonSize,tempSize,ax::TextHAlignment::LEFT);
					mapNameLabel->setColor(Color3B(0,255,0));
					mapNameLabel->setAnchorPoint(ax::Vec2(0,0));
					//mapNameLabel->setHorizontalAlignment(ax::TextHAlignment::LEFT);        //����λ��
					//mapNameLabel->setVerticalAlignment(kCCVerticalTextAlignmentCenter);//���¾���
					MenuItemLabel* mserLabel1=MenuItemLabel::create(mapNameLabel,CC_CALLBACK_1(KuiCometoMap::movemapCallBack, this));
					mserLabel1->setTag(_MapId);
					sprintf(nKeyName,"%d",inMapIdx[j]+1);
                    int mapKey = std::hash<std::string>{}(nKeyName);
					mserLabel1->setTag(mapKey);
					mserLabel1->setPosition(ax::Vec2(backBtnW+2,startPosY));
					backBtnW += mapNameLabel->getContentSize().width;
					maplistMenu->addChild(mserLabel1,_MapId);
				}
			}
			startPosY-=30; //�����е�ƫ��
		}
		mapData.Clear();
	}
	//��������
	_PageLayer_3 = Layer::create();
	_PageLayer_3->setPosition(CCPointPage);
	ParentNode_ItemEx->addChild(_PageLayer_3);
	//�Ƽ����� �����һ���ҳ����
	pageSprite = Sprite::create("ui/calendarpage.png");
	pageSprite->setAnchorPoint(ax::Vec2(0,0));
	pageSprite->setPosition(ax::Vec2(19,23));
	_PageLayer_3->addChild(pageSprite,-1);

	//��������
	_PageLayer_4 = Layer::create();
	_PageLayer_4->setPosition(CCPointPage);
	ParentNode_ItemEx->addChild(_PageLayer_4);
	//�Ƽ����� �����һ���ҳ����
	pageSprite = Sprite::create("ui/calendarpage.png");
	pageSprite->setAnchorPoint(ax::Vec2(0,0));
	pageSprite->setPosition(ax::Vec2(19,23));
	_PageLayer_4->addChild(pageSprite,-1);

	//��������
	_PageLayer_5 = Layer::create();
	_PageLayer_5->setPosition(CCPointPage);
	ParentNode_ItemEx->addChild(_PageLayer_5);
	//�Ƽ����� �����һ���ҳ����
	pageSprite = Sprite::create("ui/calendarpage.png");
	pageSprite->setAnchorPoint(ax::Vec2(0,0));
	pageSprite->setPosition(ax::Vec2(19,23));
	_PageLayer_5->addChild(pageSprite,-1);


	_PageLayer_1->setVisible(true);
	_PageLayer_2->setVisible(false);
	_PageLayer_3->setVisible(false);
	_PageLayer_4->setVisible(false);
	_PageLayer_5->setVisible(false);
	setBtnTouchEnble(1,true);
	setBtnTouchEnble(2,false);
	setBtnTouchEnble(3,false);
	setBtnTouchEnble(4,false);
	setBtnTouchEnble(5,false);
}

void KuiCometoMap::closeShowPad(Node *pNode)
{

}
void KuiCometoMap::touchAwardBtnEvent(Ref *pSender, ax::ui::AbstractCheckButton::TouchEventType type)
{
	if  (!g_pCoreShell || !g_GameWorld) return;
	KUiPlayerSetTimeInfo	Info;
	memset(&Info, 0, sizeof(KUiPlayerSetTimeInfo));
	switch (type)
	{
	case Widget::TouchEventType::BEGAN://Touch Down
		{
			if (pSender)
			{
				Button* nTbtn = (Button*)pSender;
				nTbtn->setTitleColor(ax::Color3B::WHITE);
				int  nTag = nTbtn->getTag();
				switch(nTag)
				{
				case 1:
					Info.nYinLiangTime = _nCurTime;
					break;
				case 2:
					 Info.nExpTime = _nCurTime;
					break;
				case 3:
					Info.nJinBiTime = _nCurTime;
					break;
				case 4:
					Info.nExpPiontTime = _nCurTime;
					break;
				default:
					break;
				}
				g_pCoreShell->GetGameData(GDI_PLAYER_SETTIME_INFO,(intptr_t)&Info,nTag-1);
				//nTbtn->setTouchEnabled(false);
			}
		}
		break;
	case Widget::TouchEventType::MOVED://Touch Move
		break;
	case Widget::TouchEventType::ENDED://Touch Up
		{
			if (pSender)
			{
				Button* nTbtn = (Button*)pSender;
				nTbtn->setTitleColor(ax::Color3B::YELLOW);
			}
		}
		break;
	case Widget::TouchEventType::CANCELED:
		break;
	default:
		break;
	}
}

void KuiCometoMap::touchPageBtnEvent(Ref *pSender, ax::ui::AbstractCheckButton::TouchEventType type)
{
	if  (!g_pCoreShell || !g_GameWorld) return;

	switch (type)
	{
	case Widget::TouchEventType::BEGAN://Touch Down
		{
			if (pSender)
			{
				Button* nTbtn = (Button*)pSender;
				nTbtn->setTitleColor(ax::Color3B::YELLOW);
				switch(nTbtn->getTag())
				{
				case AUTOBTN_PAGE_BASE:
					{//��������
						_curPage = AUTOBTN_PAGE_BASE;
						_PageLayer_1->setVisible(true);
						_PageLayer_2->setVisible(false);
						_PageLayer_3->setVisible(false);
						_PageLayer_4->setVisible(false);
						_PageLayer_5->setVisible(false);
						setBtnTouchEnble(1,true);
						setBtnTouchEnble(2,false);
						setBtnTouchEnble(3,false);
						setBtnTouchEnble(4,false);
						setBtnTouchEnble(5,false);

					}
					break;
				case AUTOBTN_PAGE_ART:
					{//�Ƽ�����
						_curPage = AUTOBTN_PAGE_ART;
						_PageLayer_1->setVisible(false);
						_PageLayer_2->setVisible(true);
						_PageLayer_3->setVisible(false);
						_PageLayer_4->setVisible(false);
						_PageLayer_5->setVisible(false);
						setBtnTouchEnble(1,false);
						setBtnTouchEnble(2,true);
						setBtnTouchEnble(3,false);
						setBtnTouchEnble(4,false);
						setBtnTouchEnble(5,false);

					}
					break;
				case AUTOBTN_PAGE_ITEM_1:
					{//��Ʒ1����
						_curPage = AUTOBTN_PAGE_ITEM_1;
						_PageLayer_1->setVisible(false);
						_PageLayer_2->setVisible(false);
						_PageLayer_3->setVisible(true);
						_PageLayer_4->setVisible(false);
						_PageLayer_5->setVisible(false);
						setBtnTouchEnble(1,false);
						setBtnTouchEnble(2,false);
						setBtnTouchEnble(3,true);
						setBtnTouchEnble(4,false);
						setBtnTouchEnble(5,false);
					}
					break;
				case AUTOBTN_PAGE_ITEM_2:
					{//��Ʒ2����
						_curPage = AUTOBTN_PAGE_ITEM_2;
						_PageLayer_1->setVisible(false);
						_PageLayer_2->setVisible(false);
						_PageLayer_3->setVisible(false);
						_PageLayer_4->setVisible(true);
						_PageLayer_5->setVisible(false);
						setBtnTouchEnble(1,false);
						setBtnTouchEnble(2,false);
						setBtnTouchEnble(3,false);
						setBtnTouchEnble(4,true);
						setBtnTouchEnble(5,false);
					}
					break;
				case AUTOBTN_PAGE_OTHER:
					{//��������
						_curPage = AUTOBTN_PAGE_OTHER;
						_PageLayer_1->setVisible(false);
						_PageLayer_2->setVisible(false);
						_PageLayer_3->setVisible(false);
						_PageLayer_4->setVisible(false);
						_PageLayer_5->setVisible(true);

						setBtnTouchEnble(1,false);
						setBtnTouchEnble(2,false);
						setBtnTouchEnble(3,false);
						setBtnTouchEnble(4,false);
						setBtnTouchEnble(5,true);

					}
					break;
				default:
					break;
				}
			}
		}
		//m_pDisplayValueLabel->setText(CCString::createWithFormat("Touch Down")->getCString());
		break;
	case Widget::TouchEventType::MOVED://Touch Move
		break;

	case Widget::TouchEventType::ENDED://Touch Up
		{
			if (pSender)
			{
				Button* nTbtn = (Button*)pSender;
				nTbtn->setTitleColor(ax::Color3B::WHITE);
				if  (nTbtn->getTag()==AUTOBTN_PAGE_BASE)
				{
					//buttonCallBackFunc(NULL);
					//messageBox(nTbtn->getTag().c_str(),"���°�ť");
				}
			}
		}
		break;
	case Widget::TouchEventType::CANCELED:
		break;
	default:
		break;
	}
}

void KuiCometoMap::setBtnTouchEnble(int index,bool enble)
{
	switch(index)
	{
	case 1:
		{
			if (m_AwardButton_yl)
			    m_AwardButton_yl->setTouchEnabled(enble);//����/����
			if (m_AwardButton_fs)
			    m_AwardButton_fs->setTouchEnabled(enble);//����/��Ե
			if (m_AwardButton_jb)
			    m_AwardButton_jb->setTouchEnabled(enble);//���
			if (m_AwardButton_jf)
			    m_AwardButton_jf->setTouchEnabled(enble);//����
		}
		break;
	case 3:
		{
			if (m_AwardButton_yl)
				m_AwardButton_yl->setTouchEnabled(enble);//����/����
			if (m_AwardButton_fs)
				m_AwardButton_fs->setTouchEnabled(enble);//����/��Ե
			if (m_AwardButton_jb)
				m_AwardButton_jb->setTouchEnabled(enble);//���
			if (m_AwardButton_jf)
				m_AwardButton_jf->setTouchEnabled(enble);//����
		}
		break;
	case 2:
		{
			if (m_AwardButton_yl)
				m_AwardButton_yl->setTouchEnabled(enble);//����/����
			if (m_AwardButton_fs)
				m_AwardButton_fs->setTouchEnabled(enble);//����/��Ե
			if (m_AwardButton_jb)
				m_AwardButton_jb->setTouchEnabled(enble);//���
			if (m_AwardButton_jf)
				m_AwardButton_jf->setTouchEnabled(enble);//����
		}
		break;
	case 4:
		{
			if (m_AwardButton_yl)
				m_AwardButton_yl->setTouchEnabled(enble);//����/����
			if (m_AwardButton_fs)
				m_AwardButton_fs->setTouchEnabled(enble);//����/��Ե
			if (m_AwardButton_jb)
				m_AwardButton_jb->setTouchEnabled(enble);//���
			if (m_AwardButton_jf)
				m_AwardButton_jf->setTouchEnabled(enble);//����
		}
		break;
	case 5:
		{
			if (m_AwardButton_yl)
				m_AwardButton_yl->setTouchEnabled(enble);//����/����
			if (m_AwardButton_fs)
				m_AwardButton_fs->setTouchEnabled(enble);//����/��Ե
			if (m_AwardButton_jb)
				m_AwardButton_jb->setTouchEnabled(enble);//���
			if (m_AwardButton_jf)
				m_AwardButton_jf->setTouchEnabled(enble);//����
		}
		break;
	default:
		break;
	}
}

void KuiCometoMap::movemapCallBack(Ref* menuItem)
{
	if (menuItem)
	{
		MenuItemLabel *nCerBox  = (MenuItemLabel * )menuItem;
		if (nCerBox)
		{
            //char msg[64];
			//sprintf(msg,"map:%d,index:%s",nCerBox->getTag(),nCerBox->getTag().c_str());
			//messageBox(msg,"1111");
			//����ʼִ�пͻ��˴��ͽű�
			if (g_pCoreShell)
			{
				char nTempstr[32];
				sprintf(nTempstr, "%s",  reinterpret_cast<const char *>(nCerBox->getTag()));
				int nIndex =g_Atoui(nTempstr);
			    g_pCoreShell->OperationRequest(GOI_PLAYER_SYSSHOP,0,nIndex-1,3);
			}
		}
	}
}
// -------------------------------------------------------------------------
// ����	: ��������
// -------------------------------------------------------------------------
void KuiCometoMap::UpdateData()
{
	//KUiSkillData m_Skills[SKILLTREE_MAX_SKILL_COUNT]
	if (!g_GameWorld || !g_pCoreShell) return;
}

//����ȷ��ȡ����ť,���������ò���󣬵��ò�ص�����
void KuiCometoMap::setcoloseButton(Ref * callbackListener,const std::function<void(ax::Node*)>& callfun)
{
	m_callbackListener = callbackListener;
	m_callfun = callfun;

	//���һ���رհ�ť��\spr\Ui3\activityguide\closebutton.spr
	char nSprName[128]={0};
	ZeroMemory(nSprName,sizeof(nSprName));
	sprintf(nSprName,"\\spr\\Ui3\\activityguide\\closebutton.spr");
	g_StrLower(nSprName);
	int m_nWidth,m_nHeight,nFrams;
	Texture2D *bgCur = NULL;
	SPRFRAMSINFO nSprInfo;
	ZeroMemory(&nSprInfo,sizeof(nSprInfo));
	bgCur = _getinidata.getinidata_one(nSprName,0,&m_nWidth,&m_nHeight,&nFrams,&nSprInfo);
	if (bgCur==NULL)
		return;
	Sprite * sprite_close_normal =  Sprite::createWithTexture(bgCur);
	bgCur = _getinidata.getinidata_one(nSprName,1,&m_nWidth,&m_nHeight,&nFrams,&nSprInfo);
	if (bgCur==NULL)
		return;
	Sprite * sprite_close_select =  Sprite::createWithTexture(bgCur);
	MenuItemSprite * closeConfirm = MenuItemSprite::create(sprite_close_normal, sprite_close_select, CC_CALLBACK_1(KuiCometoMap::buttonCallBackFunc, this));
	closeConfirm->setPosition(ax::Vec2(0,0));

	Menu * menu = Menu::create(closeConfirm,NULL);
	menu->setPosition(ax::Vec2(m_size.width/2,13));//�˵�λ������Ϊ����������
	ParentNode_ItemEx->addChild(menu);         //�ӵ�������
}
//ִ���ϲ����Ļص��������رյ�����
void KuiCometoMap::buttonCallBackFunc(Ref * pSender)
{
	Node * node = NULL;
	if (pSender)
		node = dynamic_cast<Node *>(pSender);
	//node->setTag(3000);//����tag���ڵ��ò���Ի�ȡ��
    if (nShowLayer)
		nShowLayer->removeShowSprite();

	if (m_callfun &&  m_callbackListener)
		(m_callfun)(node);

	this->removeFromParentAndCleanup(true);
}

//�رյ�����
void KuiCometoMap::closePopLayer(Ref * pSender)
{
	if (nShowLayer)
		nShowLayer->removeShowSprite();

	this->removeFromParentAndCleanup(true);
}

//��д����ע�ắ�������¸�����������
void KuiCometoMap::registerWithTouchDispatcher()
{
	//����Ĵ������ȼ�����Ϊ��128����CCMenuͬ������֤�������·��Ĵ���
    auto touchListener = ax::EventListenerTouchOneByOne::create();
    touchListener->setSwallowTouches(true);
    ax::Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(touchListener, this);
//	//ax::Director::getInstance()->getTouchDispatcher()->addTargetedDelegate(this,128,true);
}

Rect KuiCometoMap::getRect(Node* pNode,int i)
{
	Rect  rc;
	rc.origin     = pNode->getPosition(); //��������ͼ��ƫ��
	//visibleSize
	rc.size       = pNode->getContentSize();

	rc.origin.x   = m_origin.x - m_size.width/2 + rc.origin.x; //��������
	rc.origin.y   = m_origin.y - m_size.height/2+ rc.origin.y; //���ϱ�    //- rc.origin.y

	return rc;
}
//��������ccTouchBegan������true
bool KuiCometoMap::ccTouchBegan(Touch *pTouch, Event *pEvent)
{
	//CCSetIterator it = pTouch->begin();
	//Touch* touch = (Touch*)(*it);
	//containsPoint(convertTouchToNodeSpaceAR(touch))
	Point location = pTouch->getLocation();//Ŀ�ĵ�����?��Ļ�����
	//case SMCT_UI_RENASCENCE:	//ѡ������
	//	if (g_pCoreShell)
	//	   g_pCoreShell->OperationRequest(GOI_PLAYER_RENASCENCE,0,true);//nSelAction=0��ԭ������, nSelAction=1���س�
    //buttonCallBackFunc(NULL);
	return true;
}

void KuiCometoMap::update(float delta)
{
	//messageBox("����ѭ��","update");
  if (isOpen && g_pCoreShell)
  {
	  switch(_curPage)
	  {
	  case AUTOBTN_PAGE_BASE:{
		KUiPlayerActivityInfo	Info;
		memset(&Info, 0, sizeof(KUiPlayerActivityInfo));
		ZeroMemory(&Info,sizeof(Info));
		g_pCoreShell->GetGameData(GDI_PLAYER_TIME_INFO, (intptr_t)&Info, 0);

		if (m_TimeInfo)
		   m_TimeInfo->setString(UTEXT(Info.nTimeInfo,1).c_str());

		float ver = (60.00*Info.nHour+Info.nMin)/1400;
		if (m_timeProcessSprite)
		{
			if (ver>=0 && ver<=1)
			   m_timeProcessSprite->setScaleX(ver);
			else
			   m_timeProcessSprite->setScaleX(1);
		}
		//UIMessageBox2(Info.nTimeInfo);
		/*if (Info.nHour*60+Info.nMin<=1440)
			n_ProcessFg.Set2IntValue(Info.nHour*60+Info.nMin,1440);   //������	  24Сʱ
		else
			n_ProcessFg.Set2IntValue(1440,1440);*/
		_nCurTime      = Info.nHour*60+Info.nMin; //(��λ������)
		_nMoneyExpTime = Info.nYinLiangTime; //��������� 0
		_nFuYuanTime   = Info.nExpTime;   //��Ե������ 1
		_nJiFenTime    = Info.nExpPiontTime;    //����       3
		_nJinBiTime    = Info.nJinBiTime;    //���       2


		if (_nCurTime-Info.nExpTime>=1)
		{//��Ե������ 1
			m_AwardButton_fs->setTitleColor(ax::Color3B::YELLOW);
			m_AwardButton_fs->setTouchEnabled(true);
		}
		else
		{
			m_AwardButton_fs->setTitleColor(ax::Color3B::GRAY);
			m_AwardButton_fs->setTouchEnabled(false);
		}


		if (_nCurTime-Info.nYinLiangTime>=1)
		{//��������� 0
			m_AwardButton_yl->setTitleColor(ax::Color3B::YELLOW);
			m_AwardButton_yl->setTouchEnabled(true);
		}
		else
		{
			m_AwardButton_yl->setTitleColor(ax::Color3B::GRAY);
			m_AwardButton_yl->setTouchEnabled(false);
		}

		if (_nCurTime-Info.nExpPiontTime>=60)
		{//����
			m_AwardButton_jf->setTitleColor(ax::Color3B::YELLOW);
			m_AwardButton_jf->setTouchEnabled(true);
		}
		else
		{
			m_AwardButton_jf->setTitleColor(ax::Color3B::GRAY);
			m_AwardButton_jf->setTouchEnabled(false);
		}

		if (_nCurTime-Info.nJinBiTime>=60)
		{ //���
			m_AwardButton_jb->setTitleColor(ax::Color3B::YELLOW);
			m_AwardButton_jb->setTouchEnabled(true);
		}
		else
		{
			m_AwardButton_jb->setTitleColor(ax::Color3B::GRAY);
			m_AwardButton_jb->setTouchEnabled(false);
		}
			 }
			 break;
	  default:
		  setBtnTouchEnble(AUTOBTN_PAGE_BASE,false);
		  break;
	  }

  }
}

//void KuiCometoMap::draw()
//{
//	//messageBox("����ѭ��","draw");
//}


int KuiCometoMap::FilterTextColor(char* pMsgBuff, unsigned short nMsgLength)
{
	nMsgLength = TClearSpecialCtrlInEncodedText(pMsgBuff, nMsgLength, KTC_ENTER);          //��ֹ�س�����
	nMsgLength = TClearSpecialCtrlInEncodedText(pMsgBuff, nMsgLength, KTC_COLOR);          //��ֹ��ɫ����
	nMsgLength = TClearSpecialCtrlInEncodedText(pMsgBuff, nMsgLength, KTC_COLOR_RESTORE);  //��ֹ��ɫ��ԭ
	nMsgLength = TClearSpecialCtrlInEncodedText(pMsgBuff, nMsgLength, KTC_BORDER_COLOR);   //��ֹ�������
	nMsgLength = TClearSpecialCtrlInEncodedText(pMsgBuff, nMsgLength, KTC_BORDER_RESTORE); //��ֹ�������
	nMsgLength = TClearSpecialCtrlInEncodedText(pMsgBuff, nMsgLength, KTC_INLINE_PIC);     //��ֹ��ͼ����
	nMsgLength = TClearSpecialCtrlInEncodedText(pMsgBuff, nMsgLength, KTC_LINK);           //��ֹ��������
	//	return TEncodeText(pMsgBuff, nMsgLength);
	return  nMsgLength;
}
