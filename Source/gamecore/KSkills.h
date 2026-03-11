// KSkills.h: interface for the KSkills class.
//
//////////////////////////////////////////////////////////////////////
#ifndef KSkillsH
#define KSkillsH
#include "engine/KPlatform.h"
#include "KMissle.h"
// #include "KWorldMsg.h"
#include "KRUImage.h"
#include "KNpc.h"
#include "engine/KNode.h"
#include "KMissleMagicAttribsData.h"
#include "Skill.h"
#define MAX_MISSLE_DAMAGEATTRIB 50  // ԭ����15  �˺����ܵ�����
typedef KSkill KFightSkill;
class KMissle;
class KSkill;
// Skill[0]���ã�ÿ��skill��[0]����!!ע�⣡
enum
{
    Missle_StartEvent = 1,
    Missle_FlyEvent,
    Missle_CollideEvent,
    Missle_VanishEvent,
};

// ----------------------------------------------------------------------------
// �������ڿ���һ�����ܵ��ӵ���Ŀ��������Щ���ܣ��ӵ������������
class KNpcMissileList
{
private:
    class KIdxNode : public KNode
    {
    public:
        int m_nIndex;

    public:
        KIdxNode() { m_nIndex = 0; };
    };

    KList m_listIndex;  // �ӵ����У�����˼����������������ӵ�
    INT m_nCount;       // ��ǰ�ӵ�����
    INT m_nNpcIndex;    // ������Npc

public:
    KNpcMissileList(int nNpcIndex);
    virtual ~KNpcMissileList();

    // �ӵ���������ʱ�����
    int OnCreated(int nIndex, int nMaxCount);

    // �ӵ����ŵ�ʱ�����
    int OnVanish(INT nIndex);

private:
    int DelMissile(INT nCount);
};

class KSkill : public ISkill

{
private:
public:
    unsigned int m_ulLevel;
    int m_nEnChance;         // ���ܵļӳ�
    char m_szSkillIcon[80];  // ����ͼ��·��
    char m_szPreCastEffectFile[100];
    char m_szManPreCastSoundFile[100];
    char m_szFMPreCastSoundFile[100];
    eSkillLRInfo m_eLRSkillInfo;  // 0 ��ʾ���Ҽ��Կɣ�1��ʾֻ�������Ҽ����ܣ�2��ʾ���Ҽ���������
    unsigned short m_usReqLevel;  //  ������Ҫ����С�ȼ�
    int getUseAttackRate() { return m_bUseAttackRate; };  //	�Ƿ���������
    int getIsExpSkill() { return m_nIsExpSkill; };
    int getShowAddition() { return m_nShowAddition; };
    int getSkillSeries() const { return m_nSeries; };

private:
    int m_nCharClass;           //	���ĸ�ϵ��
    eSKillStyle m_eSkillStyle;  //  ��ǰ�ļ�������
    // int				m_nSkillTime;			//	�ü��ܵ�����ʱ��
    int m_bClientSend;  //  �ü����Ƿ�Է�������˵��Ч
    // int				m_bHaveLoad;			//	��ʶ�ü����Ƿ��Ѿ����أ����û���������
    int m_nInteruptTypeWhenMove;  //  �ӵ��ļ����Ƿ��ܷ����ߵ��ƶ�����ֹ
    int m_bHeelAtParent;  //	���ӵ�ʵ�ʼ���ʱ��λ�ø��ݸ���ǰλ�ö�ȷ��,�������ɲ����ǿ�parentλ�þ���

