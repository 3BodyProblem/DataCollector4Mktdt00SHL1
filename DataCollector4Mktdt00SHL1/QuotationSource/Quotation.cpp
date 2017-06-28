#include <math.h>
#include <algorithm>
#include "Quotation.h"
#include "../Infrastructure/IniFile.h"
#include "../DataCollector4Mktdt00SHL1.h"


#pragma pack(1)

typedef struct
{
    char Name[8];
    char Mask[10][7];
    unsigned short Unit;	//�ֱ���
    unsigned char  Deci;	//С����λ��
    unsigned long  PriceRate;	//�۸�Ŵ���
} LONKIND4X_REC;

typedef struct
{
    unsigned char MarketNumber;
    unsigned char GroupNumber[8];
    unsigned char Reserved[7];
} LONKIND_HEAD;

typedef struct
{
    char Type;
    char Name[8];
    char TimeFields;
    unsigned short Time[4][2];
    char Mask[10][6];
    unsigned short VolRate,AmtRate;
    unsigned short Unit;	//�ֱ���
    unsigned char  Deci;	//С����λ��
    unsigned long  ExRate;
    unsigned short PRate;	//
    unsigned long  AlarmVol;
    unsigned long  AlarmAmt;//��Ԫ
    unsigned short AlarmTurn;//1/10000
    char Reserved[19];
} LONKIND_REC;

#pragma pack()


WorkStatus::WorkStatus()
: m_eWorkStatus( ET_SS_UNACTIVE )
{
}

WorkStatus::WorkStatus( const WorkStatus& refStatus )
{
	CriticalLock	section( m_oLock );

	m_eWorkStatus = refStatus.m_eWorkStatus;
}

WorkStatus::operator enum E_SS_Status()
{
	CriticalLock	section( m_oLock );

	return m_eWorkStatus;
}

std::string& WorkStatus::CastStatusStr( enum E_SS_Status eStatus )
{
	static std::string	sUnactive = "δ����";
	static std::string	sDisconnected = "�Ͽ�״̬";
	static std::string	sConnected = "��ͨ״̬";
	static std::string	sLogin = "��¼�ɹ�";
	static std::string	sInitialized = "��ʼ����";
	static std::string	sWorking = "����������";
	static std::string	sUnknow = "����ʶ��״̬";

	switch( eStatus )
	{
	case ET_SS_UNACTIVE:
		return sUnactive;
	case ET_SS_DISCONNECTED:
		return sDisconnected;
	case ET_SS_CONNECTED:
		return sConnected;
	case ET_SS_LOGIN:
		return sLogin;
	case ET_SS_INITIALIZING:
		return sInitialized;
	case ET_SS_WORKING:
		return sWorking;
	default:
		return sUnknow;
	}
}

WorkStatus&	WorkStatus::operator= ( enum E_SS_Status eWorkStatus )
{
	CriticalLock	section( m_oLock );

	if( m_eWorkStatus != eWorkStatus )
	{
		QuoCollector::GetCollector()->OnLog( TLV_INFO, "WorkStatus::operator=() : Exchange Session Status [%s]->[%s]"
											, CastStatusStr(m_eWorkStatus).c_str(), CastStatusStr(eWorkStatus).c_str() );
				
		m_eWorkStatus = eWorkStatus;
	}

	return *this;
}


///< ----------------------------------------------------------------


Quotation::Quotation()
: m_nQuoDate( 0 ), m_nQuoTime( 0 ), SimpleTask( "Thread::Scanner" )
{
	m_mapMkDetail.clear();
	m_mapCode2Type.clear();
	::memset( &m_stKindMask, 0, sizeof(m_stKindMask) );
	m_uchIdx = 0;		//		"��ָ֤��"
	m_uchAG = 0;		//"��֤����"
	m_uchBG = 0;		//"��֤�¹�"
	m_uchJJ = 0;		//"��֤����"
	m_uchZQ = 0;		//"��֤ծȯ"
	m_uchZZ = 0;	//"��֤תծ"
	m_uchHG = 0;	//	"��֤�ع�"
	m_uchETF = 0;	//"��֤ETF"
	m_uchJJT = 0;	//	"����ͨ"
	m_uchQZ = 0;		//"��֤Ȩ֤"
	m_uchYX = 0;		//"��֤����"
	m_uchQT = 0;	//		"��֤����"
	m_uchKFJJ = 0;	//	"���Ż���"
	m_uchFXJS = 0;//		"���վ�ʾ"
	m_uchSZTS = 0;//		"��֤����"
	m_uchFJZS = 0;//		"����ָ��"
}

Quotation::~Quotation()
{
	Release();
}

WorkStatus& Quotation::GetWorkStatus()
{
	return m_oWorkStatus;
}

int Quotation::GetMKTDTNowDateTime(unsigned int &out_ulDate, unsigned int &out_ulTime)
{
	TMKTDTHead*		pstHead = NULL;

	out_ulDate = 0;
	out_ulTime = 0;

	if( !m_oMktdt00File.IsOpened() )
	{
		return -1;
	}

	m_oMktdt00File.GetHead( &pstHead );
	out_ulDate = pstHead->uiTrDate;
	out_ulTime = pstHead->uiTrTime;

	return 0;
}

int Quotation::PrepareCodesTable()
{
	int					nErrCode = -1;
	int					nAddCount = 0;
	int					nRecordCount = 0;
	char				pszFilePath[128] = { 0 };
	TMKTDTHead*			pstHead = NULL;
	TMKTDTFieldData*	pstFieldData = NULL;
	TFjyHead*			pstFjyHead = NULL;
	TFjyFieldData*		pstRecData = NULL;
	CriticalLock		section( m_oLock );

	m_setCodes.clear();
	///< ---------------------- mktdt00.txt
	if( 0 > (nErrCode = m_oMktdt00File.ReloadFromFile()) )
	{
		QuoCollector::GetCollector()->OnLog( TLV_INFO, "Quotation::PrepareCodesTable() : cannot load file mktdt00.txt, errorcode:%d", nErrCode );
		return nErrCode;
	}

	nRecordCount = m_oMktdt00File.GetRecCount();
	for( int i = 0; i < nRecordCount; ++i )
	{
		if( (nErrCode = m_oMktdt00File.GetRecord( i, &pstFieldData )) < 0 )
		{
			QuoCollector::GetCollector()->OnLog( TLV_INFO, "Quotation::PrepareCodesTable() : cannot fetch record from mktdt00.txt, pos:%d, errorcode:%d", i, nErrCode );
			continue;
		}

		m_setCodes.insert( std::string( pstFieldData->szSecurityID, 6 ) );
	}

	///< --------------------- fjyxxxxxx.txt
	::sprintf( pszFilePath, "%s%d.txt", Configuration::GetConfig().GetFjyFilePath().c_str(), DateTime::Now().DateToLong() );
	if( ( nErrCode = m_oFjyFile.ChkOpen( pszFilePath ) ) < 0 )
	{
		QuoCollector::GetCollector()->OnLog( TLV_INFO, "Quotation::PrepareCodesTable() : cannot load file fjyxxxxx.txt, errorcode:%d", nErrCode );
		return nErrCode;
	}

	m_oFjyFile.GetHead( &pstFjyHead );
	if( NULL != pstFjyHead )
	{
		for( int i=0; i<pstFjyHead->uiRecCount; i++ )
		{
			if( 0 > m_oFjyFile.GetRecord(i, &pstRecData) )
			{
				break;
			}

			std::string	sCode( pstRecData->szNonTrCode, 6 );

			if( m_setCodes.find( sCode ) == m_setCodes.end() ) {
				m_setCodes.insert( sCode );
				nAddCount++;
			}
		}
	}
	QuoCollector::GetCollector()->OnLog( TLV_INFO, "Quotation::Initialize() : load code from fjy.txt, number=%d", nAddCount );

	///< ------------------------ scsixxxxxx.txt
	::sprintf( pszFilePath, "%s%d.txt", Configuration::GetConfig().GetZzzsFilePath().c_str(), DateTime::Now().DateToLong() );
	if( 0 >= (nErrCode = m_oZzzsFile.Open( pszFilePath ) ) )
	{
		QuoCollector::GetCollector()->OnLog( TLV_INFO, "Quotation::PrepareCodesTable() : cannot load file scsixxxxx.txt, errorcode:%d", nErrCode );
		return nErrCode;
	}

	do 
	{
		if( 1 == m_oZzzsFile.FieldToInteger("JLLX") )		//ֻ����ָ������
		{
			char	szCode[128] = { 0 };

			if( 0 < m_oZzzsFile.ReadString("ZSDM", szCode, sizeof(szCode)) )
			{
				m_setCodes.insert( std::string( szCode, 6 ) );
			}
		}
	} while( 0 < m_oZzzsFile.Next() );

	return 1;
}

