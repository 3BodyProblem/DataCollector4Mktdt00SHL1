#include "FjyReader.h"
#include "../DataCollector4Mktdt00SHL1.h"


CReadFjyFile::CReadFjyFile()
{
	memset(&m_stHead, 0, sizeof(m_stHead));
	m_pstRecData = NULL;
	m_uiRecCount = 0;
	m_uiRecSize = 0;
	m_pszFileData = NULL;
	m_uiFileDataSize = 0;

	m_sFilePath = "";
	m_ushProcFileErrCount = 0;


	m_poMD1FieldRecord = NULL;
	m_ushMD1FieldCount = 0;

}
void CReadFjyFile::inner_delete(void)
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
	if(NULL != m_poMD1FieldRecord)
	{
		delete [] m_poMD1FieldRecord;
		m_poMD1FieldRecord = NULL;

	}
}

void CReadFjyFile::Close(void)
{
	CriticalLock	section( m_oLock );

	m_fInput.close();
	memset(&m_stHead, 0, sizeof(m_stHead));
	m_uiRecCount = 0;
}
int CReadFjyFile::Instance()
{
	if(0 > SetFieldInfo())
		return -1;
	return 1;
}

void CReadFjyFile::Release(void)
{
	CriticalLock	section( m_oLock );
	
	inner_delete();
}

CReadFjyFile::~CReadFjyFile()
{
	//Ϊ�˱������������ﲻ���������ж�
	inner_delete();
}

int  CReadFjyFile::ChkOpen( std::string filename )
{
	Close();
	if ( m_sFilePath!= filename )
	{
		m_sFilePath = filename;
	}
	m_fInput.close();
	return ReloadFromFile();
}
//..........................................................................................................................
int  CReadFjyFile::ReloadFromFile(void)
{
	CriticalLock	section( m_oLock );
	register int	errorcode;

	errorcode = inner_loadfromfile();
	m_fInput.close();
	
	return(errorcode);
}

