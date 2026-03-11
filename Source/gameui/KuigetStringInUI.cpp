#include "engine/KPlatform.h"
#include "gamecore/KCore.h"
#include "engine/CXmlStream.h"
#include "KuigetStringInUI.h"
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

KuigetStringInUI::KuigetStringInUI()
{
	isOpen = false;
	//memset(Skills,0,sizeof(KUiSkillData) * 50);
	ptitleLabel       = NULL;
	//pXuLabel          = NULL;
	colorLayer        = NULL;
	red               = NULL;
	pMoneyEditBox     = NULL;
	//m_nMoney = 0;
	//m_nXu = 0;
	//ZeroMemory(strTitle,sizeof(strTitle));
	ZeroMemory(strCallBackfun,sizeof(strCallBackfun));
	ZeroMemory(strErrorfun,sizeof(strErrorfun));
	minMsgLen =1;
	maxMsgLen =256;
	m_StartPos.x=0;
	m_StartPos.y=0;
	inPutKind   =0;
}

KuigetStringInUI::~KuigetStringInUI()
{
	isOpen = false;
	//pMoneyLabel  = NULL;
	colorLayer        = NULL;
	red->release();
	//red               = NULL;
}

//����ȷ��������
KuigetStringInUI* KuigetStringInUI::create(const char* pszTitle,
                                           int nMinLen,
                                           int nMaxLen,
                                           const char* pszCallback,
                                           const char* pszError,
                                           int bNum,
                                           Ref* callbackListener,
                                           const std::function<void(ax::Node*)>& callfun,
                                           int outKind)
{
	KuigetStringInUI * popLayer = KuigetStringInUI::create();
	//popLayer->setTitle(title);
	popLayer->inPutKind =bNum;
	popLayer->minMsgLen =nMinLen;
	popLayer->maxMsgLen =nMaxLen;
	popLayer->outPutKind=outKind;
	popLayer->addDialogData(pszTitle);
	//���ð�ť
	popLayer->setcoloseButton(callbackListener,callfun);
	popLayer->isOpen = true;
	if (!outKind)
	{
		sprintf(popLayer->strCallBackfun, "%s", pszCallback);
		sprintf(popLayer->strErrorfun, "%s", pszError);
	}

	return popLayer;
}

//��ʼ��
bool KuigetStringInUI::init()
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
	//���������ı���ͼƬ������������λ��Ϊ��Ļ�е�
	//SpriteFrame * f_tips_box = sfCache->spriteFrameByName("ui/state/skill.png");
	//Sprite * spriteBox = Sprite::create("ui/state/skill.png");//Sprite::createWithSpriteFrame(f_tips_box);
	//\\spr\\Ui3\\������\\���������.spr
	char nSprName[128]={0};
	ZeroMemory(nSprName,sizeof(nSprName));
	sprintf(nSprName,"\\spr\\Ui3\\����Ի���\\����������2.spr");
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
	    //��ֹ��㴥��
	return true;
}