int Quotation::GetShNameTableData( unsigned long RecordPos, tagSHL1ReferenceData_LF100* Out )
{
	long				lRetVal;
	int i;
	char szName[16];
	TMKTDTFieldData * pstFieldData;
	unsigned int uiStreamID=0;		//1��ָ����2��Ʊ��3ծȯ 4���� 
	if( Out == NULL )
	{
		return 0;
	}

	::memset( Out, 0, sizeof(tagSHL1ReferenceData_LF100) );

	try
	{
		m_oMktdt00File.GetRecord( RecordPos, &pstFieldData );
		memcpy(Out->Code, pstFieldData->szSecurityID, sizeof(Out->Code));
		lRetVal = atol( Out->Code );
		memset(szName, 0, sizeof(szName));
		memcpy(szName, pstFieldData->szSymbol, 8);
		for(i=0; i<8; i++)
		{
			if(' ' != szName[i])
				break;
		}

		memcpy(Out->Name, &szName[i], sizeof(Out->Name));
		*((unsigned long *)(Out->PreName))  = lRetVal;
		uiStreamID = pstFieldData->uiStreamID;		//1��ָ����2��Ʊ��3ծȯ 4���� 
	}
	catch( ... )
	{
		QuoCollector::GetCollector()->OnLog( TLV_WARN, "Quotation::Initialize() : GetShNameTableData�����쳣" );
		return -5;
	}
		//add by liuqy 20111102 for ȡָ�����͵�ID

	//add by liuqy 20121129 for ������֤�˼����մ���
	memcpy(szName, Out->Name, 8);
	szName[8] = 0;
/*
	//end add by liuqy
	if( IsIndex( Out->Code ) || (1 == uiStreamID && m_blMKTDTOpenFlag && !m_blActivateDBF))		//1��ָ����2��Ʊ��3ծȯ 4���� ��
	{
		Out->Type = Global_Process.GetShMarket().GetType(SHZSNAME);
	}
	else
		Out->Type = GetOutIdxType(Out->Code, szName);
*/
	return 1;
}

int Quotation::GetZzzsIdxData( tagSHL1SnapData_LF102* pLFData, tagSHL1SnapData_HF103* pHFData )
{
	char szCode[128];
	if(0 >= m_oZzzsFile.ReadString("ZSDM", szCode, sizeof(szCode)) )
		return -1;
	memcpy(pHFData->Code, szCode, sizeof(pHFData->Code));
	memcpy(pLFData->Code, szCode, sizeof(pLFData->Code));
	pHFData->Volume = m_oZzzsFile.FieldToFloat("CJL") / 100 ;

	pHFData->Amount = m_oZzzsFile.FieldToFloat("CJJE") * 10000;
	pLFData->Close =(unsigned long)( m_oZzzsFile.FieldToFloat("ZRSP") * 100 + 0.5);
	pLFData->High = (unsigned long)( m_oZzzsFile.FieldToFloat("DRZD") * 100 + 0.5);
	pLFData->Low = (unsigned long)( m_oZzzsFile.FieldToFloat("DRZX") * 100 + 0.5);
	pHFData->Now = (unsigned long)( m_oZzzsFile.FieldToFloat("SSZS") * 100 + 0.5);
	pLFData->Open = (unsigned long)( m_oZzzsFile.FieldToFloat("DRKP") * 100 + 0.5);
	return 1;
}

//add by liuqy 20120131 for ������ָ֤������ ������ָ֤���Ĵ���
long Quotation::rl_BuildZZIdxCode(int &inout_lSerial, int & out_lKindNum)
{
	tagSHL1ReferenceData_LF100			tempName = { 0 };
	tagSHL1ReferenceExtension_LF101		tempNameEx = { 0 };
	tagSHL1SnapData_LF102				tempIndexLF = { 0 };
	tagSHL1SnapData_HF103				tempIndexHF = { 0 };
	int i;
	int lRet;
	int lHalfFlag;
	char	szCode[128];
	char	szTmp[128];
	char		pszFjyPath[128] = { 0 };

	::sprintf( pszFjyPath, "%s%d.txt", Configuration::GetConfig().GetZzzsFilePath().c_str(), DateTime::Now().DateToLong() );
	lRet = m_oZzzsFile.Open( pszFjyPath );
	if ( 0 >= lRet)
	{
		return lRet;
	}

	do 
	{
		//ֻ����ָ������
		if(1 == m_oZzzsFile.FieldToInteger("JLLX"))
		{
			if(0 < m_oZzzsFile.ReadString("ZSDM", szCode, sizeof(szCode)) 
				&& 0 < m_oZzzsFile.ReadString("JC", szTmp, sizeof(szTmp)))
			{
				memcpy(tempName.Name, szTmp, sizeof(tempName.Name));
				memcpy(tempName.Code, szCode, sizeof(tempName.Code));
				//�������Ƴ�������ϵͳֻȡ��8���ֽڣ��п����а���ַ���������Ҫ����Ƿ���ڰ���ַ�����
				for(i=0,lHalfFlag=0; i<sizeof(tempName.Name); i++)
					if(0 != (0x80 & tempName.Name[i]))
						lHalfFlag++;
				//�������������ʾ�������
				if(lHalfFlag & 1)
					tempName.Name[sizeof(tempName.Name) - 1] = 0;
				//if(0 > Global_Process.GetShNameTable().GetSerial(tempName.Code))
				if( m_setCodes.find( std::string( tempName.Code, 6 ) ) != m_setCodes.end() )
				{						
					++inout_lSerial;

/*					if( Global_Process.GetShNameTable().PushOne( &tempName ) )
					{
						out_lKindNum++;
						GetZzzsIdxData(&tempIndex );
						CODEASSIGN( tempIndex,  tempName );
						if( !Global_Process.GetShIndexTable().PushOne( &tempIndex ) )
						{
							Global_Process.WriteError( 0, "transrcsh", "<BuildZZIdxCode>ָ��PushOne(%d)����\n", inout_lSerial );
						}
						
						CODEASSIGN( tempNameEx, tempName );
						tempNameEx.SFlag = '*';
						Global_Process.GetShNameTableEx().PushOne( &tempNameEx );					
					}*/
				}
			}
		}
	} while (0 < m_oZzzsFile.Next());

	return 0;	
}

