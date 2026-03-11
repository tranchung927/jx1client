//�а�ť�ġ������롡�Ի���
//
#include "gamecore/KCore.h"

#ifndef WIN32
#include <sys/select.h>
#include <ctype.h>
//typedef struct fd_set FD_SET;
#endif
#include "network/Socket.h"

#include "engine/KPlatform.h"
#include "engine/CXmlStream.h"
#include "KuiLockscree.h"
#include "Klogin_f.h"
#include "engine/Text.h"
#include "engine/KIniFile.h"
#include "gamecore/CoreShell.h"
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
#include "engine/dataChecksum.h"
extern iCoreShell*	g_pCoreShell;
#include <list>
USING_NS_AX;

KuiLockscree::KuiLockscree()
{
	isOpen = false;
	colorLayer        = NULL;
	red               = NULL;
	m_pUiLayer        = NULL;
	m_StartPos.x=0;
	m_StartPos.y=0;
}

KuiLockscree::~KuiLockscree()
{
	isOpen = false;
	colorLayer        = NULL;
}

//����ȷ��������
KuiLockscree * KuiLockscree::create(Ref * callbackListener,const std::function<void(ax::Node*)>& callfun)
{
	KuiLockscree * popLayer = KuiLockscree::create();
	//popLayer->setTitle(title);
	popLayer->addDialogData();
	//���ð�ť
	popLayer->setcoloseButton(callbackListener,callfun);
	popLayer->isOpen = true;
	return popLayer;
}

//��ʼ��
bool KuiLockscree::init()
{
	if ( !Layer::init() ){
		return false;
	}
	//����������Ӧ

	this->scheduleUpdate();                   //���ú���
	auto listener = ax::EventListenerTouchOneByOne::create();
ax::Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, this);   //��ֹ��㴥��
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
	Color4B color(112, 128, 144, 0);
	colorLayer = LayerColor::create(color);//��ɫ��
	colorLayer->setPosition(ax::Vec2::ZERO);
	colorLayer->setContentSize(visibleSize);//���úʹ��ڵĴ�С
	this->addChild(colorLayer);

	ParentNode_ItemEx = colorLayer;
	//��ñ���ͼƬ�Ĵ�С
	Size contentSize = ParentNode_ItemEx->getContentSize();
	m_size = contentSize;
	m_origin = ParentNode_ItemEx->getPosition();
	//colorLayer->setContentSize(contentSize);//���úʹ��ڵĴ�С

	return true;
}

void KuiLockscree::addDialogData()
{
	Size nSize;
	nSize.width =128;
	nSize.height=24;
	m_StartPos.x =10;//+ITEM_CELL_SIZE/2;
	m_StartPos.y =140-24;//+ITEM_CELL_SIZE/2;

	m_pUiLayer = Layer::create();
	ParentNode_ItemEx->addChild(m_pUiLayer);

	//�������������水ť
	Button* button = Button::create();
	button->setTouchEnabled(true);
	button->setScale9Enabled(true);
	button->loadTextures("ui/textbg.png", "ui/textbg.png", "");
	button->setTitleFontSize(14);
	button->setTitleText(UTEXT("��ס��Ļ��,���ȷ�Ͻ���",1).c_str());
	button->setPosition(ax::Vec2(m_size.width/2,60));
	button->setTag(1);
	button->addTouchEventListener(CC_CALLBACK_2(KuiLockscree::oktouchEvent, this));
	m_pUiLayer->addChild(button);

	char nSprName[128]={0};
	ZeroMemory(nSprName,sizeof(nSprName));
	sprintf(nSprName,"\\Spr\\Ui3\\����Ի���\\������������ȷ��.spr");
	g_StrLower(nSprName);
	int m_nWidth,m_nHeight,nFrams;
	Texture2D *bgCur = NULL;
	SPRFRAMSINFO nSprInfo;
	Point nStartPos;
	nStartPos.x = m_size.width/2+20;
	nStartPos.y = m_size.height/2;
	ZeroMemory(&nSprInfo,sizeof(nSprInfo));
	bgCur = _getinidata.getinidata_one(nSprName,0,&m_nWidth,&m_nHeight,&nFrams,&nSprInfo);
	if (bgCur==NULL)
		return;
	Sprite * sprite_ok_btn_confirm_normal =  Sprite::createWithTexture(bgCur);//Sprite::create("close.png");//Sprite::createWithSpriteFrame(f_tips_btn_confirm_normal);
	bgCur = _getinidata.getinidata_one(nSprName,1,&m_nWidth,&m_nHeight,&nFrams,&nSprInfo);
	if (bgCur==NULL)
		return;
	Sprite * sprite_ok_btn_confirm_select =  Sprite::createWithTexture(bgCur);
	MenuItemSprite * okConfirm =NULL;

	okConfirm = MenuItemSprite::create(sprite_ok_btn_confirm_normal, sprite_ok_btn_confirm_select, CC_CALLBACK_1(KuiLockscree::buttonCallBackFunc, this));
	okConfirm->setPosition(ax::Vec2(-28,11));
	//�ر�
	ZeroMemory(nSprName,sizeof(nSprName));
	sprintf(nSprName,"\\Spr\\Ui3\\����Ի���\\������������ȡ��.spr");
	g_StrLower(nSprName);
	bgCur = _getinidata.getinidata_one(nSprName,0,&m_nWidth,&m_nHeight,&nFrams,&nSprInfo);
	if (bgCur==NULL)
		return;
	Sprite * sprite_close_normal =  Sprite::createWithTexture(bgCur);
	bgCur = _getinidata.getinidata_one(nSprName,1,&m_nWidth,&m_nHeight,&nFrams,&nSprInfo);
	if (bgCur==NULL)
		return;
	Sprite * sprite_close_select = Sprite::createWithTexture(bgCur);
	MenuItemSprite * closeConfirm = MenuItemSprite::create(sprite_close_normal, sprite_close_select, CC_CALLBACK_1(KuiLockscree::buttonCallBackFunc, this));
	closeConfirm->setPosition(ax::Vec2(28,11));

	//�й�
	ZeroMemory(nSprName,sizeof(nSprName));
	sprintf(nSprName,"\\spr\\Ui3\\����\\��̯\\�й�.spr");
	g_StrLower(nSprName);
	bgCur = _getinidata.getinidata_one(nSprName,0,&m_nWidth,&m_nHeight,&nFrams,&nSprInfo);
	if (bgCur==NULL)
		return;
	Sprite * sprite_tuo_normal =  Sprite::createWithTexture(bgCur);
	bgCur = _getinidata.getinidata_one(nSprName,1,&m_nWidth,&m_nHeight,&nFrams,&nSprInfo);
	if (bgCur==NULL)
		return;
	Sprite * sprite_tuo_select =  Sprite::createWithTexture(bgCur);
	MenuItemSprite * tuoConfirm = MenuItemSprite::create(sprite_tuo_normal, sprite_tuo_select, CC_CALLBACK_1(KuiLockscree::tuoguanCallBackFunc, this));
	tuoConfirm->setPosition(ax::Vec2(0,34));
	//Point bgSpritePoint = ParentNode_ItemEx->getPosition();
	Menu * menu = Menu::create(okConfirm,closeConfirm,tuoConfirm,NULL);
	menu->setPosition(ax::Vec2(m_size.width/2,3));  //�˵�λ������Ϊ����������
	ParentNode_ItemEx->addChild(menu);         //�ӵ�������
}