    // ����������
    CLIENTACTION m_nCharActionId;  //	���������ʱ������ʲô����
    int m_nWaitTime;               //	�ü����������������������ʱ��
    int m_bIsAura;                 //	�Ƿ�Ϊ�⻷����
    int m_bIsPassivity;            //	�Ƿ�Ϊ��������
    int m_bIsMelee;                //	�Ƿ�Ϊ��������������ʱ
    int m_bUseAttackRate;          //	�Ƿ���������
    int m_bTargetOnly;             //	ֻ��ѡ��Ŀ���ʱ����ܷ���
    int m_bTargetEnemy;            //	���ܵ�Ŀ���Ƿ�Ϊ����
    int m_bTargetAlly;             //	���ܵ�Ŀ���Ƿ�Ϊͬ��
    int m_bTargetObj;              //	���ܵ�Ŀ���Ƿ�Ϊ���
    int m_bTargetSelf;             //	���ܵ�Ŀ���Ƿ����Ϊ����
    int m_bTargetOther;            //	���ܵ�Ŀ����Ϊ�Ի��� ������
    int m_bTargetNoNpc;            //	���ܵ�Ŀ����Ϊ�Ի��� ������
    int m_eRelation;
    char m_szName[32];    //	��������
    unsigned long m_nId;  //	����Id
    int m_nAttrib;        //  �书����
    int m_bBaseSkill;     //	�Ƿ�Ϊ���������
    int m_bByMissle;  //	���ɸ����ܲ���ʱ���Ƿ��Ǹ������Ϊ���㻹���Ե�ǰ���ӵ�Ϊ����
    int m_bIsPhysical;  //	�Ƿ�Ϊ������
    int m_nCost;  //	����ʹ��ʱ����Ҫ���ѵ���������������������Ǯ������
    NPCATTRIB m_nSkillCostType;  //	���ü�������������������ȵ�����
    int m_nMinTimePerCast;       //	���ü��ܵ���С���ʱ��
    int m_nMinTimePerCastOnHorse;                           // �������ܵ�ʱ����
    int m_nChildSkillNum;                                   //	ͬʱ�����Ӽ��ܵ�����
    eMisslesForm m_eMisslesForm;                            //	�ӵ�����״
    int m_nValue1;                                          //	������������1
    int m_nValue2;                                          //	������������2
    int m_nEventSkillLevel;                                 //  �¼��ӵ����ܵȼ�
    KMagicAttrib m_MissleAttribs[MAXSKILLLEVELSETTINGNUM];  // �����ӵ���������ݡ���ͬ�ȼ��ļ��ܣ��ӵ���������Կ��Ա仯��
    int m_nMissleAttribsNum;
    KMagicAttrib m_DamageAttribs[MAX_MISSLE_DAMAGEATTRIB];  // Ӧ�õ���״̬��ص�����
    int m_nDamageAttribsNum;
    KMagicAttrib m_ImmediateAttribs
        [MAXSKILLLEVELSETTINGNUM];  // MAXSKILLLEVELSETTINGNUM��״̬������,�����˺������ܷ����ӵ����жԷ���������Ч���˺�,���漼�ܵȼ��仯���ı�.
    int m_nImmediateAttribsNum;
    KMagicAttrib m_StateAttribs[MAXSKILLLEVELSETTINGNUM];  // Ӧ�õ�״̬��ص�����
    int m_nStateAttribsNum;                                // ���������10
    // eMissleFollowKind   m_eMissleFollowKind;
    // int					m_nFollowKindIndex;
    // unsigned long				m_dwFollowKindID;
    int m_nChildSkillId;  //	�����������Ӽ���Id;	//���ü���Ϊ��������ʱ����������
    int m_nChildSkillLevel;
    int m_bFlyingEvent;  //	�Ƿ���Ҫ�ڷ��й�����Ϣ�����ǣ�������ػص�����
    int m_bStartEvent;  //	�Ƿ���Ҫ�ڼ��ܵ�һ��Activeʱ��������ػص�����
    int m_bCollideEvent;  //	�Ƿ���Ҫ���Ӽ���ħ����ײʱ��������ػص�����
    int m_bVanishedEvent;  //	�Ƿ���Ҫ���Ӽ�������ʱ��������صĻص�����
    int m_nFlySkillId;    //	�� �����е���ؼ���
    int m_nFlyEventTime;  //	ÿ����֡�ص�FlyEvent;
    int m_nShowEvent;     //  �Ƿ���ʾ�ӵ��¼�
    int m_nStartSkillId;  //	���ܸողŷ���ʱ���������¼�ʱ������Ҫ����ؼ���id
    int m_nVanishedSkillId;  //	���ܷ������ӵ�����ʱ�����ļ���Id;
    int m_nCollideSkillId;   //	���ܷ������ӵ���ײ����������ļ���Id;
    int m_bMustBeHit;  //	�Ƿ�Ϊ���м��ܣ������䴦��򵥻��Ա㽵�ͷ���������
    eMisslesGenerateStyle m_eMisslesGenerateStyle;  // ͬʱ���ɵĶ���ӵ���DoWait��ʱ��˳��
    int m_nMisslesGenerateData;                     // �������
    int m_nMaxShadowNum;                            //	������Ĳ�Ӱ��
    int m_bNeedShadow;
    int m_nAttackRadius;      // ���
    int m_nBackAttackRadius;  // ���ݵĹ�������
    int m_nStateSpecialId;    // ״̬��ħ���⻷id��
    // int				    m_nMaxTimes;			//ĳЩ������ħ�������ͬʱ���ֶ��ٸ�����

