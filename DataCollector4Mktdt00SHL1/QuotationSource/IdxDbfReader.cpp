#include "IdxDbfReader.h"


MReadIdxDbfFile::MReadIdxDbfFile()
{
	memset((char *)&m_DbfHeadInfo,0x00,sizeof(tagIdxDbfHeadInfo));
	m_DbfFieldRecord = NULL;
	m_FieldCount = 0;
	m_RecordData = NULL;
	m_FileContent = NULL;
	m_RecordCount = 0;
	m_CurRecordNo = 0;
	m_nRecordLength = 0;
	m_LastFileLength = 0;
	m_LastHeadRecordCount = 0;
	m_FileLength = 0;

}
//..........................................................................................................................
MReadIdxDbfFile::~MReadIdxDbfFile()
{
	//为了避免死锁，这里不进行锁的判断
	inner_delete();
}
//..........................................................................................................................
int  MReadIdxDbfFile::Open( std::string filename )
{
	Close();
	if ( m_sFilePath!= filename )
	{
		m_sFilePath = filename;
		m_fInput.close();
	}
	
	if ( inner_loaddefaultfieldinfo()!=1 )
	{
		return (-1);
	}
	int errorcode = ReloadFromFile();
	if (errorcode!=1 )
	{
		return errorcode;
	}
	return (1);
}
//..........................................................................................................................
void MReadIdxDbfFile::Close(void)
{
	CriticalLock	section( m_oLock );	

	inner_delete();
}
//..........................................................................................................................
void MReadIdxDbfFile::CloseDbfFile()
{
	Close();
	CriticalLock	section( m_oLock );	

	m_fInput.close();
};
//..........................................................................................................................
void MReadIdxDbfFile::inner_delete(void)
{
	if ( m_DbfFieldRecord != NULL )
	{
		delete [] m_DbfFieldRecord;
		m_DbfFieldRecord = NULL;
	}
	m_FieldCount = 0;

	if ( m_RecordData != NULL )
	{
		delete [] m_RecordData;
		m_RecordData = NULL;
	}

	if( m_FileContent != NULL)
	{
		delete[] m_FileContent;
		m_FileContent = NULL;
	}
	m_RecordCount = 0;
	m_nRecordLength = 0;
	m_FileLength = 0;
	m_LastFileLength = 0;
	m_CurRecordNo = 0;
	m_LastHeadRecordCount = 0;
	memset((char *)&m_DbfHeadInfo,0x00,sizeof(tagIdxDbfHeadInfo));
	m_sFilePath = "";
}
//..........................................................................................................................
int  MReadIdxDbfFile::inner_loadfieldinfo(void)
{
	/*
	register int			errorcode;
	register int			i;	
	char					keyname[32];

	if ( m_FieldFilePtr.IsOpened() == false )
	{
		if ( (errorcode = m_FieldFilePtr.Open(m_FieldFileName)) < 0 )
		{
			//打开文件错误
			return(errorcode);
		}
	}
	m_nRecordLength = 0;
	m_FieldCount = m_FieldFilePtr.ReadInteger( "FieldInfo", "Count", 0 );
	if ( m_DbfFieldRecord != NULL )
	{
		delete []m_DbfFieldRecord;
		m_DbfFieldRecord = NULL;
	}
	if ( m_FieldCount>0 )
	{
		int fieldoffset = 0;
		m_DbfFieldRecord = new tagFixDbfFieldInfo[m_FieldCount];
		if ( m_DbfFieldRecord==NULL )
		{
			m_FieldFilePtr.Close();
			return ERROR_DBF_APPLEMEMORY;
		}
		for ( i=0; i<m_FieldCount; i++ )
		{
			sprintf( keyname, "FieldName_%d", i );
			strcpy( m_DbfFieldRecord[i].FieldName, m_FieldFilePtr.ReadString( "FieldInfo", keyname, "" ).c_str());
			
			sprintf( keyname, "FieldType_%d", i );
			m_DbfFieldRecord[i].FieldType = m_FieldFilePtr.ReadInteger( "FieldInfo", keyname, 0 );
			
			m_DbfFieldRecord[i].FieldOffset = fieldoffset;
			
			sprintf( keyname, "FieldSize_%d", i );
			m_DbfFieldRecord[i].FieldSize = m_FieldFilePtr.ReadInteger( "FieldInfo", keyname, 0 );
			
			sprintf( keyname, "DecSize_%d", i );
			m_DbfFieldRecord[i].DecSize = m_FieldFilePtr.ReadInteger( "FieldInfo", keyname, 0 );
			
			fieldoffset += m_DbfFieldRecord[i].FieldSize;
			m_nRecordLength += m_DbfFieldRecord[i].FieldSize;
		}
	}
	m_FieldFilePtr.Close();
	if ( m_FieldCount<=0 || m_nRecordLength<=0 )
	{
		return ERROR_DBF_FIELDDATA;
	}
	*/
	return 1;
};
//..........................................................................................................................
int  MReadIdxDbfFile::inner_parsehead(char *pInBuf, unsigned int nInSize)
{
	char				tempbuf[32];
	register int		n;

	memset((char *)&m_DbfHeadInfo, 0x00, sizeof(tagIdxDbfHeadInfo));
	if(nInSize < RECORDHEADLEN || pInBuf[0] == ' ' || pInBuf[0] == '\r' || pInBuf[0] == '\n')
	{
		return ERROR_IDX_HEADLEN;
	}
	//解析版本号
	strncpy(m_DbfHeadInfo.Ver, pInBuf, 2);
	if(atoi(m_DbfHeadInfo.Ver) < 0)
	{
		return ERROR_IDX_HEADVER;
	}

	//解析交易日期
	memset(tempbuf, 0, sizeof(tempbuf));
	strncpy(tempbuf, pInBuf+3, 8);
	n = atoi(tempbuf);
	if(n < 0 || (n/10000) < 2000 || (n/10000) > 10000 || (n%10000)/100 < 1 || (n%10000)/100 > 12 || (n%100) < 1 || (n%100) > 31)
	{
		return ERROR_IDX_HEADDATE;
	}
	m_DbfHeadInfo.TradeDate = n;

	//解析自然日期
	memset(tempbuf, 0, sizeof(tempbuf));
	strncpy(tempbuf, pInBuf+12, 8);
	n = atoi(tempbuf);
	if(n < 0 || (n/10000) < 2000 || (n/10000) > 10000 || (n%10000)/100 < 1 || (n%10000)/100 > 12 || (n%100) < 1 || (n%100) > 31)
	{
		return ERROR_IDX_HEADDATE;
	}
	m_DbfHeadInfo.NaturalDate = n;

	//解析自然时间
	memset(tempbuf, 0, sizeof(tempbuf));
	strncpy(tempbuf, pInBuf+21, 6);
	n = atoi(tempbuf);
	if(n < 0 || n > 235959)
	{
		return ERROR_IDX_HEADDATE;
	}
	m_DbfHeadInfo.NaturalTime = n;


	memset(tempbuf, 0, sizeof(tempbuf));
	strncpy(tempbuf, pInBuf+28, 10);
	n = atoi(tempbuf);
	if(n < 0)
	{
		return ERROR_IDX_HEADRECORD;
	}
	m_DbfHeadInfo.RecordCount = n;
	return 1;
}
//..........................................................................................................................
int  MReadIdxDbfFile::inner_loadfromfile(void)
{
	register int			errorcode;
	register unsigned int	i, j, k, nLine;
	char					tempbuf[MAXRECORDLEN];
	bool					bLineEnd;

	if ( m_FieldCount <=0 )
	{
		if ((errorcode = inner_loaddefaultfieldinfo())!=1 )
		{
			return (errorcode);
		}
	}

	if ( !m_fInput.is_open() )
	{
		m_fInput.open( m_sFilePath.c_str(), std::ios::in|std::ios::binary );
		if ( !m_fInput.is_open() )
		{
			return -1;
		}
	}

	std::streampos	nCurPos = m_fInput.tellg();
	m_fInput.seekg( 0, std::ios::end );
	m_FileLength = m_fInput.tellg();
	m_fInput.seekg( nCurPos );
	if(m_FileLength == 0)
	{
		m_fInput.close();
		return ERROR_IDX_LENZERO;
	}

	if ( m_LastFileLength < (m_FileLength+1))
	{
		//文件长度有变化
		if ( m_FileContent != NULL )
		{
			delete []m_FileContent;
			m_FileContent = NULL;
		}
	}

	if ( m_FileContent == NULL)
	{
		m_FileContent = new char[m_FileLength+1];
		if ( m_FileContent == NULL )
		{
			//申请内存错误
			m_fInput.close();
			m_LastFileLength = 0;
			return ERROR_IDX_APPFILEMEMORY;
		}
		m_LastFileLength = m_FileLength+1;
	}

	memset(m_FileContent, 0, m_LastFileLength);
	
	m_fInput.seekg( 0, std::ios::beg );
	m_fInput.read( m_FileContent, m_FileLength );
	errorcode = m_fInput.gcount();
	if ( errorcode != m_FileLength )
	{
		m_fInput.close();
		return ERROR_IDX_FILEREAD;
	}
	//add by liuqy 20120420 for 文件读完就关闭，不然下来的行情写不入
	m_fInput.close();
	//解析文件
	j = 0;
	nLine = 0;
	m_RecordCount = 0;
	bLineEnd = false;
	for(i = 0 ; i <= m_FileLength-1; i++)
	{
		if(m_FileContent[i] == 0x0D || m_FileContent[i] == 0x0A  || i == m_FileLength-1)
		{
			if(0 == j && i == m_FileLength-1)
				break;
			tempbuf[j] = 0;
			nLine++;
			bLineEnd = true;
			if(m_FileContent[i+1] == 0x0A || m_FileContent[i+1] == 0x0D )
				i++; //要跳一下0X0A的字符。
		}
		else
		{
			tempbuf[j++] = m_FileContent[i];
			if(j >= MAXRECORDLEN-1)
			{
				m_fInput.close();
				return ERROR_IDX_RECORDTOOLONG;
			}
		}
		if(bLineEnd)
		{
			if(nLine == 1) //处理头
			{
				errorcode = inner_parsehead(tempbuf, j);
				if(errorcode < 1)
				{
					m_fInput.close();
					return errorcode;
				}

				if ( m_LastHeadRecordCount != m_DbfHeadInfo.RecordCount)
				{
					//数据库记录数发生变化
					if ( m_RecordData != NULL )
					{
						delete []m_RecordData;
						m_RecordData = NULL;
					}
				}
				m_LastHeadRecordCount = m_DbfHeadInfo.RecordCount;
				if ( m_RecordData==NULL )
				{
					m_RecordData = new char[m_DbfHeadInfo.RecordCount * m_nRecordLength];
				}
				if ( m_RecordData == NULL )
				{
					m_fInput.close();
					m_LastHeadRecordCount = 0;
					return ERROR_IDX_APPRECORDMEMORY;
				}
				memset(m_RecordData, 0, m_DbfHeadInfo.RecordCount * m_nRecordLength);
			}
			else  //处理各记录类
			{
				if((m_nRecordLength+m_FieldCount-1) == j)
				{
					//add by liuqy 20120405 当空间不够时，不能再处理了
					if(m_RecordCount >= m_DbfHeadInfo.RecordCount)
					{
						m_fInput.close();
						return ERROR_IDX_RECORDCOUNT;
					}
					for (k = 0; k < m_FieldCount; k++ )
					{
						memcpy( m_RecordData+m_nRecordLength*m_RecordCount+ m_DbfFieldRecord[k].FieldOffset,
							&tempbuf[m_DbfFieldRecord[k].FieldOffset+k], m_DbfFieldRecord[k].FieldSize);
					}
					m_RecordCount++;
				}
			}
			memset(tempbuf, 0, sizeof(tempbuf));
			j = 0;
			bLineEnd = false;
		}
	}
	if((nLine-1) != m_DbfHeadInfo.RecordCount)
	{
		m_fInput.close();
		return ERROR_IDX_RECORDCOUNT;
	}
	m_CurRecordNo = 0;

	return(1);
}