//add by liuqy 20150304 for ���ӷǽ��״���
long Quotation::rl_BuildMKTDTFjyCode(int &inout_lSerial, int * out_plKindNum, bool in_blLoadIdx)
{
	tagSHL1ReferenceData_LF100			tempName = { 0 };
	tagSHL1ReferenceExtension_LF101		tempNameEx = { 0 };
	tagSHL1SnapData_LF102				tempStockLF = { 0 };
	tagSHL1SnapData_HF103				tempStockHF = { 0 };
	int i;
	int lRet;
	char	szName[16];
	TFjyHead*	pstFjyHead;
	TFjyFieldData *	pstRecData;

	m_oFjyFile.GetHead(&pstFjyHead);

	for(i=0; i<pstFjyHead->uiRecCount; i++) 
	{
		if(0 > m_oFjyFile.GetRecord(i, &pstRecData))
			break;
		memset(&tempName, 0, sizeof(tempName));
		memcpy(tempName.Code, pstRecData->szNonTrCode, 6);
		lRet = atol(tempName.Code);
		memcpy(tempName.Name, pstRecData->szNonTrName, sizeof(tempName.Name));
		memcpy(szName, tempName.Name, sizeof(tempName.Name));
		szName[sizeof(tempName.Name)] = 0;

		if( m_setCodes.find( std::string( tempName.Code, 6 ) ) != m_setCodes.end() )
		{
/*			if( IsIndex( stExistCode.Code ) )		//1��ָ����2��Ʊ��3ծȯ 4���� ��
			{
				 //ֻ����ָ��
				if(!in_blLoadIdx)
					continue;
				tempName.Kind = Global_Process.GetShMarket().GetIdxType();
			}
			else
			{
				//���س�ָ��������д���		
				if(in_blLoadIdx)
					continue;
				tempName.Kind = GetOutIdxType(stExistCode.Code, szName);
			}
*/
			*((unsigned long *)(tempName.PreName)) = lRet;

			++inout_lSerial;
/*
			Global_Process.GetShTradingPhaseCode().PushOne(&stTradingPhaseCode);

			if( !Global_Process.GetShNameTable().PushOne( &tempName ) )
			{
				Global_Process.WriteError( 0, "transrcsh", "<BuildMKTDTFjyCode>���PushOne(%d)����\n", inout_lSerial );
			}
			else
			{
				if(32 > tempName.Type)
				{
					out_plKindNum[tempName.Type]++;
					GetMKTDTFjyData(i, &tempStock, true );

					//GetZzzsIdxData(&tempIndex );
					CODEASSIGN( tempStock,  tempName );
					if( !Global_Process.GetShStockTable().PushOne( &tempStock ) )
					{
						Global_Process.WriteError( 0, "transrcsh", "<BuildMKTDTFjyCode>ָ��PushOne(%d)����\n", inout_lSerial );
					}
							
					CODEASSIGN( tempNameEx, tempName );
					tempNameEx.SFlag = ' ';
					Global_Process.GetShNameTableEx().PushOne( &tempNameEx );					
				}
			}
*/
		}
	} 
	
	return 1;	
}

