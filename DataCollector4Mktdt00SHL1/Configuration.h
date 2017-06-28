#ifndef __DATA_COLLECTOR_H__
#define __DATA_COLLECTOR_H__


#pragma warning(disable: 4786)
#include <vector>
#include <string>
#include "Infrastructure/Lock.h"


extern	HMODULE						g_oModule;						///< ��ǰdllģ�����


/**
 * @class						LinkConfig
 * @brief						һ��CTP������Ϣ
 * @date						2017/5/15
 * @author						barry
 */
class CTPLinkConfig
{
public:
    std::string					m_sParticipant;			///< �����߱��
    std::string					m_sUID;					///< �û�ID
    std::string					m_sPswd;				///< ��¼����
	std::vector<std::string>	m_vctFrontServer;		///< ǰ�÷�������ַ
	std::vector<std::string>	m_vctNameServer;		///< ���Ʒ�������ַ
};


/**
 * @class						Configuration
 * @brief						������Ϣ
 * @date						2017/5/15
 * @author						barry
 */
class Configuration
{
protected:
	Configuration();

public:
	/**
	 * @brief					��ȡ���ö���ĵ������ö���
	 */
	static Configuration&		GetConfig();

	/**
	 * @brief					����������
	 * @return					==0					�ɹ�
								!=					����
	 */
    int							Initialize();

public:
	/**
	 * @brief					ȡ�ÿ�������Ŀ¼(���ļ���)
	 */
	const std::string&			GetDumpFolder() const;

	/**
	 * @brief					��ȡ���������
	 */
	const std::string&			GetExchangeID() const;

	/**
	 * @brief					��ȡ�г����
	 */
	unsigned int				GetMarketID() const;

	/**
	 * @brief					��ȡ�����ļ�·��
	 */
	const std::string&			GetMktdt00FilePath() const;

	/**
	 * @brief					��ȡ�ǽ����ļ�·��
	 */
	const std::string&			GetFjyFilePath() const;

	/**
	 * @brief					��ָ֤���ļ�·��
	 */
	const std::string&			GetZzzsFilePath() const;

	const std::string&			GetLonKindPath() const;

private:
	unsigned int				m_nMarketID;			///< �г����
	std::string					m_sExchangeID;			///< ���������
	std::string					m_sDumpFileFolder;		///< ��������·��(��Ҫ���ļ���)
	std::string					m_sMktdt00FilePath;		///< �����ļ�·��
	std::string					m_sFjyFilePath;			///< �ǽ����ļ�����·��(�������ļ��е����ڲ���)
	std::string					m_sZzzsFilePath;		///< ��ָ֤���ļ�·��
	std::string					m_s4xLonKindFilePath;	///< Lonkind�ļ�·��
};


#endif







