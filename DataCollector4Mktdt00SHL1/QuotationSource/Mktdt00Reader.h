#ifndef __CTP_BASIC_CACHE_H__
#define __CTP_BASIC_CACHE_H__


#pragma warning(disable:4786)
#include <map>
#include <string>
#include <fstream>
#include <stdexcept>
#include "../Configuration.h"
#include "../Infrastructure/Lock.h"
#include "../Infrastructure/Thread.h"
#include "../Infrastructure/DateTime.h"


///< ------------------------------ ������붨�� -----------------------------
//ϵͳ��
#define			ERROR_MKTDT_APPFIELDMEMORY		-100	//�����ֶζ����ڴ����
#define			ERROR_MKTDT_APPFILEMEMORY		-101	//�����ļ����ݳ��ȡ�
#define			ERROR_MKTDT_APPRECORDMEMORY		-102	//�����¼����
//�ֶ���
#define			ERROR_MKTDT_FIELDDATA			-110	//��ȡfield����
#define			ERROR_MKTDT_FIELDCOUNT			-111	//��ȡfield��������

//�ļ�������
#define			ERROR_MKTDT_LENZERO				-120	//�����ļ�����Ϊ��
#define			ERROR_MKTDT_FILEREAD			-121	//���ļ�������GetLength���Ȳ�ƥ��
#define			ERROR_MKTDT_FILERECORD			-122	//�ļ�
//�ļ�ͷ��
#define			ERROR_MKTDT_HEADLEN				-130	//�ļ�ͷ���ȴ���
#define			ERROR_MKTDT_HEADVER				-131	//�޷�����ͷ���汾
#define			ERROR_MKTDT_HEADDATE			-132	//�޷�����ͷ������
#define			ERROR_MKTDT_HEADRECORD			-133	//�޷�����ͷ����¼��
#define			ERROR_MKTDT_HEADANALYZE			-134	//��������

//�ļ�������
#define			ERROR_MKTDT_RECORDCOUNT			-140	//�ļ����Ⱥ�ͷ����¼������
#define			ERROR_MKTDT_RECORDTOOLONG		-141

#define			ERROR_MKTDT_NOTFOUNDRECORD		-142	//û�м�¼


#define			MKTDT_FIELD_COUNT	128	//�ֶθ���
#define			MKTDT_RECORD_DATALEN	1024


#define			MKTDT_FIELD_MD1_COUNT				13
#define			MKTDT_FIELD_MD2_COUNT				33
#define			MKTDT_FIELD_MD3_COUNT				33
#define			MKTDT_FIELD_MD4_COUNT				35


#define			MKTDT_FIELD_MD1_MAGIC_FLAG			"MD001"
#define			MKTDT_FIELD_MD2_MAGIC_FLAG			"MD002"
#define			MKTDT_FIELD_MD3_MAGIC_FLAG			"MD003"
#define			MKTDT_FIELD_MD4_MAGIC_FLAG			"MD004"


//-------------------------------------------------------------------------------------------------------------------------
//��¼��������

#pragma pack(1)

//��¼ͷ
typedef struct
{
    char                    szFlags[6];		//��ʼ��ʶ
	char					szVer[8];		//�汾
	unsigned int			uiBodyLength;	//����
	unsigned int			uiRecCount;		//��¼��
	unsigned int			uiReportID;
	char					szSenderCmpID[6];	//���ͷ���ʾ��
	unsigned int			uiTrDate;
	unsigned int			uiTrTime;
	unsigned int			uiUpdateType;	//���ͷ�ʽ 0 = ����Full refresh	1 = ����Incremental���ݲ�֧�֣�
	char					szSesStatus[8];	//�г�����״̬ ȫ�г�����״̬��
								//���ֶ�Ϊ8λ�ַ���������ÿλ��ʾ�ض��ĺ��壬�޶�������ո�
								//	��1λ��'S'��ʾȫ�г������ڼ䣨����ǰ����'T'��ʾȫ�г����ڽ����ڼ䣨���м����У��� 'E'��ʾȫ�г����ڱ����ڼ䡣
								//	��2λ��'1'��ʾ���̼��Ͼ��۽�����־��δ����ȡ'0'��
								//	��3λ��'1'��ʾ�г����������־��δ����ȡ'0'

} TMKTDTHead;
//..........................................................................................................................
//DBF�ֶ�����
typedef struct
{
	float					dPrice;		//	�����	��ǰ�����
	double					dVolume;	//	������	
}TMKTDTBuySell;

