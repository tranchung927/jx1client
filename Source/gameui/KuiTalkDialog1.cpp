//�а�ť�ġ������롡�Ի���
//
#include "engine/KPlatform.h"
#include "gamecore/KCore.h"
#include "engine/CXmlStream.h"
#include "KuiTalkDialog1.h"
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
extern iCoreShell*	g_pCoreShell;
#include <list>
USING_NS_AX;

KuiTalkDialog1::KuiTalkDialog1()
{
	isOpen = false;
	colorLayer        = NULL;
	red               = NULL;
	__Kind      =-1;
	__nParam    =0;
	ZeroMemory(__strParam,sizeof(__strParam));
}

KuiTalkDialog1::~KuiTalkDialog1()
{
	isOpen = false;
	colorLayer = NULL;
	__nParam    =0;
}

//����ȷ��������
KuiTalkDialog1* KuiTalkDialog1::create(const char* pContent,
                                       const char* OkbtnStr,
                                       const char* NobtnStr,
                                       int nKind,
                                       Ref* callbackListener,
                                       const std::function<void(ax::Node*)>& callfun,
                                       unsigned int nParam,
                                       char* strParam)
{
	KuiTalkDialog1 * popLayer = KuiTalkDialog1::create();
	popLayer->__nParam = nParam;
	if (strParam)
	   sprintf(popLayer->__strParam, "%s", strParam);
	//popLayer->setTitle(title);
	popLayer->addDialogData(pContent,nKind);
	//���ð�ť
	popLayer->setcoloseButton(OkbtnStr,NobtnStr,callbackListener,callfun);
	popLayer->isOpen = true;
	return popLayer;
}


//��ʼ��
bool KuiTalkDialog1::init()
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
	sprintf(nSprName,"\\Spr\\Ui3\\NPC�Ի���\\�Ի���2.spr");
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
    ax::Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, this);
	return true;
}


