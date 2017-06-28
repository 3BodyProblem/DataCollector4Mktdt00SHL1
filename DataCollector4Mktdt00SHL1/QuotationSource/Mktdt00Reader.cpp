#include "Mktdt00Reader.h"
#include "../DataCollector4Mktdt00SHL1.h"


CReadMktdtFile::CReadMktdtFile()
{
	memset(&m_stHead, 0, sizeof(m_stHead));
	m_pstRecData = NULL;
	m_uiRecCount = 0;
	m_uiRecSize = 0;
	m_pszFileData = NULL;
	m_uiFileDataSize = 0;

	m_ushProcFileErrCount = 0;

	m_poMD1FieldRecord = NULL;
	m_ushMD1FieldCount = 0;
	m_poMD2FieldRecord = NULL;
	m_ushMD2FieldCount = 0;
	m_poMD3FieldRecord = NULL;
	m_ushMD3FieldCount = 0;
	m_poMD4FieldRecord = NULL;
	m_ushMD4FieldCount = 0;
}

void CReadMktdtFile::inner_delete()
{
	m_uiRecCount = 0;
	m_uiRecSize = 0;
	m_uiFileDataSize = 0;

	if ( m_pszFileData != NULL )
	{
		delete [] m_pszFileData;
		m_pszFileData = NULL;
	}
	
	if ( m_pstRecData != NULL )
	{
		delete [] m_pstRecData;
		m_pstRecData = NULL;
	}
	memset(&m_stHead, 0, sizeof(m_stHead));
	m_fInput.close();
	m_sFilePath = "";

	m_ushMD1FieldCount = 0;
	m_ushMD2FieldCount = 0;
	m_ushMD3FieldCount = 0;
	m_ushMD4FieldCount = 0;
	
	if(NULL != m_poMD3FieldRecord)
	{
		delete [] m_poMD3FieldRecord;
		m_poMD3FieldRecord = NULL;
	}
	if(NULL != m_poMD4FieldRecord )
	{
		delete [] m_poMD4FieldRecord;
		m_poMD4FieldRecord = NULL;
	}
	if(NULL != m_poMD2FieldRecord)
	{
		delete [] m_poMD2FieldRecord;
		m_poMD2FieldRecord = NULL;

	}
	if(NULL != m_poMD1FieldRecord)
	{
		delete [] m_poMD1FieldRecord;
		m_poMD1FieldRecord = NULL;

	}
}

void CReadMktdtFile::Close(void)
{
	CriticalLock	section( m_oLock );
	
	m_fInput.close();	
	memset(&m_stHead, 0, sizeof(m_stHead));
	m_uiRecCount = 0;
}

int CReadMktdtFile::Instance()
{
	if(0 > SetFieldInfo())
		return -1;
	return 1;
}

void CReadMktdtFile::Release(void)
{
	CriticalLock	section( m_oLock );	

	inner_delete();
}


CReadMktdtFile::~CReadMktdtFile()
{
	//Ϊ�˱������������ﲻ���������ж�
	inner_delete();
}

int  CReadMktdtFile::ChkOpen( std::string filename )
{
	Close();

	if( m_sFilePath!= filename )
	{
		m_sFilePath = filename;
	}

	m_fInput.close();

	return ReloadFromFile();
}

int  CReadMktdtFile::ReloadFromFile()
{
	CriticalLock	section( m_oLock );	
	
	int	errorcode = inner_loadfromfile();

	m_fInput.close();

	return errorcode;
}