typedef struct
{
    char                    szStreamID[5];	//�����������ͱ�ʶ�� MD001 ��ʾָ���������ݸ�ʽ���ͣ�����ָ��Ŀǰʵ�ʾ���Ϊ4λС����MD002 ��ʾ��Ʊ��A��B�ɣ��������ݸ�ʽ���ͣ�	MD003 ��ʾծȯ�������ݸ�ʽ���ͣ�MD004 ��ʾ�����������ݸ�ʽ���ͣ�
	unsigned int			uiStreamID;		//1��ָ����2��Ʊ��3ծȯ 4���� 
	char					szSecurityID[6];		//����
	char					szSymbol[8];		//����
	double					dTradeVolume;		//	�ɽ�����	
	double					dTotalValueTraded;	//	�ɽ����	
	float					dPreClosePx;			//�������̼�	
	float					dOpenPrice;		//���տ��̼�	
	float					dHighPrice;		//	��߼�	
	float					dLowPrice;		//	��ͼ�	
	float					dTradePrice;	//	���¼�	���³ɽ���
	float					dClosePx;		//	�����̼�
	TMKTDTBuySell			stBuy[5];		//����
	TMKTDTBuySell			stSell[5];		//����

	float					dPreCloseIOPV;	//����T-1������ʱ��IOPV	��ѡ�ֶΣ�����MDStreamID=MD004ʱ���ڸ��ֶΡ�
	float					dIOPV;			//����IOPV	��ѡ�ֶΣ�����MDStreamID=MD004ʱ���ڸ��ֶΡ�
	char					szTradingPhaseCode[8];	//��Ʒʵʱ�׶μ���־	���ֶ�Ϊ8λ�ַ���������ÿλ��ʾ�ض��ĺ��壬�޶�������ո�
							//��1λ����S����ʾ����������ǰ��ʱ�Σ���C����ʾ���Ͼ���ʱ�Σ���T����ʾ��������ʱ�Σ���B����ʾ����ʱ�Σ���E����ʾ����ʱ�Σ���P����ʾ��Ʒͣ�ơ�
							//��2λ������������ո�
							//��3λ����0����ʾδ���У���1����ʾ�����С�
	unsigned int			uiTimestamp;	//ʱ���	HH:MM:SS.000

} TMKTDTFieldData;
typedef struct
{
    char                    FieldName[32];  //�ֶ�����
    char                    FieldType;      //�ֶ����� C �ַ� N ���� L D ����
    unsigned long           FieldOffset;    //����ÿһ����¼��ƫ����
    unsigned char           FieldSize;      //�ֶγ���
    unsigned char           DecSize;        //С��λ��    
} TMktdtFieldInfo;

#pragma pack()


///< ----------------------------------------------------------------------------------------------


/**
 * @class					CReadMktdtFile
 * @brief					�Ϻ�Level1��Mktdt00.txt�ļ���ȡ��
 * @author					barry
 */
class CReadMktdtFile
{
public:
	int  ChkOpen( std::string filename );
	void Close(void);
	void Release(void);
	int Instance();
	unsigned int		GetRecCount() {return	m_uiRecCount;};

protected:
	__inline void inner_delete();
	__inline int  inner_loadfromfile();
	__inline int  inner_parsehead(char *pszField[], unsigned short ushFieldCount);
	__inline int  inner_parseData(char *pszField[], unsigned short ushFieldCount);
	int SetFieldInfo();
	int inner_SetMD1fieldinfo();
	int inner_SetMD2fieldinfo();
	int inner_SetMD3fieldinfo();
	int inner_SetMD4fieldinfo();
	int  inner_AnalyzeField(char * in_pszData, int &lPos, char *out_pszField[], unsigned short in_ushFieldSize, bool & out_blLineEnd);
public:
	CReadMktdtFile( );
	virtual ~CReadMktdtFile();
public:
	int  ReloadFromFile();
public:
	int  GetHead(TMKTDTHead ** out_ppstHead);
	int  GetRecord(unsigned short in_ushRecPos, TMKTDTFieldData ** out_ppstFileData);
	int  IsOpened(void){return m_fInput.is_open();};

public:
	CriticalObject			m_oLock;				///< �ٽ�������
	TMKTDTHead				m_stHead;
	TMKTDTFieldData	*		m_pstRecData;
	unsigned int			m_uiRecCount;
	unsigned int			m_uiRecSize;
	char				*	m_pszFileData;
	unsigned int			m_uiFileDataSize;
	unsigned short			m_ushProcFileErrCount;

	unsigned short			m_ushPrtErrCount;
	int						m_iErrorCode;
	unsigned int			m_uiLine;

	TMktdtFieldInfo	*		m_poMD1FieldRecord;
	unsigned short			m_ushMD1FieldCount;
	TMktdtFieldInfo	*		m_poMD2FieldRecord;
	unsigned short			m_ushMD2FieldCount;
	TMktdtFieldInfo	*		m_poMD3FieldRecord;
	unsigned short			m_ushMD3FieldCount;
	TMktdtFieldInfo	*		m_poMD4FieldRecord;
	unsigned short			m_ushMD4FieldCount;

protected:
	std::string				m_sFilePath;			///< mktdt00.txt·��
	std::ifstream			m_fInput;				///< ���ļ�����
};




#endif






