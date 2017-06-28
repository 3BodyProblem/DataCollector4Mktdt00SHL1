#ifndef __MEngine_MReadIdxDbfFileH__
#define __MEngine_MReadIdxDbfFileH__


#include <string>
#include <fstream>
#include <stdexcept>
#include "../Configuration.h"
#include "../Infrastructure/Lock.h"
#include "../Infrastructure/Thread.h"
#include "../Infrastructure/DateTime.h"


//错误代码定义
//系统类
#define			ERROR_IDX_APPFIELDMEMORY		-100	//申请字段对象内存错误
#define			ERROR_IDX_APPFILEMEMORY			-101	//申请文件内容长度。
#define			ERROR_IDX_APPRECORDMEMORY		-102	//申请记录内容
//字段类
#define			ERROR_IDX_FIELDDATA				-110	//读取field错误
//文件整体类
#define			ERROR_IDX_LENZERO				-120	//整体文件长度为零
#define			ERROR_IDX_FILEREAD				-121	//读文件长度与GetLength长度不匹配
//文件头类
#define			ERROR_IDX_HEADLEN				-130	//文件头长度错误
#define			ERROR_IDX_HEADVER				-131	//无法解析头部版本
#define			ERROR_IDX_HEADDATE				-132	//无法解析头部日期
#define			ERROR_IDX_HEADRECORD			-133	//无法解析头部记录数
//文件内容类
#define			ERROR_IDX_RECORDCOUNT			-140	//文件长度和头部记录数不符
#define			ERROR_IDX_RECORDTOOLONG			-141

//-------------------------------------------------------------------------------------------------------------------------
//记录参数定义
#define			RECORDFIELDCOUNT				20
#define			RECORDHEADLEN					38
#define			MAXRECORDLEN					2048
//记录头
typedef struct
{
    char                    Ver[3];				//每日传递的版本号
    unsigned long			TradeDate;			//信息的日期
	unsigned long			NaturalDate;		//自然日期
	unsigned long			NaturalTime;		//自然时间
	unsigned long			RecordCount;		//记录条数
} tagIdxDbfHeadInfo;
//..........................................................................................................................
//DBF字段描述
typedef struct
{
    char                    FieldName[32];  //字段名称
    char                    FieldType;      //字段类型 C 字符 N 数字 L D 日期
    unsigned long           FieldOffset;    //对于每一条纪录的偏移量
    unsigned char           FieldSize;      //字段长度
    unsigned char           DecSize;        //小数位数    
} tagIdxDbfFieldInfo;
//--------------------------------------------------------------------------------------------------------------------------

//注意：仅仅用于读取DBF文件
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
	CriticalObject			m_oLock;				///< 临界区对象
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
	std::string				m_sFilePath;			///< csi20170303.txt路径
	std::ifstream			m_fInput;				///< 读文件对象
	int						m_nRecordLength;
};


#endif