void KuigetStringInUI::addDialogData(const char* pszTitle)
{
	if  (!g_pCoreShell) return;

	//Left=24
	//Top=72
	m_StartPos.x =24;//+ITEM_CELL_SIZE/2;
	m_StartPos.y =72;//+ITEM_CELL_SIZE/2;

    ptitleLabel = Label::createWithTTF(UTEXT(pszTitle,1).c_str(),"fonts/gb_song.ttf", 12);//Arial
	ptitleLabel->setAnchorPoint(ax::Vec2(0,0));
	ptitleLabel->setPosition(ax::Vec2(5,m_size.height-20));
	ParentNode_ItemEx->addChild(ptitleLabel,1);
	/*
	pXuLabel = Label::createWithTTF("","fonts/gb_song.ttf", 12);//Arial
	pXuLabel->setAnchorPoint(ax::Vec2(0,0));
	pXuLabel->setPosition(ax::Vec2(60,105));
	ParentNode_ItemEx->addChild(pXuLabel,1);*/
	pMoneyEditBox = EditBox::create(Size(112,24),ax::ui::Scale9Sprite::create()); //"ui/statelife.png"
	const ax::Color3B nfcolor ={255,255,255};
	if (pMoneyEditBox)
	{
		//pEditBox->setContentSize()
		//pMoneyEditBox->setText("your money");
		//pMoneyEditBox->setPlaceHolder("money");
		pMoneyEditBox->setFontColor(nfcolor);
		pMoneyEditBox->setFontSize(24);
		pMoneyEditBox->setMaxLength(maxMsgLen);
		pMoneyEditBox->setPosition(ax::Vec2(57,50));
		if (inPutKind<=1)
		   pMoneyEditBox->setInputMode(EditBox::InputMode::DECIMAL);
		else
		   pMoneyEditBox->setInputMode(ax::ui::EditBox::InputMode::ANY);

		pMoneyEditBox->setReturnType(ax::ui::EditBox::KeyboardReturnType::DONE);
		pMoneyEditBox->setDelegate(this);
		//pMoneyEditBox->setDefaultTouchPriority(-128);

		/*Layout *custom_item = Layout::create();
		custom_item->setContentSize(Size(112,24));
		custom_item->addChild(pMoneyEditBox);
		*/
		ParentNode_ItemEx->addChild(pMoneyEditBox);
	}

	Color4B color(112, 128, 144, 150);
	colorLayer = LayerColor::create(color);//��ɫ��
	colorLayer->setPosition(ax::Vec2::ZERO);
	colorLayer->setContentSize(ax::Size::ZERO);//���úʹ��ڵĴ�С
	//colorLayer->addChild(pMoneyEditBox);
	ParentNode_ItemEx->addChild(colorLayer,1000);
	//����һ������ �ı侫����ɫ
	red = TintBy::create(0.2,0,-255,-255);//����ʱ��+��ɫ
	red->retain();

	////ax::Director::getInstance()->getTouchDispatcher()->addTargetedDelegate(this,-128,true);
}


//����ȷ��ȡ����ť,���������ò���󣬵��ò�ص�����
void KuigetStringInUI::setcoloseButton(Ref * callbackListener,const std::function<void(ax::Node*)>& callfun)
{
	m_callbackListener = callbackListener;
	m_callfun = callfun;

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
	if (outPutKind >=1)
	  okConfirm = MenuItemSprite::create(sprite_ok_btn_confirm_normal, sprite_ok_btn_confirm_select, CC_CALLBACK_1(KuigetStringInUI::_getstringgCallBackFunc, this));
	else
	  okConfirm = MenuItemSprite::create(sprite_ok_btn_confirm_normal, sprite_ok_btn_confirm_select, CC_CALLBACK_1(KuigetStringInUI::getstringgCallBackFunc, this));

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
	MenuItemSprite * closeConfirm = MenuItemSprite::create(sprite_close_normal, sprite_close_select, CC_CALLBACK_1(KuigetStringInUI::buttonCallBackFunc, this));
	closeConfirm->setPosition(ax::Vec2(28,11));

	//Point bgSpritePoint = ParentNode_ItemEx->getPosition();
	Menu * menu = Menu::create(okConfirm,closeConfirm,NULL);
	menu->setPosition(ax::Vec2(m_size.width/2,3));  //�˵�λ������Ϊ����������
	ParentNode_ItemEx->addChild(menu);         //�ӵ�������
}