int Quotation::SetKindInfo()
{
	inifile::IniFile			oIniFile;
	int							nErrCode = 0;
	int							iKindCount = 0;
	tagSHL1MarketInfo_LF106		tagHead = { 0 };
	tagSHL1KindDetail_LF107		tagKindInfo = { 0 };
	LONKIND4X_REC				stLonkind[32] = { 0 };
	int							iLonkindcount = 0;
	LONKIND_HEAD				stLonKindHead = { 0 };
	LONKIND_REC					stLonKindRec[16] = { 0 };
	std::string					strData;
	char						szTmp[128];
	int lLen,j,k,i;

	if( 0 < oIniFile.load( Configuration::GetConfig().GetLonKindPath().c_str() ) )	//��4X��4xlonkind.ini�ļ��ж���������
	{
		iLonkindcount = oIniFile.getIntValue( std::string("market_sh"), std::string("count"), nErrCode );
		if( 0 != nErrCode )	{
			iLonkindcount = 0;
		}

		for( i = 0; i < iLonkindcount; i++ )
		{
			//��תָ��
			sprintf(szTmp, "name%0d", i);
			strData = oIniFile.getStringValue( std::string("market_sh"), std::string(szTmp), nErrCode );
			if( 0 == strData.length() || nErrCode != 0 )	{
				break;
			}

			::memcpy( stLonkind[i].Name, strData.c_str(), sizeof( stLonkind[i].Name) );
			//��һ���ֱ���
			sprintf( szTmp, "unit%0d", i );
			stLonkind[i].Unit = oIniFile.getIntValue( std::string("market_sh"), std::string(szTmp), nErrCode );
			if( 0 != nErrCode )	{
				stLonkind[i].Unit = 100;
			}
			//С����λ��
			sprintf(szTmp, "deci%0d", i);
			stLonkind[i].Deci = oIniFile.getIntValue( std::string("market_sh"), std::string(szTmp), nErrCode );
			if( 0 != nErrCode )	{
				stLonkind[i].Deci = 3;
			}
			//�۸�Ŵ���
			sprintf(szTmp, "PriceRate%0d", i);
			stLonkind[i].PriceRate = oIniFile.getIntValue( std::string("market_sh"), std::string(szTmp), nErrCode );
			if( 0 != nErrCode )	{
				stLonkind[i].PriceRate = 100;
			}

			//���� �ж������ʱ���м��á�,�����������һ����43��ͷ����83��ͷ������Ϊ��43****,83****
			sprintf(szTmp, "mask%0d", i);
			std::string	sMaskVal = oIniFile.getStringValue( std::string("market_sh"), std::string(szTmp), nErrCode );
			if( 0 == sMaskVal.length() || nErrCode != 0 )
			{
				sMaskVal = "*";
			}
			strcpy( szTmp, sMaskVal.c_str() );

			j=0;
			lLen = 0;
			k=0;
			while(szTmp[lLen])
			{
				if('0' <=szTmp[lLen] && '9' >= szTmp[lLen])
				{
					if(k<6)
						stLonkind[i].Mask[j][k] = szTmp[lLen];
					k++;
				}
				else
				{
					if(',' == szTmp[lLen])
					{
						k = 0;
						j++;
					}
				}
				if(j >= MAX_MASK_COUNT)
					break;

				lLen++;
			}
		}
		iLonkindcount = i;
		if( 0 >= iLonkindcount )
		{
			QuoCollector::GetCollector()->OnLog( TLV_WARN, "Quotation::SetKindInfo() : �������ļ�:%s�����ļ���market_sh�����޺Ϸ���¼", Configuration::GetConfig().GetLonKindPath().c_str() );
			return -1;
		}
	}
	else
	{
		QuoCollector::GetCollector()->OnLog( TLV_WARN, "Quotation::SetKindInfo() : �������ļ�:%s����", Configuration::GetConfig().GetLonKindPath().c_str() );
		return -1;
	}

	for( i = 0; i < iLonkindcount; i++ )
	{
		if( iKindCount >= MAX_KIND_MASK )
		{
			break;
		}

		memset( &tagKindInfo, 0, sizeof( tagKindInfo ) );
		strncpy( tagKindInfo.KindName, stLonkind[i].Name, 8 );
		tagKindInfo.PriceRate = log10( (double)stLonkind[i].PriceRate );
		tagKindInfo.LotSize = stLonkind[i].Unit;
		if(iKindCount < MAX_KIND_MASK)
		{
			for(m_stKindMask[iKindCount].uchCount=0; m_stKindMask[iKindCount].uchCount<10; m_stKindMask[iKindCount].uchCount++)
			{
				for(lLen=0; lLen<6; lLen++)
				{
					if('*' == stLonkind[i].Mask[m_stKindMask[iKindCount].uchCount][lLen] 
						|| 0 == stLonkind[i].Mask[m_stKindMask[iKindCount].uchCount][lLen] )
						
						break;
					m_stKindMask[iKindCount].uchMask[m_stKindMask[iKindCount].uchCount][lLen] = stLonkind[i].Mask[m_stKindMask[iKindCount].uchCount][lLen];
				}
				if(0 == lLen)
					break;
			}
		}
		else
		{
			break;
		}

		QuoCollector::GetCollector()->OnImage( 107, (char*)&tagKindInfo, sizeof(tagKindInfo), false );
		++iKindCount;
	}

	if(0 == memcmp(tagKindInfo.KindName, SHZSNAME, lLen))
	{
		m_uchIdx = m_mapMkDetail.size();		//"��ָ֤��"
	}
	else if(0 == memcmp(tagKindInfo.KindName, SHAGNAME, lLen))
	{
		m_uchAG = m_mapMkDetail.size();		//"��֤����"
	}
	else if(0 == memcmp(tagKindInfo.KindName, SHBGNAME, lLen))
	{
		m_uchBG = m_mapMkDetail.size();		//"��֤�¹�"
	}
	else if(0 == memcmp(tagKindInfo.KindName, SHJJNAME, lLen))
	{
		m_uchJJ = m_mapMkDetail.size();		//"��֤����"
	}
	else if(0 == memcmp( tagKindInfo.KindName, SHZQNAME, lLen))
	{
		m_uchZQ = m_mapMkDetail.size();		//"��֤ծȯ"
	}
	else if(0 == memcmp( tagKindInfo.KindName, SHZZNAME, lLen))
	{
		m_uchZZ = m_mapMkDetail.size();	//"��֤תծ"
	}
	else if(0 == memcmp( tagKindInfo.KindName, SHHGNAME, lLen))
	{
		m_uchHG = m_mapMkDetail.size();	//	"��֤�ع�"
	}
	else if(0 == memcmp( tagKindInfo.KindName, SHETFNAME, lLen))
	{
		m_uchETF = m_mapMkDetail.size();	//"��֤ETF"
	}
	else if(0 == memcmp( tagKindInfo.KindName, SHJJTNAME, lLen))
	{
		m_uchJJT = m_mapMkDetail.size();	//	"����ͨ"
	}
	else if(0 == memcmp( tagKindInfo.KindName, SHYXNAME, lLen))
	{
		m_uchYX = m_mapMkDetail.size();		//"��֤����"
	}
	else if(0 == memcmp( tagKindInfo.KindName, SHQZNAME, lLen))
	{
		m_uchQZ = m_mapMkDetail.size();		//"��֤Ȩ֤"
	}
	else if(0 == memcmp( tagKindInfo.KindName, SHQTNAME, lLen))
	{
		m_uchQT = m_mapMkDetail.size();	//		"��֤����"
	}
	else if(0 == memcmp( tagKindInfo.KindName, SHKFJJNAME, lLen))
	{
		m_uchKFJJ = m_mapMkDetail.size();	//	"���Ż���"
	}
	else if(0 == memcmp( tagKindInfo.KindName, SHFXJSNAME, lLen))
	{
		m_uchFXJS = m_mapMkDetail.size();//		"���վ�ʾ"
	}
	else if(0 == memcmp( tagKindInfo.KindName, SHSZTSNAME, lLen))
	{
		m_uchSZTS = m_mapMkDetail.size();//		"��֤����"
	}
	else if(0 == memcmp( tagKindInfo.KindName, SHFJJSNAME, lLen))
	{
		m_uchFJZS = m_mapMkDetail.size();//		"����ָ��"
	}
	m_mapMkDetail[m_mapMkDetail.size()] = tagKindInfo;

	//���û��������ǿ������һ������
	memset( &tagKindInfo, 0, sizeof( tagKindInfo ) );
	strncpy( tagKindInfo.KindName, SHQTNAME, 8 );
	tagKindInfo.PriceRate = 2;
	tagKindInfo.LotSize = 100;
	QuoCollector::GetCollector()->OnImage( 107, (char*)&tagKindInfo, sizeof(tagKindInfo), false );

	//���A��û���ҵ�����������
	if(0 == m_uchAG)
		m_uchAG = m_uchQT;
	//���B��û���ҵ�����������
	if(0 == m_uchBG)
		m_uchBG = m_uchQT;

	if(0  == m_uchJJ )		//"��֤����"
		m_uchJJ = m_uchQT;

	if(0  == m_uchZQ )		//"��֤ծȯ"
		m_uchZQ = m_uchQT;
	if(0  == m_uchZZ )	//"��֤תծ"
		m_uchZZ = m_uchQT;
	if(0  == m_uchHG )	//	"��֤�ع�"
		m_uchHG = m_uchQT;
	if(0  == m_uchETF)	//"��֤ETF"
		m_uchETF = m_uchQT;
	if(0  == m_uchJJT)	//	"����ͨ"
		m_uchJJT = m_uchQT;
	if(0  == m_uchQZ)		//"��֤Ȩ֤"
		m_uchQZ = m_uchQT;
	if(0  == m_uchYX )	//"��֤����"
		m_uchYX = m_uchQT;
	if(0  == m_uchKFJJ )	//	"���Ż���"
		m_uchKFJJ = m_uchQT;
	if(0  == m_uchFXJS)//		"���վ�ʾ"
		m_uchFXJS = m_uchQT;
	if(0  == m_uchSZTS )//		"��֤����"
		m_uchSZTS = m_uchQT;
	if(0  == m_uchFJZS )//		"����ָ��"
		m_uchFJZS = m_uchQT;

	CriticalLock		section( m_oLock );
	tagHead.KindCount = iKindCount;
	tagHead.PeriodsCount = 2;
	tagHead.MarketPeriods[0][0] = 9*60+30;
	tagHead.MarketPeriods[0][1] = 11*60+30;
	tagHead.MarketPeriods[1][0] = 13*60;
	tagHead.MarketPeriods[1][1] = 15*60;

	QuoCollector::GetCollector()->OnImage( 106, (char*)&tagHead, sizeof(tagHead), false );

	return 1;
}

int Quotation::GetMemIndexData( unsigned long RecordPos, tagSHL1SnapData_LF102* pLFData, tagSHL1SnapData_HF103* pHFData )
{
	TMKTDTFieldData*		pstFieldData = NULL;
	TMKTDTHead*				pstHead = NULL;

	if( NULL == pLFData || NULL == pHFData )
	{
		return 0;
	}

	::memset( pLFData, 0, sizeof( tagSHL1SnapData_LF102 ) );
	::memset( pHFData, 0, sizeof( tagSHL1SnapData_HF103 ) );

	try
	{
		m_oMktdt00File.GetRecord( RecordPos, &pstFieldData );
		m_oMktdt00File.GetHead( &pstHead );

		if( 999999.9000 > pstFieldData->dPreClosePx )
		{
			pLFData->Close = (long)( pstFieldData->dPreClosePx*100+0.5 );
		}
		else
		{
			pLFData->Close = 0;
		}

		if(999999.9000 > pstFieldData->dOpenPrice)
		{
			pLFData->Open = (unsigned long)( pstFieldData->dOpenPrice *100+0.5 );
		}
		else
		{
			pLFData->Open = 0;
		}
		if(999999.9000 > pstFieldData->dHighPrice)
		{
			pLFData->High = (unsigned long)( pstFieldData->dHighPrice *100+0.5 );
		}
		else
		{
			pLFData->High = 0;
		}
		if(999999.9000 > pstFieldData->dLowPrice)
		{
			pLFData->Low = (unsigned long)( pstFieldData->dLowPrice*100+0.5 );
		}
		else
		{
			pLFData->Low = 0;
		}
		if(150000 <= pstHead->uiTrTime && 0 < pstFieldData->dClosePx)
		{
			if(999999.9000 > pstFieldData->dClosePx)
				pHFData->Now = (unsigned long)( pstFieldData->dClosePx *100+0.5 );
			else
				pHFData->Now = 0 ;
		}
		else
		{
			if(999999.9000 > pstFieldData->dTradePrice)
			{
				pHFData->Now = (unsigned long)( pstFieldData->dTradePrice *100+0.5 );
			}
			else
			{
				pHFData->Now = 0;
			}
		}

		pHFData->Amount = pstFieldData->dTotalValueTraded ;
		pHFData->Volume = __int64(pstFieldData->dTradeVolume);
	}
	catch( ... )
	{
		return -3;
	}

	return 1;
}

