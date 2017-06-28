#ifndef __CTP_QUOTATION_PROTOCAL_CTP_DL_H__
#define	__CTP_QUOTATION_PROTOCAL_CTP_DL_H__
#pragma pack(1)


typedef struct
{
	char						Code[20];					///< ��Լ����
	char						Name[64];					///< ��Լ����
	unsigned char				Kind;						///< ֤ȯ����
	char						PreName[4];					//���ǰ׺[�������г����Ϻ��г�����]
	unsigned int				LotSize;					///< һ�ֵ��ڼ��ź�Լ
	char						UnderlyingCode[20];			///< ���֤ȯ����
	unsigned int				ContractMult;				///< ��Լ����
	unsigned int				XqPrice;					///< ��Ȩ�۸�[*�Ŵ���]
	unsigned int				StartDate;					///< �׸�������(YYYYMMDD)
	unsigned int				EndDate;					///< �������(YYYYMMDD)
	unsigned int				XqDate;						///< ��Ȩ��(YYYYMM)
	unsigned int				DeliveryDate;				///< ������(YYYYMMDD)
	unsigned int				ExpireDate;					///< ������(YYYYMMDD)
	unsigned short				TypePeriodIdx;				///< ���ཻ��ʱ���λ��
	unsigned char				EarlyNightFlag;             ///< ����orҹ�̱�־ 1������ 2��ҹ�� 
	unsigned int				PriceTick;					///< ��С�䶯��λ
} tagSHL1ReferenceData_LF100;


typedef struct
{
	char						Code[20];					//���ƴ����
	unsigned short				MKind;						//����
	unsigned char				CType;						//��ϸ���ί��ʹ�õķ���
	unsigned char				BType;						//�������A��B�ɵ�
	unsigned char				SFlag;						//ͣ�Ʊ�ǣ��Ƿ�ɽ��ף���Ҫ����
	unsigned char				PLimit;						//�ǡ���ͣ�Ƿ���Ч
	unsigned int				Worth;						//ÿ����ֵ��*�Ŵ���10000
	unsigned int				ExTts;						//���׵�λ
	unsigned int				MaxExVol;					//���������
	unsigned int				MinExVol;					//��С��������
} tagSHL1ReferenceExtension_LF101;


typedef struct
{
	char						Code[20];					///< ��Լ����
	unsigned int				Open;						///< ���̼�[*�Ŵ���]
	unsigned int				Close;						///< ���ռ�[*�Ŵ���]
	unsigned int				High;						///< ������ͣ�۸�[*�Ŵ���], 0��ʾ������
	unsigned int				Low;						///< ���յ�ͣ�۸�[*�Ŵ���], 0��ʾ������
} tagSHL1SnapData_LF102;


typedef struct
{
	char						Code[20];					///< ��Լ����
	unsigned int				Now;						///< ���¼�[*�Ŵ���]
	unsigned int				Voip;						///< ����ģ�⾻ֵ��Ȩ֤���[�Ŵ�1000��][ETF������LOF��Ȩ֤��ֵ��Ч]
	double						Amount;						///< �ܳɽ����[Ԫ]
	unsigned __int64			Volume;						///< �ܳɽ���[��/��]
} tagSHL1SnapData_HF103;


typedef struct
{
	unsigned int				Price;						///< ί�м۸�[* �Ŵ���]
	unsigned __int64			Volume;						///< ί����[��]
} tagBuySellItem;


typedef struct
{
	char						Code[20];					///< ��Լ����
	tagBuySellItem				Buy[5];						///< ���嵵
	tagBuySellItem				Sell[5];					///< ���嵵
} tagSHL1SnapBuySell_HF104;


typedef struct
{
	char						Key[32];					///< ������ֵ
	unsigned int				MarketDate;					///< �г�����
	unsigned int				MarketTime;					///< �г�ʱ��
	unsigned char				MarketStatus;				///< �г�״̬[0��ʼ�� 1������]
} tagSHL1MarketStatus_HF105;


typedef struct
{
	char						Key[32];					///< ������ֵ
	unsigned int				MarketID;					///< �г����
	unsigned int				KindCount;					///< �������
	unsigned int				WareCount;					///< ��Ʒ����
	unsigned int				PeriodsCount;				///< ����ʱ����Ϣ�б���
	unsigned int				MarketPeriods[8][2];		///< ����ʱ��������Ϣ�б�
} tagSHL1MarketInfo_LF106;


typedef struct
{
	char						Key[20];					///< ������ֵ
	char						KindName[64];				///< ��������
	unsigned int				PriceRate;					///< �۸�Ŵ���[10�Ķ��ٴη�]
	unsigned int				LotSize;					///< ���֡�����
	unsigned int				WareCount;					///< �÷������Ʒ����
} tagSHL1KindDetail_LF107;



#pragma pack()
#endif