int  MReadIdxDbfFile::inner_findfieldno(const char * fieldname)
{
	register int				i;

	if ( m_DbfFieldRecord == NULL || fieldname == NULL )
	{
		return(-1);
	}

	for ( i=0;i<m_FieldCount;i++ )
	{
		if ( strncmp(fieldname,m_DbfFieldRecord[i].FieldName,11) == 0 )
		{
			return(i);
		}
	}

	return(-2);
}

int MReadIdxDbfFile::inner_loaddefaultfieldinfo() //载入中证指数默认字段配置配置。
{
	register int			i = 0;	

	m_nRecordLength = 0;
	m_FieldCount = RECORDFIELDCOUNT;
	if ( m_DbfFieldRecord != NULL )
	{
		delete []m_DbfFieldRecord;
		m_DbfFieldRecord = NULL;
	}
	if ( m_FieldCount>0 )
	{
		int fieldoffset = 0;
		m_DbfFieldRecord = new tagIdxDbfFieldInfo[m_FieldCount];
		if ( m_DbfFieldRecord==NULL )
		{
			return ERROR_IDX_APPFIELDMEMORY;
		}
		memset(m_DbfFieldRecord, 0, sizeof(tagIdxDbfFieldInfo)*m_FieldCount);

		strcpy(m_DbfFieldRecord[i].FieldName, "JLLX");
		m_DbfFieldRecord[i].FieldType = 'C';
		m_DbfFieldRecord[i].FieldSize = 2;
		m_DbfFieldRecord[i].FieldOffset = 0;
		i++;

		strcpy(m_DbfFieldRecord[i].FieldName, "RESERVE");
		m_DbfFieldRecord[i].FieldType = 'C';
		m_DbfFieldRecord[i].FieldSize = 4;
		i++;


		strcpy(m_DbfFieldRecord[i].FieldName, "ZSDM");
		m_DbfFieldRecord[i].FieldType = 'C';
		m_DbfFieldRecord[i].FieldSize = 6;
		i++;

		strcpy(m_DbfFieldRecord[i].FieldName, "JC");
		m_DbfFieldRecord[i].FieldType = 'C';
		m_DbfFieldRecord[i].FieldSize = 20;
		i++;

		strcpy(m_DbfFieldRecord[i].FieldName, "SCDM");
		m_DbfFieldRecord[i].FieldType = 'C';
		m_DbfFieldRecord[i].FieldSize = 1;
		i++;

//新版去掉时间戳
// 		strcpy(m_DbfFieldRecord[i].FieldName, "SJC");
// 		m_DbfFieldRecord[i].FieldType = 'C';
// 		m_DbfFieldRecord[i].FieldSize = 6;
// 		i++;

		strcpy(m_DbfFieldRecord[i].FieldName, "SSZS");
		m_DbfFieldRecord[i].FieldType = 'N';
		m_DbfFieldRecord[i].FieldSize = 11;
		m_DbfFieldRecord[i].DecSize = 4;
		i++;
		
		strcpy(m_DbfFieldRecord[i].FieldName, "DRKP");
		m_DbfFieldRecord[i].FieldType = 'N';
		m_DbfFieldRecord[i].FieldSize = 11;
		m_DbfFieldRecord[i].DecSize = 4;
		i++;
			
			
		strcpy(m_DbfFieldRecord[i].FieldName, "DRZD");
		m_DbfFieldRecord[i].FieldType = 'N';
		m_DbfFieldRecord[i].FieldSize = 11;
		m_DbfFieldRecord[i].DecSize = 4;
		i++;
			
		strcpy(m_DbfFieldRecord[i].FieldName, "DRZX");
		m_DbfFieldRecord[i].FieldType = 'N';
		m_DbfFieldRecord[i].FieldSize = 11;
		m_DbfFieldRecord[i].DecSize = 4;
		i++;

			
		strcpy(m_DbfFieldRecord[i].FieldName, "DRSP");
		m_DbfFieldRecord[i].FieldType = 'N';
		m_DbfFieldRecord[i].FieldSize = 11;
		m_DbfFieldRecord[i].DecSize = 4;
		i++;
			
		strcpy(m_DbfFieldRecord[i].FieldName, "ZRSP");
		m_DbfFieldRecord[i].FieldType = 'N';
		m_DbfFieldRecord[i].FieldSize = 11;
		m_DbfFieldRecord[i].DecSize = 4;
		i++;

		strcpy(m_DbfFieldRecord[i].FieldName, "ZD");
		m_DbfFieldRecord[i].FieldType = 'N';
		m_DbfFieldRecord[i].FieldSize = 11;
		m_DbfFieldRecord[i].DecSize = 4;
		i++;
			
		strcpy(m_DbfFieldRecord[i].FieldName, "ZDF");
		m_DbfFieldRecord[i].FieldType = 'N';
		m_DbfFieldRecord[i].FieldSize = 11;
		m_DbfFieldRecord[i].DecSize = 4;
		i++;

		strcpy(m_DbfFieldRecord[i].FieldName, "CJL");
		m_DbfFieldRecord[i].FieldType = 'N';
		m_DbfFieldRecord[i].FieldSize = 14;
		m_DbfFieldRecord[i].DecSize = 0;
		i++;
			
		strcpy(m_DbfFieldRecord[i].FieldName, "CJJE");
		m_DbfFieldRecord[i].FieldType = 'N';
		m_DbfFieldRecord[i].FieldSize = 16;
		m_DbfFieldRecord[i].DecSize = 5;
		i++;

        strcpy(m_DbfFieldRecord[i].FieldName, "HL");
		m_DbfFieldRecord[i].FieldType = 'N';
		m_DbfFieldRecord[i].FieldSize = 12;
		m_DbfFieldRecord[i].DecSize = 8;
		i++;

		strcpy(m_DbfFieldRecord[i].FieldName, "BZBZ");
		m_DbfFieldRecord[i].FieldType = 'C';
		m_DbfFieldRecord[i].FieldSize = 1;
		i++;
			
		strcpy(m_DbfFieldRecord[i].FieldName, "ZSXH");
		m_DbfFieldRecord[i].FieldType = 'N';
		m_DbfFieldRecord[i].FieldSize = 4;
		m_DbfFieldRecord[i].DecSize = 0;
		i++;
			
		strcpy(m_DbfFieldRecord[i].FieldName, "DRSP2");
		m_DbfFieldRecord[i].FieldType = 'N';
		m_DbfFieldRecord[i].FieldSize = 11;
		m_DbfFieldRecord[i].DecSize = 4;
		i++;
			
		strcpy(m_DbfFieldRecord[i].FieldName, "DRSP3");
		m_DbfFieldRecord[i].FieldType = 'N';
		m_DbfFieldRecord[i].FieldSize = 11;
		m_DbfFieldRecord[i].DecSize = 4;
		i++;
	}
	m_nRecordLength += m_DbfFieldRecord[0].FieldSize;
	for(i = 1; i < m_FieldCount; i++)
	{
		m_DbfFieldRecord[i].FieldOffset = m_DbfFieldRecord[i-1].FieldOffset+ m_DbfFieldRecord[i-1].FieldSize;
		m_nRecordLength += m_DbfFieldRecord[i].FieldSize;
	}
	if ( m_FieldCount<=0 || m_nRecordLength<=0 )
	{
		return ERROR_IDX_FIELDDATA;
	}
	return 1;

}
//..........................................................................................................................
int  MReadIdxDbfFile::ReloadFromFile(void)
{
	register int				errorcode;

	CriticalLock	section( m_oLock );	

	errorcode = inner_loadfromfile();

	return(errorcode);
}
//..........................................................................................................................
int  MReadIdxDbfFile::ReadString(unsigned short fieldno,char * value,unsigned short insize)
{
	register int				errorcode;

	CriticalLock	section( m_oLock );	

	if ( m_DbfFieldRecord == NULL || m_RecordData == NULL || fieldno >= m_FieldCount || insize < m_DbfFieldRecord[fieldno].FieldSize )
	{
		return(-1);
	}

	memcpy(value,m_RecordData + m_nRecordLength * m_CurRecordNo + m_DbfFieldRecord[fieldno].FieldOffset,m_DbfFieldRecord[fieldno].FieldSize);
	errorcode = m_DbfFieldRecord[fieldno].FieldSize;

	if(errorcode < insize)
		value[errorcode] = 0;

	return(errorcode);
}
//..........................................................................................................................
int  MReadIdxDbfFile::ReadString(const char * fieldname,char * value,unsigned short insize)
{
	register int				errorcode;
	register int				fieldno;
	CriticalLock				section( m_oLock );	

	if ( (fieldno = inner_findfieldno(fieldname)) < 0 )
	{
		return(fieldno);
	}

	if ( m_DbfFieldRecord == NULL || m_RecordData == NULL || fieldno >= m_FieldCount || insize < m_DbfFieldRecord[fieldno].FieldSize )
	{
		return(-1);
	}
	
	memcpy(value,m_RecordData + m_nRecordLength * m_CurRecordNo + m_DbfFieldRecord[fieldno].FieldOffset,m_DbfFieldRecord[fieldno].FieldSize);
	errorcode = m_DbfFieldRecord[fieldno].FieldSize;

	if(errorcode < insize)
		value[errorcode] = 0;

	return(errorcode);
}
//..........................................................................................................................
int  MReadIdxDbfFile::ReadInteger(unsigned short fieldno,int * value)
{
	char						tempbuf[256];
	register int				errorcode;

	if ( (errorcode = ReadString(fieldno,tempbuf,256)) < 0 )
	{
		return(errorcode);
	}

	* value = strtol(tempbuf,NULL,10);

	return(1);
}
//..........................................................................................................................
int  MReadIdxDbfFile::ReadInteger(const char * fieldname,int * value)
{
	char						tempbuf[256];
	register int				errorcode;
	
	if ( (errorcode = ReadString(fieldname,tempbuf,256)) < 0 )
	{
		return(errorcode);
	}
	
	* value = strtol(tempbuf,NULL,10);
	
	return(1);
}
//..........................................................................................................................
int  MReadIdxDbfFile::ReadFloat(unsigned short fieldno,double * value)
{
	char						tempbuf[256];
	register int				errorcode;
	
	if ( (errorcode = ReadString(fieldno,tempbuf,256)) < 0 )
	{
		return(errorcode);
	}
	
	* value = strtod(tempbuf,NULL);
	
	return(1);
}
//..........................................................................................................................
int  MReadIdxDbfFile::ReadFloat(const char * fieldname,double * value)
{
	char						tempbuf[256];
	register int				errorcode;
	
	if ( (errorcode = ReadString(fieldname,tempbuf,256)) < 0 )
	{
		return(errorcode);
	}
	
	* value = strtod(tempbuf,NULL);
	
	return(1);
}
//..........................................................................................................................
int  MReadIdxDbfFile::First(void)
{
	CriticalLock	section( m_oLock );	
	m_CurRecordNo = 0;

	return(1);
}
//..........................................................................................................................
int  MReadIdxDbfFile::Last(void)
{
	CriticalLock	section( m_oLock );	

	m_CurRecordNo = m_RecordCount-1;
	
	return(1);
}
//..........................................................................................................................
int  MReadIdxDbfFile::Prior(void)
{
	CriticalLock	section( m_oLock );	

	if ( m_CurRecordNo == 0 )
	{
		return(-1);
	}

	m_CurRecordNo --;

	return(1);
}
//..........................................................................................................................
int  MReadIdxDbfFile::Next(void)
{
	CriticalLock	section( m_oLock );	

	if ( m_CurRecordNo >= (int)(m_RecordCount - 1) )
	{
		return(-1);
	}
	
	m_CurRecordNo ++;
	
	return(1);
}
//..........................................................................................................................
int  MReadIdxDbfFile::Goto(int recno)
{
	CriticalLock	section( m_oLock );	
	
	if ( recno < 0 || recno >= (int)m_RecordCount )
	{
		return(-1);
	}

	m_CurRecordNo = recno;
	
	return(1);
}

