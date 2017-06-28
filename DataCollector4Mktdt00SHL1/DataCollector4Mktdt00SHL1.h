#ifndef __DATA_TABLE_PAINTER_H__
#define	__DATA_TABLE_PAINTER_H__


#include "Interface.h"
#include "Configuration.h"
#include "Infrastructure/Lock.h"
#include "QuotationSource/Quotation.h"


/**
 * @class						T_LOG_LEVEL
 * @brief						日志类型
 * @author						barry
 */
enum T_LOG_LEVEL
{
	TLV_INFO = 0,
	TLV_WARN = 1,
	TLV_ERROR = 2,
	TLV_DETAIL = 3,
};


/**
 * @class						QuoCollector
 * @brief						行情数据采集模块主干类
 * @date						2017/5/15
 * @author						barry
 */
class QuoCollector
{
protected:
	QuoCollector();

public:
	/**
	 * @brief					获取数据采集对象的单键引用
	 */
	static QuoCollector&		GetCollector();

	/**
	 * @brief					初始化数据采集器
	 * @param[in]				pIDataHandle				行情回调接口
	 * @return					==0							初始化成功
								!=0							出错
	 */
	int							Initialize( I_DataHandle* pIDataHandle );

	/**
	 * @brief					释放资源
	 */
	void						Release();

public:
	/**
	 * @brief					从本地文件或行情端口重新恢复加载所有行情数据
	 * @note					是一个同步的函数，在行情初始化完成后才会返回
	 * @return					==0							成功
								!=0							出错
	 */
	int							RecoverQuotation();

	/**
	 * @brief					停止
	 */
	void						Halt();

public:
	/**
	 * @brief					重载返回行情回调接口地址
	 * @return					行情回调接口指针地址
	 */
	I_DataHandle*				operator->();

	/**
	 * @brief					取得采集模块的当前状态
	 * @param[out]				pszStatusDesc				返回出状态描述串
	 * @param[in,out]			nStrLen						输入描述串缓存长度，输出描述串有效内容长度
	 * @return					返回模块当前状态值
	 */
	enum E_SS_Status			GetCollectorStatus( char* pszStatusDesc, unsigned int& nStrLen );

protected:
	I_DataHandle*				m_pCbDataHandle;			///< 数据(行情/日志回调接口)
	Quotation					m_oQuotationData;			///< 实时行情数据会话对象
};


/**
 * @brief						DLL导出接口
 * @author						barry
 * @date						2017/4/1
 */
extern "C"
{
	/**
	 * @brief								初始化数据采集模块
	 * @param[in]							pIDataHandle				行情功能回调
	 * @return								==0							初始化成功
											!=							出错
	 */
	__declspec(dllexport) int __stdcall		Initialize( I_DataHandle* pIDataHandle );

	/**
	 * @brief								释放数据采集模块
	 */
	__declspec(dllexport) void __stdcall	Release();

	/**
	 * @brief								重新初始化并加载行情数据
	 * @note								是一个同步的函数，在行情初始化完成后才会返回
 	 * @return								==0							成功
											!=0							出错
	 */
	__declspec(dllexport) int __stdcall		RecoverQuotation();

	/**
	 * @brief								暂时数据采集
	 */
	__declspec(dllexport) void __stdcall	HaltQuotation();

	/**
	 * @brief								获取模块的当前状态
	 * @param[out]							pszStatusDesc				返回出状态描述串
	 * @param[in,out]						nStrLen						输入描述串缓存长度，输出描述串有效内容长度
	 * @return								返回模块当前状态值
	 */
	__declspec(dllexport) int __stdcall		GetStatus( char* pszStatusDesc, unsigned int& nStrLen );

	/**
	 * @brief								获取市场编号
	 * @return								市场ID
	 */
	__declspec(dllexport) int __stdcall		GetMarketID();

	/**
	 * @brief								是否为行情传输的采集器
	 * @return								true						是传输模块的行情采集插件
											false						顶层源的行情采集插件
	 */
	__declspec(dllexport) bool __stdcall	IsProxy();

	/**
	 * @brief								单元测试导出函数
	 */
	__declspec(dllexport) void __stdcall	ExecuteUnitTest();
}




#endif