int  CReadMktdtFile::inner_loadfromfile()
{
	int						i;
	register int			errorcode;
	unsigned int			uiFileLen, uiFieldCount;
	char*					pszField[MKTDT_FIELD_COUNT];
	bool					blFlag;
	bool					blLineEnd;
	CriticalLock			section( m_oLock );	

	m_uiLine = 0;
	m_ushProcFileErrCount++;

	if( !m_fInput.is_open() )
	{
		m_fInput.open( m_sFilePath.c_str(), std::ios::in|std::ios::binary );
		if( !m_fInput.is_open() )
		{
			m_ushPrtErrCount++;
			QuoCollector::GetCollector()->OnLog( TLV_WARN, "CReadMktdtFile::inner_loadfromfile() : failed 2 open mktdt00.txt, %s", m_sFilePath.c_str() );
			m_iErrorCode = errorcode;
			return -1;
		}
	}

	std::streampos	nCurPos = m_fInput.tellg();
	m_fInput.seekg( 0, std::ios::end );
	uiFileLen = m_fInput.tellg();
	m_fInput.seekg( nCurPos );

	if( uiFileLen == 0 )
	{
		m_fInput.close();
		m_ushPrtErrCount++;
		errorcode = ERROR_MKTDT_LENZERO;
		if(m_iErrorCode != errorcode && ( 1 == m_ushPrtErrCount || 0 == m_ushProcFileErrCount % 15))
		{
			QuoCollector::GetCollector()->OnLog( TLV_WARN, "CReadMktdtFile::inner_loadfromfile() : mktdt00.txt is empty" );
		}
		m_iErrorCode = errorcode;
		if(20 < m_ushProcFileErrCount)
			return ERROR_MKTDT_LENZERO ;
		return 0;
	}

	if ( m_uiFileDataSize < (uiFileLen+1) || NULL == m_pszFileData)
	{
		if ( m_pszFileData != NULL )
		{
			delete []m_pszFileData;
			m_pszFileData = NULL;
		}
		m_pszFileData = new char[uiFileLen+1];
		if (NULL == m_pszFileData)
		{
			m_fInput.close();
			errorcode =  GetLastError();
			m_ushPrtErrCount++;
			if(m_iErrorCode != errorcode && ( 1 == m_ushPrtErrCount || 0 == m_ushProcFileErrCount % 15))
			{
				QuoCollector::GetCollector()->OnLog( TLV_WARN, "CReadMktdtFile::inner_loadfromfile() : failed 2 allocate memory, errorcode=%d", errorcode );
			}
			m_iErrorCode = errorcode;

			m_uiFileDataSize = 0;
			if(20 < m_ushProcFileErrCount)
				return ERROR_MKTDT_APPFILEMEMORY ;
			return 0;
		}
		m_uiFileDataSize = uiFileLen+1;	
	}

	m_fInput.seekg( 0, std::ios::beg );
	m_fInput.read( m_pszFileData, uiFileLen );
	errorcode = m_fInput.gcount();
	if( errorcode != uiFileLen )
	{
		m_fInput.close();
		m_ushPrtErrCount++;
		if(m_iErrorCode != errorcode && ( 1 == m_ushPrtErrCount || 0 == m_ushProcFileErrCount % 15))
		{
			QuoCollector::GetCollector()->OnLog( TLV_WARN, "CReadMktdtFile::inner_loadfromfile() : �ļ�����%d ��ȡ���ݷ���:%d", uiFileLen, errorcode  );
		}
		m_iErrorCode = errorcode;

		if(20 < m_ushProcFileErrCount)
			return ERROR_MKTDT_FILEREAD;
		return 0;
	}
	m_pszFileData[errorcode] = 0;
	m_fInput.close();
	m_uiLine = 0;
	m_uiRecCount = 0;

	::memset( pszField, NULL, sizeof(pszField) );
	uiFieldCount = 0;
	pszField[uiFieldCount++] = &m_pszFileData[0];

	blFlag = false;
	//�ȴ���ͷ��
	for( i = 0 ; i <= uiFileLen; i++ )
	{
		if(m_pszFileData[i] == 0x0D || m_pszFileData[i] == 0x0A  || i == uiFileLen || 0 == m_pszFileData[i])
		{
			break;
		}
		if('|' == m_pszFileData[i] || 0 == m_pszFileData[i])
		{
			m_pszFileData[i] = 0;
			pszField[uiFieldCount++] = &m_pszFileData[i+1];
		}
	}
	if(0 > (errorcode = inner_parsehead(pszField, uiFieldCount)))
		return errorcode;
	if(m_uiRecSize < m_stHead.uiRecCount || NULL == m_pstRecData)
	{
		if(NULL != m_pstRecData)
			delete [] m_pstRecData;
		m_pstRecData = new TMKTDTFieldData[ m_stHead.uiRecCount + 1];
		if (NULL == m_pstRecData)
		{
			m_ushPrtErrCount++;
			errorcode =  GetLastError();
			if(m_iErrorCode != errorcode && ( 1 == m_ushPrtErrCount || 0 == m_ushProcFileErrCount % 15))
			{
				QuoCollector::GetCollector()->OnLog( TLV_WARN, "CReadMktdtFile::inner_loadfromfile() : �����ļ����󣺷����ڴ����%d", GetLastError() );
			}
			m_iErrorCode = errorcode;

			if(20 < m_ushProcFileErrCount)
				return ERROR_MKTDT_APPRECORDMEMORY;
			return 0;
		}
		m_uiRecSize = m_stHead.uiRecCount+1;					
	}

	blFlag = false;
	blLineEnd = true;
	while(i < uiFileLen && !blFlag)
	{
		if( 0 == m_pszFileData[i])
			break;
		if(m_pszFileData[i] == 0x0D || m_pszFileData[i] == 0x0A || blLineEnd)
		{
			if(m_pszFileData[i] == 0x0D && m_pszFileData[i+1] == 0x0A )
			{
				m_pszFileData[i] = 0;				
				i++;
				i++;
			}
			else if(m_pszFileData[i] == 0x0A && m_pszFileData[i+1] == 0x0D )
			{
				m_pszFileData[i] = 0;				
				i++;
				i++;
			}
			else if(m_pszFileData[i] == 0x0D || m_pszFileData[i] == 0x0A )
			{
				m_pszFileData[i] = 0;				
				i++;
			}

			//�����������з������ٴ����������
			if(m_pszFileData[i] == 0x0A || m_pszFileData[i] == 0x0D )
			{
				i++;
				break;
			}
			if(blLineEnd || 1 < uiFieldCount)
				m_uiLine++;
			blLineEnd = false;
			errorcode = inner_AnalyzeField(m_pszFileData, i, pszField, MKTDT_FIELD_COUNT, blLineEnd);
			if(0 < errorcode)
			{
				uiFieldCount = errorcode;
				if(0 > (errorcode = inner_parseData(pszField, uiFieldCount)))
				{
					m_ushPrtErrCount++;
					if(m_iErrorCode != errorcode && ( 1 == m_ushPrtErrCount || 0 == m_ushProcFileErrCount % 15))
					{
						if(ERROR_MKTDT_NOTFOUNDRECORD == errorcode)
						{
							QuoCollector::GetCollector()->OnLog( TLV_WARN, "CReadMktdtFile::inner_loadfromfile() : �����¼���󣺼�¼������ȷ" );
						}
						else if(ERROR_MKTDT_FIELDCOUNT == errorcode)
						{
							QuoCollector::GetCollector()->OnLog( TLV_WARN, "CReadMktdtFile::inner_loadfromfile() : �����¼�����ֶ���������ȷ" );
						}
						else
						{
							QuoCollector::GetCollector()->OnLog( TLV_WARN, "CReadMktdtFile::inner_loadfromfile() : �����¼����%d", errorcode );
						}

					}
					m_iErrorCode = errorcode;

					if(20 < m_ushProcFileErrCount)
						return errorcode;
					return 0;
				}

			}

			uiFieldCount = 0;
			pszField[uiFieldCount++] = &m_pszFileData[i];
		}
		if(blLineEnd)
		{

		}
		else
		{
			if('|' == m_pszFileData[i] || 0 == m_pszFileData[i])
			{
				m_pszFileData[i] = 0;
				if(1 == uiFieldCount && 0 == strcmp(pszField[0], "TRAILER"))
				{
					blFlag = true;
					break;
				}
				pszField[uiFieldCount++] = &m_pszFileData[i+1];
			}
			
			i++;
		}
	}
	if(!blFlag || m_uiLine-1 != m_stHead.uiRecCount)
	{
		m_ushPrtErrCount++;
		errorcode = -1;
	//	if(m_iErrorCode != errorcode && ( 1 == m_ushPrtErrCount || 0 == m_ushProcFileErrCount % 15))
		{
			QuoCollector::GetCollector()->OnLog( TLV_WARN, "CReadMktdtFile::inner_loadfromfile() : �ļ�������־��%s �ļ���¼����С:%d��ϵͳҪ��%d��¼,���ü�¼��%d", blFlag ? "����":"δ����",  m_uiLine-1, m_stHead.uiRecCount, m_uiRecCount );
		}
		m_iErrorCode = errorcode;

		if(20 < m_ushProcFileErrCount)
			return errorcode;
		return 0;
		
	}
	
	m_ushProcFileErrCount = 0;
	m_ushPrtErrCount = 0;
	m_iErrorCode = 0;

	
	return(1);
}



int  CReadMktdtFile::GetHead(TMKTDTHead ** out_ppstHead)
{
	*out_ppstHead = &m_stHead;
	return 1;
}
int  CReadMktdtFile::GetRecord(unsigned short in_ushRecPos, TMKTDTFieldData ** out_ppstFileData)
{
	if( in_ushRecPos >= m_uiRecCount)
		return ERROR_MKTDT_NOTFOUNDRECORD;

	* out_ppstFileData = &m_pstRecData[in_ushRecPos];
	return 1;

}


