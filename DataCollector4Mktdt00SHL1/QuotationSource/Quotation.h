#ifndef __CTP_QUOTATION_H__
#define __CTP_QUOTATION_H__


#pragma warning(disable:4786)
#include <set>
#include <map>
#include <string>
#include <stdexcept>
#include "FjyReader.h"
#include "IdxDbfReader.h"
#include "Mktdt00Reader.h"
#include "../Configuration.h"
#include "../Infrastructure/Lock.h"
#include "../Infrastructure/Thread.h"
#include "../Infrastructure/DateTime.h"
#include "../Mktdt00_SHL1_QuoProtocal.h"


#define		SHZSNAME		"上证指数"
#define		SHAGNAME		"上证Ａ股"
#define		SHBGNAME		"上证Ｂ股"
#define		SHJJNAME		"上证基金"
#define		SHZQNAME		"上证债券"
#define		SHZZNAME		"上证转债"
#define		SHHGNAME		"上证回购"
#define		SHETFNAME		"上证ETF"
#define		SHJJTNAME		"基金通"
#define		SHQZNAME		"上证权证"
#define		SHYXNAME		"上证优先"
#define		SHQTNAME		"上证其它"
#define		SHKFJJNAME		"开放基金"
//add by liuqy 20121129 for 增加上证退及风险处理
#define		SHFXJSNAME		"风险警示"
#define		SHSZTSNAME		"上证退市"
#define		SHFJJSNAME		"附加指数"//14
#define MAX_MASK_COUNT	10
#define MAX_KIND_MASK	32


typedef struct
{
    unsigned char uchCount;	//个数
    unsigned char uchMask[MAX_MASK_COUNT][7];	//特征码
}TKindMask;


/**
 * @class			WorkStatus
 * @brief			工作状态管理
 * @author			barry
 */
class WorkStatus
{
public:
	/**
	 * @brief				应状态值映射成状态字符串
	 */
	static	std::string&	CastStatusStr( enum E_SS_Status eStatus );

public:
	/**
	 * @brief			构造
	 * @param			eMkID			市场编号
	 */
	WorkStatus();
	WorkStatus( const WorkStatus& refStatus );

	/**
	 * @brief			赋值重载
						每次值变化，将记录日志
	 */
	WorkStatus&			operator= ( enum E_SS_Status eWorkStatus );

	/**
	 * @brief			重载转换符
	 */
	operator			enum E_SS_Status();

private:
	CriticalObject		m_oLock;				///< 临界区对象
	enum E_SS_Status	m_eWorkStatus;			///< 行情工作状态
};


/**
 * @class			Quotation
 * @brief			会话管理对象
 * @detail			封装了针对商品期货期权各市场的初始化、管理控制等方面的方法
 * @author			barry
 */
class Quotation : public SimpleTask
{
public:
	Quotation();
	~Quotation();

	/**
	 * @brief			释放ctp行情接口
	 */
	int					Release();

	/**
	 * @brief			初始化ctp行情接口
	 * @return			>=0			成功
						<0			错误
	 * @note			整个对象的生命过程中，只会启动时真实的调用一次
	 */
	int					Initialize();

public:///< 公共方法函数
	/**
	 * @brief			获取会话状态信息
	 */
	WorkStatus&			GetWorkStatus();

	/**
	 * @brief			发送登录请求包
	 */
    void				SendLoginRequest();

