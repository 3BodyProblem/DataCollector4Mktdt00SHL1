#ifndef __MEngine_MReadIdxDbfFileH__
#define __MEngine_MReadIdxDbfFileH__


#include <string>
#include <fstream>
#include <stdexcept>
#include "../Configuration.h"
#include "../Infrastructure/Lock.h"
#include "../Infrastructure/Thread.h"
#include "../Infrastructure/DateTime.h"


//������붨��
//ϵͳ��
#define			ERROR_IDX_APPFIELDMEMORY		-100	//�����ֶζ����ڴ����
#define			ERROR_IDX_APPFILEMEMORY			-101	//�����ļ����ݳ��ȡ�
#define			ERROR_IDX_APPRECORDMEMORY		-102	//�����¼����
//�ֶ���
#define			ERROR_IDX_FIELDDATA				-110	//��ȡfield����
//�ļ�������
#define			ERROR_IDX_LENZERO				-120	//�����ļ�����Ϊ��
#define			ERROR_IDX_FILEREAD				-121	//���ļ�������GetLength���Ȳ�ƥ��
//�ļ�ͷ��
#define			ERROR_IDX_HEADLEN				-130	//�ļ�ͷ���ȴ���
#define			ERROR_IDX_HEADVER				-131	//�޷�����ͷ���汾
#define			ERROR_IDX_HEADDATE				-132	//�޷�����ͷ������
#define			ERROR_IDX_HEADRECORD			-133	//�޷�����ͷ����¼��
//�ļ�������
#define			ERROR_IDX_RECORDCOUNT			-140	//�ļ����Ⱥ�ͷ����¼������
#define			ERROR_IDX_RECORDTOOLONG			-141

//-------------------------------------------------------------------------------------------------------------------------
//��¼��������
#define			RECORDFIELDCOUNT				20
#define			RECORDHEADLEN					38
#define			MAXRECORDLEN					2048
//��¼ͷ
typedef struct
{
    char                    Ver[3];				//ÿ�մ��ݵİ汾��
    unsigned long			TradeDate;			//��Ϣ������
	unsigned long			NaturalDate;		//��Ȼ����
	unsigned long			NaturalTime;		//��Ȼʱ��
	unsigned long			RecordCount;		//��¼����
} tagIdxDbfHeadInfo;
//..........................................................................................................................
//DBF�ֶ�����
typedef struct
{
    char                    FieldName[32];  //�ֶ�����
    char                    FieldType;      //�ֶ����� C �ַ� N ���� L D ����
    unsigned long           FieldOffset;    //����ÿһ����¼��ƫ����
    unsigned char           FieldSize;      //�ֶγ���
    unsigned char           DecSize;        //С��λ��    
} tagIdxDbfFieldInfo;
//--------------------------------------------------------------------------------------------------------------------------

//ע�⣺�������ڶ�ȡDBF�ļ�
class MReadIdxDbfFile
{
protected:
	__inline void inner_delete(void);
	__inline int  inner_loadfieldinfo(void);
	__inline int  inner_loadfromfile(void);
	__inline int  inner_parsehead(char *pInBuf, unsigned int nInSize);
	__inline int  inner_findfieldno(const char * fieldname);
	__inline int  inner_loaddefaultfieldinfo(void);
public:
	MReadIdxDbfFile( );
	virtual ~MReadIdxDbfFile();
public:
	int  Open( std::string filename );
	void Close(void);
	void CloseDbfFile(void);
public:
	int  ReloadFromFile(void);
public:
	int  ReadString(unsigned short fieldno,char * value,unsigned short insize);
	int  ReadString(const char * fieldname,char * value,unsigned short insize);
	int  ReadInteger(unsigned short fieldno,int * value);
	int  ReadInteger(const char * fieldname,int * value);
	int  ReadFloat(unsigned short fieldno,double * value);
	int  ReadFloat(const char * fieldname,double * value);
public:
	int  First(void);
	int  Last(void);
	int  Prior(void);
	int  Next(void);
	int  Goto(int recno);
public:
	int  GetDbfDate(void);
public:
	int  GetRecordCount(void);
	int  GetFieldCount(void);
public:
	int  FieldToInteger(char * FieldName);
	double FieldToFloat(char * FieldName);
	int  FieldToInteger(unsigned short fieldno);
	double FieldToFloat(unsigned short fieldno);
	int  IsOpened(void){return m_fInput.is_open();};
	int  FindRecord(char * FieldName,char * Value);
	int  FindRecord(unsigned short fieldno,char * Value);
public:
	CriticalObject			m_oLock;				///< �ٽ�������
	tagIdxDbfHeadInfo		m_DbfHeadInfo;
	tagIdxDbfFieldInfo	*	m_DbfFieldRecord;
	int						m_FieldCount;
	char				*	m_RecordData;
	char				*   m_FileContent;
	int						m_RecordCount;
	int						m_CurRecordNo;
	int						m_LastHeadRecordCount;
	int						m_LastFileLength;
	int						m_FileLength;
protected:
	std::string				m_sFilePath;			///< csi20170303.txt·��
	std::ifstream			m_fInput;				///< ���ļ�����
	int						m_nRecordLength;
};


#endif