//..........................................................................................................................
int  CReadMktdtFile::inner_parsehead(char *pszField[], unsigned short ushFieldCount)
{

	if(9 != ushFieldCount)
		return ERROR_MKTDT_FIELDCOUNT;


	memcpy(m_stHead.szFlags, pszField[0], sizeof(m_stHead.szFlags));		//��ʼ��ʶ
	memcpy(m_stHead.szVer, pszField[1], sizeof(m_stHead.szVer));			//�汾
	m_stHead.uiBodyLength = atoi(pszField[2]);	//����
	m_stHead.uiRecCount = atoi(pszField[3]);		//��¼��
	m_stHead.uiReportID = atoi(pszField[4]);	//�������
	memcpy(m_stHead.szSenderCmpID, pszField[5], sizeof(m_stHead.szSenderCmpID));	//���ͷ���ʾ��
	//��7��
	pszField[6][8] = 0;
	m_stHead.uiTrDate = atoi(pszField[6]);

	pszField[6][9 + 2] = 0;
	m_stHead.uiTrTime = atoi(&pszField[6][9]) * 10000;
	pszField[6][9 + 2 + 3] = 0;
	m_stHead.uiTrTime += (atoi(&pszField[6][12]) * 100);
	pszField[6][9 + 2 + 3 + 3] = 0;
	m_stHead.uiTrTime += atoi(&pszField[6][15]) ;
	//��8��				
	m_stHead.uiUpdateType = atoi(pszField[7]);	//���ͷ�ʽ 0 = ����Full refresh	1 = ����Incremental���ݲ�֧�֣�
	memcpy(m_stHead.szSesStatus, pszField[8], sizeof(m_stHead.szSesStatus));	//�г�����״̬ ȫ�г�����״̬��
				//���ֶ�Ϊ8λ�ַ���������ÿλ��ʾ�ض��ĺ��壬�޶�������ո�
				//	��1λ��'S'��ʾȫ�г������ڼ䣨����ǰ����'T'��ʾȫ�г����ڽ����ڼ䣨���м����У��� 'E'��ʾȫ�г����ڱ����ڼ䡣
				//	��2λ��'1'��ʾ���̼��Ͼ��۽�����־��δ����ȡ'0'��
				//	��3λ��'1'��ʾ�г����������־��δ����ȡ'0'


	if(0 != memcmp(m_stHead.szFlags, "HEADER", sizeof(m_stHead.szFlags)))
		return ERROR_MKTDT_HEADANALYZE;

	return 1;

}