int  CReadFjyFile::inner_loadfromfile(void)
{
	register int			errorcode;
	int i;
	unsigned int		uiFileLen, uiFieldCount;
	char * pszField[Fjy_FIELD_COUNT];
// 	bool	blFlag; ȡ���ļ�������
	bool	blLineEnd;

	CriticalLock	section( m_oLock );

	m_ushProcFileErrCount ++;
	m_uiLine = 0;
	if ( !m_fInput.is_open() )
	{
		m_fInput.open( m_sFilePath.c_str(), std::ios::in|std::ios::binary );
		if ( !m_fInput.is_open() )
		{
			m_ushPrtErrCount++;
			if(m_iErrorCode != errorcode && ( 1 == m_ushPrtErrCount || 0 == m_ushProcFileErrCount % 15))
			{
				QuoCollector::GetCollector()->OnLog( TLV_WARN, "CReadFjyFile::inner_loadfromfile() : failed 2 open fjy.txt, %s", m_sFilePath.c_str() );
			}
			m_iErrorCode = errorcode;

			if(20 < m_ushProcFileErrCount)
				return(errorcode);
			return 0;
		}
	}

	std::streampos	nCurPos = m_fInput.tellg();
	m_fInput.seekg( 0, std::ios::end );
	uiFileLen = m_fInput.tellg();
	m_fInput.seekg( nCurPos );

	if(uiFileLen == 0)
	{
		m_fInput.close();
		m_ushPrtErrCount++;
		errorcode = ERROR_Fjy_LENZERO;
		if(m_iErrorCode != errorcode && ( 1 == m_ushPrtErrCount || 0 == m_ushProcFileErrCount % 15))
		{
			QuoCollector::GetCollector()->OnLog( TLV_WARN, "CReadFjyFile::inner_loadfromfile() : �ļ�����Ϊ0�Ĵ���" );
		}
		m_iErrorCode = errorcode;
		if(20 < m_ushProcFileErrCount)
			return ERROR_Fjy_LENZERO ;
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
				QuoCollector::GetCollector()->OnLog( TLV_WARN, "CReadFjyFile::inner_loadfromfile() : ���ļ����󣺷����ڴ����%d", errorcode );
			}
			m_iErrorCode = errorcode;

			m_uiFileDataSize = 0;
			if(20 < m_ushProcFileErrCount)
				return ERROR_Fjy_APPFILEMEMORY ;
			return 0;
		}
		m_uiFileDataSize = uiFileLen+1;	
	}

	m_fInput.seekg( 0, std::ios::beg );
	m_fInput.read( m_pszFileData, uiFileLen );
	errorcode = m_fInput.gcount();
	if ( errorcode != uiFileLen )
	{
		m_fInput.close();
		m_ushPrtErrCount++;
		if(m_iErrorCode != errorcode && ( 1 == m_ushPrtErrCount || 0 == m_ushProcFileErrCount % 15))
		{
			QuoCollector::GetCollector()->OnLog( TLV_WARN, "CReadFjyFile::inner_loadfromfile() : ���ļ������ļ�����%d ��ȡ���ݷ���:%d", uiFileLen, errorcode );
		}
		m_iErrorCode = errorcode;

		if(20 < m_ushProcFileErrCount)
			return ERROR_Fjy_FILEREAD;
		return 0;
	}
	m_pszFileData[errorcode] = 0;
	m_fInput.close();
	m_uiLine = 0;
	m_uiRecCount = 0;

	memset(pszField, NULL, sizeof(pszField));
	uiFieldCount = 0;
	pszField[uiFieldCount++] = &m_pszFileData[0];
	m_stHead.uiRecCount = 0;
	if(m_stHead.uiRecCount+1 >=  m_uiRecSize || NULL == m_pstRecData)
	{
		if(NULL != m_pstRecData)
			delete [] m_pstRecData;
		m_pstRecData = new TFjyFieldData[m_uiRecSize + 2048 ];
		if (NULL == m_pstRecData)
		{
			m_ushPrtErrCount++;
			errorcode =  GetLastError();
			if(m_iErrorCode != errorcode && ( 1 == m_ushPrtErrCount || 0 == m_ushProcFileErrCount % 15))
			{
				QuoCollector::GetCollector()->OnLog( TLV_WARN, "CReadFjyFile::inner_loadfromfile() : �����ļ����󣺷����ڴ����%d", GetLastError() );
			}
			m_iErrorCode = errorcode;
						
			if(20 < m_ushProcFileErrCount)
				return ERROR_Fjy_APPRECORDMEMORY;
			return 0;
		}
		m_uiRecSize += 2048;					
	}


	i = 0;