	/**
	 * @brief			刷数据到发送缓存
	 * @param[in]		pQuotationData			行情数据结构
	 * @param[in]		bInitialize				初始化数据的标识
	 */
	void				FlushQuotation( char* pQuotationData, bool bInitialize );
	unsigned char	GetIdxType() { return m_uchIdx;};		//		"上证指数"
	unsigned char	GetAGType() { return m_uchAG;};		//"上证Ａ股"
	unsigned char	GetBGType() { return m_uchBG;};		//"上证Ｂ股"
	unsigned char	GetJJType() { return m_uchJJ;};		//"上证基金"
	unsigned char	GetZQType() { return m_uchZQ;};		//"上证债券"
	unsigned char	GetZZType() { return m_uchZZ;};	//"上证转债"
	unsigned char	GetHGType() { return m_uchHG;};	//	"上证回购"
	unsigned char	GetETFType() { return m_uchETF;};	//"上证ETF"
	unsigned char	GetJJTType() { return m_uchJJT;};	//	"基金通"
	unsigned char	GetQZType() { return m_uchQZ;};		//"上证权证"
	unsigned char	GetYXType() { return m_uchYX;};		//"上证优先"
	unsigned char	GetQTType() { return m_uchQT;};	//		"上证其他"
	unsigned char	GetKFJJType() { return m_uchKFJJ;};	//	"开放基金"
	unsigned char	GetFXJSType() { return m_uchFXJS;};//		"风险警示"
	unsigned char	GetSZTSType() { return m_uchSZTS;};//		"上证退市"
	unsigned char	GetFJZSType() { return m_uchFJZS;};//		"附加指数"
protected:
	/**
	 * @brief			任务函数(内循环)
	 * @return			==0					成功
						!=0					失败
	 */
	virtual int			Execute();

	int					ScanFile();
	int					GetMKTDTNowDateTime(unsigned int &out_ulDate, unsigned int &out_ulTime);

	/**
	 * @brief			优先级说明: MKTDT代码>fjyYYYYMMDD.TXT非交易性代码>中证指数>国债预发行>开放式基金>附加指数
	 */
	int					PrepareCodesTable();

	/**
	 * @brief			构造快照数据
	 */
	int					BuildImageData();

	int					SetKindInfo();
	int					GetMemIndexData( unsigned long RecordPos, tagSHL1SnapData_LF102* pLFData, tagSHL1SnapData_HF103* pHFData );
	int					GetMemStockData(  unsigned long RecordPos, tagSHL1SnapData_LF102* pLFData, tagSHL1SnapData_HF103* pHFData, tagSHL1SnapBuySell_HF104* pBSData, unsigned long  &ulVoip, bool in_blFirstFlag = true );
	int					GetShNameTableData( unsigned long RecordPos, tagSHL1ReferenceData_LF100* Out );
	long				rl_BuildZZIdxCode(int &inout_lSerial, int & out_lKindNum);
	int					GetZzzsIdxData( tagSHL1SnapData_LF102* pLFData, tagSHL1SnapData_HF103* pHFData );
	long				rl_BuildMKTDTFjyCode( int &inout_lSerial, int *out_plKindNum, bool in_blLoadIdx = false );

private:
	CriticalObject			m_oLock;				///< 临界区对象
	WorkStatus				m_oWorkStatus;			///< 工作状态
	CReadFjyFile			m_oFjyFile;				///< 非交易文件扫描类
	CReadMktdtFile			m_oMktdt00File;			///< 行情文件扫描类
	MReadIdxDbfFile			m_oZzzsFile;			///< 中证指数扫描类
	std::set<std::string>	m_setCodes;				///< 商品代码集合
private:
	unsigned int			m_nQuoDate;				///< 行情日期
	unsigned int			m_nQuoTime;				///< 行情时间
	TKindMask				m_stKindMask[MAX_KIND_MASK + 1];
	std::map<char,tagSHL1KindDetail_LF107>	m_mapMkDetail;
	std::map<std::string,char>				m_mapCode2Type;
private:
	unsigned char	m_uchIdx;		//		"上证指数"
	unsigned char	m_uchAG;		//"上证Ａ股"
	unsigned char	m_uchBG;		//"上证Ｂ股"
	unsigned char	m_uchJJ;		//"上证基金"
	unsigned char	m_uchZQ;		//"上证债券"
	unsigned char	m_uchZZ;	//"上证转债"
	unsigned char	m_uchHG;	//	"上证回购"
	unsigned char	m_uchETF;	//"上证ETF"
	unsigned char	m_uchJJT;	//	"基金通"
	unsigned char	m_uchQZ;		//"上证权证"
	unsigned char	m_uchYX;		//"上证优先"
	unsigned char	m_uchQT;	//		"上证其他"
	unsigned char	m_uchKFJJ;	//	"开放基金"
	unsigned char	m_uchFXJS;//		"风险警示"
	unsigned char	m_uchSZTS;//		"上证退市"
	unsigned char	m_uchFJZS;//		"附加指数"
};




#endif