int Quotation::GetMemStockData(  unsigned long RecordPos, tagSHL1SnapData_LF102* pLFData, tagSHL1SnapData_HF103* pHFData, tagSHL1SnapBuySell_HF104* pBSData, unsigned long  &ulVoip, bool in_blFirstFlag )
{
	int							prate = 100;
	double						dTemp = 0.0L;
	double						rate = 1;
	char						code[6] = {0};
	TMKTDTFieldData*			pstFieldData = NULL;
	TMKTDTHead*					pstHead = NULL;
	int							i;

	::memset( pLFData, 0, sizeof( tagSHL1SnapData_LF102 ) );
	::memset( pHFData, 0, sizeof( tagSHL1SnapData_HF103 ) );
	::memset( pBSData, 0, sizeof( tagSHL1SnapBuySell_HF104 ) );

	try
	{
		m_oMktdt00File.GetRecord( RecordPos, &pstFieldData );
		memcpy( code, pstFieldData->szSecurityID, sizeof(code) );
		m_oMktdt00File.GetHead( &pstHead );

		char	cKind = m_mapCode2Type[std::string( code, 6 )];//modify by liuqy 20121129 for ������֤�˼����մ���
		prate = m_mapMkDetail[cKind].PriceRate;

		if( 10 > prate )
		{
			prate = 100;
			if( GetETFType() == cKind || GetBGType() == cKind 
				||	GetJJType() == cKind || GetHGType() == cKind 
				||	GetQZType() == cKind)
			{
				prate = 1000;
			}
			else if( GetJJTType() == cKind )
			{
				prate = 10000;
			}
		}

		if(9999999.900 > pstFieldData->dPreClosePx)
			pLFData->Close = (long)(( pstFieldData->dPreClosePx + 0.5/prate)*prate);
		else
			pLFData->Close = 0;
		if(9999999.900 > pstFieldData->dOpenPrice)
			pLFData->Open = (unsigned long)(( pstFieldData->dOpenPrice + 0.5/prate)*prate);
		else
			pLFData->Open = 0;
		if(9999999.900 > pstFieldData->dHighPrice)
			pLFData->High = (unsigned long)(( pstFieldData->dHighPrice + 0.5/prate)*prate);
		else
			pLFData->High = 0;
		if(9999999.900 > pstFieldData->dLowPrice)
			pLFData->Low = (unsigned long)(( pstFieldData->dLowPrice + 0.5/prate)*prate);
		else
			pLFData->Low = 0;

		if(150000 <= pstHead->uiTrTime && 0 < pstFieldData->dClosePx)
		{
			if(9999999.900 > pstFieldData->dClosePx)
				pHFData->Now = (unsigned long)(( pstFieldData->dClosePx + 0.5/prate)*prate);
			else
				pHFData->Now = 0 ;
		}
		else
		{
			if(9999999.900 > pstFieldData->dTradePrice)
				pHFData->Now = (unsigned long)(( pstFieldData->dTradePrice + 0.5/prate)*prate);
			else
				pHFData->Now = 0;
		}
		if(9999999.900 > pstFieldData->dIOPV)
			ulVoip = (unsigned long)(( pstFieldData->dIOPV + 0.5/prate)*prate);

		else
			ulVoip =0;

		if ( !memcmp(code,"519",3) )
		{
			pLFData->Open   = 0;
			pLFData->High   = 0;
			pLFData->Low    = 0;
		}

		dTemp = pstFieldData->dTradeVolume;

		if( cKind == GetHGType())
		{
			pHFData->Volume = dTemp*100;
			rate = 100;
		}
		else if( cKind == GetQZType() )
		{
			pHFData->Volume = dTemp*100.0f;
			rate = 1.0/1.0f;
		}
		else if( cKind == GetETFType() )
		{
			pHFData->Volume = dTemp/1.0f;
			rate = 1.0/1.0f;
		}
		else
			pHFData->Volume = dTemp*1;

		pHFData->Amount = pstFieldData->dTotalValueTraded ;

		for(i=0; i<5; i++)
		{
			pBSData->Buy[i].Price = unsigned long (prate*(pstFieldData->stBuy[i].dPrice+0.5/prate));
			pBSData->Sell[i].Price = unsigned long (prate*(pstFieldData->stSell[i].dPrice+0.5/prate));
			pBSData->Buy[i].Volume =  __int64( pstFieldData->stBuy[i].dVolume * rate);
			pBSData->Sell[i].Volume = __int64( pstFieldData->stSell[i].dVolume * rate);
		}		
	}
	catch( ... )
	{
		return -3;
	}

	pHFData->Voip = 0;

	return 1;
}

