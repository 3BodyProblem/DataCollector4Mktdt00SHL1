#ifndef __DATA_COLLECTOR_H__
#define __DATA_COLLECTOR_H__


#pragma warning(disable: 4786)
#include <vector>
#include <string>
#include "Infrastructure/Lock.h"


extern	HMODULE						g_oModule;						///< 当前dll模块变量


/**
 * @class						LinkConfig
 * @brief						一组CTP连接信息
 * @date						2017/5/15
 * @author						barry
 */
class CTPLinkConfig
{
public:
    std::string					m_sParticipant;			///< 参与者编号
    std::string					m_sUID;					///< 用户ID
    std::string					m_sPswd;				///< 登录密码
	std::vector<std::string>	m_vctFrontServer;		///< 前置服务器地址
	std::vector<std::string>	m_vctNameServer;		///< 名称服务器地址
};


/**
 * @class						Configuration
 * @brief						配置信息
 * @date						2017/5/15
 * @author						barry
 */
class Configuration
{
protected:
	Configuration();

public:
	/**
	 * @brief					获取配置对象的单键引用对象
	 */
	static Configuration&		GetConfig();

	/**
	 * @brief					加载配置项
	 * @return					==0					成功
								!=					出错
	 */
    int							Initialize();

public:
	/**
	 * @brief					取得快照落盘目录(含文件名)
	 */
	const std::string&			GetDumpFolder() const;

	/**
	 * @brief					获取交易所编号
	 */
	const std::string&			GetExchangeID() const;

	/**
	 * @brief					获取市场编号
	 */
	unsigned int				GetMarketID() const;

	/**
	 * @brief					获取行情文件路径
	 */
	const std::string&			GetMktdt00FilePath() const;

	/**
	 * @brief					获取非交易文件路径
	 */
	const std::string&			GetFjyFilePath() const;

	/**
	 * @brief					中证指数文件路径
	 */
	const std::string&			GetZzzsFilePath() const;

	const std::string&			GetLonKindPath() const;

private:
	unsigned int				m_nMarketID;			///< 市场编号
	std::string					m_sExchangeID;			///< 交易所编号
	std::string					m_sDumpFileFolder;		///< 快照落盘路径(需要有文件名)
	std::string					m_sMktdt00FilePath;		///< 行情文件路径
	std::string					m_sFjyFilePath;			///< 非交易文件配置路径(不包含文件中的日期部分)
	std::string					m_sZzzsFilePath;		///< 中证指数文件路径
	std::string					m_s4xLonKindFilePath;	///< Lonkind文件路径
};


#endif







