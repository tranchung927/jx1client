#include "engine/KPlatform.h"
#include "gamecore/KCore.h"
#include "engine/CXmlStream.h"
#include "KuiMylistView.h"
#include "KuiItem.h"
#include "engine/KIniFile.h"
#include "gamecore/CoreShell.h"
#include "gamecore/KPlayer.h"
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
#include "gameui/KuiSkilldesc.h"
#include "gameui/KCheckbox.h"
extern iCoreShell*	g_pCoreShell;
#include <list>
USING_NS_AX;

KuiMylistView::KuiMylistView()
{
	isFollowRole =false;
	active = false;
	isRun  = false;
	radius = 60;
	nRolePoint=ax::Vec2(0,0);
	nRoleIndex = 0;

	jsSprite=NULL;
	bgSprite=NULL;
	_BackAttackRadius=0;
	_CurSkillId=0;
	_Beilv=0;
	_useSkillModel=0;
}

KuiMylistView::~KuiMylistView()
{
}

//�ڶ������������������֮��ļн�
//��Ϊ����ĽǶ����Ԧ�Ϊ��λ����������Ҫת�����Զ�Ϊ��λ
float KuiMylistView::getAngleSigned()
{//ccpSub(centerPoint, currentPoint),CCPointMake(0, 1))
    float rad = getRad(centerPoint,currentPoint);

	return 180.f/3.141592654*rad;

}

float KuiMylistView::getRad(Point p1,Point p2)
{
	float xx = p2.x - p1.x;
	float yy = p2.y - p1.y;
	// б��
	float xie = sqrt(pow(xx, 2) + pow(yy, 2));
	// yy >= 0 �������� 0 �� �� ֮�䡣(0~180��)
	// yy < 0 �������� �� �� 2�� ֮�䡣(180��~360��)
	float rad = yy >= 0 ? (acos(xx / xie)) : (3.141592654 * 2 - acos(xx / xie));
	return rad;
}


KuiMylistView * KuiMylistView::create()
{
	KuiMylistView *widget = new KuiMylistView();
	if (widget && widget->init())
	{
		widget->autorelease();

		//widget->setTouchMode(kCCTouchesOneByOne);
		widget->setTouchEnabled(true);
		widget->setEnabled(true);
		widget->setIgnoreAnchorPointForPosition(true);
		widget->setAnchorPoint(ax::Vec2(0.5f, 0.5f));
		return widget;
	}
	CC_SAFE_DELETE(widget);
	return NULL;
}


KuiMylistView* KuiMylistView::initWithCenter(){

	visibleSize = ax::Director::getInstance()->getVisibleSize();//�ɼ���
	//pCoreShell = NULL;
	//__GameWorld = NULL;
	moveEndPoint = ax::Vec2(0,0);

	if(!isFollowRole){
		centerPoint =ax::Vec2(visibleSize.width,-100.0f);
	}else{
		centerPoint =ax::Vec2(visibleSize.width,-100.0f);  //��������
	}
	currentPoint = centerPoint; //�ƶ���ǰ������

	/*Color4B color(112, 128, 144, 150);
	colorLayer = LayerColor::create(color);//��ɫ��
	colorLayer->setContentSize(ax::Size(200,200));//���úʹ��ڵĴ�С
	colorLayer->setPosition(CCPointMake(visibleSize.width-200,10));
	colorLayer->setAnchorPoint(ax::Vec2(0,0));
	//colorLayer->addChild(spriteBox);
	this->addChild(colorLayer);*/

    jsSprite = Sprite::create("useskill_z.png");
	jsSprite->setPosition(centerPoint);//����spr
	bgSprite = Sprite::create("useskill_b.png");
	bgSprite->setPosition(centerPoint);   //����spr
	//bgSprite->setTag(1000);
	//bgSprite->retain();
	//jsSprite->retain();
	this->addChild(bgSprite);
	this->addChild(jsSprite);
	if(isFollowRole){
		//this->setVisible(false);
		setSpriteVisible(false);
	}
	 //��ֹ��㴥��
	return this;
}


/*KuiMylistView* KuiMylistView::menuWithItem(MenuItem* item)
{
	return create(item, NULL);
}*/


// ��дregisterWithTouchDispatcher
void KuiMylistView::registerWithTouchDispatcher()
{
	//�������ȼ���Ϊ1��ֻҪ��CCScrollView��tableView��CCControlButton�ȣ���Щ���ȼ���Ϊ0���;Ϳ���
	////ax::Director::getInstance()->getTouchDispatcher()->addTargetedDelegate(this,-129,true);
	//Director* pDirector = ax::Director::getInstance();
	//pDirector->getTouchDispatcher()->addTargetedDelegate(this, this->getTouchPriority(),true);//false
}


bool KuiMylistView::ccTouchBegan(Touch* touch, Event* event)
{
	if (g_GameWorld && g_pCoreShell && g_pCoreShell->getTradeState()) return false;

	_useSkillModel=g_GameWorld->getUseSkillModel();//0 Ϊ�Զ� 1 Ϊ�ֶ�

	return true;
}

void KuiMylistView::ccTouchMoved(Touch* touch, Event* event)
{

	if (g_GameWorld && g_pCoreShell && g_pCoreShell->getTradeState()) return;
	//if (pCoreShell) pCoreShell->setPadCanMove(PAD_KIND_ITEM,false);
	//int nBeilv = 2;
	Point touchPoint = touch->getLocationInView();                     //��Ļ����ϵ +x ���� +������
    touchPoint = Director::getInstance()->convertToUI(touchPoint); //ת�������½�gl�������(0,0)

	float nDistance = touchPoint.distance(centerPoint);
	if (nDistance > radius)
	{//������Χ
        currentPoint = centerPoint + ((touchPoint - centerPoint).getNormalized() * radius);
	}else {
		currentPoint = touchPoint;
	}
}

void KuiMylistView::ccTouchEnded(Touch* touch, Event* event)
{

	if (g_GameWorld && g_pCoreShell && g_pCoreShell->getTradeState()) return;
	currentPoint = centerPoint;
	isRun = true;
}

void KuiMylistView::ccTouchCancelled(Touch *touch, Event* event)
{

}


void  KuiMylistView::setSpriteOpacity(int ver)
{
	if (jsSprite)
		jsSprite->setOpacity(ver);
	if (bgSprite)
		bgSprite->setOpacity(ver);
}

void  KuiMylistView::setSpriteVisible(bool ver)
{
	if (jsSprite)
		jsSprite->setVisible(ver);
	if (bgSprite)
		bgSprite->setVisible(ver);

}

void  KuiMylistView::setSpritePiont(Point aPoint)
{
	centerPoint  = aPoint;
	currentPoint = centerPoint;              //�ƶ���ǰ������
	if (jsSprite)
		jsSprite->setPosition(centerPoint);   //����spr
	if (bgSprite)
		bgSprite->setPosition(centerPoint);   //����spr
}