int Quotation::BuildImageData()
{
	unsigned int					uiRecCount = 0;
	int								nSerial = -1;
	int								KindNum[32] = { 0 };
	double							dTemp = 0.0;
	register int					nErrCode = 0;
	tagSHL1ReferenceData_LF100		tempName = { 0 };
	tagSHL1ReferenceExtension_LF101	tempNameEx = { 0 };
	tagSHL1SnapData_LF102			tempStockLF = { 0 };
	tagSHL1SnapData_HF103			tempStockHF = { 0 };
	tagSHL1SnapBuySell_HF104		tempStockBS = { 0 };
/*	tagSHMem_MarketHead				tempHead;*/
//	tagSHMem_IndexData				tagIndex000001;
	bool							blZZIdxProcFlag = false;	//add by liuqy 20120131 for ������ָ֤������
	TMKTDTHead * pstHead;
	unsigned long ulVoip = 0;
	bool blPrcIdxMKTDTFJYFlag;
	CriticalLock	section( m_oLock );

	if( DateTime::Now().DateToLong() == m_nQuoDate )
	{
		int	nDiff = DateTime::Now().TimeToLong() - m_nQuoTime;

		if( nDiff > 60 * 10 ) {
			QuoCollector::GetCollector()->OnLog( TLV_WARN, "Quotation::BuildImageData() : invalid market time, nowt=%u, mktime=%u", DateTime::Now().TimeToLong(), m_nQuoTime );
		}
	}

	m_oMktdt00File.GetHead( &pstHead );
	uiRecCount = m_oMktdt00File.GetRecCount();
	for( int i = 0; i < uiRecCount; ++i )
	{
		nErrCode = GetShNameTableData( i, &tempName );
		if( nErrCode < 0 )
		{
			QuoCollector::GetCollector()->OnLog( TLV_WARN, "Quotation::BuildImageData() : GetShNameTableData(%d)[Err:%d]", i, nErrCode );
			continue;
		}

		if( m_setCodes.find( std::string( tempName.Code, sizeof(tempName.Code) ) ) == m_setCodes.end() )
		{
			continue;
		}

		if(0 > rl_BuildZZIdxCode(nSerial, KindNum[0]))
		{
			QuoCollector::GetCollector()->OnLog( TLV_WARN, "Quotation::BuildImageData() : ������ָ֤������д���" );
			//��ָ֤������ʧ�ܣ�ҲҪ��ϵͳ���������Բ����д�����
		}

		//�ȴ���ָ��
		rl_BuildMKTDTFjyCode(nSerial, KindNum);

		::memcpy( tempName.Code+0, tempNameEx.Code+0, sizeof(tempName.Code) );

		TMKTDTFieldData * pstFieldData;
		m_oMktdt00File.GetRecord( i, &pstFieldData );		//modify by liuqy 20160218 �ݲ������2λ��־
		if( 'P' == pstFieldData->szTradingPhaseCode[0] )
		{
			tempNameEx.SFlag = '*';
		}
		else
		{
			tempNameEx.SFlag = ' ';
		}

		m_mapCode2Type[std::string(tempName.Code, 6)] = tempName.Kind;
		QuoCollector::GetCollector()->OnImage( 100, (char*)&tempName, sizeof(tempName), false );
		QuoCollector::GetCollector()->OnImage( 101, (char*)&tempNameEx, sizeof(tempNameEx), false );

		KindNum[tempName.Kind]++;

		if( tempName.Kind == GetIdxType() )//ָ��			
		{
			GetMemIndexData( i, &tempStockLF, &tempStockHF );
			QuoCollector::GetCollector()->OnImage( 102, (char*)&tempStockLF, sizeof(tempStockLF), false );
			QuoCollector::GetCollector()->OnImage( 103, (char*)&tempStockHF, sizeof(tempStockHF), false );
		}
		else				 //��ָ��ȫ�����ɸ���
		{							
			GetMemStockData( i, &tempStockLF, &tempStockHF, &tempStockBS, ulVoip );
			QuoCollector::GetCollector()->OnImage( 102, (char*)&tempStockLF, sizeof(tempStockLF), false );
			QuoCollector::GetCollector()->OnImage( 103, (char*)&tempStockHF, sizeof(tempStockHF), false );
			QuoCollector::GetCollector()->OnImage( 104, (char*)&tempStockBS, sizeof(tempStockBS), false );
		}
	}

	rl_BuildMKTDTFjyCode(nSerial, KindNum);

	//000001���������Ϊ000002,000003,000011�ĺ�
/*	char szFdId[6] = { '0', '0', '0', '0', '0', '1' };
	if( Global_Process.GetShIndexTable().GetCodeData( szFdId, &tagIndex000001 ) )
	{
		tagIndex000001.Amount = 0;
		tagIndex000001.Volume = 0;
		szFdId[5] = '2';
		if( Global_Process.GetShIndexTable().GetCodeData( szFdId, &tempIndex ) )
		{
			//add by liuqy 20130502 ���A�ɵ���Ϊ0ʱ����������
			if(0 < tempIndex.Volume)
			{
				tagIndex000001.Amount += tempIndex.Amount;
				tagIndex000001.Volume += tempIndex.Volume;

				szFdId[5] = '3';
				if( Global_Process.GetShIndexTable().GetCodeData( szFdId, &tempIndex ) )
				{
					tagIndex000001.Amount += tempIndex.Amount;
					tagIndex000001.Volume += tempIndex.Volume;
				}

			}
		}
		Global_Process.GetShIndexTable().UpdateOne( &tagIndex000001 );
	}*/

	/*����г���Ϣͷ*/
/*	Global_Process.GetShMarket().GetMarketHead( &tempHead );

	tempHead.MarketStatus = Global_Process.GetWorkStatus() == STATUS_TRANSPT ? 1 : 0;
	tempHead.Date = m_OldDay;
	tempHead.Time = m_OldTime;
	tempHead.WareCount = Global_Process.GetShNameTable().GetCount();
	Global_Process.GetShMarket().SetMarketHead( &tempHead );

	Global_Process.StatKind( KindNum, sizeof(KindNum)/sizeof(KindNum[0]) );

	//����У����
	Global_Process.GetShMarket().GetMarketHead( &tempHead );
	Global_Process.CalCheckCode( tempHead.CheckCode );
	Global_Process.GetShMarket().SetMarketHead( &tempHead );
	memcpy( m_OldMd5, tempHead.CheckCode, 16 );*/

	return 0;
}

int Quotation::Initialize()
{
	if( GetWorkStatus() == ET_SS_UNACTIVE )
	{
		int					nErrCode = -1;

		QuoCollector::GetCollector()->OnLog( TLV_INFO, "Quotation::Initialize() : ............ Session Activating............" );

		Release();

		if( (nErrCode=m_oMktdt00File.ChkOpen( Configuration::GetConfig().GetMktdt00FilePath() )) < 0 )
		{
			QuoCollector::GetCollector()->OnLog( TLV_WARN, "Quotation::Initialize() : failed 2 open mktdt00.txt, errorcode=%d", nErrCode );
			return -1;
		}

		GetMKTDTNowDateTime( m_nQuoDate, m_nQuoTime );
		QuoCollector::GetCollector()->OnLog( TLV_INFO, "Quotation::Initialize() : ���γ�ʼ��,ϵͳʹ��%s��[%d %d]�������", "MKTDT00", m_nQuoDate, m_nQuoTime );

		if( ( nErrCode = PrepareCodesTable() ) < 0 )
		{
			QuoCollector::GetCollector()->OnLog( TLV_WARN, "Quotation::Initialize() : Ԥװ��dbf����[Err:%d]\n", nErrCode );
			return nErrCode;
		}

		if( ( nErrCode = BuildImageData() ) < 0 )
		{
			QuoCollector::GetCollector()->OnLog( TLV_WARN, "Quotation::Initialize() : Ԥװ��dbf����[Err:%d]\n", nErrCode );
			return nErrCode;
		}

		if( ( nErrCode = SimpleTask::Activate() ) < 0 )
		{
			QuoCollector::GetCollector()->OnLog( TLV_WARN, "Quotation::Initialize() : failed 2 activate quotation file scanner, errorcode=%d", nErrCode );
			return nErrCode;
		}

		m_oWorkStatus = ET_SS_DISCONNECTED;				///< ����Quotation�Ự��״̬
		QuoCollector::GetCollector()->OnLog( TLV_INFO, "Quotation::Initialize() : ............ Quotation Activated!.............." );
	}

	return 0;
}

int Quotation::Release()
{
	m_nQuoDate = 0;
	m_nQuoTime = 0;
	m_mapMkDetail.clear();
	m_mapCode2Type.clear();
	::memset( &m_stKindMask, 0, sizeof(m_stKindMask) );
	m_uchIdx = 0;		//		"��ָ֤��"
	m_uchAG = 0;		//"��֤����"
	m_uchBG = 0;		//"��֤�¹�"
	m_uchJJ = 0;		//"��֤����"
	m_uchZQ = 0;		//"��֤ծȯ"
	m_uchZZ = 0;	//"��֤תծ"
	m_uchHG = 0;	//	"��֤�ع�"
	m_uchETF = 0;	//"��֤ETF"
	m_uchJJT = 0;	//	"����ͨ"
	m_uchQZ = 0;		//"��֤Ȩ֤"
	m_uchYX = 0;		//"��֤����"
	m_uchQT = 0;	//		"��֤����"
	m_uchKFJJ = 0;	//	"���Ż���"
	m_uchFXJS = 0;//		"���վ�ʾ"
	m_uchSZTS = 0;//		"��֤����"
	m_uchFJZS = 0;//		"����ָ��"

	if( m_oMktdt00File.IsOpened() )
	{
		QuoCollector::GetCollector()->OnLog( TLV_INFO, "Quotation::Release() : ............ Destroying .............." );

		m_oFjyFile.Close();
		m_oMktdt00File.Close();
		m_oWorkStatus = ET_SS_UNACTIVE;	///< ����Quotation�Ự��״̬

		QuoCollector::GetCollector()->OnLog( TLV_INFO, "Quotation::Release() : ............ Destroyed! .............." );
	}

	return 0;
}

