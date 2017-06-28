#ifndef __DATA_DUMP_H__
#define __DATA_DUMP_H__


#pragma warning(disable:4786)
#include <string>
#include <fstream>
#include "../Configuration.h"
#include "../Infrastructure/DateTime.h"


/**
 * @brief					连接路径和文件名(处理了是否路径有带分隔符的问题)
 * @param[in]				sPath				保存路径
 * @param[in]				sFileName			文件名
 * @detail					不需要考虑sPath是否以/或\结束
 */
std::string	JoinPath( std::string sPath, std::string sFileName );


/**
 * @brief					生成文件名
 * @param[in]				sFolderPath			保存路径
 * @param[in]				sFileName			文件名
 * @param[in]				nMkDate				日期( 通过日期生成 -> 文件扩展名(星期的索引值) )
 * @note					本函数生成的文件名，可以通过“星期”，保证最多只生成七个文件名，保证不占用过多磁盘空间
 */
std::string	GenFilePathByWeek( std::string sFolderPath, std::string sFileName, unsigned int nMkDate );


/**
 * @class					MemoDumper
 * @brief					将内存数据进行序列化/反序列化
 * @note					文件头是一个日期值(交易日期,unsigned int)
							&
							写文件时会先写入一个临时文件名，然后再rename，以保证文件内容的"完整性"
 * @author					barry
 */
template<class TYPE>
class MemoDumper
{
public:
	/**
	 * @brief				构造
	 * @detail				打开或创建一个文件
	 * @param[in]			pszFileFolder		文件目录
	 * @param[in]			bIsRead				true:为加载一个文件; true:为写一个文件
	 * @param[in]			nTradingDay			行情交易日期
	 * @param[in]			bAppendModel		追加模式(默认为“否”)
	 */
	MemoDumper( bool bIsRead, const char* pszFileFolder, unsigned int nTradingDay, bool bAppendModel = false );
	MemoDumper();
	~MemoDumper();

	/**
	 * @brief				打开或创建一个文件
	 * @param[in]			pszFileFolder		文件目录
	 * @param[in]			bIsRead				true:为加载一个文件; true:为写一个文件
	 * @param[in]			nTradingDay			行情交易日期
	 * @return				true				成功
	 */
	bool					Open( bool bIsRead, const char* pszFileFolder, unsigned int nTradingDay );

	/**
	 * @brief				关闭文件
	 */
	void					Close();

	/**
	 * @brief				是否打开成功
	 */
	bool					IsOpen();

	/**
	 * @brief				取得文件数据对应的行情交易日期
	 * @return				>0			合法数据
							=0			非法数据
	 */
	unsigned int			GetTradingDay();

	/**
	 * @brief				读/写函数
	 * @return				>0			成功，且有后续待读数据
							=0			已经读不出数据
							<0			出错
	 */
	int						Read( TYPE& refData );
	int						Read( char* pData, unsigned int nDataLen );
	int						Write( const TYPE& refData );
	int						Write( const char* pData, unsigned int nDataLen );

protected:
	bool					m_bAppendModel;			///< 文件写追加模式
	unsigned int			m_nTradingDay;			///< 行情数据的交易日期
	std::ifstream			m_fInput;				///< 写文件
	std::ofstream			m_fOutput;				///< 读文件
	bool					m_bIsRead;				///< 是否为读文件
	char					m_pszTargetFile[128];	///< 目标写文件
	char					m_pszTmpFilePath[128];	///< 临时写文件
};


template<class TYPE>
MemoDumper<TYPE>::~MemoDumper()
{
	Close();
}

template<class TYPE>
MemoDumper<TYPE>::MemoDumper()
 : m_bIsRead( true ), m_nTradingDay( 0 ), m_bAppendModel( false )
{
	::memset( m_pszTargetFile, 0, sizeof(m_pszTargetFile) );
	::memset( m_pszTmpFilePath, 0, sizeof(m_pszTmpFilePath) );
}