int  CReadMktdtFile::inner_parseData(char *pszField[], unsigned short ushFieldCount)
{
	

	if(NULL == m_pstRecData || m_uiRecSize <= m_uiRecCount)
	{
		return ERROR_MKTDT_NOTFOUNDRECORD;
	}
	if( 13 > ushFieldCount)
	{
		return ERROR_MKTDT_FIELDCOUNT;
	}


	//��1���ֶ�
	memcpy(m_pstRecData[m_uiRecCount].szStreamID, pszField[0], sizeof(m_pstRecData[m_uiRecCount].szStreamID));
	m_pstRecData[m_uiRecCount].uiStreamID = atoi(&pszField[0][2]);
	//�� 2���ֶ�
	memcpy(m_pstRecData[m_uiRecCount].szSecurityID, pszField[1], sizeof(m_pstRecData[m_uiRecCount].szSecurityID));

	//��3���ֶ�
	memcpy(m_pstRecData[m_uiRecCount].szSymbol, pszField[2], sizeof(m_pstRecData[m_uiRecCount].szSymbol));

	//��4���ֶ�
	m_pstRecData[m_uiRecCount].dTradeVolume = atof(pszField[3]);		//�ɽ�����

	//��5���ֶ�
	m_pstRecData[m_uiRecCount].dTotalValueTraded = atof(pszField[4]) + 0.000001;		//�ɽ����
	//�� 6���ֶ�
	m_pstRecData[m_uiRecCount].dPreClosePx = atof(pszField[5]);			//�������̼�	

	//�� 7���ֶ�
	m_pstRecData[m_uiRecCount].dOpenPrice = atof(pszField[6]);		//���տ��̼�	
	
	//��8���ֶ�
	m_pstRecData[m_uiRecCount].dHighPrice = atof(pszField[7]);		//	��߼�	
	
	//��9���ֶ�
	m_pstRecData[m_uiRecCount].dLowPrice = atof(pszField[8]);		//	��ͼ�	

	//��10���ֶ�
	m_pstRecData[m_uiRecCount].dTradePrice = atof(pszField[9]);	//	���¼�	���³ɽ���
	
	//��11���ֶ�
	m_pstRecData[m_uiRecCount].dClosePx = atof(pszField[10]);		//	�����̼�	
	//�� 12���ֶ�
	if(1 == m_pstRecData[m_uiRecCount].uiStreamID) //��ָ��
	{
		memcpy(m_pstRecData[m_uiRecCount].szTradingPhaseCode, pszField[11], 
					sizeof(m_pstRecData[m_uiRecCount].szTradingPhaseCode));
		//�� 13���ֶ�

		pszField[12][2] = 0;
		m_pstRecData[m_uiRecCount].uiTimestamp = atoi(pszField[12]) * 10000;

		pszField[12][3+2] = 0;

		m_pstRecData[m_uiRecCount].uiTimestamp += (atoi(&pszField[12][3]) * 100);
		pszField[12][3+2+3] = 0;
		m_pstRecData[m_uiRecCount].uiTimestamp += atoi(&pszField[12][6]) ;
		
		m_uiRecCount++;
	}
	else
	{

		m_pstRecData[m_uiRecCount].stBuy[0].dPrice = atof(pszField[11]);		//	�����һ
		//��13���ֶ�
		m_pstRecData[m_uiRecCount].stBuy[0].dVolume = atof(pszField[12]);		//	�����һ

		//��14���ֶ�
		m_pstRecData[m_uiRecCount].stSell[0].dPrice = atof(pszField[13]);	//	������һ
		//��15���ֶ�
		m_pstRecData[m_uiRecCount].stSell[0].dVolume =atof(pszField[14]);		//	������һ
		//��16���ֶ�
		m_pstRecData[m_uiRecCount].stBuy[1].dPrice = atof(pszField[15]);		//	����۶�
		//��17���ֶ�
		m_pstRecData[m_uiRecCount].stBuy[1].dVolume = atof(pszField[16]);	//����۶�
	
		//��18���ֶ�
		m_pstRecData[m_uiRecCount].stSell[1].dPrice = atof(pszField[17]);	//	����۶�
		//��19���ֶ�
		m_pstRecData[m_uiRecCount].stSell[1].dVolume = atof(pszField[18]);		//	����۶�
		//��20���ֶ�
		m_pstRecData[m_uiRecCount].stBuy[2].dPrice = atof(pszField[19]);		//	�������
		
		//��21���ֶ�
		m_pstRecData[m_uiRecCount].stBuy[2].dVolume = atof(pszField[20]);	//	�������
				
		//��22���ֶ�
		m_pstRecData[m_uiRecCount].stSell[2].dPrice = atof(pszField[21]);	//	�������
		
		//��23���ֶ�
		m_pstRecData[m_uiRecCount].stSell[2].dVolume = atof(pszField[22]);		//	�������
		
		//��24���ֶ�
		m_pstRecData[m_uiRecCount].stBuy[3].dPrice = atof(pszField[23]);	//	�������
				
		//��25���ֶ�
		m_pstRecData[m_uiRecCount].stBuy[3].dVolume = atof(pszField[24]);		//	�������

		//��26���ֶ�
		m_pstRecData[m_uiRecCount].stSell[3].dPrice = atof(pszField[25]);	//	�������
				
		//��27���ֶ�
		m_pstRecData[m_uiRecCount].stSell[3].dVolume = atof(pszField[26]);		//	�������
		//��28���ֶ�
		m_pstRecData[m_uiRecCount].stBuy[4].dPrice = atof(pszField[27]);		//	�������
				
		//��29���ֶ�
		m_pstRecData[m_uiRecCount].stBuy[4].dVolume = atof(pszField[28]);	//	�������
				
		//��30���ֶ�
		m_pstRecData[m_uiRecCount].stSell[4].dPrice = atof(pszField[29]);		//	�������
				
		//�� 31���ֶ�
		m_pstRecData[m_uiRecCount].stSell[4].dVolume = atof(pszField[30]);		//	�������
		//��32���ֶ�
		if(4 == m_pstRecData[m_uiRecCount].uiStreamID) //�ǻ���
		{
			if( 35 > ushFieldCount)
			{
				return ERROR_MKTDT_FIELDCOUNT;
			}
			m_pstRecData[m_uiRecCount].dPreCloseIOPV = atof(pszField[31]);
			//��33���ֶ�
			m_pstRecData[m_uiRecCount].dIOPV = atof(pszField[32]);
			//��34���ֶ�
			memcpy(m_pstRecData[m_uiRecCount].szTradingPhaseCode, pszField[33],
				sizeof(m_pstRecData[m_uiRecCount].szTradingPhaseCode));

			//��35���ֶ�
			pszField[34][2] = 0;
			m_pstRecData[m_uiRecCount].uiTimestamp = atoi(pszField[34]) * 10000;
			
			pszField[34][3+2] = 0;			
			m_pstRecData[m_uiRecCount].uiTimestamp += (atoi(&pszField[34][3]) * 100);

			pszField[34][3+2+3] = 0;
			m_pstRecData[m_uiRecCount].uiTimestamp += atoi(&pszField[34][6]) ;
			
			m_uiRecCount++;

		}
		else
		{
			if( 33 > ushFieldCount)
			{
				return ERROR_MKTDT_FIELDCOUNT;
			}
			memcpy(m_pstRecData[m_uiRecCount].szTradingPhaseCode, pszField[31],
						sizeof(m_pstRecData[m_uiRecCount].szTradingPhaseCode));

			//��33���ֶ�
			pszField[32][2] = 0;
			m_pstRecData[m_uiRecCount].uiTimestamp = atoi(pszField[32]) * 10000;
			
			pszField[32][3+2] = 0;			
			m_pstRecData[m_uiRecCount].uiTimestamp += (atoi(&pszField[32][3]) * 100);
			
			pszField[32][3+2+3] = 0;
			m_pstRecData[m_uiRecCount].uiTimestamp += atoi(&pszField[32][6]) ;
			
			m_uiRecCount++;
		}
	}


	return 1;
	
}
int  CReadMktdtFile::inner_AnalyzeField(char * in_pszData, int &lPos, char *out_pszField[], unsigned short in_ushFieldSize,
										bool & out_blLineEnd)
{
	TMktdtFieldInfo	* pFieldInfo = NULL;
	unsigned short		ushFieldCount = 0;
	int lFieldTypePos = 0;
	int i;
	out_blLineEnd = false;
	if(0 == memcmp(MKTDT_FIELD_MD1_MAGIC_FLAG, &in_pszData[lPos], strlen(MKTDT_FIELD_MD1_MAGIC_FLAG)))
	{
		pFieldInfo = m_poMD1FieldRecord;
		ushFieldCount =m_ushMD1FieldCount;
		lFieldTypePos = 1;
	}
	else if(0 == memcmp(MKTDT_FIELD_MD2_MAGIC_FLAG, &in_pszData[lPos], strlen(MKTDT_FIELD_MD2_MAGIC_FLAG)))
	{
		pFieldInfo = m_poMD2FieldRecord;
		ushFieldCount =m_ushMD2FieldCount;
		lFieldTypePos = 2;
	}
	else if(0 == memcmp(MKTDT_FIELD_MD3_MAGIC_FLAG, &in_pszData[lPos], strlen(MKTDT_FIELD_MD3_MAGIC_FLAG)))
	{
		pFieldInfo = m_poMD3FieldRecord;
		ushFieldCount =m_ushMD3FieldCount;
		lFieldTypePos = 3;
	}
	else if(0 == memcmp(MKTDT_FIELD_MD4_MAGIC_FLAG, &in_pszData[lPos], strlen(MKTDT_FIELD_MD4_MAGIC_FLAG)))
	{
		pFieldInfo = m_poMD4FieldRecord;
		ushFieldCount =m_ushMD4FieldCount;
		lFieldTypePos = 4;
	}
	else
	{
		//δ�ҵ�
		return 0;
	}
	if(NULL ==  pFieldInfo || 0 == 	ushFieldCount )
	{
		QuoCollector::GetCollector()->OnLog( TLV_WARN, "CReadMktdtFile::inner_AnalyzeField() : ���ļ�%d�У�����%d��¼ʱ��δ���ֶ���Ϣ", m_uiLine+1, lFieldTypePos );
		return -1;
	}
	if(in_ushFieldSize < ushFieldCount)
	{
		QuoCollector::GetCollector()->OnLog( TLV_WARN, "CReadMktdtFile::inner_AnalyzeField() : ���ļ�%d�У�����%d��¼ʱ���ֶγߴ�%d����%d", m_uiLine+1, lFieldTypePos, in_ushFieldSize, ushFieldCount );
		return -2;
	}

	for (i = 0; i < ushFieldCount-1; i++ )
	{
		out_pszField[i] = &in_pszData[lPos];
		lPos += pFieldInfo[i].FieldSize; 
		if('|' != in_pszData[lPos] )
		{
			QuoCollector::GetCollector()->OnLog( TLV_WARN, "CReadMktdtFile::inner_AnalyzeField() : ���ļ�%d�У�����%d��¼ʱ����%d�ֶ�ʱ��û���ҵ��ָ����������Ǵ�λ", m_uiLine+1, lFieldTypePos, i+1 );
			return -1;
		}
		in_pszData[lPos] = 0;
		lPos ++;
	}
	out_pszField[i] = &in_pszData[lPos];
	lPos += pFieldInfo[i].FieldSize; 
	if(in_pszData[lPos] == 0x0D && in_pszData[lPos+1] == 0x0A)
	{
		in_pszData[lPos] = 0;
		lPos ++;
		out_blLineEnd = true;

	}
	if(in_pszData[lPos] == 0x0D || in_pszData[lPos] == 0x0A)
	{
		out_blLineEnd = true;
		
	}
	in_pszData[lPos] = 0;
	lPos ++;

	return ushFieldCount;

}