int Quotation::ScanFile()
{
	int									nErrCode = 0;
//	tagSHMem_MarketHead					tempHead;
	tagSHL1ReferenceData_LF100			tempName = { 0 };
	tagSHL1SnapData_LF102				tempStockLF = { 0 };
	tagSHL1SnapData_HF103				tempStockHF = { 0 };
	tagSHL1SnapBuySell_HF104			tempStockBS = { 0 };
	tagSHL1SnapData_LF102				tempIndexLF000001 = { 0 };
	tagSHL1SnapData_HF103				tempIndexHF000001 = { 0 };
	unsigned int						ushRecCount = m_oMktdt00File.GetRecCount();

	for( int i = 0; i < ushRecCount; ++i )
	{
		nErrCode = GetShNameTableData( i, &tempName );
		if( nErrCode < 0 )
		{
			QuoCollector::GetCollector()->OnLog( TLV_WARN, "Quotation::ScanFile() : <ScanFile>GetScanNameTableData(%d)[Err:%d]\n", i, nErrCode );
			continue;
		}

		if( tempName.Kind ==  GetIdxType() )
		{
			memset(&tempStockLF, 0, sizeof(tempStockLF));
			memset(&tempStockHF, 0, sizeof(tempStockHF));
			memset(&tempIndexLF000001, 0, sizeof(tempIndexLF000001));
			//ָ��
			GetMemIndexData( i, &tempStockLF, &tempStockHF );
/*			if( memcmp(tempName.Code, "000001", 6) == 0 )
			{
				//add by liuqy 20091208 for ��������Ʊ����ġ�
				if(m_blActivateDBF)
					rv_SetJumpChkDiffPos(i);
				memcpy( &tagIndex000001, &tempIndex, sizeof(tempIndex) );
				//add by liuqy 20130515 for ����ָ������������
				Global_Process.Update000001NPush( &tempIndex );
			}
			else
			{
				//add by liuqy 20140421 for ��������500��˯��
				if(Global_Process.UpdateIndex( &tempIndex, true ))
					ushCntPushCount ++;
			}
*/
		}
		else
		{
			unsigned long	ulVoip = 0;
			GetMemStockData( i, &tempStockLF, &tempStockHF, &tempStockBS, ulVoip, false );
/*			tagSHMem_StockData			tempStock0 = {0};
			Global_Process.GetShStockTable().GetCodeData( tempStock.Code, &tempStock0 );
			//modify by liuqy 20130219 for ֻҪ���ձ仯������Ҫ�����ǵ�ͣ�۸�
			if( tempStock0.HighLimit == 0 || tempStock0.LowLimit == 0 || tempStock.Close != tempStock0.Close)
			{
				Global_Process.CalZDPrice( &tempName, tempStock.Close, (long *)&tempStock.HighLimit, (long *)&tempStock.LowLimit );
			}
			else
			{
				tempStock.HighLimit = tempStock0.HighLimit;
				tempStock.LowLimit = tempStock0.LowLimit;
			}
			//SHQZNAME   ��֤Ȩ֤
			if( Global_Process.GetShMarket().GetQZType()	== tempName.Type)
			{
				//add by liuqy 20091104 for ��ΪȨ֤��Ϣ(Type=9)��ETF(Type=7)ʱ����Ҫʹ��������Ʊ�����������㣬���������ݱ仯Ҳ��ı������ݣ������������ȽϾ��������µ������Ƿ��иı䣬����Щλ���е����ݶ�Ϊ��ͬ����Ҫ����
				if(m_blActivateDBF)
					rv_SetJumpChkDiffPos(i);
				Global_Process.CalQzyj( &tempName, &tempStock, &tempStock.Voip );
			}
			//SHETFNAME ��֤ETF
			if( Global_Process.GetShMarket().GetETFType()== tempName.Type)
			{
				//modify by liuqy 20150401 for ��ɨ��MKTDT00ʱ����ȡ������������,��Ϊɨ��ʱ����ȥ��ȡ�������ɵ�����
				if(m_blActivateDBF)
				{

					//add by liuqy 20091104 for ��ΪȨ֤��Ϣ(Type=9)��ETF(Type=7)ʱ����Ҫʹ��������Ʊ�����������㣬���������ݱ仯Ҳ��ı������ݣ������������ȽϾ��������µ������Ƿ��иı䣬����Щλ���е����ݶ�Ϊ��ͬ����Ҫ����
					if(m_blActivateDBF)
						rv_SetJumpChkDiffPos(i);
					if(!Global_Process.GetEtfMoin( &tempName, &tempStock.Voip ))
					{
						tempStock.Voip = ulVoip;
					}
				}
				else
					tempStock.Voip = ulVoip;
			}
			//ծȯתծ���ǵ�ͣ,����Ϊ0
			//SHZZNAME ��֤תծ
			//SHZQNAME ��֤ծȯ

			if( Global_Process.GetShMarket().GetZZType()== tempName.Type ||
				Global_Process.GetShMarket().GetZQType()	== tempName.Type )
			{
				tempStock.HighLimit = 0;
				tempStock.LowLimit = 0;
			}
			//end add by liuqy 
			//add by liuqy 20140421 for ��������500��˯��
			if(Global_Process.UpdateStock( &tempStock , true))
				ushCntPushCount ++;
		}*/
	}
/*
	//000001���������Ϊ000002,000003,000011�ĺ�
	char szFdId[6] = { '0', '0', '0', '0', '0', '1' };
	if( Global_Process.GetShIndexTable().GetCodeData( szFdId, &tagIndex000001 ) )// and this line by huq 20130328
	{
		tagIndex000001.Amount = 0;
		tagIndex000001.Volume = 0;
		szFdId[5] = '2';
		if( Global_Process.GetShIndexTable().GetCodeData( szFdId, &tempIndex ) )
		{
			//add by liuqy 20130502 ���A�ɵ���Ϊ0ʱ����������
			if(0 < tempIndex.Volume)
			{
				tagIndex000001.Amount += tempIndex.Amount;
				tagIndex000001.Volume += tempIndex.Volume;
				szFdId[5] = '3';
				if( Global_Process.GetShIndexTable().GetCodeData( szFdId, &tempIndex ) )
				{
					tagIndex000001.Amount += tempIndex.Amount;
					tagIndex000001.Volume += tempIndex.Volume;
				}

			}
		}

		Global_Process.UpdateIndex( &tagIndex000001, true );
	}
*/
	}

	return 0;
}

int Quotation::Execute()
{
	QuoCollector::GetCollector()->OnLog( TLV_INFO, "Quotation::Execute() : ............ Enter .............." );

	while( true == SimpleTask::IsAlive() )
	{
		SimpleTask::Sleep( 1000*1 );

		ScanFile();
	}

	QuoCollector::GetCollector()->OnLog( TLV_INFO, "Quotation::Execute() : ............ Leave .............." );

	return 0;
}