void KuiLockscree::tuoguanCallBackFunc(Ref * pSender)
{
	if (g_pCoreShell)
		g_pCoreShell->OperationRequest(GOI_LIXIAN, 0, 0);  //������������
}

//�����ť�ص�����
void KuiLockscree::oktouchEvent(Ref *pSender, ax::ui::AbstractCheckButton::TouchEventType type)
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
				buttonCallBackFunc(NULL);
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
				nTbtn->setTitleColor(ax::Color3B::WHITE);
			}
		}
		break;
	case Widget::TouchEventType::CANCELED:
		break;

	default:
		break;
	}
}

//����ȷ��ȡ����ť,���������ò���󣬵��ò�ص�����
void KuiLockscree::setcoloseButton(Ref * callbackListener,const std::function<void(ax::Node*)>& callfun)
{
	m_callbackListener = callbackListener;
	m_callfun = callfun;

}
//ִ���ϲ����Ļص��������رյ�����
void KuiLockscree::buttonCallBackFunc(Ref * pSender)
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
void KuiLockscree::closePopLayer(Ref * pSender)
{
	this->removeFromParentAndCleanup(true);
}

//��д����ע�ắ�������¸�����������
void KuiLockscree::registerWithTouchDispatcher()
{
    auto touchListener = ax::EventListenerTouchOneByOne::create();
    touchListener->setSwallowTouches(true);
    ax::Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(touchListener, this);
	//����Ĵ������ȼ�����Ϊ��128����CCMenuͬ������֤�������·��Ĵ���
	//ax::Director::getInstance()->getTouchDispatcher()->addTargetedDelegate(this,-128,true);
}

Rect KuiLockscree::getRect(Node* pNode,int i)
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
bool KuiLockscree::ccTouchBegan(Touch *pTouch, Event *pEvent)
{
	//CCSetIterator it = pTouch->begin();
	//Touch* touch = (Touch*)(*it);
	//containsPoint(convertTouchToNodeSpaceAR(touch))
	Point location = pTouch->getLocation();//Ŀ�ĵ�����?��Ļ�����
	//case SMCT_UI_RENASCENCE:	             //ѡ������
    //buttonCallBackFunc(NULL);
	return true;
}

void KuiLockscree::update(float delta)
{
	//messageBox("����ѭ��","update");
	if (isOpen)
	{
	   // m_nMoney = g_pCoreShell->GetGameData(GDI_PLAYER_HOLD_MONEY, 0, 0);
	  //	m_nXu    = g_pCoreShell->GetGameData(GDI_PLAYER_HOLD_XU, 0, 0);
	}
}

int KuiLockscree::FilterTextColor(char* pMsgBuff, unsigned short nMsgLength)
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
