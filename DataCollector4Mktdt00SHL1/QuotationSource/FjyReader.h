#ifndef __MEngine_CReadFjyFileH__20140918
#define __MEngine_CReadFjyFileH__20140918


#include <string>
#include <fstream>
#include <stdexcept>
#include "../Configuration.h"
#include "../Infrastructure/Lock.h"
#include "../Infrastructure/Thread.h"
#include "../Infrastructure/DateTime.h"


///< -------------------------- 错误代码定义 -----------------------------------------------
//系统类
#define			ERROR_Fjy_APPFIELDMEMORY		-100	//申请字段对象内存错误
#define			ERROR_Fjy_APPFILEMEMORY		-101	//申请文件内容长度。
#define			ERROR_Fjy_APPRECORDMEMORY		-102	//申请记录内容
//字段类
#define			ERROR_Fjy_FIELDDATA			-110	//读取field错误
#define			ERROR_Fjy_FIELDCOUNT			-111	//读取field个数错误

//文件整体类
#define			ERROR_Fjy_LENZERO				-120	//整体文件长度为零
#define			ERROR_Fjy_FILEREAD			-121	//读文件长度与GetLength长度不匹配
#define			ERROR_Fjy_FILERECORD			-122	//文件
//文件头类
#define			ERROR_Fjy_HEADLEN				-130	//文件头长度错误
#define			ERROR_Fjy_HEADVER				-131	//无法解析头部版本
#define			ERROR_Fjy_HEADDATE			-132	//无法解析头部日期
#define			ERROR_Fjy_HEADRECORD			-133	//无法解析头部记录数
#define			ERROR_Fjy_HEADANALYZE			-134	//解析错误

//文件内容类
#define			ERROR_Fjy_RECORDCOUNT			-140	//文件长度和头部记录数不符
#define			ERROR_Fjy_RECORDTOOLONG		-141

#define			ERROR_Fjy_NOTFOUNDRECORD		-142	//没有记录


#define			Fjy_FIELD_COUNT	128	//字段个数
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
//记录参数定义

#pragma pack(1)

//记录头
typedef struct
{
	unsigned int			uiRecCount;		//记录数
} TFjyHead;

//..........................................................................................................................
//DBF字段描述
typedef struct
{
	float					dPrice;		//	申买价	当前买入价
	double					dVolume;	//	申买量	
}TFjyBuySell;

typedef struct
{
	char	szType[5]; // 参考数据类型	C5
	char	szNonTrCode[6]; //	非交易证券代码	C6
	char	szNonTrName[8]; //	非交易证券名称	C8
	char	szCode[6]; //	产品证券代码	C6
	char	szName[8];	//产品证券名称	C8
	char 	szBusType[2];	//非交易业务类型		c2
	char	szNonTrBDate[8]; //	非交易订单输入开始日期	C8
	char	szNonTrEDate[8]; //	非交易订单输入结束日期	C8
	double	dNonTrOrdHands; // 非交易订单整手数	N12
	double	dNonTrOrdMinHands; // 非交易订单最小订单数量	N12
	double	dNonTrOrdMaxHands; // 非交易订单的最大订单数量	N12
	double	dNonTrPrice;	// 非交易价格	N13(5)
	double	dIPOVol;	//	IPO总量	N16
	char	chIPOAMethod;	// IPO分配方法	C1
	char	szIPOSellDate[8]; // IPO竞价分配或比例配售日期	C8 YYYYMMDD
	char	szIPOVfNumbDate[8]; //IPO验资或配号日期	C8 YYYYMMDD
	char	szIPOLttDrawDate[8]; // IPO摇号抽签的日期	C8 YYYYMMDD
	double	dIPOLowRngPrice; // IPO申购价格区间下限	N11(3)
	double	dIPOHighRngPrice; // IPO申购价格区间上限	N11(3)
	double	dIPOPropPlac; // IPO比例配售比例	N11(3)
	char	szRghtsIssueDate[8]; // 配股股权登记日	C8 YYYYMMDD
	char	szDividendDate[8]; // 配股除权日	C8 YYYYMMDD
	double	dRightOffRate;	// 配股比例	N11(6)
	double	dRightOffVol; // 配股总量	N16
	double	dIOPV2; // T-2日基金收益/基金净值	N13(5)
	double	dIOPV1; // T-1日基金收益/基金净值	N13(5)
	char	szRemarks[50]; //	备注	C50
} TFjyFieldData;
typedef struct
{
    char                    FieldName[32];  //字段名称
    char                    FieldType;      //字段类型 C 字符 N 数字 L D 日期
    unsigned long           FieldOffset;    //对于每一条纪录的偏移量
    unsigned char           FieldSize;      //字段长度
    unsigned char           DecSize;        //小数位数    
} TFjyFieldInfo;

#pragma pack()

//--------------------------------------------------------------------------------------------------------------------------

//注意：仅仅用于读取DBF文件
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

	int inner_SetMD1fieldinfo(); //载入字段配置。

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
	CriticalObject			m_oLock;				///< 临界区对象
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
	std::string				m_sFilePath;			///< fjy.txt路径
	std::ifstream			m_fInput;				///< 读文件对象
};
//--------------------------------------------------------------------------------------------------------------------------
#endif
//--------------------------------------------------------------------------------------------------------------------------