void Quotation::SendLoginRequest()
{
	QuoCollector::GetCollector()->OnLog( TLV_INFO, "Quotation::SendLoginRequest() : sending hq login message" );
/*
	CThostFtdcReqUserLoginField	reqUserLogin = { 0 };

	strcpy( reqUserLogin.UserProductInfo,"�Ϻ�Ǭ¡�߿Ƽ����޹�˾" );
	strcpy( reqUserLogin.TradingDay, m_pCTPApi->GetTradingDay() );
	memcpy( reqUserLogin.Password, Configuration::GetConfig().GetHQConfList().m_sPswd.c_str(), Configuration::GetConfig().GetHQConfList().m_sPswd.length() );
	memcpy( reqUserLogin.UserID, Configuration::GetConfig().GetHQConfList().m_sUID.c_str(), Configuration::GetConfig().GetHQConfList().m_sUID.length() );
	memcpy( reqUserLogin.BrokerID, Configuration::GetConfig().GetHQConfList().m_sParticipant.c_str(), Configuration::GetConfig().GetHQConfList().m_sParticipant.length() );

	if( 0 == m_pCTPApi->ReqUserLogin( &reqUserLogin, 0 ) )
	{
		QuoCollector::GetCollector()->OnLog( TLV_INFO, "Quotation::SendLoginRequest() : login message sended!" );
	}
	else
	{
		QuoCollector::GetCollector()->OnLog( TLV_ERROR, "Quotation::SendLoginRequest() : failed 2 send login message" );
	}
*/
}

void Quotation::FlushQuotation( char* pQuotationData, bool bInitialize )
{
/*	double							dRate = 1.;				///< �Ŵ���
	int								nSerial = 0;			///< ��Ʒ����������ֵ
	tagDLReferenceData_LF1003		tagName = { 0 };		///< ��Ʒ������Ϣ�ṹ
	tagDLSnapData_HF1005			tagSnapHF = { 0 };		///< �����������
	tagDLSnapData_LF1004			tagSnapLF = { 0 };		///< �����������
	tagDLSnapBuySell_HF1006			tagSnapBS = { 0 };		///< ��λ��Ϣ
	tagDLMarketStatus_HF1007		tagStatus = { 0 };		///< �г�״̬��Ϣ
	unsigned int					nSnapTradingDate = 0;	///< ���ս�������

	::strncpy( tagName.Code, pQuotationData->InstrumentID, sizeof(tagName.Code) );
	::memcpy( tagSnapHF.Code, pQuotationData->InstrumentID, sizeof(tagSnapHF.Code) );
	::memcpy( tagSnapLF.Code, pQuotationData->InstrumentID, sizeof(tagSnapLF.Code) );
	::memcpy( tagSnapBS.Code, pQuotationData->InstrumentID, sizeof(tagSnapBS.Code) );

	if( (nSerial=QuoCollector::GetCollector()->OnQuery( 1003, (char*)&tagName, sizeof(tagName) )) <= 0 )
	{
		return;
	}

	dRate = CTPQuoImage::GetRate( tagName.Kind );

	if( true == bInitialize ) {	///< ��ʼ������
		tagSnapLF.PreOpenInterest = pQuotationData->PreOpenInterest*dRate+0.5;
	}
	if( pQuotationData->UpperLimitPrice > 0 ) {
		tagSnapLF.UpperPrice = pQuotationData->UpperLimitPrice*dRate+0.5;
	}
	if( pQuotationData->LowerLimitPrice > 0 ) {
		tagSnapLF.LowerPrice = pQuotationData->LowerLimitPrice*dRate+0.5;
	}

	tagSnapLF.Open = pQuotationData->OpenPrice*dRate+0.5;
	tagSnapLF.Close = pQuotationData->ClosePrice*dRate+0.5;
	tagSnapLF.PreClose = pQuotationData->PreClosePrice*dRate+0.5;
	tagSnapLF.PreSettlePrice = pQuotationData->PreSettlementPrice*dRate+0.5;
	tagSnapLF.SettlePrice = pQuotationData->SettlementPrice*dRate+0.5;
	tagSnapLF.PreOpenInterest = pQuotationData->PreOpenInterest;

	tagSnapHF.High = pQuotationData->HighestPrice*dRate+0.5;
	tagSnapHF.Low = pQuotationData->LowestPrice*dRate+0.5;
	tagSnapHF.Now = pQuotationData->LastPrice*dRate+0.5;
	tagSnapHF.Position = pQuotationData->OpenInterest;
	tagSnapHF.Volume = pQuotationData->Volume;

//	if( EV_MK_ZZ == eMkID )		///< ֣���г��ĳɽ�������⴦�� = ��� * ��Լ��λ
//		tagSnapHF.Amount = pQuotationData->Turnover * refNameTable.ContractMult;

	tagSnapHF.Amount = pQuotationData->Turnover;
	tagSnapBS.Buy[0].Price = pQuotationData->BidPrice1*dRate+0.5;
	tagSnapBS.Buy[0].Volume = pQuotationData->BidVolume1;
	tagSnapBS.Sell[0].Price = pQuotationData->AskPrice1*dRate+0.5;
	tagSnapBS.Sell[0].Volume = pQuotationData->AskVolume1;
	tagSnapBS.Buy[1].Price = pQuotationData->BidPrice2*dRate+0.5;
	tagSnapBS.Buy[1].Volume = pQuotationData->BidVolume2;
	tagSnapBS.Sell[1].Price = pQuotationData->AskPrice2*dRate+0.5;
	tagSnapBS.Sell[1].Volume = pQuotationData->AskVolume2;
	tagSnapBS.Buy[2].Price = pQuotationData->BidPrice3*dRate+0.5;
	tagSnapBS.Buy[2].Volume = pQuotationData->BidVolume3;
	tagSnapBS.Sell[2].Price = pQuotationData->AskPrice3*dRate+0.5;
	tagSnapBS.Sell[2].Volume = pQuotationData->AskVolume3;
	tagSnapBS.Buy[3].Price = pQuotationData->BidPrice4*dRate+0.5;
	tagSnapBS.Buy[3].Volume = pQuotationData->BidVolume4;
	tagSnapBS.Sell[3].Price = pQuotationData->AskPrice4*dRate+0.5;
	tagSnapBS.Sell[3].Volume = pQuotationData->AskVolume4;
	tagSnapBS.Buy[4].Price = pQuotationData->BidPrice5*dRate+0.5;
	tagSnapBS.Buy[4].Volume = pQuotationData->BidVolume5;
	tagSnapBS.Sell[4].Price = pQuotationData->AskPrice5*dRate+0.5;
	tagSnapBS.Sell[4].Volume = pQuotationData->AskVolume5;

	char	pszTmpDate[12] = { 0 };
	::memcpy( pszTmpDate, pQuotationData->UpdateTime, sizeof(TThostFtdcTimeType) );
	pszTmpDate[2] = 0;
	pszTmpDate[5] = 0;
	pszTmpDate[8] = 0;
	int		nSnapUpdateTime = atol(pszTmpDate);
	nSnapUpdateTime = nSnapUpdateTime*100+atol(&pszTmpDate[3]);
	nSnapUpdateTime = nSnapUpdateTime*100+atol(&pszTmpDate[6]);
	if( (nSnapTradingDate=::atol( pQuotationData->TradingDay )) >= 0 && nSnapUpdateTime > 0 )
	{	///< ��������+ʱ��
		::strcpy( tagStatus.Key, "mkstatus" );
		tagStatus.MarketStatus = 1;
		tagStatus.MarketDate = nSnapTradingDate;
		tagStatus.MarketTime = nSnapUpdateTime;
		QuoCollector::GetCollector()->OnData( 1007, (char*)&tagStatus, sizeof(tagStatus), false );
	}

	QuoCollector::GetCollector()->OnData( 1004, (char*)&tagSnapLF, sizeof(tagSnapLF), false );
	QuoCollector::GetCollector()->OnData( 1005, (char*)&tagSnapHF, sizeof(tagSnapHF), false );
	QuoCollector::GetCollector()->OnData( 1006, (char*)&tagSnapBS, sizeof(tagSnapBS), false );*/
}







