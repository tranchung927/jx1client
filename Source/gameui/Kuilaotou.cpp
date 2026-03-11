//�а�ť�ġ������롡�Ի���
//
#include "engine/KPlatform.h"
#include "gamecore/KCore.h"
#include "engine/CXmlStream.h"
#include "Kuilaotou.h"
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

Kuilaotou::Kuilaotou()
{
	isOpen = false;
	//memset(Skills,0,sizeof(KUiSkillData) * 50);
	//pMoneyLabel       = NULL;
	//pXuLabel          = NULL;
	colorLayer        = NULL;
	red               = NULL;

	//m_nMoney = 0;
	//m_nXu = 0;
	//m_AnswerCount = 0;
	m_StartPos.x=0;
	m_StartPos.y=0;
}

Kuilaotou::~Kuilaotou()
{
	isOpen = false;
	//pMoneyLabel  = NULL;
	colorLayer        = NULL;
	//if (red)
	  //  red->release();
	//red               = NULL;
}

//����ȷ��������
Kuilaotou * Kuilaotou::create(Ref * callbackListener,const std::function<void(ax::Node*)>& callfun)
{
	Kuilaotou * popLayer = Kuilaotou::create();
	//popLayer->setTitle(title);
	popLayer->addDialogData();
	//���ð�ť
	popLayer->setcoloseButton(callbackListener,callfun);
	popLayer->isOpen = true;
	return popLayer;
}


//��ʼ��
bool Kuilaotou::init()
{
	if ( !Layer::init() ){
		return false;
	}

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
	sprintf(nSprName,"\\spr\\Ui3\\������ϵͳ\\������\\�������.spr");
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
	//����������Ӧ

	this->scheduleUpdate();                   //���ú���
	auto listener = ax::EventListenerTouchOneByOne::create();
ax::Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, this); //��ֹ��㴥��
	return true;
}


void Kuilaotou::addDialogData()
{
	if  (!g_pCoreShell || !g_GameWorld ||!nisgetinfo) return;

	Size nSize;
	nSize.width =350;
	nSize.height=140;
	m_StartPos.x =10;//+ITEM_CELL_SIZE/2;
	m_StartPos.y =140-24;//+ITEM_CELL_SIZE/2;
	char nTempStr[128];
	KIniFile strInfo;

	if  (strInfo.Load("\\ui\\ui3\\��������.ini"))
	{
		strInfo.GetString("StringQuest", "GetGift","", nTempStr, sizeof(nTempStr));
		strInfo.Clear();
	}
	//sprintf(nTempStr,pContent);
	Label *pstrLabel = Label::createWithTTF(UTEXT(nTempStr,1).c_str(),UI_GAME_FONT_DEFAULT,14,nSize,TextHAlignment::CENTER);//Arial
	pstrLabel->setColor(ax::Color3B::YELLOW);
	pstrLabel->setPosition(ax::Vec2(nSize.width/2+30,m_size.height-100));
	ParentNode_ItemEx->addChild(pstrLabel);


	/*Label *ptextLabel = Label::createWithTTF(UTEXT("�����Ļ�س�",1).c_str(),UI_GAME_FONT_DEFAULT,14,nSize,TextHAlignment::CENTER);//Arial
	ptextLabel->setColor(ax::Color3B::YELLOW);
	ptextLabel->setPosition(ax::Vec2(m_size.width/2,20));
	ParentNode_ItemEx->addChild(ptextLabel);*/

	/*Color4B color(112, 128, 144, 150);
	colorLayer = LayerColor::create(color);//��ɫ��
	colorLayer->setPosition(ax::Vec2::ZERO);
	colorLayer->setContentSize(ax::Size::ZERO);//���úʹ��ڵĴ�С
	ParentNode_ItemEx->addChild(colorLayer,1000);
	*/
	//����һ������ �ı侫����ɫ
	//red = TintBy::create(0.2,0,-255,-255);//����ʱ��+��ɫ
	//red->retain();
}


