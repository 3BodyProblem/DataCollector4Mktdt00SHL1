#ifndef __CTP_QUOTATION_PROTOCAL_CTP_DL_H__
#define	__CTP_QUOTATION_PROTOCAL_CTP_DL_H__
#pragma pack(1)


typedef struct
{
	char						Key[20];					///< ������ֵ
	unsigned int				MarketID;					///< �г����
	unsigned int				MarketDate;					///< �г�����
	unsigned int				KindCount;					///< �������
	unsigned int				WareCount;					///< ��Ʒ����
	unsigned int				PeriodsCount;				///< ����ʱ����Ϣ�б���
	unsigned int				MarketPeriods[8][2];		///< ����ʱ��������Ϣ�б�
} tagSHL1MarketInfo_LF149;


typedef struct
{
	char						Key[20];					///< ������ֵ
	char						KindName[64];				///< ��������
	unsigned int				PriceRate;					///< �۸�Ŵ���[10�Ķ��ٴη�]
	unsigned int				LotSize;					///< ���֡�����
	unsigned int				WareCount;					///< �÷������Ʒ����
} tagSHL1KindDetail_LF150;


typedef struct
{
	char						Key[20];					///< ������ֵ
	unsigned int				MarketTime;					///< �г�ʱ��
	unsigned char				MarketStatus;				///< �г�״̬[0��ʼ�� 1������]
} tagSHL1MarketStatus_HF151;


typedef struct
{
	char						Code[20];					///< ��Լ����
	char						Name[64];					///< ��Լ����
	unsigned short				Kind;						///< ֤ȯ����
} tagSHL1ReferenceData_LF152;


typedef struct
{
	char						Code[20];					///< ���ƴ����
	unsigned char				StopFlag;					///< ͣ�Ʊ�ǣ��Ƿ�ɽ��ף���Ҫ����
	unsigned char				PLimit;						///< �ǡ���ͣ�Ƿ���Ч
	unsigned int				Worth;						///< ÿ����ֵ��*�Ŵ���10000
	unsigned int				ExTts;						///< ���׵�λ
	unsigned int				MaxExVol;					///< ���������
	unsigned int				MinExVol;					///< ��С��������
} tagSHL1ReferenceExtension_LF153;


typedef struct
{
	char						Code[20];					///< ��Լ����
	unsigned int				PreClose;					///< ���ռ�[*�Ŵ���]
	unsigned int				Open;						///< ���̼�[*�Ŵ���]
	unsigned int				Close;						///< ���ռ�[*�Ŵ���]
	unsigned int				HighLimit;					///< ��ͣ�޼�[*�Ŵ���]
	unsigned int				LowLimit;					///< ��ͣ�޼�[*�Ŵ���]
} tagSHL1SnapData_LF154;


typedef struct
{
	char						Code[20];					///< ��Լ����
	unsigned int				High;						///< ������ͣ�۸�[*�Ŵ���], 0��ʾ������
	unsigned int				Low;						///< ���յ�ͣ�۸�[*�Ŵ���], 0��ʾ������
	unsigned int				Now;						///< ���¼�[*�Ŵ���]
	unsigned int				Voip;						///< ����ģ�⾻ֵ��Ȩ֤���[�Ŵ�1000��][ETF������LOF��Ȩ֤��ֵ��Ч]
	double						Amount;						///< �ܳɽ����[Ԫ]
	unsigned __int64			Volume;						///< �ܳɽ���[��/��]
} tagSHL1SnapData_HF155;


typedef struct
{
	unsigned int				Price;						///< ί�м۸�[* �Ŵ���]
	unsigned __int64			Volume;						///< ί����[��]
} tagSHL1BuySellItem;


typedef struct
{
	char						Code[20];					///< ��Լ����
	tagSHL1BuySellItem			Buy[5];						///< ���嵵
	tagSHL1BuySellItem			Sell[5];					///< ���嵵
} tagSHL1SnapBuySell_HF156;



#pragma pack()
#endif









