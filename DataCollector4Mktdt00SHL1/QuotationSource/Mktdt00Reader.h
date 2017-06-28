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


///< ------------------------------ 错误代码定义 -----------------------------
//系统类
#define			ERROR_MKTDT_APPFIELDMEMORY		-100	//申请字段对象内存错误
#define			ERROR_MKTDT_APPFILEMEMORY		-101	//申请文件内容长度。
#define			ERROR_MKTDT_APPRECORDMEMORY		-102	//申请记录内容
//字段类
#define			ERROR_MKTDT_FIELDDATA			-110	//读取field错误
#define			ERROR_MKTDT_FIELDCOUNT			-111	//读取field个数错误

//文件整体类
#define			ERROR_MKTDT_LENZERO				-120	//整体文件长度为零
#define			ERROR_MKTDT_FILEREAD			-121	//读文件长度与GetLength长度不匹配
#define			ERROR_MKTDT_FILERECORD			-122	//文件
//文件头类
#define			ERROR_MKTDT_HEADLEN				-130	//文件头长度错误
#define			ERROR_MKTDT_HEADVER				-131	//无法解析头部版本
#define			ERROR_MKTDT_HEADDATE			-132	//无法解析头部日期
#define			ERROR_MKTDT_HEADRECORD			-133	//无法解析头部记录数
#define			ERROR_MKTDT_HEADANALYZE			-134	//解析错误

//文件内容类
#define			ERROR_MKTDT_RECORDCOUNT			-140	//文件长度和头部记录数不符
#define			ERROR_MKTDT_RECORDTOOLONG		-141

#define			ERROR_MKTDT_NOTFOUNDRECORD		-142	//没有记录


#define			MKTDT_FIELD_COUNT	128	//字段个数
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
//记录参数定义

#pragma pack(1)

//记录头
typedef struct
{
    char                    szFlags[6];		//起始标识
	char					szVer[8];		//版本
	unsigned int			uiBodyLength;	//长度
	unsigned int			uiRecCount;		//记录数
	unsigned int			uiReportID;
	char					szSenderCmpID[6];	//发送方标示符
	unsigned int			uiTrDate;
	unsigned int			uiTrTime;
	unsigned int			uiUpdateType;	//发送方式 0 = 快照Full refresh	1 = 增量Incremental（暂不支持）
	char					szSesStatus[8];	//市场行情状态 全市场行情状态：
								//该字段为8位字符串，左起每位表示特定的含义，无定义则填空格。
								//	第1位：'S'表示全市场启动期间（开市前），'T'表示全市场处于交易期间（含中间休市）， 'E'表示全市场处于闭市期间。
								//	第2位：'1'表示开盘集合竞价结束标志，未结束取'0'。
								//	第3位：'1'表示市场行情结束标志，未结束取'0'

} TMKTDTHead;
//..........................................................................................................................
//DBF字段描述
typedef struct
{
	float					dPrice;		//	申买价	当前买入价
	double					dVolume;	//	申买量	
}TMKTDTBuySell;

typedef struct
{
    char                    szStreamID[5];	//行情数据类型标识符 MD001 表示指数行情数据格式类型，其中指数目前实际精度为4位小数；MD002 表示股票（A、B股）行情数据格式类型；	MD003 表示债券行情数据格式类型；MD004 表示基金行情数据格式类型；
	unsigned int			uiStreamID;		//1是指数，2股票，3债券 4基金 
	char					szSecurityID[6];		//代码
	char					szSymbol[8];		//名称
	double					dTradeVolume;		//	成交数量	
	double					dTotalValueTraded;	//	成交金额	
	float					dPreClosePx;			//昨日收盘价	
	float					dOpenPrice;		//今日开盘价	
	float					dHighPrice;		//	最高价	
	float					dLowPrice;		//	最低价	
	float					dTradePrice;	//	最新价	最新成交价
	float					dClosePx;		//	今收盘价
	TMKTDTBuySell			stBuy[5];		//申买
	TMKTDTBuySell			stSell[5];		//申卖

	float					dPreCloseIOPV;	//基金T-1日收盘时刻IOPV	可选字段，仅当MDStreamID=MD004时存在该字段。
	float					dIOPV;			//基金IOPV	可选字段，仅当MDStreamID=MD004时存在该字段。
	char					szTradingPhaseCode[8];	//产品实时阶段及标志	该字段为8位字符串，左起每位表示特定的含义，无定义则填空格。
							//第1位：‘S’表示启动（开市前）时段，‘C’表示集合竞价时段，‘T’表示连续交易时段，‘B’表示休市时段，‘E’表示闭市时段，‘P’表示产品停牌。
							//第2位：。无意义填空格。
							//第3位：‘0’表示未上市，‘1’表示已上市。
	unsigned int			uiTimestamp;	//时间戳	HH:MM:SS.000

} TMKTDTFieldData;
typedef struct
{
    char                    FieldName[32];  //字段名称
    char                    FieldType;      //字段类型 C 字符 N 数字 L D 日期
    unsigned long           FieldOffset;    //对于每一条纪录的偏移量
    unsigned char           FieldSize;      //字段长度
    unsigned char           DecSize;        //小数位数    
} TMktdtFieldInfo;

#pragma pack()


///< ----------------------------------------------------------------------------------------------


/**
 * @class					CReadMktdtFile
 * @brief					上海Level1的Mktdt00.txt文件读取类
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
	CriticalObject			m_oLock;				///< 临界区对象
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
	std::string				m_sFilePath;			///< mktdt00.txt路径
	std::ifstream			m_fInput;				///< 读文件对象
};




#endif






