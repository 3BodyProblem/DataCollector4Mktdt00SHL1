#ifndef __MEngine_CReadFjyFileH__20140918
#define __MEngine_CReadFjyFileH__20140918


#include <string>
#include <fstream>
#include <stdexcept>
#include "../Configuration.h"
#include "../Infrastructure/Lock.h"
#include "../Infrastructure/Thread.h"
#include "../Infrastructure/DateTime.h"


///< -------------------------- ������붨�� -----------------------------------------------
//ϵͳ��
#define			ERROR_Fjy_APPFIELDMEMORY		-100	//�����ֶζ����ڴ����
#define			ERROR_Fjy_APPFILEMEMORY		-101	//�����ļ����ݳ��ȡ�
#define			ERROR_Fjy_APPRECORDMEMORY		-102	//�����¼����
//�ֶ���
#define			ERROR_Fjy_FIELDDATA			-110	//��ȡfield����
#define			ERROR_Fjy_FIELDCOUNT			-111	//��ȡfield��������

//�ļ�������
#define			ERROR_Fjy_LENZERO				-120	//�����ļ�����Ϊ��
#define			ERROR_Fjy_FILEREAD			-121	//���ļ�������GetLength���Ȳ�ƥ��
#define			ERROR_Fjy_FILERECORD			-122	//�ļ�
//�ļ�ͷ��
#define			ERROR_Fjy_HEADLEN				-130	//�ļ�ͷ���ȴ���
#define			ERROR_Fjy_HEADVER				-131	//�޷�����ͷ���汾
#define			ERROR_Fjy_HEADDATE			-132	//�޷�����ͷ������
#define			ERROR_Fjy_HEADRECORD			-133	//�޷�����ͷ����¼��
#define			ERROR_Fjy_HEADANALYZE			-134	//��������

//�ļ�������
#define			ERROR_Fjy_RECORDCOUNT			-140	//�ļ����Ⱥ�ͷ����¼������
#define			ERROR_Fjy_RECORDTOOLONG		-141

#define			ERROR_Fjy_NOTFOUNDRECORD		-142	//û�м�¼


#define			Fjy_FIELD_COUNT	128	//�ֶθ���
#define			Fjy_RECORD_DATALEN	1024


#define			Fjy_FIELD_MD1_COUNT				13
#define			Fjy_FIELD_MD2_COUNT				33
#define			Fjy_FIELD_MD3_COUNT				33
#define			Fjy_FIELD_MD4_COUNT				35


#define			Fjy_FIELD_MD1_MAGIC_FLAG				"MD001"
#define			Fjy_FIELD_MD2_MAGIC_FLAG				"MD002"
#define			Fjy_FIELD_MD3_MAGIC_FLAG				"MD003"
#define			Fjy_FIELD_MD4_MAGIC_FLAG				"MD004"


//-------------------------------------------------------------------------------------------------------------------------
//��¼��������

#pragma pack(1)

//��¼ͷ
typedef struct
{
	unsigned int			uiRecCount;		//��¼��
} TFjyHead;

//..........................................................................................................................
//DBF�ֶ�����
typedef struct
{
	float					dPrice;		//	�����	��ǰ�����
	double					dVolume;	//	������	
}TFjyBuySell;