// 	blFlag = false;
	blLineEnd = true;
	while(i < uiFileLen /*&& !blFlag*/)
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
			errorcode = inner_AnalyzeField(m_pszFileData, i, pszField, Fjy_FIELD_COUNT, blLineEnd);
			if(0 < errorcode)
			{
				uiFieldCount = errorcode;
				if(0 > (errorcode = inner_parseData(pszField, uiFieldCount)))
				{
					m_ushPrtErrCount++;
					if(m_iErrorCode != errorcode && ( 1 == m_ushPrtErrCount || 0 == m_ushProcFileErrCount % 15))
					{
						if(ERROR_Fjy_NOTFOUNDRECORD == errorcode)
						{
							QuoCollector::GetCollector()->OnLog( TLV_WARN, "CReadFjyFile::inner_loadfromfile() : �����¼���󣺼�¼������ȷ" );
						}
						else if(ERROR_Fjy_FIELDCOUNT == errorcode)
						{
							QuoCollector::GetCollector()->OnLog( TLV_WARN, "CReadFjyFile::inner_loadfromfile() : �����¼�����ֶ���������ȷ" );
						}
						else
						{
							QuoCollector::GetCollector()->OnLog( TLV_WARN, "CReadFjyFile::inner_loadfromfile() : �����¼����%d", errorcode );
						}

					}
					m_iErrorCode = errorcode;

					if(20 < m_ushProcFileErrCount)
						return errorcode;
					return 0;
				}
				else
				{
					
					m_stHead.uiRecCount ++;
					if(m_stHead.uiRecCount+1 >=  m_uiRecSize || NULL == m_pstRecData)
					{
						TFjyFieldData * pHead = m_pstRecData;
						m_pstRecData = new TFjyFieldData[m_uiRecSize + 1024 ];
						if (NULL == m_pstRecData)
						{
							m_pstRecData = pHead;
							m_ushPrtErrCount++;
							errorcode =  GetLastError();
							if(m_iErrorCode != errorcode && ( 1 == m_ushPrtErrCount || 0 == m_ushProcFileErrCount % 15))
							{
								QuoCollector::GetCollector()->OnLog( TLV_WARN, "CReadFjyFile::inner_loadfromfile() : �����ļ����󣺷����ڴ����%d", GetLastError() );
							}
							m_iErrorCode = errorcode;
							
							if(20 < m_ushProcFileErrCount)
								return ERROR_Fjy_APPRECORDMEMORY;
							return 0;
						}
						else
						{
							if(NULL != m_pstRecData)
							{

								memcpy(m_pstRecData, pHead, sizeof(TFjyFieldData)*m_stHead.uiRecCount);

								delete [] pHead;

							}
						}
						m_uiRecSize += 1024;					
					}
					
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
// 					blFlag = true;
					break;
				}
				pszField[uiFieldCount++] = &m_pszFileData[i+1];
			}

			i++;
		}
	}
	if(/*!blFlag ||*/ m_uiLine!= m_stHead.uiRecCount) //huq mid m_uiLine-1  to m_uiLine
	{
		m_ushPrtErrCount++;
		errorcode = -1;
	//	if(m_iErrorCode != errorcode && ( 1 == m_ushPrtErrCount || 0 == m_ushProcFileErrCount % 15))
		{
			QuoCollector::GetCollector()->OnLog( TLV_WARN, "CReadFjyFile::inner_loadfromfile() : �ļ���¼����С:%d��ϵͳҪ��%d��¼,���ü�¼��%d", m_uiLine-1, m_stHead.uiRecCount, m_uiRecCount );
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



int  CReadFjyFile::GetHead(TFjyHead ** out_ppstHead)
{
	*out_ppstHead = &m_stHead;
	return 1;
}
int  CReadFjyFile::GetRecord(unsigned short in_ushRecPos, TFjyFieldData ** out_ppstFileData)
{
	if( in_ushRecPos >= m_uiRecCount)
		return ERROR_Fjy_NOTFOUNDRECORD;

	* out_ppstFileData = &m_pstRecData[in_ushRecPos];
	return 1;

}


//..........................................................................................................................
// int  CReadFjyFile::inner_parsehead(char *pszField[], unsigned short ushFieldCount)
// {
// 
// 	if(9 != ushFieldCount)
// 		return ERROR_Fjy_FIELDCOUNT;
// 
// 
// 	memcpy(m_stHead.szFlags, pszField[0], sizeof(m_stHead.szFlags));		//��ʼ��ʶ
// 	memcpy(m_stHead.szVer, pszField[1], sizeof(m_stHead.szVer));			//�汾
// 	m_stHead.uiBodyLength = atoi(pszField[2]);	//����
// 	m_stHead.uiRecCount = atoi(pszField[3]);		//��¼��
// 	m_stHead.uiReportID = atoi(pszField[4]);	//�������
// 	memcpy(m_stHead.szSenderCmpID, pszField[5], sizeof(m_stHead.szSenderCmpID));	//���ͷ���ʾ��
// 	//��7��
// 	pszField[6][8] = 0;
// 	m_stHead.uiTrDate = atoi(pszField[6]);
// 
// 	pszField[6][9 + 2] = 0;
// 	m_stHead.uiTrTime = atoi(&pszField[6][9]) * 10000;
// 	pszField[6][9 + 2 + 3] = 0;
// 	m_stHead.uiTrTime += (atoi(&pszField[6][12]) * 100);
// 	pszField[6][9 + 2 + 3 + 3] = 0;
// 	m_stHead.uiTrTime += atoi(&pszField[6][15]) ;
// 	//��8��				
// 	m_stHead.uiUpdateType = atoi(pszField[7]);	//���ͷ�ʽ 0 = ����Full refresh	1 = ����Incremental���ݲ�֧�֣�
// 	memcpy(m_stHead.szSesStatus, pszField[8], sizeof(m_stHead.szSesStatus));	//�г�����״̬ ȫ�г�����״̬��
// 				//���ֶ�Ϊ8λ�ַ���������ÿλ��ʾ�ض��ĺ��壬�޶�������ո�
// 				//	��1λ��'S'��ʾȫ�г������ڼ䣨����ǰ����'T'��ʾȫ�г����ڽ����ڼ䣨���м����У��� 'E'��ʾȫ�г����ڱ����ڼ䡣
// 				//	��2λ��'1'��ʾ���̼��Ͼ��۽�����־��δ����ȡ'0'��
// 				//	��3λ��'1'��ʾ�г����������־��δ����ȡ'0'
// 
// 
// 	if(0 != memcmp(m_stHead.szFlags, "HEADER", sizeof(m_stHead.szFlags)))
// 		return ERROR_Fjy_HEADANALYZE;
// 
// 	return 1;
// 
// }

int  CReadFjyFile::inner_parseData(char *pszField[], unsigned short ushFieldCount)
{
	

	if(NULL == m_pstRecData || m_uiRecSize <= m_uiRecCount)
	{
		return ERROR_Fjy_NOTFOUNDRECORD;
	}
	if( 13 > ushFieldCount)
	{
		return ERROR_Fjy_FIELDCOUNT;
	}
	memcpy(m_pstRecData[m_uiRecCount].szType, pszField[0], sizeof(m_pstRecData[m_uiRecCount].szType)); // �ο���������	C5
	memcpy(m_pstRecData[m_uiRecCount].szNonTrCode, pszField[1], sizeof(m_pstRecData[m_uiRecCount].szNonTrCode)); //	�ǽ���֤ȯ����	C6
	memcpy(m_pstRecData[m_uiRecCount].szNonTrName, pszField[2], sizeof(m_pstRecData[m_uiRecCount].szNonTrName)); //	�ǽ���֤ȯ����	C8
	memcpy(m_pstRecData[m_uiRecCount].szCode, pszField[3], sizeof(m_pstRecData[m_uiRecCount].szCode)); //	��Ʒ֤ȯ����	C6
	memcpy(m_pstRecData[m_uiRecCount].szName, pszField[4], sizeof(m_pstRecData[m_uiRecCount].szName));	//��Ʒ֤ȯ����	C8
	memcpy(m_pstRecData[m_uiRecCount].szBusType, pszField[5], sizeof(m_pstRecData[m_uiRecCount].szBusType));	//�ǽ���ҵ������		c2
	memcpy(m_pstRecData[m_uiRecCount].szNonTrBDate, pszField[6], sizeof(m_pstRecData[m_uiRecCount].szNonTrBDate)); //	�ǽ��׶������뿪ʼ����	C8
	memcpy(m_pstRecData[m_uiRecCount].szNonTrEDate, pszField[7], sizeof(m_pstRecData[m_uiRecCount].szNonTrEDate)); //	�ǽ��׶��������������	C8
	m_pstRecData[m_uiRecCount].dNonTrOrdHands = atof(pszField[8]); // �ǽ��׶���������	N12
	m_pstRecData[m_uiRecCount].dNonTrOrdMinHands = atof(pszField[9]); // �ǽ��׶�����С��������	N12
	m_pstRecData[m_uiRecCount].dNonTrOrdMaxHands = atof(pszField[10]); // �ǽ��׶�������󶩵�����	N12
	m_pstRecData[m_uiRecCount].dNonTrPrice = atof(pszField[11]);	// �ǽ��׼۸�	N13(5)
	m_pstRecData[m_uiRecCount].dIPOVol = atof(pszField[12]);	//	IPO����	N16
	m_pstRecData[m_uiRecCount].chIPOAMethod = atof(pszField[13]);	// IPO���䷽��	C1
	memcpy(m_pstRecData[m_uiRecCount].szIPOSellDate, pszField[14], sizeof(m_pstRecData[m_uiRecCount].szIPOSellDate)); // IPO���۷���������������	C8 YYYYMMDD
	memcpy(m_pstRecData[m_uiRecCount].szIPOVfNumbDate, pszField[15], sizeof(m_pstRecData[m_uiRecCount].szIPOVfNumbDate)); //IPO���ʻ��������	C8 YYYYMMDD
	memcpy(m_pstRecData[m_uiRecCount].szIPOLttDrawDate, pszField[16], sizeof(m_pstRecData[m_uiRecCount].szIPOLttDrawDate)); // IPOҡ�ų�ǩ������	C8 YYYYMMDD
	m_pstRecData[m_uiRecCount].dIPOLowRngPrice = atof(pszField[17]); // IPO�깺�۸���������	N11(3)
	m_pstRecData[m_uiRecCount].dIPOHighRngPrice = atof(pszField[18]); // IPO�깺�۸���������	N11(3)
	m_pstRecData[m_uiRecCount].dIPOPropPlac = atof(pszField[19]); // IPO�������۱���	N11(3)
	memcpy(m_pstRecData[m_uiRecCount].szRghtsIssueDate, pszField[20], sizeof(m_pstRecData[m_uiRecCount].szRghtsIssueDate)); // ��ɹ�Ȩ�Ǽ���	C8 YYYYMMDD
	memcpy(m_pstRecData[m_uiRecCount].szDividendDate, pszField[21], sizeof(m_pstRecData[m_uiRecCount].szDividendDate)); // ��ɳ�Ȩ��	C8 YYYYMMDD
	m_pstRecData[m_uiRecCount].dRightOffRate = atof(pszField[22]);	// ��ɱ���	N11(6)
	m_pstRecData[m_uiRecCount].dRightOffVol = atof(pszField[23]); // �������	N16
	m_pstRecData[m_uiRecCount].dIOPV2 = atof(pszField[24]); // T-2�ջ�������/����ֵ	N13(5)
	m_pstRecData[m_uiRecCount].dIOPV1 = atof(pszField[25]); // T-1�ջ�������/����ֵ	N13(5)
	memcpy(m_pstRecData[m_uiRecCount].szRemarks, pszField[26], sizeof(m_pstRecData[m_uiRecCount].szRemarks)); //	��ע	C50


		
	m_uiRecCount++;


	return 1;
	
}
int  CReadFjyFile::inner_AnalyzeField(char * in_pszData, int &lPos, char *out_pszField[], unsigned short in_ushFieldSize,
										bool & out_blLineEnd)
{
	TFjyFieldInfo	* pFieldInfo = NULL;
	unsigned short		ushFieldCount = 0;
	int lFieldTypePos = 0;
	int i;
	out_blLineEnd = false;
// 	if(0 == memcmp(Fjy_FIELD_MD1_MAGIC_FLAG, &in_pszData[lPos], strlen(Fjy_FIELD_MD1_MAGIC_FLAG)))
	{
		pFieldInfo = m_poMD1FieldRecord;
		ushFieldCount =m_ushMD1FieldCount;
		lFieldTypePos = 1;
	}
// 	else
// 	{
// 		//δ�ҵ�
// 		return 0;
// 	}
	if(NULL ==  pFieldInfo || 0 == 	ushFieldCount )
	{
		QuoCollector::GetCollector()->OnLog( TLV_WARN, "CReadFjyFile::inner_AnalyzeField() : ���ļ�%d�У�����%d��¼ʱ��δ���ֶ���Ϣ", m_uiLine+1, lFieldTypePos );
		return -1;
	}
	if(in_ushFieldSize < ushFieldCount)
	{
		QuoCollector::GetCollector()->OnLog( TLV_WARN, "CReadFjyFile::inner_AnalyzeField() : ���ļ�%d�У�����%d��¼ʱ���ֶγߴ�%d����%d", m_uiLine+1, lFieldTypePos, in_ushFieldSize, ushFieldCount );
		return -2;
	}

	for (i = 0; i < ushFieldCount-1; i++ )
	{
		out_pszField[i] = &in_pszData[lPos];
		lPos += pFieldInfo[i].FieldSize; 
		if('|' != in_pszData[lPos] )
		{
			QuoCollector::GetCollector()->OnLog( TLV_WARN, "CReadFjyFile::inner_AnalyzeField() : ���ļ�%d�У�����%d��¼ʱ����%d�ֶ�ʱ��û���ҵ��ָ����������Ǵ�λ", m_uiLine+1, lFieldTypePos, i+1 );
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
int  CReadFjyFile::SetFieldInfo()
{
	int lRet;
	if(0 > (lRet = inner_SetMD1fieldinfo()))
		return lRet;
	return 1;

}

int CReadFjyFile::inner_SetMD1fieldinfo() //�����ֶ����á�
{
	register int			i = 0;	
	TFjyFieldInfo		olstMDFieldRecord[Fjy_FIELD_COUNT];
	
		
	//	1	type �ο���������	C5
	strcpy(olstMDFieldRecord[i].FieldName, "type");
	olstMDFieldRecord[i].FieldType = 'C';
	olstMDFieldRecord[i].FieldSize = 5;
	olstMDFieldRecord[i].FieldOffset = 0;
	i++;
	
	//	2	NonTrCode	�ǽ���֤ȯ����	C6
	strcpy(olstMDFieldRecord[i].FieldName, "NonTrCode");
	olstMDFieldRecord[i].FieldType = 'C';
	olstMDFieldRecord[i].FieldSize = 6;
	i++;
	// 		3	NonTrName	�ǽ���֤ȯ����	C8
	
	strcpy(olstMDFieldRecord[i].FieldName, "NonTrName");
	olstMDFieldRecord[i].FieldType = 'C';
	olstMDFieldRecord[i].FieldSize = 8;
	i++;
	//	4	Code	��Ʒ֤ȯ����	C6
	strcpy(olstMDFieldRecord[i].FieldName, "Code");
	olstMDFieldRecord[i].FieldType = 'C';
	olstMDFieldRecord[i].FieldSize = 6;
	i++;
	//	5	Name	��Ʒ֤ȯ����	C8
	
	strcpy(olstMDFieldRecord[i].FieldName, "Name");
	olstMDFieldRecord[i].FieldType = 'C';
	olstMDFieldRecord[i].FieldSize = 8;
	i++;
	// 	6 BusType	�ǽ���ҵ������		c2
	
	strcpy(olstMDFieldRecord[i].FieldName, "BusType");
	olstMDFieldRecord[i].FieldType = 'C';
	olstMDFieldRecord[i].FieldSize = 2;
	i++;
	// 	7	NonTrBDate	�ǽ��׶������뿪ʼ����	C8
	strcpy(olstMDFieldRecord[i].FieldName, "NonTrBDate");
	olstMDFieldRecord[i].FieldType = 'C';
	olstMDFieldRecord[i].FieldSize = 8;
	i++;

	// 	8	NonTrEDate	�ǽ��׶��������������	C8
	strcpy(olstMDFieldRecord[i].FieldName, "NonTrEDate");
	olstMDFieldRecord[i].FieldType = 'C';
	olstMDFieldRecord[i].FieldSize = 8;
	i++;

	// 	9	NonTrOrdHands �ǽ��׶���������	N12
	strcpy(olstMDFieldRecord[i].FieldName, "NonTrOrdHands");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 12;
	i++;

	// 	10	NonTrOrdMinHands �ǽ��׶�����С��������	N12
	strcpy(olstMDFieldRecord[i].FieldName, "NonTrOrdMinHands");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 12;
	i++;
	
	// 	11	NonTrOrdMaxHands �ǽ��׶�������󶩵�����	N12
	strcpy(olstMDFieldRecord[i].FieldName, "NonTrOrdMaxHands");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 12;
	i++;
	
	// 	12	NonTrPrice �ǽ��׼۸�	N13(5)
	strcpy(olstMDFieldRecord[i].FieldName, "NonTrPrice");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 13;
	olstMDFieldRecord[i].DecSize = 5;
	i++;
	
	// 	13	IPOVol	IPO����	N16
	strcpy(olstMDFieldRecord[i].FieldName, "IPOVol");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 16;
	i++;
	
	// 	14	IPOAMethod IPO���䷽��	C1
	strcpy(olstMDFieldRecord[i].FieldName, "IPOAMethod");
	olstMDFieldRecord[i].FieldType = 'C';
	olstMDFieldRecord[i].FieldSize = 1;
	i++;
	
	// 	15	IPOSellDate IPO���۷���������������	C8 YYYYMMDD
	strcpy(olstMDFieldRecord[i].FieldName, "IPOSellDate");
	olstMDFieldRecord[i].FieldType = 'C';
	olstMDFieldRecord[i].FieldSize = 8;
	i++;
	
	// 	16	IPOVfNumbDate IPO���ʻ��������	C8 YYYYMMDD
	strcpy(olstMDFieldRecord[i].FieldName, "IPOVfNumbDate");
	olstMDFieldRecord[i].FieldType = 'C';
	olstMDFieldRecord[i].FieldSize = 8;
	i++;
	
	// 	17	IPOLttDrawDate IPOҡ�ų�ǩ������	C8 YYYYMMDD
	strcpy(olstMDFieldRecord[i].FieldName, "IPOLttDrawDate");
	olstMDFieldRecord[i].FieldType = 'C';
	olstMDFieldRecord[i].FieldSize = 8;
	i++;
	
	// 	18	IPOLowRngPrice IPO�깺�۸���������	N11(3)
	strcpy(olstMDFieldRecord[i].FieldName, "IPOLowRngPrice");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	
	// 	19	IPOHighRngPrice IPO�깺�۸���������	N11(3)
	strcpy(olstMDFieldRecord[i].FieldName, "IPOHighRngPrice");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	
	// 	20	IPOPropPlac IPO�������۱���	N11(3)
	strcpy(olstMDFieldRecord[i].FieldName, "IPOPropPlac");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 3;
	i++;
	 	
	// 	21	RghtsIssueDate ��ɹ�Ȩ�Ǽ���	C8 YYYYMMDD
	strcpy(olstMDFieldRecord[i].FieldName, "RghtsIssueDate");
	olstMDFieldRecord[i].FieldType = 'C';
	olstMDFieldRecord[i].FieldSize = 8;
	i++;
	
	// 	22	DividendDate ��ɳ�Ȩ��	C8 YYYYMMDD
	strcpy(olstMDFieldRecord[i].FieldName, "DividendDate");
	olstMDFieldRecord[i].FieldType = 'C';
	olstMDFieldRecord[i].FieldSize = 8;
	i++;
	
	// 	23	RightOffRate ��ɱ���	N11(6)
	strcpy(olstMDFieldRecord[i].FieldName, "RightOffRate");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 11;
	olstMDFieldRecord[i].DecSize = 6;
	i++;
	
	// 	24	RightOffVol �������	N16
	strcpy(olstMDFieldRecord[i].FieldName, "RightOffVol");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 16;
	i++;
	
	// 	25	IOPV2 T-2�ջ�������/����ֵ	N13(5)
	strcpy(olstMDFieldRecord[i].FieldName, "IOPV2");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 13;
	olstMDFieldRecord[i].DecSize = 5;
	i++;
	
	// 	26	IOPV1 T-1�ջ�������/����ֵ	N13(5)
	strcpy(olstMDFieldRecord[i].FieldName, "IOPV1");
	olstMDFieldRecord[i].FieldType = 'N';
	olstMDFieldRecord[i].FieldSize = 13;
	olstMDFieldRecord[i].DecSize = 5;
	i++;
	 	
	// 	27	Remarks	��ע	C50
	strcpy(olstMDFieldRecord[i].FieldName, "Remarks");
	olstMDFieldRecord[i].FieldType = 'C';
	olstMDFieldRecord[i].FieldSize =50;
	i++;

	if(NULL == m_poMD1FieldRecord || i > m_ushMD1FieldCount)
	{
		m_ushMD1FieldCount = i;
		
		if ( m_poMD1FieldRecord != NULL )
		{
			delete []m_poMD1FieldRecord;
			m_poMD1FieldRecord = NULL;
		}
		m_poMD1FieldRecord = new TFjyFieldInfo[m_ushMD1FieldCount];
		if ( m_poMD1FieldRecord==NULL )
		{
			m_ushMD1FieldCount = 0;
			QuoCollector::GetCollector()->OnLog( TLV_WARN, "CReadFjyFile::inner_AnalyzeField() : �����ֶ���Ϣ1�������ڴ����%d", GetLastError() );
			return ERROR_Fjy_APPFIELDMEMORY;
		}

	}
	else
		m_ushMD1FieldCount = i;
	memcpy(m_poMD1FieldRecord, &olstMDFieldRecord[0], sizeof(TFjyFieldInfo)*m_ushMD1FieldCount);


	for(i = 1; i < m_ushMD1FieldCount; i++)
	{
		m_poMD1FieldRecord[i].FieldOffset = m_poMD1FieldRecord[i-1].FieldOffset+ m_poMD1FieldRecord[i-1].FieldSize;
	}

	return 1;

}