int  MReadIdxDbfFile::GetDbfDate(void)
{
	int nHeadDate = 0;
	CriticalLock	section( m_oLock );	

	nHeadDate = m_DbfHeadInfo.TradeDate;

	return nHeadDate;
}

int  MReadIdxDbfFile::GetRecordCount(void)
{
	register int				errorcode;
	
	CriticalLock	section( m_oLock );	

	errorcode = m_RecordCount;

	return(errorcode);
}
//..........................................................................................................................
int  MReadIdxDbfFile::GetFieldCount(void)
{
	register int				errorcode;
	
	CriticalLock	section( m_oLock );	
	
	errorcode = m_FieldCount;
	
	return(errorcode);
}
//--------------------------------------------------------------------------------------------------------------------------------
int   MReadIdxDbfFile::FieldToInteger(char * FieldName)
{
	int ret = 0;
	ReadInteger(FieldName, &ret);

	return ret;
}
//--------------------------------------------------------------------------------------------------------------------------------
double MReadIdxDbfFile::FieldToFloat(char * FieldName)
{
	double ret = 0;
	ReadFloat(FieldName, &ret);

	return ret;
}
//--------------------------------------------------------------------------------------------------------------------------------
int   MReadIdxDbfFile::FieldToInteger(unsigned short fieldno)
{
	int ret = 0;
	ReadInteger(fieldno, &ret);

	return ret;
}
//--------------------------------------------------------------------------------------------------------------------------------
double MReadIdxDbfFile::FieldToFloat(unsigned short fieldno)
{
	double ret = 0;
	ReadFloat(fieldno, &ret);

	return ret;
}
//--------------------------------------------------------------------------------------------------------------------------
int  MReadIdxDbfFile::FindRecord(char * FieldName, char * Value)
{
	register int								i;
	char										tempbuf[256];

	memset(tempbuf, 0x20, 256);
	memcpy(tempbuf, Value, strlen(Value));

	for ( i=0; i<GetRecordCount(); i++ )
	{
		if (Goto(i)!=1)
			break;
		
		ReadString(FieldName, tempbuf, 255);
		if ( !memcmp(Value, tempbuf, strlen(Value)) )
		{
			return(1);
		}
	}

	return(0);
}
//--------------------------------------------------------------------------------------------------------------------------
int  MReadIdxDbfFile::FindRecord(unsigned short fieldno, char * Value)
{
	register int								i;
	char										tempbuf[256];

	memset(tempbuf, 0x20, 256);
	memcpy(tempbuf, Value, strlen(Value));

	for ( i=0; i<GetRecordCount(); i++ )
	{
		if (Goto(i)!=1)
			break;
		
		ReadString(fieldno, tempbuf, 255);
		if ( !memcmp(Value, tempbuf, strlen(Value)) )
		{
			return(1);
		}
	}

	return(0);
}
//..............................................................................................................................