    int m_nEquiptLimited;  // ������ʱ�Ե�ǰװ��������
    int m_nIsExpSkill;     // �Ƿ����������ȵļ���
    int m_nSKillExp;       // ��ǰ�ȼ��ĸü��ܵ���������
    int m_nHorseLimited;   // �������� 0��ʾû�κ�����
                          //		   1��ʾ��������
                          ////		   2��ʾ��������
    int m_bDoHurt;  // �˺�����

    int m_bIsSkillPhysical;  // ������
    int m_bIsSkillMagic;     // ħ������

    int m_nIsMagic;  // ���⹥
    int m_nSeries;
    int m_nShowAddition;

public:
    KSkill();
    virtual ~KSkill();
    // ���������g_OrdinSkillsSetting���к�
    // int
    int OnMissleEvent(unsigned short usEvent, KMissle* pMissle) const;
    int IsTargetOnly() const { return m_bTargetOnly; };
    int IsTargetEnemy() const { return m_bTargetEnemy; };
    int IsTargetSelf() const { return m_bTargetSelf; };
    int IsTargetAlly() const { return m_bTargetAlly; };
    int IsTargetObj() const { return m_bTargetObj; };
    int IsTargetOther() const { return m_bTargetOther; };
    int IsNeedShadow() const { return m_bNeedShadow; };
    // int					GetMaxTimes()const{return m_nMaxTimes;};
    static int GetSkillIdFromName(char* szSkillName);
    unsigned short GetSkillReqLevel() const { return m_usReqLevel; };

    int GetParam1() const { return m_nValue1; };
    int GetParam2() const { return m_nValue2; };

    eSkillLRInfo GetSkillLRInfo() const { return m_eLRSkillInfo; };
    const char* GetPreCastEffectFile() const { return m_szPreCastEffectFile; };
    const char* GetPreCastSoundFile(int bIsFeMale) const
    {
        return bIsFeMale ? m_szFMPreCastSoundFile : m_szManPreCastSoundFile;
    };

    const char* GetSkillName() { return m_szName; };
    int GetSkillStyle() { return m_eSkillStyle; };
    int GetInfoFromTabFile(int nCol);
    int GetInfoFromTabFile(KTabFile* pSkillsSettingFile, int nRow);
    int Load(int nCol);
    //------------------�����ܽӿ�
    int Cast(int nLauncher,
             int nParam1,
             int nParam2,
             int nWaitTime                    = 0,
             eSkillLauncherType eLauncherType = SKILL_SLT_Npc,
             int nMaxShangHai                 = 0,
             int nIsEuq                       = 0) const;  // ����ʱ����
    //---------------------------
    int GetChildSkillNum(int nLevel) const;
    int GetAttackRadius() const { return m_nAttackRadius; };
    int ChangeAttackRadius(int nDis)
    {
        if (nDis >= m_nBackAttackRadius)
        {
            m_nAttackRadius = m_nBackAttackRadius;
            return m_nAttackRadius;
        }

        if (m_nAttackRadius < 50)
            m_nAttackRadius = m_nBackAttackRadius;
        return m_nAttackRadius;
    };