//�����ֶ���Ϣ
int  CReadMktdtFile::SetFieldInfo()
{
	int lRet;
	if(0 > (lRet = inner_SetMD1fieldinfo()))
		return lRet;
	if(0 > (lRet = inner_SetMD2fieldinfo()))
		return lRet;
	if(0 > (lRet = inner_SetMD3fieldinfo()))
		return lRet;
	if(0 > (lRet = inner_SetMD4fieldinfo()))
		return lRet;
	return 1;

}


int CReadMktdtFile::inner_SetMD1fieldinfo() //�����ֶ����á�
{
	register int			i = 0;	
	TMktdtFieldInfo		olstMDFieldRecord[MKTDT_FIELD_COUNT];
	
		
	//	1	MDStreamID	������������	C5
	strcpy(olstMDFieldRecord[i].FieldName, "MDStreamID");
	olstMDFieldRecord[i].FieldType = 'C';
	olstMDFieldRecord[i].FieldSize = 5;
	olstMDFieldRecord[i].FieldOffset = 0;
	i++;
	
	//	2	SecurityID	��Ʒ����	C6
	strcpy(olstMDFieldRecord[i].FieldName, "SecurityID");
	olstMDFieldRecord[i].FieldType = 'C';
	olstMDFieldRecord[i].FieldSize = 6;
	i++;
	// 		3	Symbol	��Ʒ����	C8
	
	strcpy(olstMDFieldRecord[i].FieldName, "Symbol");
	olstMDFieldRecord[i].FieldType = 'C';
	olstMDFieldRecord[i].FieldSize = 8;
	i++;
	// 		4	TradeVolume	�ɽ�����	N16
	
	strcpy(olstMDFieldRecord[i].FieldName, "TradeVolume");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 16;
	i++;
	// 		5	TotalValueTraded	�ɽ����	N16(2)
	strcpy(olstMDFieldRecord[i].FieldName, "TotalValueTraded");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 16;
	olstMDFieldRecord[i].DecSize = 2;
	i++;
	// 		6	PreClosePx	�������̼�	N11(4)
	
	strcpy(olstMDFieldRecord[i].FieldName, "PreClosePx");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 4;
	i++;
	// 		7	OpenPrice	���տ��̼�	N11(4)
	strcpy(olstMDFieldRecord[i].FieldName, "OpenPrice");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 4;
	i++;
	// 		8	HighPrice	��߼�	N11(4)
	
	strcpy(olstMDFieldRecord[i].FieldName, "HighPrice");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 4;
	i++;
	// 		9	LowPrice	��ͼ�	N11(4)
	strcpy(olstMDFieldRecord[i].FieldName, "LowPrice");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 4;
	i++;
	// 		10	TradePrice	���¼�	N11(4)
	strcpy(olstMDFieldRecord[i].FieldName, "TradePrice");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 4;
	i++;
	// 		11	ClosePx	�����̼�	N11(4)
		
	strcpy(olstMDFieldRecord[i].FieldName, "ClosePx");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 4;
	i++;
		
	// 		12	TradingPhaseCode	ָ��ʵʱ�׶μ���־	C8
	strcpy(olstMDFieldRecord[i].FieldName, "TradingPhaseCode");
	olstMDFieldRecord[i].FieldType = 'C';
	olstMDFieldRecord[i].FieldSize = 8;
	i++;
	
	// 		13	Timestamp	ʱ���	C12
	strcpy(olstMDFieldRecord[i].FieldName, "Timestamp");
	olstMDFieldRecord[i].FieldType = 'C';
	olstMDFieldRecord[i].FieldSize = 12;
	i++;
	
	if(NULL == m_poMD1FieldRecord || i > m_ushMD1FieldCount)
	{
		m_ushMD1FieldCount = i;
		
		if ( m_poMD1FieldRecord != NULL )
		{
			delete []m_poMD1FieldRecord;
			m_poMD1FieldRecord = NULL;
		}
		m_poMD1FieldRecord = new TMktdtFieldInfo[m_ushMD1FieldCount];
		if ( m_poMD1FieldRecord==NULL )
		{
			m_ushMD1FieldCount = 0;
			QuoCollector::GetCollector()->OnLog( TLV_WARN, "CReadMktdtFile::inner_AnalyzeField() : �����ֶ���Ϣ1�������ڴ����%d", GetLastError());
			return ERROR_MKTDT_APPFIELDMEMORY;
		}

	}
	else
		m_ushMD1FieldCount = i;
	memcpy(m_poMD1FieldRecord, &olstMDFieldRecord[0], sizeof(TMktdtFieldInfo)*m_ushMD1FieldCount);


	for(i = 1; i < m_ushMD1FieldCount; i++)
	{
		m_poMD1FieldRecord[i].FieldOffset = m_poMD1FieldRecord[i-1].FieldOffset+ m_poMD1FieldRecord[i-1].FieldSize;
	}

	return 1;

}
int CReadMktdtFile::inner_SetMD2fieldinfo() 
{
	register int			i = 0;	
	TMktdtFieldInfo		olstMDFieldRecord[MKTDT_FIELD_COUNT];

		
	//	1	MDStreamID	������������	C5
	strcpy(olstMDFieldRecord[i].FieldName, "MDStreamID");
	olstMDFieldRecord[i].FieldType = 'C';
	olstMDFieldRecord[i].FieldSize = 5;
	olstMDFieldRecord[i].FieldOffset = 0;
	i++;
	
	//	2	SecurityID	��Ʒ����	C6
	strcpy(olstMDFieldRecord[i].FieldName, "SecurityID");
	olstMDFieldRecord[i].FieldType = 'C';
	olstMDFieldRecord[i].FieldSize = 6;
	i++;
	// 		3	Symbol	��Ʒ����	C8
	
	strcpy(olstMDFieldRecord[i].FieldName, "Symbol");
	olstMDFieldRecord[i].FieldType = 'C';
	olstMDFieldRecord[i].FieldSize = 8;
	i++;
	// 		4	TradeVolume	�ɽ�����	N16
	
	strcpy(olstMDFieldRecord[i].FieldName, "TradeVolume");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 16;
	i++;
	// 		5	TotalValueTraded	�ɽ����	N16(2)
	strcpy(olstMDFieldRecord[i].FieldName, "TotalValueTraded");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 16;
	olstMDFieldRecord[i].DecSize = 2;
	i++;
	// 		6	PreClosePx	�������̼�	N11(3)
	
	strcpy(olstMDFieldRecord[i].FieldName, "PreClosePx");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	// 		7	OpenPrice	���տ��̼�	N11(3)
	strcpy(olstMDFieldRecord[i].FieldName, "OpenPrice");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	// 		8	HighPrice	��߼�	N11(3)
	
	strcpy(olstMDFieldRecord[i].FieldName, "HighPrice");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	// 		9	LowPrice	��ͼ�	N11(3)
	strcpy(olstMDFieldRecord[i].FieldName, "LowPrice");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	// 		10	TradePrice	���¼�	N11(3)
	strcpy(olstMDFieldRecord[i].FieldName, "TradePrice");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	// 		11	ClosePx	�����̼�	N11(3)
	
	strcpy(olstMDFieldRecord[i].FieldName, "ClosePx");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	
	// 12	BuyPrice1	�����һ	N11(3)
	strcpy(olstMDFieldRecord[i].FieldName, "BuyPrice1");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	// 13	BuyVolume1	������һ	N12
	strcpy(olstMDFieldRecord[i].FieldName, "BuyVolume1");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 12;
	i++;
	// 14	SellPrice1	������һ	N11(3)
	strcpy(olstMDFieldRecord[i].FieldName, "SellPrice1");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	// 15	SellVolume1	������һ	N12
	strcpy(olstMDFieldRecord[i].FieldName, "SellVolume1");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 12;
	i++;
	// 16	BuyPrice2	����۶�	N11(3)
	strcpy(olstMDFieldRecord[i].FieldName, "BuyPrice2");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	// 17	BuyVolume2	��������	N12
	strcpy(olstMDFieldRecord[i].FieldName, "BuyVolume2");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 12;
	i++;
	// 18	SellPrice2	�����۶�	N11(3)
	strcpy(olstMDFieldRecord[i].FieldName, "SellPrice2");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	// 19	SellVolume2	��������	N12
	strcpy(olstMDFieldRecord[i].FieldName, "SellVolume2");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 12;
	i++;
	// 20	BuyPrice3	�������	N11(3)
	strcpy(olstMDFieldRecord[i].FieldName, "BuyPrice3");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	// 21	BuyVolume3	��������	N12
	strcpy(olstMDFieldRecord[i].FieldName, "BuyVolume3");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 12;
	i++;
	// 22	SellPrice3	��������	N11(3)
	strcpy(olstMDFieldRecord[i].FieldName, "SellPrice3");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	// 23	SellVolume3	��������	N12
	strcpy(olstMDFieldRecord[i].FieldName, "SellVolume3");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 12;
	i++;
	// 24	BuyPrice4	�������	N11(3)
	strcpy(olstMDFieldRecord[i].FieldName, "BuyPrice4");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	// 25	BuyVolume4	��������	N12
	strcpy(olstMDFieldRecord[i].FieldName, "BuyVolume4");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 12;
	i++;
	// 26	SellPrice4	��������	N11(3)
	strcpy(olstMDFieldRecord[i].FieldName, "SellPrice4");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	// 27	SellVolume4	��������	N12
	strcpy(olstMDFieldRecord[i].FieldName, "SellVolume4");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 12;
	i++;
	// 28	BuyPrice5	�������	N11(3)
	strcpy(olstMDFieldRecord[i].FieldName, "BuyPrice5");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	// 29	BuyVolume5	��������	N12
	strcpy(olstMDFieldRecord[i].FieldName, "BuyVolume5");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 12;
	i++;
	// 30	SellPrice5	��������	N11(3)
	strcpy(olstMDFieldRecord[i].FieldName, "SellPrice5");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	// 31	SellVolume5	��������	N12
	strcpy(olstMDFieldRecord[i].FieldName, "SellVolume5");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 12;
	i++;
	// 	TradingPhaseCode	��Ʒʵʱ�׶μ���־	C8
	strcpy(olstMDFieldRecord[i].FieldName, "TradingPhaseCode");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 8;
	i++;
	// 			Timestamp	ʱ���	C12
	strcpy(olstMDFieldRecord[i].FieldName, "Timestamp");
	olstMDFieldRecord[i].FieldType = 'C';
	olstMDFieldRecord[i].FieldSize = 12;
	i++;
	if(NULL == m_poMD2FieldRecord || i > m_ushMD2FieldCount)
	{
	
		m_ushMD2FieldCount = i;
		
		if ( m_poMD2FieldRecord != NULL )
		{
			delete []m_poMD2FieldRecord;
			m_poMD2FieldRecord = NULL;
		}
		m_poMD2FieldRecord = new TMktdtFieldInfo[m_ushMD2FieldCount];
		if ( m_poMD2FieldRecord==NULL )
		{
			m_ushMD2FieldCount = 0;
			QuoCollector::GetCollector()->OnLog( TLV_WARN, "CReadMktdtFile::inner_AnalyzeField() : �����ֶ���Ϣ2�������ڴ����%d", GetLastError());
			return ERROR_MKTDT_APPFIELDMEMORY;
		}
	}
	else
		m_ushMD2FieldCount = i;

	memcpy(m_poMD2FieldRecord, &olstMDFieldRecord[0], sizeof(TMktdtFieldInfo)*m_ushMD2FieldCount);

	for(i = 1; i < m_ushMD2FieldCount; i++)
	{
		m_poMD2FieldRecord[i].FieldOffset = m_poMD2FieldRecord[i-1].FieldOffset+ m_poMD2FieldRecord[i-1].FieldSize;
	}

	return 1;

}