//����ȷ��ȡ����ť,���������ò���󣬵��ò�ص�����
void Kuilaotou::setcoloseButton(Ref * callbackListener,const std::function<void(ax::Node*)>& callfun)
{
	if  (!g_pCoreShell || !g_GameWorld ||!nisgetinfo) return;

	m_callbackListener = callbackListener;
	m_callfun = callfun;

	char nSprName[128]={0};
	ZeroMemory(nSprName,sizeof(nSprName));
	sprintf(nSprName,"\\spr\\Quest\\ExpBtn.spr");
	g_StrLower(nSprName);
	int m_nWidth,m_nHeight,nFrams;
	Texture2D *bgCur = NULL;
	SPRFRAMSINFO nSprInfo;
	Point nStartPos;
	nStartPos.x = m_size.width/2+20;
	nStartPos.y = m_size.height/2;
	//����
	ZeroMemory(&nSprInfo,sizeof(nSprInfo));
	bgCur = _getinidata.getinidata_one(nSprName,0,&m_nWidth,&m_nHeight,&nFrams,&nSprInfo);
	if (bgCur==NULL)
		return;
	Sprite * sprite_ExpBtn_normal =  Sprite::createWithTexture(bgCur);//Sprite::create("close.png");//Sprite::createWithSpriteFrame(f_tips_btn_confirm_normal);
	MenuItemSprite * ExpBtnConfirm = MenuItemSprite::create(sprite_ExpBtn_normal,sprite_ExpBtn_normal, CC_CALLBACK_1(Kuilaotou::btnCallBackFunc, this));
	ExpBtnConfirm->setPosition(ax::Vec2(-125,11));
	ExpBtnConfirm->setTag(1);
	//���
	ZeroMemory(nSprName,sizeof(nSprName));
	sprintf(nSprName,"\\spr\\Quest\\Random.spr");
	g_StrLower(nSprName);
	bgCur = _getinidata.getinidata_one(nSprName,0,&m_nWidth,&m_nHeight,&nFrams,&nSprInfo);
	if (bgCur==NULL)
		return;
	Sprite * sprite_Random_normal =  Sprite::createWithTexture(bgCur);
	MenuItemSprite * RandomConfirm = MenuItemSprite::create(sprite_Random_normal, sprite_Random_normal, CC_CALLBACK_1(Kuilaotou::btnCallBackFunc, this));
	RandomConfirm->setPosition(ax::Vec2(-4,11));
	RandomConfirm->setTag(2);
	//����
	ZeroMemory(nSprName,sizeof(nSprName));
	sprintf(nSprName,"\\spr\\Quest\\Money.spr");
	g_StrLower(nSprName);
	bgCur = _getinidata.getinidata_one(nSprName,0,&m_nWidth,&m_nHeight,&nFrams,&nSprInfo);
	if (bgCur==NULL)
		return;
	Sprite * sprite_Money_normal =  Sprite::createWithTexture(bgCur);
	MenuItemSprite * MoneyConfirm = MenuItemSprite::create(sprite_Money_normal,sprite_Money_normal, CC_CALLBACK_1(Kuilaotou::btnCallBackFunc, this));
	MoneyConfirm->setPosition(ax::Vec2(125,11));
	MoneyConfirm->setTag(3);
	Point bgSpritePoint = ParentNode_ItemEx->getPosition();
	Menu * menu = Menu::create(ExpBtnConfirm,RandomConfirm,MoneyConfirm,NULL);
	//menu->setPosition(ax::Vec2(bgSpritePoint.x-m_size.width/2,bgSpritePoint.y-m_size.height/2));//�˵�λ������Ϊ����������
	menu->setPosition(ax::Vec2(m_size.width/2,58)); //�˵�λ������Ϊ����������
	ParentNode_ItemEx->addChild(menu);         //�ӵ�������
}

void Kuilaotou::btnCallBackFunc(Ref * pSender)
{
	if  (!g_pCoreShell || !g_GameWorld ||!nisgetinfo)
	{
		buttonCallBackFunc(NULL);
		return;
	}

	if (pSender)
	{
		Node * node = NULL;
			node = dynamic_cast<Node *>(pSender);
		int inselindex = node->getTag();
		if  (inselindex>=1 && inselindex<=3)
		    g_pCoreShell->OperationRequest(GOI_DATAU,inselindex,0);
	}
	buttonCallBackFunc(NULL);
}
//ִ���ϲ����Ļص��������رյ�����
void Kuilaotou::buttonCallBackFunc(Ref * pSender)
{
	Node * node = NULL;
	if (pSender)
		node = dynamic_cast<Node *>(pSender);
	//node->setTag(3000);//����tag���ڵ��ò���Ի�ȡ��
	if (m_callfun &&  m_callbackListener)
		(m_callfun)(node);

	this->removeFromParentAndCleanup(true);
}

//�رյ�����
void Kuilaotou::closePopLayer(Ref * pSender)
{
	this->removeFromParentAndCleanup(true);
}

//��д����ע�ắ�������¸�����������
void Kuilaotou::registerWithTouchDispatcher()
{
    auto touchListener = ax::EventListenerTouchOneByOne::create();
    touchListener->setSwallowTouches(true);
    ax::Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(touchListener, this);
	//����Ĵ������ȼ�����Ϊ��128����CCMenuͬ������֤�������·��Ĵ���
	//ax::Director::getInstance()->getTouchDispatcher()->addTargetedDelegate(this,-128,true);
}

Rect Kuilaotou::getRect(Node* pNode,int i)
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
bool Kuilaotou::ccTouchBegan(Touch *pTouch, Event *pEvent)
{
	//CCSetIterator it = pTouch->begin();
	//Touch* touch = (Touch*)(*it);
	//containsPoint(convertTouchToNodeSpaceAR(touch))
	Point location = pTouch->getLocation();//Ŀ�ĵ�����?��Ļ�����
	//case SMCT_UI_RENASCENCE:	//ѡ������
		//if (g_pCoreShell)
		//   g_pCoreShell->OperationRequest(GOI_PLAYER_RENASCENCE,0,true);//nSelAction=0��ԭ������, nSelAction=1���س�
    buttonCallBackFunc(NULL);
	return true;
}

void Kuilaotou::update(float delta)
{
	//messageBox("����ѭ��","update");
	if (isOpen && g_pCoreShell)
	{
	}
}

int Kuilaotou::FilterTextColor(char* pMsgBuff, unsigned short nMsgLength)
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