    int getBackAttackRadius() { return m_nBackAttackRadius; };
    int CalculateDamage() const;
    void LoadSkillLevelData(unsigned int ulLevel, int nParam);  // �Ӽ��������ļ��л��nLevel�ȼ������Ա䶯��������
    friend class KMissle;
    int GetSkillCost(void*) const { return m_nCost; };
    NPCATTRIB GetSkillCostType() const { return m_nSkillCostType; };
    int GetItemLimit(int) const;
    CLIENTACTION GetActionType() const { return m_nCharActionId; };
    int GetDelayPerCast() const { return m_nMinTimePerCast; };         // ���ü��ܵ���С�ⶳʱ��
    int GetHorsePerCast() const { return m_nMinTimePerCastOnHorse; };  // �����ü��ܵ���С�ⶳʱ��
    unsigned int GetMissleGenerateTime(int nNo) const;
    int GetChildSkillNum() const { return m_nChildSkillNum; };
    int GetChildSkillId() const { return m_nChildSkillId; };
    int GetChildSkillLevel() const { return m_nChildSkillLevel; };

    int GetSkillId() { return m_nId; };
    void SetSkillId(int nId) { m_nId = nId; };
    void SetSkillLevel(unsigned int ulLevel) { m_ulLevel = ulLevel; };

    int IsAura() const { return m_bIsAura; };
    int IsPhysical() const { return m_bIsPhysical; };
    int IsMagic() const { return m_nIsMagic; };
    int IsAttrib() const { return m_nAttrib; };
    int GetEquiptLimited() const { return m_nEquiptLimited; };  // ��ȡ������������
    int GetIsExpSkill() const { return m_nIsExpSkill; };        // ��ȡ���ܵ���������
    int GetSkillExp() const { return m_nSKillExp; };            // ��ȡ���ܵ���������
    int GetHorseLimited() const { return m_nHorseLimited; };    // ��ȡ��������
    int GetMeleeType() const { return m_eMisslesForm; };
    int GetStateSpecailId() const { return m_nStateSpecialId; };        // ���ܹ⻷��״̬ID��
    int CanCastSkill(int nLauncher, int& nParam1, int& nParam2) const;  //
    int NewCanCastSkill(int nLauncher, int nParam1, int nParam2);
    void PlayPreCastSound(int bIsFeMale, int nX, int nY) const;
    KMagicAttrib* GetDamageAttribs() { return m_DamageAttribs; };
    KMagicAttrib* GetStateAttribs() { return m_StateAttribs; };
    void SetStateTpye(int i, int inValAttribsTpye) { m_StateAttribs[i].nAttribType = inValAttribsTpye; };
    void SetStateVala(int i, int inValA) { m_StateAttribs[i].nValue[0] = inValA; };
    void SetStateValb(int i, int inValB) { m_StateAttribs[i].nValue[1] = inValB; };
    void SetStateValc(int i, int inValC) { m_StateAttribs[i].nValue[2] = inValC; };

