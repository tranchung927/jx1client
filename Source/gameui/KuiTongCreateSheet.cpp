#include "engine/KPlatform.h"
#include "gamecore/KCore.h"
#include "engine/CXmlStream.h"
#include "KuiTongCreateSheet.h"
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
#include "gamecore/KSkillManager.h"
#include "gamecore/KItem.h"
extern iCoreShell*	g_pCoreShell;
#include <list>
USING_NS_AX;
KuiTongCreateSheet::KuiTongCreateSheet()
{
	isOpen = false;
	__isCanMove=true;
	m_bScrolling=false;
	//memset(Skills,0,sizeof(KUiSkillData) * 50);
	colorLayer        = NULL;
	red               = NULL;
	m_pMainUiLayer    = NULL;
	__nSelCamp        = -1;
	pNameEditBox      = NULL;
	btnBox_zp         = NULL;
	btnBox_zl         = NULL;
	btnBox_xp         = NULL;
	pCampInfoLabel    = NULL;
	pTongNameLabel    = NULL;
	pCampTitleLabel   = NULL;
}

KuiTongCreateSheet::~KuiTongCreateSheet()
{
	isOpen = false;
	colorLayer   = NULL;
	//CC_SAFE_RELEASE(m_array);
	if (red)
	    red->release();

}

//����ȷ��������
KuiTongCreateSheet* KuiTongCreateSheet::create(const char* title,
                                               Ref* callbackListener,
                                               const std::function<void(ax::Node*)>& callfun)
{
	KuiTongCreateSheet * popLayer = KuiTongCreateSheet::create();
	popLayer->addDialogData();
	//���ð�ť
	popLayer->setcoloseButton(callbackListener,callfun);
	popLayer->isOpen = true;
	return popLayer;
}