template<class TYPE>
MemoDumper<TYPE>::MemoDumper( bool bIsRead, const char* pszFileFolder, unsigned int nTradingDay, bool bAppendModel )
 : m_bIsRead( bIsRead ), m_nTradingDay( 0 ), m_bAppendModel( bAppendModel )
{
	::printf( "MemoDumper::MemoDumper() : File Path = %s, read flag = %d, trading day = %u\n", pszFileFolder, bIsRead, nTradingDay );

	if( NULL != pszFileFolder )
	{
		Open( bIsRead, pszFileFolder, nTradingDay );
	}
}

template<class TYPE>
void MemoDumper<TYPE>::Close()
{
	if( true == IsOpen() )
	{
		::printf( "%s", "MemoDumper::Close() : File Closing..\n" );

		if( true == m_bIsRead )
		{
			m_fInput.close();
		}
		else
		{
			m_fOutput.close();
			::MoveFileEx( m_pszTmpFilePath, m_pszTargetFile, MOVEFILE_REPLACE_EXISTING );
		}
	}

	::printf( "%s", "MemoDumper::Close() : File Closed.\n" );
}

template<class TYPE>
bool MemoDumper<TYPE>::Open( bool bIsRead, const char* pszFileFolder, unsigned int nTradingDay )
{
	m_bIsRead = bIsRead;
	::memset( m_pszTargetFile, 0, sizeof(m_pszTargetFile) );
	::printf( "MemoDumper::Open() : File Path = %s, read flag = %d, trading day = %u\n", pszFileFolder, bIsRead, nTradingDay );

	if( true == m_bIsRead )
	{
		m_fInput.open( pszFileFolder, std::ios::in|std::ios::binary );

		if( m_fInput.is_open() )
		{
			m_fInput.seekg( 0, std::ios::beg );
			m_fInput.read( (char*)&m_nTradingDay, sizeof(m_nTradingDay) );
		}
	}
	else
	{
		m_nTradingDay = nTradingDay;
		::strcpy( m_pszTmpFilePath, pszFileFolder );
		::strcpy( m_pszTargetFile, pszFileFolder );
		::strcat( m_pszTmpFilePath, ".tmp" );

		if( true == m_bAppendModel )
		{
			m_fOutput.open( m_pszTmpFilePath , std::ios::out|std::ios::binary|std::ios::app );
		}
		else
		{
			m_fOutput.open( m_pszTmpFilePath , std::ios::out|std::ios::binary );
		}

		if( !m_fOutput.is_open() ) {
			m_nTradingDay = 0;
		}

		m_fOutput.write( (char*)&m_nTradingDay, sizeof(m_nTradingDay) );
	}

	return IsOpen();
}

template<class TYPE>
unsigned int MemoDumper<TYPE>::GetTradingDay()
{
	return m_nTradingDay;
}

template<class TYPE>
bool MemoDumper<TYPE>::IsOpen()
{
	if( true == m_bIsRead )
	{
		return m_fInput.is_open();
	}
	else
	{
		return m_fOutput.is_open();
	}
}

template<class TYPE>
int MemoDumper<TYPE>::Read( TYPE& refData )
{
	return Read( (char*)&refData, sizeof(TYPE) );
}

template<class TYPE>
int MemoDumper<TYPE>::Read( char* pData, unsigned int nDataLen )
{
	if( false == IsOpen() )
	{
		return -1;
	}

//	if( true == m_fInput.eof() )
	if( m_fInput.peek() == EOF || true == m_fInput.eof() )
	{
		return 0;		///< 文件读完
	}

	m_fInput.read( pData, nDataLen );

	return 1;
}

template<class TYPE>
int MemoDumper<TYPE>::Write( const TYPE& refData )
{
	return Write( (const char*)&refData, sizeof(TYPE) );
}

template<class TYPE>
int MemoDumper<TYPE>::Write( const char* pData, unsigned int nDataLen )
{
	if( false == IsOpen() )
	{
		return -1;
	}

	m_fOutput.write( pData, nDataLen );

	return 1;
}






#endif