void KuigetStringInUI::_getstringgCallBackFunc(Ref * pSender)
{
	std::string strText;
	if (pMoneyEditBox)
	{
		strText = pMoneyEditBox->getText();
		if (strText.length()>maxMsgLen || strText.length()< minMsgLen)
		{
#ifdef WIN32
			messageBox("����Ϊ�ջ����","����");
#else
			messageBox(G2U("����Ϊ�ջ����").c_str(),G2U("����").c_str());
#endif
			return;
		}
	}
	else
	{
#ifdef WIN32
		messageBox("�������󣮣�","����");
#else
		messageBox(G2U("�������󣮣�").c_str(),G2U("����").c_str());
#endif
		return;
	}
	//��ʼ����ַ��ĺϷ���
	std::string strcheck="1";
	strcheck+=strText;

	int count = 0,spaceNum=0,chineseNum=0,other=0,letter=0,xiaoxie=0;
	for(int i = 0; i <strcheck.length();i++)
	{
		//  if(str[i] < 0 && str[i] > 9);
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
		else if(strcheck[i]&0x80)//if(strcheck[0]>=0xB0)
			++chineseNum;    //�ж��Ƿ�������
		else
		{//���ź���ע�� ������������
			++other;
		}
	}

	if (inPutKind==1)
	{//����
		if (spaceNum>0 || xiaoxie>0|| chineseNum >0 || other>0 || letter>0)
		{

#ifdef WIN32
			messageBox("���зǷ��ַ�,����\nֻ����������!","����");
#else
			messageBox(G2U("���зǷ��ַ�,����\nֻ����������!").c_str(),G2U("����").c_str());
#endif
			return;
		}
	}
	else if (inPutKind>=2)
	{//�ַ���
		if (spaceNum>0)
		{

#ifdef WIN32
			messageBox("���зǷ��ַ�,����\n��������ո�!","����");
#else
			messageBox(G2U("���зǷ��ַ�,����\n��������ո�!").c_str(),G2U("����").c_str());
#endif
			return;
		}
	}
	//��ʼ����
	Node * node =	NULL;

	if (pSender)
	{
		node = dynamic_cast<Node *>(pSender);
        int nikey = std::hash<std::string>{}(strText);
		node->setTag(nikey);//����tag���ڵ��ò���Ի�ȡ��
	}

	if (m_callfun &&  m_callbackListener)
	{
		//ִ�е��ò�ص����������ݲ���Node
		(m_callfun)(node);
	}
	this->removeFromParentAndCleanup(true);


}

void KuigetStringInUI::getstringgCallBackFunc(Ref * pSender)
{
	std::string strText;
	if (pMoneyEditBox)
	{
		strText = pMoneyEditBox->getText();
		if (strText.length()>maxMsgLen || strText.length()< minMsgLen)
		{
#ifdef WIN32
			messageBox("����Ϊ�ջ����","����");
#else
			messageBox(G2U("����Ϊ�ջ����").c_str(),G2U("����").c_str());
#endif
			return;
		}
	}
	else
	{
#ifdef WIN32
		messageBox("�������󣮣�","����");
#else
		messageBox(G2U("�������󣮣�").c_str(),G2U("����").c_str());
#endif
		return;
	}
	//��ʼ����ַ��ĺϷ���
	std::string strcheck="1";
	strcheck+=strText;

	int count = 0,spaceNum=0,chineseNum=0,other=0,letter=0,xiaoxie=0;
	for(int i = 0; i <strcheck.length();i++)
	{
		//  if(str[i] < 0 && str[i] > 9);
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
		else if(strcheck[i]&0x80)//if(strcheck[0]>=0xB0)
			++chineseNum;    //�ж��Ƿ�������
		else
		{//���ź���ע�� ������������
			++other;
		}
	}

	PLAYER_STRINGUI pSRING;
	sprintf(pSRING.m_SCallback,"%s",strCallBackfun);
	sprintf(pSRING.m_SError,"%s",strErrorfun);
	pSRING.m_nNum=inPutKind;
	char nContent[125]={0};
	if (inPutKind==1)
	{//����
		if (spaceNum>0 || xiaoxie>0|| chineseNum >0 || other>0 || letter>0)
		{

#ifdef WIN32
			messageBox("���зǷ��ַ�,����\nֻ����������!","����");
#else
			messageBox(G2U("���зǷ��ַ�,����\nֻ����������!").c_str(),G2U("����").c_str());
#endif
			return;
		}

		sprintf(pSRING.m_Content,"%s",strText.c_str());
	}
	else if (inPutKind>=2)
	{//�ַ���
		if (spaceNum>0)
		{

#ifdef WIN32
			messageBox("���зǷ��ַ�,����\n��������ո�!","����");
#else
			messageBox(G2U("���зǷ��ַ�,����\n��������ո�!").c_str(),G2U("����").c_str());
#endif
			return;
		}
		sprintf(pSRING.m_Content,"%s",strText.c_str());
	}
	else
	{
		buttonCallBackFunc(NULL);
		return;
	}

	if (g_pCoreShell)
	   g_pCoreShell->OperationRequest(GOI_STRINGBOX,(uintptr_t)&pSRING, pSRING.m_nNum);

	buttonCallBackFunc(NULL);
}
//ִ���ϲ����Ļص��������رյ�����
void KuigetStringInUI::buttonCallBackFunc(Ref * pSender)
{
	Node * node =	NULL;

	if (pSender)
	{
		Node * node = dynamic_cast<Node *>(pSender);
		node->setTag(3000);//����tag���ڵ��ò���Ի�ȡ��
	}

	if (m_callfun &&  m_callbackListener)
	{
		//ִ�е��ò�ص����������ݲ���Node
		(m_callfun)(node);
	}
	this->removeFromParentAndCleanup(true);
}