typedef struct
{
	char	szType[5]; // �ο���������	C5
	char	szNonTrCode[6]; //	�ǽ���֤ȯ����	C6
	char	szNonTrName[8]; //	�ǽ���֤ȯ����	C8
	char	szCode[6]; //	��Ʒ֤ȯ����	C6
	char	szName[8];	//��Ʒ֤ȯ����	C8
	char 	szBusType[2];	//�ǽ���ҵ������		c2
	char	szNonTrBDate[8]; //	�ǽ��׶������뿪ʼ����	C8
	char	szNonTrEDate[8]; //	�ǽ��׶��������������	C8
	double	dNonTrOrdHands; // �ǽ��׶���������	N12
	double	dNonTrOrdMinHands; // �ǽ��׶�����С��������	N12
	double	dNonTrOrdMaxHands; // �ǽ��׶�������󶩵�����	N12
	double	dNonTrPrice;	// �ǽ��׼۸�	N13(5)
	double	dIPOVol;	//	IPO����	N16
	char	chIPOAMethod;	// IPO���䷽��	C1
	char	szIPOSellDate[8]; // IPO���۷���������������	C8 YYYYMMDD
	char	szIPOVfNumbDate[8]; //IPO���ʻ��������	C8 YYYYMMDD
	char	szIPOLttDrawDate[8]; // IPOҡ�ų�ǩ������	C8 YYYYMMDD
	double	dIPOLowRngPrice; // IPO�깺�۸���������	N11(3)
	double	dIPOHighRngPrice; // IPO�깺�۸���������	N11(3)
	double	dIPOPropPlac; // IPO�������۱���	N11(3)
	char	szRghtsIssueDate[8]; // ��ɹ�Ȩ�Ǽ���	C8 YYYYMMDD
	char	szDividendDate[8]; // ��ɳ�Ȩ��	C8 YYYYMMDD
	double	dRightOffRate;	// ��ɱ���	N11(6)
	double	dRightOffVol; // �������	N16
	double	dIOPV2; // T-2�ջ�������/����ֵ	N13(5)
	double	dIOPV1; // T-1�ջ�������/����ֵ	N13(5)
	char	szRemarks[50]; //	��ע	C50
} TFjyFieldData;
typedef struct
{
    char                    FieldName[32];  //�ֶ�����
    char                    FieldType;      //�ֶ����� C �ַ� N ���� L D ����
    unsigned long           FieldOffset;    //����ÿһ����¼��ƫ����
    unsigned char           FieldSize;      //�ֶγ���
    unsigned char           DecSize;        //С��λ��    
} TFjyFieldInfo;

#pragma pack()

//--------------------------------------------------------------------------------------------------------------------------

//ע�⣺�������ڶ�ȡDBF�ļ�
class CReadFjyFile
{
public:
	int  ChkOpen( std::string filename );
	void Close(void);
	void Release(void);
	int Instance();

	unsigned int		GetRecCount() {return	m_uiRecCount;};

protected:
	__inline void inner_delete(void);
	__inline int  inner_loadfromfile(void);
	//__inline int  inner_parsehead(char *pszField[], unsigned short ushFieldCount);
	__inline int  inner_parseData(char *pszField[], unsigned short ushFieldCount);
	int SetFieldInfo();

	int inner_SetMD1fieldinfo(); //�����ֶ����á�

	int  inner_AnalyzeField(char * in_pszData, int &lPos, char *out_pszField[], unsigned short in_ushFieldSize, bool & out_blLineEnd);

public:
	CReadFjyFile( );
	virtual ~CReadFjyFile();
public:
	int  ReloadFromFile(void);
public:

	int  GetHead(TFjyHead ** out_ppstHead);
	int  GetRecord(unsigned short in_ushRecPos, TFjyFieldData ** out_ppstFileData);


	int  IsOpened(void){return m_fInput.is_open();};
	
public:
	CriticalObject			m_oLock;				///< �ٽ�������
	TFjyHead				m_stHead;
	TFjyFieldData	*		m_pstRecData;
	unsigned int			m_uiRecCount;
	unsigned int			m_uiRecSize;
	char				*	m_pszFileData;
	unsigned int			m_uiFileDataSize;
	unsigned short			m_ushProcFileErrCount;

	unsigned short			m_ushPrtErrCount;
	int						m_iErrorCode;
	unsigned int			m_uiLine;

	TFjyFieldInfo	*		m_poMD1FieldRecord;
	unsigned short			m_ushMD1FieldCount;

protected:
	std::string				m_sFilePath;			///< fjy.txt·��
	std::ifstream			m_fInput;				///< ���ļ�����
};
//--------------------------------------------------------------------------------------------------------------------------
#endif
//--------------------------------------------------------------------------------------------------------------------------