int CReadMktdtFile::inner_SetMD3fieldinfo() 
{
	register int			i = 0;	
	TMktdtFieldInfo		olstMDFieldRecord[MKTDT_FIELD_COUNT];

		
	//	1	MDStreamID	������������	C5
	strcpy(olstMDFieldRecord[i].FieldName, "MDStreamID");
	olstMDFieldRecord[i].FieldType = 'C';
	olstMDFieldRecord[i].FieldSize = 5;
	olstMDFieldRecord[i].FieldOffset = 0;
	i++;
	
	//	2	SecurityID	��Ʒ����	C6
	strcpy(olstMDFieldRecord[i].FieldName, "SecurityID");
	olstMDFieldRecord[i].FieldType = 'C';
	olstMDFieldRecord[i].FieldSize = 6;
	i++;
	// 		3	Symbol	��Ʒ����	C8
	
	strcpy(olstMDFieldRecord[i].FieldName, "Symbol");
	olstMDFieldRecord[i].FieldType = 'C';
	olstMDFieldRecord[i].FieldSize = 8;
	i++;
	// 		4	TradeVolume	�ɽ�����	N16
	
	strcpy(olstMDFieldRecord[i].FieldName, "TradeVolume");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 16;
	i++;
	// 		5	TotalValueTraded	�ɽ����	N16(2)
	strcpy(olstMDFieldRecord[i].FieldName, "TotalValueTraded");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 16;
	olstMDFieldRecord[i].DecSize = 2;
	i++;
	// 		6	PreClosePx	�������̼�	N11(3)
	
	strcpy(olstMDFieldRecord[i].FieldName, "PreClosePx");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	// 		7	OpenPrice	���տ��̼�	N11(3)
	strcpy(olstMDFieldRecord[i].FieldName, "OpenPrice");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	// 		8	HighPrice	��߼�	N11(3)
	
	strcpy(olstMDFieldRecord[i].FieldName, "HighPrice");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	// 		9	LowPrice	��ͼ�	N11(3)
	strcpy(olstMDFieldRecord[i].FieldName, "LowPrice");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	// 		10	TradePrice	���¼�	N11(3)
	strcpy(olstMDFieldRecord[i].FieldName, "TradePrice");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	// 		11	ClosePx	�����̼�	N11(3)
		
	strcpy(olstMDFieldRecord[i].FieldName, "ClosePx");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
		
	// 12	BuyPrice1	�����һ	N11(3)
	strcpy(olstMDFieldRecord[i].FieldName, "BuyPrice1");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	// 13	BuyVolume1	������һ	N12
	strcpy(olstMDFieldRecord[i].FieldName, "BuyVolume1");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 12;
	i++;
	// 14	SellPrice1	������һ	N11(3)
	strcpy(olstMDFieldRecord[i].FieldName, "SellPrice1");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	// 15	SellVolume1	������һ	N12
	strcpy(olstMDFieldRecord[i].FieldName, "SellVolume1");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 12;
	i++;
	// 16	BuyPrice2	����۶�	N11(3)
	strcpy(olstMDFieldRecord[i].FieldName, "BuyPrice2");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	// 17	BuyVolume2	��������	N12
	strcpy(olstMDFieldRecord[i].FieldName, "BuyVolume2");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 12;
	i++;
	// 18	SellPrice2	�����۶�	N11(3)
	strcpy(olstMDFieldRecord[i].FieldName, "SellPrice2");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	// 19	SellVolume2	��������	N12
	strcpy(olstMDFieldRecord[i].FieldName, "SellVolume2");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 12;
	i++;
	// 20	BuyPrice3	�������	N11(3)
	strcpy(olstMDFieldRecord[i].FieldName, "BuyPrice3");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	// 21	BuyVolume3	��������	N12
	strcpy(olstMDFieldRecord[i].FieldName, "BuyVolume3");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 12;
	i++;
	// 22	SellPrice3	��������	N11(3)
	strcpy(olstMDFieldRecord[i].FieldName, "SellPrice3");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	// 23	SellVolume3	��������	N12
	strcpy(olstMDFieldRecord[i].FieldName, "SellVolume3");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 12;
	i++;
	// 24	BuyPrice4	�������	N11(3)
	strcpy(olstMDFieldRecord[i].FieldName, "BuyPrice4");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	// 25	BuyVolume4	��������	N12
	strcpy(olstMDFieldRecord[i].FieldName, "BuyVolume4");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 12;
	i++;
	// 26	SellPrice4	��������	N11(3)
	strcpy(olstMDFieldRecord[i].FieldName, "SellPrice4");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	// 27	SellVolume4	��������	N12
	strcpy(olstMDFieldRecord[i].FieldName, "SellVolume4");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 12;
	i++;
	// 28	BuyPrice5	�������	N11(3)
	strcpy(olstMDFieldRecord[i].FieldName, "BuyPrice5");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	// 29	BuyVolume5	��������	N12
	strcpy(olstMDFieldRecord[i].FieldName, "BuyVolume5");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 12;
	i++;
	// 30	SellPrice5	��������	N11(3)
	strcpy(olstMDFieldRecord[i].FieldName, "SellPrice5");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	// 31	SellVolume5	��������	N12
	strcpy(olstMDFieldRecord[i].FieldName, "SellVolume5");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 12;
	i++;
	// 	TradingPhaseCode	��Ʒʵʱ�׶μ���־	C8
	strcpy(olstMDFieldRecord[i].FieldName, "TradingPhaseCode");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 8;
	i++;
	// 		Timestamp	ʱ���	C12
	strcpy(olstMDFieldRecord[i].FieldName, "Timestamp");
	olstMDFieldRecord[i].FieldType = 'C';
	olstMDFieldRecord[i].FieldSize = 12;
	i++;
	if(NULL == m_poMD3FieldRecord || i > m_ushMD3FieldCount)
	{

	
		m_ushMD3FieldCount = i;
		
		if ( m_poMD3FieldRecord != NULL )
		{
			delete []m_poMD3FieldRecord;
			m_poMD3FieldRecord = NULL;
		}
		m_poMD3FieldRecord = new TMktdtFieldInfo[m_ushMD3FieldCount];
		if ( m_poMD3FieldRecord==NULL )
		{
			m_ushMD3FieldCount = 0;
			QuoCollector::GetCollector()->OnLog( TLV_WARN, "CReadMktdtFile::inner_AnalyzeField() : �����ֶ���Ϣ3�������ڴ����%d", GetLastError());
			return ERROR_MKTDT_APPFIELDMEMORY;
		}
	}
	else
		m_ushMD3FieldCount = i;
	
	memcpy(m_poMD3FieldRecord, &olstMDFieldRecord[0], sizeof(TMktdtFieldInfo)*m_ushMD3FieldCount);
	

	for(i = 1; i < m_ushMD3FieldCount; i++)
	{
		m_poMD3FieldRecord[i].FieldOffset = m_poMD3FieldRecord[i-1].FieldOffset+ m_poMD3FieldRecord[i-1].FieldSize;
	}

	return 1;

}
int CReadMktdtFile::inner_SetMD4fieldinfo() 
{
	register int			i = 0;	
	TMktdtFieldInfo		olstMDFieldRecord[MKTDT_FIELD_COUNT];
		
	//	1	MDStreamID	������������	C5
	strcpy(olstMDFieldRecord[i].FieldName, "MDStreamID");
	olstMDFieldRecord[i].FieldType = 'C';
	olstMDFieldRecord[i].FieldSize = 5;
	olstMDFieldRecord[i].FieldOffset = 0;
	i++;
	
	//	2	SecurityID	��Ʒ����	C6
	strcpy(olstMDFieldRecord[i].FieldName, "SecurityID");
	olstMDFieldRecord[i].FieldType = 'C';
	olstMDFieldRecord[i].FieldSize = 6;
	i++;
	// 		3	Symbol	��Ʒ����	C8
	
	strcpy(olstMDFieldRecord[i].FieldName, "Symbol");
	olstMDFieldRecord[i].FieldType = 'C';
	olstMDFieldRecord[i].FieldSize = 8;
	i++;
	// 		4	TradeVolume	�ɽ�����	N16
	
	strcpy(olstMDFieldRecord[i].FieldName, "TradeVolume");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 16;
	i++;
	// 		5	TotalValueTraded	�ɽ����	N16(2)
	strcpy(olstMDFieldRecord[i].FieldName, "TotalValueTraded");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 16;
	olstMDFieldRecord[i].DecSize = 2;
	i++;
	// 		6	PreClosePx	�������̼�	N11(3)
	
	strcpy(olstMDFieldRecord[i].FieldName, "PreClosePx");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	// 		7	OpenPrice	���տ��̼�	N11(3)
	strcpy(olstMDFieldRecord[i].FieldName, "OpenPrice");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	// 		8	HighPrice	��߼�	N11(3)
	
	strcpy(olstMDFieldRecord[i].FieldName, "HighPrice");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	// 		9	LowPrice	��ͼ�	N11(3)
	strcpy(olstMDFieldRecord[i].FieldName, "LowPrice");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	// 		10	TradePrice	���¼�	N11(3)
	strcpy(olstMDFieldRecord[i].FieldName, "TradePrice");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	// 		11	ClosePx	�����̼�	N11(3)
	
	strcpy(olstMDFieldRecord[i].FieldName, "ClosePx");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	
	// 12	BuyPrice1	�����һ	N11(3)
	strcpy(olstMDFieldRecord[i].FieldName, "BuyPrice1");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	// 13	BuyVolume1	������һ	N12
	strcpy(olstMDFieldRecord[i].FieldName, "BuyVolume1");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 12;
	i++;
	// 14	SellPrice1	������һ	N11(3)
	strcpy(olstMDFieldRecord[i].FieldName, "SellPrice1");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	// 15	SellVolume1	������һ	N12
	strcpy(olstMDFieldRecord[i].FieldName, "SellVolume1");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 12;
	i++;
	// 16	BuyPrice2	����۶�	N11(3)
	strcpy(olstMDFieldRecord[i].FieldName, "BuyPrice2");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	// 17	BuyVolume2	��������	N12
	strcpy(olstMDFieldRecord[i].FieldName, "BuyVolume2");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 12;
	i++;
	// 18	SellPrice2	�����۶�	N11(3)
	strcpy(olstMDFieldRecord[i].FieldName, "SellPrice2");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	// 19	SellVolume2	��������	N12
	strcpy(olstMDFieldRecord[i].FieldName, "SellVolume2");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 12;
	i++;
	// 20	BuyPrice3	�������	N11(3)
	strcpy(olstMDFieldRecord[i].FieldName, "BuyPrice3");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	// 21	BuyVolume3	��������	N12
	strcpy(olstMDFieldRecord[i].FieldName, "BuyVolume3");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 12;
	i++;
	// 22	SellPrice3	��������	N11(3)
	strcpy(olstMDFieldRecord[i].FieldName, "SellPrice3");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	// 23	SellVolume3	��������	N12
	strcpy(olstMDFieldRecord[i].FieldName, "SellVolume3");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 12;
	i++;
	// 24	BuyPrice4	�������	N11(3)
	strcpy(olstMDFieldRecord[i].FieldName, "BuyPrice4");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	// 25	BuyVolume4	��������	N12
	strcpy(olstMDFieldRecord[i].FieldName, "BuyVolume4");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 12;
	i++;
	// 26	SellPrice4	��������	N11(3)
	strcpy(olstMDFieldRecord[i].FieldName, "SellPrice4");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	// 27	SellVolume4	��������	N12
	strcpy(olstMDFieldRecord[i].FieldName, "SellVolume4");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 12;
	i++;
	// 28	BuyPrice5	�������	N11(3)
	strcpy(olstMDFieldRecord[i].FieldName, "BuyPrice5");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	// 29	BuyVolume5	��������	N12
	strcpy(olstMDFieldRecord[i].FieldName, "BuyVolume5");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 12;
	i++;
	// 30	SellPrice5	��������	N11(3)
	strcpy(olstMDFieldRecord[i].FieldName, "SellPrice5");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	// 31	SellVolume5	��������	N12
	strcpy(olstMDFieldRecord[i].FieldName, "SellVolume5");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 12;
	i++;
	// 32	PreCloseIOPV	����T-1������ʱ��IOPV	N11��3��
	strcpy(olstMDFieldRecord[i].FieldName, "PreCloseIOPV");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	// 33	IOPV	����IOPV	N11��3��
	strcpy(olstMDFieldRecord[i].FieldName, "IOPV");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	// 34	TradingPhaseCode	��Ʒʵʱ�׶μ���־	C8
	strcpy(olstMDFieldRecord[i].FieldName, "TradingPhaseCode");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 8;
	i++;
	
	
	// 		35	Timestamp	ʱ���	C12
	strcpy(olstMDFieldRecord[i].FieldName, "Timestamp");
	olstMDFieldRecord[i].FieldType = 'C';
	olstMDFieldRecord[i].FieldSize = 12;
	i++;
	if(NULL == m_poMD3FieldRecord || i > m_ushMD3FieldCount)
	{
		m_ushMD4FieldCount = i;
	
		if ( m_poMD4FieldRecord != NULL )
		{
			delete []m_poMD4FieldRecord;
			m_poMD4FieldRecord = NULL;
		}
		m_poMD4FieldRecord = new TMktdtFieldInfo[m_ushMD4FieldCount];
		if ( m_poMD4FieldRecord==NULL )
		{
			m_ushMD4FieldCount = 0;

			QuoCollector::GetCollector()->OnLog( TLV_WARN, "CReadMktdtFile::inner_AnalyzeField() : �����ֶ���Ϣ4�������ڴ����%d", GetLastError());
			return ERROR_MKTDT_APPFIELDMEMORY;
		}
	}
	else
		m_ushMD4FieldCount = i;

	memcpy(m_poMD4FieldRecord, &olstMDFieldRecord[0], sizeof(TMktdtFieldInfo)*m_ushMD4FieldCount);
	

	for(i = 1; i < m_ushMD4FieldCount; i++)
	{
		m_poMD4FieldRecord[i].FieldOffset = m_poMD4FieldRecord[i-1].FieldOffset+ m_poMD4FieldRecord[i-1].FieldSize;
	}

	return 1;

}