    void EnChanceSkill(int nLauncher);
    int CastStateSkill(int nLauncher, int nParam1, int nParam2, int nWaitTime) const;
    int CastToolStateSkill(int nLauncher, int nParam1, int nParam2, int nWaitTime) const;
    int CastSkill(int nLauncher, int nSkillID, int nParam1, int nParam2, int nLevel, int nWaitTime, int nMaxBei = 0)
        const;

private:
    void Vanish(KMissle*) const;
    void FlyEvent(KMissle*) const;
    void Collidsion(KMissle*) const;
    int CastMissles(int nLauncher,
                    int nParam1,
                    int nParam2,
                    int nWaitTime                    = 0,
                    eSkillLauncherType eLauncherType = SKILL_SLT_Npc,
                    int nMaxShangHai                 = 0) const;
    int CastInitiativeSkill(int nLauncher, int nParam1, int nParam2, int nWaitTime, int nMaxShangHai = 0) const;
    int CastPassivitySkill(int nLauncher, int nParam1, int nParam2, int nWaitTime, int nIsEuq = 0) const;
    ////////////////////�ӵ��˺�����//////////////
    int __CastFixed(int nLauncher,
                    int nParam1,
                    int nParam2,
                    int nWaitTime,
                    eSkillLauncherType eLauncherType,
                    int nMaxShangHai) const;  // ����
    int __CastWall(int nLauncher,
                   int nParam1,
                   int nParam2,
                   int nWaitTime,
                   eSkillLauncherType eLauncherType,
                   int nMaxShangHai) const;  // ǽ��
    int __CastLine(int nLauncher,
                   int nParam1,
                   int nParam2,
                   int nWaitTime,
                   eSkillLauncherType eLauncherType,
                   int nMaxShangHai) const;  // ����
    int __CastSpread(int nLauncher,
                     int nParam1,
                     int nParam2,
                     int nWaitTime,
                     eSkillLauncherType eLauncherType,
                     int nMaxShangHai) const;  // ����
    int __CastCircle(int nLauncher,
                     int nParam1,
                     int nParam2,
                     int nWaitTime,
                     eSkillLauncherType eLauncherType,
                     int nMaxShangHai) const;  // ����
    int __CastZone(int nLauncher,
                   int nParam1,
                   int nParam2,
                   int nWaitTime,
                   eSkillLauncherType eLauncherType,
                   int nMaxShangHai) const;  // ��������
    int __CastRound(int nLauncher,
                    int nParam1,
                    int nParam2,
                    int nWaitTime,
                    eSkillLauncherType eLauncherType,
                    int nMaxShangHai) const;  // Բ������

    int CastWall(TOrdinSkillParam* pSkillParam, int nDir, int nRefPX, int nRefPY, int nMaxShangHai = 0) const;
    int CastLine(TOrdinSkillParam* pSkillParam, int nDir, int nRefPX, int nRefPY, int nMaxShangHai = 0) const;
    int CastSpread(TOrdinSkillParam* pSkillParam, int nDir, int nRefPX, int nRefPY, int nMaxShangHai = 0) const;
    int CastCircle(TOrdinSkillParam* pSkillParam, int nDir, int nRefPX, int nRefPY, int nMaxShangHai = 0) const;
    int CastZone(TOrdinSkillParam* pSkillParam, int nDir, int nRefPX, int nRefPY, int nMaxShangHai = 0) const;
    int CastNotWall(TOrdinSkillParam* pSkillParam, int nDir, int nRefPX, int nRefPY, int nMaxShangHai = 0) const;
    int CastExtractiveLineMissle(TOrdinSkillParam* pSkillParam,
                                 int nDir,
                                 int nSrcX,
                                 int nSrcY,
                                 int nXOffset,
                                 int nYOffset,
                                 int nDesX,
                                 int nDesY,
                                 int nMaxShangHai = 0) const;
    int Param2PCoordinate(int nLauncher,
                          int nParam1,
                          int nParam2,
                          int* npPX,
                          int* npPY,
                          eSkillLauncherType eLauncherType = SKILL_SLT_Npc) const;  // ת������
    //////////////////////////////////////////////

    void CreateMissle(int nLauncher, int ChildSkillId, int nMissleIndex) const;

    int ParseString2MagicAttrib(unsigned int ulLevel,
                                char* szMagicAttribName,
                                char* szValue);  // ����ͨ���ű������õļ�������
    inline const char* MagicAttrib2String(int MagicAttrib) const;

public:
    std::string getIconPath() { return m_szSkillIcon; };
    void DrawSkillIcon(int x, int y, int Width, int Height, int nParam = -1);
    KRUImage m_RUIconImage;
    char m_szSkillDesc[300];  // ���ܵ�����
    static void GetDesc(unsigned int ulSkillId,
                        unsigned int ulCurLevel,
                        char* pszMsg,
                        int nOwnerIndex,
                        bool bGetNextLevelDesc,
                        int nAddPoint,
                        int nEnChance);
    void GetDescAboutLevel(char* pszMsg);
    char m_szDesc[512];  // ���ܵ�����
};

extern int g_nMeleeWeaponSkill[MAX_MELEEWEAPON_PARTICULARTYPE_NUM];
extern int g_nRangeWeaponSkill[MAX_RANGEWEAPON_PARTICULARTYPE_NUM];
extern int g_nHandSkill;

#endif