//��ʼ��
bool KuiTongCreateSheet::init()
{
	if ( !Layer::init() ){
		return false;
	}
	//����֡����
	//SpriteFrameCache * sfCache = SpriteFrameCache::sharedSpriteFrameCache();
	//sfCache->addSpriteFramesWithFile("p_tips.plist");
	//���ģ̬����
	winSize = ax::Director::getInstance()->getWinSize();
	/*Color4B color(112, 128, 144, 150);
	LayerColor * colorLayer = LayerColor::create(color);//��ɫ��
	colorLayer->setPosition(ax::Vec2::ZERO);
	//colorLayer->setContentSize(winSize);//���úʹ��ڵĴ�С
	this->addChild(colorLayer);
	*/
	char nSprName[128]={0};
	ZeroMemory(nSprName,sizeof(nSprName));
	sprintf(nSprName,"\\Spr\\Ui3\\������\\�����Ϣ�޸ĵ�ͼ.spr");
	g_StrLower(nSprName);
	int m_nWidth,m_nHeight,nFrams;
	Texture2D *bgCur = NULL;
	SPRFRAMSINFO nSprInfo;
	ZeroMemory(&nSprInfo,sizeof(nSprInfo));
	bgCur = _getinidata.getinidata_one(nSprName,0,&m_nWidth,&m_nHeight,&nFrams,&nSprInfo);
	if (bgCur==NULL)
		return false;
	Sprite * spriteBox = Sprite::createWithTexture(bgCur);
	ParentNode_Task= spriteBox;
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

void KuiTongCreateSheet::addDialogData()
{
	if  (!g_pCoreShell) return;

	/*pMoneyLabel = Label::createWithTTF("","fonts/gb_song.ttf", 12);//ͳ˧
	pMoneyLabel->setAnchorPoint(ax::Vec2(0,0));
	pMoneyLabel->setPosition(ax::Vec2(70,35));
	ParentNode_Task->addChild(pMoneyLabel,1);
*/
	m_pMainUiLayer = Layer::create();
	ParentNode_Task->addChild(m_pMainUiLayer);



	pTongNameLabel =Label::createWithTTF(UTEXT("�������",1).c_str(),UI_GAME_FONT_SONG, 12);//Arial
	pTongNameLabel->setAnchorPoint(ax::Vec2(0,0));
	pTongNameLabel->setPosition(ax::Vec2(20,m_size.height-48));
	pTongNameLabel->setColor(ax::Color3B::YELLOW);
	ParentNode_Task->addChild(pTongNameLabel);

	pCampTitleLabel =Label::createWithTTF(UTEXT("�����Ӫ",1).c_str(),UI_GAME_FONT_SONG, 12);//Arial
	pCampTitleLabel->setAnchorPoint(ax::Vec2(0,0));
	pCampTitleLabel->setPosition(ax::Vec2(20,m_size.height-73));
	pCampTitleLabel->setColor(ax::Color3B::YELLOW);
	ParentNode_Task->addChild(pCampTitleLabel);

	pCampInfoLabel =Label::createWithTTF(UTEXT("��Ӫ˵��",1).c_str(),UI_GAME_FONT_SONG, 12);//Arial
	pCampInfoLabel->setAnchorPoint(ax::Vec2(0,0));
	pCampInfoLabel->setPosition(ax::Vec2(20,m_size.height-105));
	pCampInfoLabel->setColor(ax::Color3B::YELLOW);
	ParentNode_Task->addChild(pCampInfoLabel);


	// Create the checkbox
	/*char nSprName[128]={0};
	int m_nWidth,m_nHeight,nFrams;
	Texture2D *bgCur = NULL;
	SPRFRAMSINFO nSprInfo;
	ZeroMemory(&nSprInfo,sizeof(nSprInfo));
	ZeroMemory(nSprName,sizeof(nSprName));
	sprintf(nSprName,"\\Spr\\Ui3\\������\\�����Ϣ�޸ĵ�ͼ.spr");
	g_StrLower(nSprName);

	bgCur = _getinidata.getinidata_one(nSprName,0,&m_nWidth,&m_nHeight,&nFrams,&nSprInfo);
	if (bgCur==NULL)
		return ;
	ImageView *titleImg = ImageView::create();
	titleImg->loadTextureByData(bgCur);
	titleImg->setPosition(ax::Vec2(m_size.width/2,m_size.height/2-15));
	m_pMainUiLayer->addChild(titleImg);
	*/

	ax::ui::Scale9Sprite *pBg;
	pBg = ax::ui::Scale9Sprite::create("ui/editbg.png");
	//pBg->setColor(ax::Color3B::GREEN);
	pNameEditBox = EditBox::create(Size(90,24),pBg);
	const ax::Color3B nfcolor ={255,255,255};
	if (pNameEditBox)
	{
		//pEditBox->setContentSize()
		//pMoneyEditBox->setText("your money");
		pNameEditBox->setPlaceHolder(UTEXT("tong name",1).c_str());
		//pNameEditBox->setFontColor(ax::Color3B::YELLOW);
		pNameEditBox->setFontSize(24);
		pNameEditBox->setMaxLength(24);
		pNameEditBox->setPosition(ax::Vec2(pTongNameLabel->getPosition().x+pTongNameLabel->getContentSize().width+50,m_size.height-40));
		pNameEditBox->setInputMode(ax::ui::EditBox::InputMode::ANY);
		pNameEditBox->setReturnType(ax::ui::EditBox::KeyboardReturnType::DONE);
		//pMoneyEditBox->setDelegate(this);
		//pMoneyEditBox->setDefaultTouchPriority(-128);
		m_pMainUiLayer->addChild(pNameEditBox);
	}

	Color4B color(112, 128, 144, 150);
	colorLayer = LayerColor::create(color);//��ɫ��
	colorLayer->setPosition(ax::Vec2::ZERO);
	colorLayer->setContentSize(ax::Size::ZERO);  //���úʹ��ڵĴ�С
	ParentNode_Task->addChild(colorLayer,1000);
	//����һ������ �ı侫����ɫ
	//red = TintBy::create(0.2,0,-255,-255);//����ʱ��+��ɫ
	//red->retain();
}

//����ȷ��ȡ����ť,���������ò���󣬵��ò�ص�����
void KuiTongCreateSheet::setcoloseButton(Ref * callbackListener,const std::function<void(ax::Node*)>& callfun)
{
	m_callbackListener = callbackListener;
	m_callfun = callfun;

	char nSprName[128]={0};
	int m_nWidth,m_nHeight,nFrams;
	Texture2D *bgCur = NULL;
	SPRFRAMSINFO nSprInfo;
	ZeroMemory(&nSprInfo,sizeof(nSprInfo));
	//����
	ZeroMemory(nSprName,sizeof(nSprName));
	sprintf(nSprName,"\\Spr\\Ui3\\����\\���ɴ���-����.spr");
	g_StrLower(nSprName);
	bgCur = _getinidata.getinidata_one(nSprName,0,&m_nWidth,&m_nHeight,&nFrams,&nSprInfo);
	if (bgCur==NULL)
		return ;
	Sprite * sprite_btnZp_normal = Sprite::createWithTexture(bgCur);
	sprite_btnZp_normal->setAnchorPoint(ax::Vec2(0,0));
	bgCur = _getinidata.getinidata_one(nSprName,1,&m_nWidth,&m_nHeight,&nFrams,&nSprInfo);
	if (bgCur==NULL)
		return ;
	Sprite * sprite_btnZp_select = Sprite::createWithTexture(bgCur);
	sprite_btnZp_select->setAnchorPoint(ax::Vec2(0,0));

	MenuItemSprite * btnZpConfirm = MenuItemSprite::create(sprite_btnZp_normal, sprite_btnZp_select, CC_CALLBACK_1(KuiTongCreateSheet::btnCallBackFunc, this));
	btnZpConfirm->setPosition(ax::Vec2(pCampTitleLabel->getPosition().x+pCampTitleLabel->getContentSize().width+15,m_size.height-65));
	btnZpConfirm->setTag(3);

	//а��
	ZeroMemory(nSprName,sizeof(nSprName));
	sprintf(nSprName,"\\Spr\\Ui3\\����\\���ɴ���-а��.spr");
	g_StrLower(nSprName);
	bgCur = _getinidata.getinidata_one(nSprName,0,&m_nWidth,&m_nHeight,&nFrams,&nSprInfo);
	if (bgCur==NULL)
		return ;
	Sprite * sprite_btnXp_normal = Sprite::createWithTexture(bgCur);
	sprite_btnXp_normal->setAnchorPoint(ax::Vec2(0,0));
	bgCur = _getinidata.getinidata_one(nSprName,1,&m_nWidth,&m_nHeight,&nFrams,&nSprInfo);
	if (bgCur==NULL)
		return ;
	Sprite * sprite_btnXp_select = Sprite::createWithTexture(bgCur);
	sprite_btnXp_select->setAnchorPoint(ax::Vec2(0,0));

	MenuItemSprite * btnXpConfirm = MenuItemSprite::create(sprite_btnXp_normal, sprite_btnXp_select, CC_CALLBACK_1(KuiTongCreateSheet::btnCallBackFunc, this));
	btnXpConfirm->setPosition(ax::Vec2(btnZpConfirm->getPosition().x+btnZpConfirm->getContentSize().width+5,m_size.height-65));
	btnXpConfirm->setTag(4);

	//����
	ZeroMemory(nSprName,sizeof(nSprName));
	sprintf(nSprName,"\\Spr\\Ui3\\����\\���ɴ���-����.spr");
	g_StrLower(nSprName);
	bgCur = _getinidata.getinidata_one(nSprName,0,&m_nWidth,&m_nHeight,&nFrams,&nSprInfo);
	if (bgCur==NULL)
		return ;
	Sprite * sprite_btnZl_normal = Sprite::createWithTexture(bgCur);
	sprite_btnZl_normal->setAnchorPoint(ax::Vec2(0,0));
	bgCur = _getinidata.getinidata_one(nSprName,1,&m_nWidth,&m_nHeight,&nFrams,&nSprInfo);
	if (bgCur==NULL)
		return ;
	Sprite * sprite_btnZl_select = Sprite::createWithTexture(bgCur);
	sprite_btnZl_select->setAnchorPoint(ax::Vec2(0,0));

	MenuItemSprite * btnZlConfirm = MenuItemSprite::create(sprite_btnZl_normal, sprite_btnZl_select, CC_CALLBACK_1(KuiTongCreateSheet::btnCallBackFunc, this));
	btnZlConfirm->setPosition(ax::Vec2(btnXpConfirm->getPosition().x+btnXpConfirm->getContentSize().width+5,m_size.height-65));
	btnZlConfirm->setTag(5);


	//ȷ��
	ZeroMemory(nSprName,sizeof(nSprName));
	sprintf(nSprName,"\\Spr\\Ui3\\����\\���ɴ���-ȷ��.spr");
	g_StrLower(nSprName);
	bgCur = _getinidata.getinidata_one(nSprName,0,&m_nWidth,&m_nHeight,&nFrams,&nSprInfo);
	if (bgCur==NULL)
		return ;
	Sprite * sprite_Ok_normal = Sprite::createWithTexture(bgCur);
	sprite_Ok_normal->setAnchorPoint(ax::Vec2(0,0));
	bgCur = _getinidata.getinidata_one(nSprName,2,&m_nWidth,&m_nHeight,&nFrams,&nSprInfo);
	if (bgCur==NULL)
		return ;
	Sprite * sprite_Ok_select = Sprite::createWithTexture(bgCur);
	sprite_Ok_select->setAnchorPoint(ax::Vec2(0,0));

	MenuItemSprite * OkConfirm = MenuItemSprite::create(sprite_Ok_normal, sprite_Ok_select, CC_CALLBACK_1(KuiTongCreateSheet::btnCallBackFunc, this));
	OkConfirm->setPosition(ax::Vec2(48,m_size.height-130));
	OkConfirm->setTag(1);

	//�ر�
	ZeroMemory(nSprName,sizeof(nSprName));
	sprintf(nSprName,"\\Spr\\Ui3\\����\\���ɴ���-ȡ��.spr");
	g_StrLower(nSprName);
	bgCur = _getinidata.getinidata_one(nSprName,0,&m_nWidth,&m_nHeight,&nFrams,&nSprInfo);
	if (bgCur==NULL)
		return ;
	Sprite * sprite_Leave_normal = Sprite::createWithTexture(bgCur);
	sprite_Leave_normal->setAnchorPoint(ax::Vec2(0,0));
	bgCur = _getinidata.getinidata_one(nSprName,2,&m_nWidth,&m_nHeight,&nFrams,&nSprInfo);
	if (bgCur==NULL)
		return ;
	Sprite * sprite_Leave_select = Sprite::createWithTexture(bgCur);
	sprite_Leave_select->setAnchorPoint(ax::Vec2(0,0));

	MenuItemSprite * LeaveConfirm = MenuItemSprite::create(sprite_Leave_normal, sprite_Leave_select, CC_CALLBACK_1(KuiTongCreateSheet::btnCallBackFunc, this));
	LeaveConfirm->setPosition(ax::Vec2(134,m_size.height-130));
	LeaveConfirm->setTag(2);
	//����˵�
	Menu * menu = Menu::create(OkConfirm,LeaveConfirm,btnZpConfirm,btnXpConfirm,btnZlConfirm,NULL);
	menu->setAnchorPoint(ax::Vec2(0,0));
	menu->setPosition(ax::Vec2(0,0));             //�˵�λ������Ϊ����������
	ParentNode_Task->addChild(menu);         //�ӵ�������


	KUiPlayerTeam mTeam;
	ZeroMemory(&mTeam,sizeof(mTeam));
	if (g_pCoreShell->TeamOperation(TEAM_OI_GD_INFO, (uintptr_t)&mTeam, 0)) //û�ж��� �ʹ���һ֧����
	{//�ж���
	}
	else
	{//û�ж���
	}
}

void KuiTongCreateSheet::btnCallBackFunc(Ref * pSender)
{
	if (!g_pCoreShell|| !g_GameWorld) return;

	Node * node = NULL;
	if (pSender)
	{
		node= dynamic_cast<Node *>(pSender);
		switch(node->getTag())
		{
		case 1://ȷ��
			{
				char Buff[32],nTempBuff[32];
				ZeroMemory(Buff,sizeof(Buff));
				ZeroMemory(nTempBuff,sizeof(nTempBuff));
				sprintf(nTempBuff, "%s", pNameEditBox->getText());
				sprintf(Buff, "%s", U2G(nTempBuff).c_str());
				if(Buff[0] && g_pCoreShell)
				{
					if(__nSelCamp != -1){
						g_pCoreShell->TongOperation(GTOI_TONG_CREATE, 0, (int)__nSelCamp,0,Buff);
						buttonCallBackFunc(NULL);
					}
					else
						pCampInfoLabel->setString(UTEXT("��ѡ�����������Ӫ",1).c_str());
				}
				else
					pCampInfoLabel->setString(UTEXT("�������������",1).c_str());
			}
			break;
		case 2://ȡ��
			buttonCallBackFunc(NULL);
			break;
		case 3:
			if (pCampInfoLabel)
				pCampInfoLabel->setString(UTEXT("���ڽ�������",1).c_str());

			__nSelCamp = camp_justice;
			break;
		case 4:
			if (pCampInfoLabel)
				pCampInfoLabel->setString(UTEXT("���ڽ���а��",1).c_str());
			__nSelCamp = camp_evil;
			break;
		case 5:
			if (pCampInfoLabel)
				pCampInfoLabel->setString(UTEXT("���ڽ���������",1).c_str());

			__nSelCamp = camp_balance;
			break;
		default:
			break;
		}
	}
}
//ִ���ϲ����Ļص��������رյ�����
void KuiTongCreateSheet::buttonCallBackFunc(Ref * pSender)
{
	Node * node = NULL;
	if (pSender)
		node= dynamic_cast<Node *>(pSender);
	//node->setTag(3000);//����tag���ڵ��ò���Ի�ȡ��
	if (m_callfun &&  m_callbackListener)
		(m_callfun)(node);

	this->removeFromParentAndCleanup(true);
}

//�رյ�����
void KuiTongCreateSheet::closePopLayer(Ref * pSender)
{
	this->removeFromParentAndCleanup(true);
}

//��д����ע�ắ�������¸�����������
void KuiTongCreateSheet::registerWithTouchDispatcher()
{
    auto touchListener = ax::EventListenerTouchOneByOne::create();
    touchListener->setSwallowTouches(true);
    ax::Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(touchListener, this);
	//����Ĵ������ȼ�����Ϊ��128����CCMenuͬ������֤�������·��Ĵ���
	//ax::Director::getInstance()->getTouchDispatcher()->addTargetedDelegate(this,128, true);
}

Rect KuiTongCreateSheet::getRect(Node* pNode,int i)
{
	Rect  rc;
	rc.origin     = pNode->getPosition(); //��������ͼ��ƫ��
	//visibleSize
	rc.size       = pNode->getContentSize();

	rc.origin.x   = m_origin.x - m_size.width/2 + rc.origin.x; //��������
	rc.origin.y   = m_origin.y - m_size.height/2+ rc.origin.y; //���ϱ�    //- rc.origin.y

	return rc;
}

void KuiTongCreateSheet::ccTouchEnded(Touch *pTouch, Event *pEvent)
{
	if (!m_bScrolling) return;
	m_bScrolling = false;

	//__loopTime = MAX_LOOP_TIME;
}

//�ƶ���
void KuiTongCreateSheet::ccTouchMoved(Touch *pTouch, Event *pEvent)
{
	if (!m_bScrolling || !__isCanMove) return;
	Point point   = ParentNode_Task->convertToNodeSpace(ax::Director::getInstance()->convertToGL(pTouch->getLocationInView()));
	Point diff    = point - m_lastPoint;                  //����ͼƬ��λ��
	ParentNode_Task->setPosition((ParentNode_Task->getPosition() + diff));
	m_origin      = ParentNode_Task->getPosition();
	m_lastPoint = point;
}
//��������ccTouchBegan������true
bool KuiTongCreateSheet::ccTouchBegan(Touch *pTouch, Event *pEvent)
{
	Point point = ParentNode_Task->convertToNodeSpace(ax::Director::getInstance()->convertToGL(pTouch->getLocationInView()));
	Rect rect = ax::Rect(0, 0, winSize.width/*ParentNode_Task->getContentSize().width*/,winSize.height/* ParentNode_Task->getContentSize().height*/);
	m_bScrolling = rect.containsPoint(point); //����ڷ�Χ�� �Ϳ����϶�
	m_lastPoint = point;
	return true;
}

//--------------------------------------------------------------------------
//	���ܣ����¶�����Ϣ
//--------------------------------------------------------------------------
void KuiTongCreateSheet::update(float delta)
{
	//messageBox("����ѭ��","update");
	if (isOpen && g_pCoreShell)
	{
	}
}