void KuiTalkDialog1::addDialogData(const char* pContent, int nKind)
{
	if  (!g_pCoreShell) return;
	Size nSize;
	nSize.width =256;
	nSize.height=24;
	char nTempStr[128];
	sprintf(nTempStr, "%s", pContent);
	__Kind = nKind;
	Label *pstrLabel = Label::createWithTTF(UTEXT(nTempStr,1).c_str(),UI_GAME_FONT_DEFAULT,14,nSize,ax::TextHAlignment::LEFT);//Arial
	pstrLabel->setColor(ax::Color3B::YELLOW);
	pstrLabel->setAnchorPoint(ax::Vec2(0,0));
	pstrLabel->setPosition(ax::Vec2(30,m_size.height-50));
	ParentNode_ItemEx->addChild(pstrLabel);

	switch(nKind)
	{
	case SMCT_UI_TONG_JOIN_APPLY://�������
	case SMCT_UI_RENASCENCE:
		{
			/*//ax::Director::getInstance()->getTouchDispatcher()->addTargetedDelegate(this,-128, true);
			Label *ptextLabel = Label::createWithTTF(UTEXT("�����Ļ�س�",1).c_str(),UI_GAME_FONT_DEFAULT,14,nSize,TextHAlignment::CENTER);//Arial
			ptextLabel->setColor(ax::Color3B::YELLOW);
			ptextLabel->setPosition(ax::Vec2(m_size.width/2,20));
			ParentNode_ItemEx->addChild(ptextLabel);*/
		}
	break;
	case SMCT_UI_TEAM_INVITE:
		{//Ӧ���������

		}
		break;
	case SMCT_UI_TEAM_APPLY:
		{//ͬ���ܾ����˵��������
		}
		break;
	default:
		break;
	}

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
void KuiTalkDialog1::setcoloseButton(const char* OkbtnStr,
                                     const char* NobtnStr,
                                     Ref* callbackListener,
                                     const std::function<void(ax::Node*)>& callfun)
{
	m_callbackListener = callbackListener;
	m_callfun = callfun;

	m_pUiLayer = Layer::create();
	ParentNode_ItemEx->addChild(m_pUiLayer);

	switch(__Kind)
	{
	case SMCT_UI_TONG_JOIN_APPLY://�������
	case SMCT_UI_TRADE:
	case SMCT_UI_TEAM_APPLY:
	case SMCT_UI_TEAM_INVITE:
		{
			//ͬ��
			char nOkStr[64],nNoStr[64];
			sprintf(nOkStr, "%s", OkbtnStr);
			sprintf(nNoStr, "%s", NobtnStr);

			Button* button = Button::create();
			button->setTouchEnabled(true);
			button->loadTextures("ui/textbg.png", "ui/textbg.png", "");
			button->setTitleText(UTEXT(nOkStr,1).c_str());
			button->setPosition(ax::Vec2(m_size.width/2-70,30));
			button->setTag(__Kind);
			button->setTag(1);
			button->addTouchEventListener(CC_CALLBACK_2(KuiTalkDialog1::touchEvent, this));
			m_pUiLayer->addChild(button);
			//ȡ��
			Button* _button = Button::create();
			_button->setTouchEnabled(true);
			_button->loadTextures("ui/textbg.png", "ui/textbg.png", "");
			_button->setTitleText(UTEXT(nNoStr,1).c_str());
			_button->setPosition(ax::Vec2(m_size.width/2+70,30));
			_button->setTag(__Kind);
			_button->setTag(0);
			_button->addTouchEventListener(CC_CALLBACK_2(KuiTalkDialog1::touchEvent, this));
			m_pUiLayer->addChild(_button);
		}
		break;
	case SMCT_UI_RENASCENCE:
		{
			//�س�����
			Button* button = Button::create();
			button->setTouchEnabled(true);
			button->loadTextures("ui/textbg.png", "ui/textbg.png", "");
			button->setTitleText(UTEXT("�س�����",1).c_str());
			button->setPosition(ax::Vec2(m_size.width/2,30));
			button->setTag(SMCT_UI_RENASCENCE);
			button->setTag(1);
			button->addTouchEventListener(CC_CALLBACK_2(KuiTalkDialog1::touchEvent, this));
			m_pUiLayer->addChild(button);
		}
		break;
	default:
		break;
	}

}
//ִ���ϲ����Ļص��������رյ�����
void KuiTalkDialog1::buttonCallBackFunc(Ref * pSender)
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
void KuiTalkDialog1::closePopLayer(Ref * pSender)
{
	this->removeFromParentAndCleanup(true);
}

//��д����ע�ắ�������¸�����������
void KuiTalkDialog1::registerWithTouchDispatcher()
{
	//����Ĵ������ȼ�����Ϊ��128����CCMenuͬ������֤�������·��Ĵ���
	////ax::Director::getInstance()->getTouchDispatcher()->addTargetedDelegate(this,-128, true);
}

Rect KuiTalkDialog1::getRect(Node* pNode,int i)
{
	Rect  rc;
	rc.origin     = pNode->getPosition(); //��������ͼ��ƫ��
	//visibleSize
	rc.size       = pNode->getContentSize();

	rc.origin.x   = m_origin.x - m_size.width/2 + rc.origin.x; //��������
	rc.origin.y   = m_origin.y - m_size.height/2+ rc.origin.y; //���ϱ�    //- rc.origin.y

	return rc;
}

void KuiTalkDialog1::touchEvent(Ref *pSender, ax::ui::AbstractCheckButton::TouchEventType type)
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
				switch (nTbtn->getTag())
				{
				   case SMCT_UI_TONG_JOIN_APPLY://�������
					   {
						   /*
						   g_pCoreShell->TongOperation(GTOI_TONG_JOIN_REPLY,
						   (uintptr_t)pPlayer, (nSelAction == 0));	//nSelAction=0����Ӧ, nSelAction=1���ܾ�
						   */
						   if (nTbtn->getTag() == 1)
						   {//ͬ��
							   g_pCoreShell->TongOperation(GTOI_TONG_JOIN_REPLY,__nParam,true,0,__strParam);	    //nSelAction=0����Ӧ, nSelAction=1���ܾ�
						   }else
							   g_pCoreShell->TongOperation(GTOI_TONG_JOIN_REPLY,__nParam,false,0,__strParam);	//nSelAction=0����Ӧ, nSelAction=1���ܾ�

						   //char nTemp[64];
						   //sprintf(nTemp,"%u:%s",__nParam,__strParam);
						   //messageBox(UTEXT(nTemp,1).c_str(),"test");
					   }
					   break;
				   case SMCT_UI_TRADE:
				    {//���뽻�׵�
						if (nTbtn->getTag() == 1)
						{//ͬ��
							g_pCoreShell->OperationRequest(GOI_TRADE_INVITE_RESPONSE,__nParam,true);	//nSelAction=0����Ӧ, nSelAction=1���ܾ�
						}else
                            g_pCoreShell->OperationRequest(GOI_TRADE_INVITE_RESPONSE,__nParam,false);	//nSelAction=0����Ӧ, nSelAction=1���ܾ�

				    }
					break;
				   case SMCT_UI_TEAM_APPLY:
					{//ͬ���ܾ����˵��������
						if (nTbtn->getTag() == 1)
						{
							//messageBox("dlalog","ͬ�����");
							g_pCoreShell->TeamOperation(TEAM_OI_APPLY_RESPONSE,__nParam,true);
						}
						else
							g_pCoreShell->TeamOperation(TEAM_OI_APPLY_RESPONSE,__nParam,false);	//nSelAction=0����Ӧ, nSelAction=1���ܾ�

					 }
					 break;
					case SMCT_UI_TEAM_INVITE://Touch Down
						{//�Լ��ܾ���ͬ��������ӵ�����
							if (__nParam)
							{
								if (nTbtn->getTag() == 1)
								    g_pCoreShell->TeamOperation(TEAM_OI_INVITE_RESPONSE,__nParam,true);
								else
                                    g_pCoreShell->TeamOperation(TEAM_OI_INVITE_RESPONSE,__nParam,false);	//nSelAction=0����Ӧ, nSelAction=1���ܾ�

							}
							//(unsigned int)sPlayer
						}
						break;
					case SMCT_UI_RENASCENCE://Touch Down
						{
							if (g_pCoreShell)
								g_pCoreShell->OperationRequest(GOI_PLAYER_RENASCENCE,0,true);//nSelAction=0��ԭ������, nSelAction=1���س�
							buttonCallBackFunc(NULL);
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
				switch (nTbtn->getTag())
				{
				case SMCT_UI_TONG_JOIN_APPLY://�������
				case SMCT_UI_TRADE:
				case SMCT_UI_TEAM_APPLY:
				case SMCT_UI_TEAM_INVITE://Touch Down
					{
						buttonCallBackFunc(NULL);
					}
					break;
				case SMCT_UI_RENASCENCE://����
					{
					}
					break;
				default:
					break;
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

//��������ccTouchBegan������true
bool KuiTalkDialog1::ccTouchBegan(Touch *pTouch, Event *pEvent)
{
	//CCSetIterator it = pTouch->begin();
	//Touch* touch = (Touch*)(*it);
	//containsPoint(convertTouchToNodeSpaceAR(touch))
	Point location = pTouch->getLocation();//Ŀ�ĵ�����?��Ļ�����
	switch(__Kind)
	{
	case SMCT_UI_RENASCENCE:
		{//ѡ������

		}
		break;
	case SMCT_UI_TEAM_INVITE:
		{
		}
		break;
	default:
		break;
	}
	return true;
}

void KuiTalkDialog1::update(float delta)
{
	//messageBox("����ѭ��","update");
	if (isOpen && g_pCoreShell)
	{
	   // m_nMoney = g_pCoreShell->GetGameData(GDI_PLAYER_HOLD_MONEY, 0, 0);
	  //	m_nXu    = g_pCoreShell->GetGameData(GDI_PLAYER_HOLD_XU, 0, 0);
	}
}

int KuiTalkDialog1::FilterTextColor(char* pMsgBuff, unsigned short nMsgLength)
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