//�رյ�����
void KuigetStringInUI::closePopLayer(Ref * pSender)
{
	this->removeFromParentAndCleanup(true);
}


//��д����ע�ắ�������¸�����������
void KuigetStringInUI::registerWithTouchDispatcher()
{
    auto touchListener = ax::EventListenerTouchOneByOne::create();
    touchListener->setSwallowTouches(true);
    ax::Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(touchListener, this);
	//����Ĵ������ȼ�����Ϊ��128����CCMenuͬ������֤�������·��Ĵ���
	//ax::Director::getInstance()->getTouchDispatcher()->addTargetedDelegate(this,2,true);
}

Rect KuigetStringInUI::getRect(Node* pNode,int i)
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
bool KuigetStringInUI::ccTouchBegan(Touch *pTouch, Event *pEvent)
{
	return true;
	//CCSetIterator it = pTouch->begin();
	//Touch* touch = (Touch*)(*it);
	//containsPoint(convertTouchToNodeSpaceAR(touch))
	Point location = pTouch->getLocation();//Ŀ�ĵ�����?��Ļ�����
	return true;
}

void KuigetStringInUI::update(float delta)
{
	//messageBox("����ѭ��","update");
	if (isOpen && g_pCoreShell)
	{
	    /*m_nMoney = g_pCoreShell->GetGameData(GDI_PLAYER_HOLD_MONEY, 0, 0);
		m_nXu    = g_pCoreShell->GetGameData(GDI_PLAYER_HOLD_XU, 0, 0);
		char nPointCount[32];
	   if (pMoneyLabel)
	   {
		   sprintf(nPointCount,"%d",m_nMoney);
		  // pSkillPointLabel->setColor(ccWit)
		   pMoneyLabel->setString(nPointCount);
	   }
	   if  (pXuLabel)
	   {
		   sprintf(nPointCount,"%d",m_nXu);
		   // pSkillPointLabel->setColor(ccWit)
		   pXuLabel->setString(nPointCount);
	   }*/
	}
}

void KuigetStringInUI::editBoxEditingDidBegin(EditBox* editBox)
{
	CCLOG("editBox %p DidBegin !", editBox);
}

void KuigetStringInUI::editBoxEditingDidEnd(EditBox* editBox)
{
	CCLOG("editBox %p DidEnd !", editBox);
}

void KuigetStringInUI::editBoxTextChanged(EditBox* editBox, const std::string& text)
{
	CCLOG("editBox %p TextChanged, text: %s ", editBox, text.c_str());
}

void KuigetStringInUI::editBoxReturn(EditBox* editBox)
{
	CCLOG("editBox %p was returned !",editBox);

	/*if (m_pEditName == editBox)
	{
		m_pTTFShowEditReturn->setString("Name EditBox return !");
	}
	else if (m_pEditPassword == editBox)
	{
		m_pTTFShowEditReturn->setString("Password EditBox return !");
	}
	else if (m_pEditEmail == editBox)
	{
		m_pTTFShowEditReturn->setString("Email EditBox return !");
	}*/
}
