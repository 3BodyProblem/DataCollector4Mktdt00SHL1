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


#define		SHZSNAME		"��ָ֤��"
#define		SHAGNAME		"��֤����"
#define		SHBGNAME		"��֤�¹�"
#define		SHJJNAME		"��֤����"
#define		SHZQNAME		"��֤ծȯ"
#define		SHZZNAME		"��֤תծ"
#define		SHHGNAME		"��֤�ع�"
#define		SHETFNAME		"��֤ETF"
#define		SHJJTNAME		"����ͨ"
#define		SHQZNAME		"��֤Ȩ֤"
#define		SHYXNAME		"��֤����"
#define		SHQTNAME		"��֤����"
#define		SHKFJJNAME		"���Ż���"
//add by liuqy 20121129 for ������֤�˼����մ���
#define		SHFXJSNAME		"���վ�ʾ"
#define		SHSZTSNAME		"��֤����"
#define		SHFJJSNAME		"����ָ��"//14
#define MAX_MASK_COUNT	10
#define MAX_KIND_MASK	32


typedef struct
{
    unsigned char uchCount;	//����
    unsigned char uchMask[MAX_MASK_COUNT][7];	//������
}TKindMask;


/**
 * @class			WorkStatus
 * @brief			����״̬����
 * @author			barry
 */
class WorkStatus
{
public:
	/**
	 * @brief				Ӧ״ֵ̬ӳ���״̬�ַ���
	 */
	static	std::string&	CastStatusStr( enum E_SS_Status eStatus );

public:
	/**
	 * @brief			����
	 * @param			eMkID			�г����
	 */
	WorkStatus();
	WorkStatus( const WorkStatus& refStatus );

	/**
	 * @brief			��ֵ����
						ÿ��ֵ�仯������¼��־
	 */
	WorkStatus&			operator= ( enum E_SS_Status eWorkStatus );

	/**
	 * @brief			����ת����
	 */
	operator			enum E_SS_Status();

private:
	CriticalObject		m_oLock;				///< �ٽ�������
	enum E_SS_Status	m_eWorkStatus;			///< ���鹤��״̬
};


/**
 * @class			Quotation
 * @brief			�Ự�������
 * @detail			��װ�������Ʒ�ڻ���Ȩ���г��ĳ�ʼ����������Ƶȷ���ķ���
 * @author			barry
 */
class Quotation : public SimpleTask
{
public:
	Quotation();
	~Quotation();

	/**
	 * @brief			�ͷ�ctp����ӿ�
	 */
	int					Release();

	/**
	 * @brief			��ʼ��ctp����ӿ�
	 * @return			>=0			�ɹ�
						<0			����
	 * @note			������������������У�ֻ������ʱ��ʵ�ĵ���һ��
	 */
	int					Initialize();

public:///< ������������
	/**
	 * @brief			��ȡ�Ự״̬��Ϣ
	 */
	WorkStatus&			GetWorkStatus();

	/**
	 * @brief			���͵�¼�����
	 */
    void				SendLoginRequest();

	/**
	 * @brief			ˢ���ݵ����ͻ���
	 * @param[in]		pQuotationData			�������ݽṹ
	 * @param[in]		bInitialize				��ʼ�����ݵı�ʶ
	 */
	void				FlushQuotation( char* pQuotationData, bool bInitialize );
	unsigned char	GetIdxType() { return m_uchIdx;};		//		"��ָ֤��"
	unsigned char	GetAGType() { return m_uchAG;};		//"��֤����"
	unsigned char	GetBGType() { return m_uchBG;};		//"��֤�¹�"
	unsigned char	GetJJType() { return m_uchJJ;};		//"��֤����"
	unsigned char	GetZQType() { return m_uchZQ;};		//"��֤ծȯ"
	unsigned char	GetZZType() { return m_uchZZ;};	//"��֤תծ"
	unsigned char	GetHGType() { return m_uchHG;};	//	"��֤�ع�"
	unsigned char	GetETFType() { return m_uchETF;};	//"��֤ETF"
	unsigned char	GetJJTType() { return m_uchJJT;};	//	"����ͨ"
	unsigned char	GetQZType() { return m_uchQZ;};		//"��֤Ȩ֤"
	unsigned char	GetYXType() { return m_uchYX;};		//"��֤����"
	unsigned char	GetQTType() { return m_uchQT;};	//		"��֤����"
	unsigned char	GetKFJJType() { return m_uchKFJJ;};	//	"���Ż���"
	unsigned char	GetFXJSType() { return m_uchFXJS;};//		"���վ�ʾ"
	unsigned char	GetSZTSType() { return m_uchSZTS;};//		"��֤����"
	unsigned char	GetFJZSType() { return m_uchFJZS;};//		"����ָ��"
protected:
	/**
	 * @brief			������(��ѭ��)
	 * @return			==0					�ɹ�
						!=0					ʧ��
	 */
	virtual int			Execute();

	int					ScanFile();
	int					GetMKTDTNowDateTime(unsigned int &out_ulDate, unsigned int &out_ulTime);

	/**
	 * @brief			���ȼ�˵��: MKTDT����>fjyYYYYMMDD.TXT�ǽ����Դ���>��ָ֤��>��ծԤ����>����ʽ����>����ָ��
	 */
	int					PrepareCodesTable();

	/**
	 * @brief			�����������
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
	CriticalObject			m_oLock;				///< �ٽ�������
	WorkStatus				m_oWorkStatus;			///< ����״̬
	CReadFjyFile			m_oFjyFile;				///< �ǽ����ļ�ɨ����
	CReadMktdtFile			m_oMktdt00File;			///< �����ļ�ɨ����
	MReadIdxDbfFile			m_oZzzsFile;			///< ��ָ֤��ɨ����
	std::set<std::string>	m_setCodes;				///< ��Ʒ���뼯��
private:
	unsigned int			m_nQuoDate;				///< ��������
	unsigned int			m_nQuoTime;				///< ����ʱ��
	TKindMask				m_stKindMask[MAX_KIND_MASK + 1];
	std::map<char,tagSHL1KindDetail_LF107>	m_mapMkDetail;
	std::map<std::string,char>				m_mapCode2Type;
private:
	unsigned char	m_uchIdx;		//		"��ָ֤��"
	unsigned char	m_uchAG;		//"��֤����"
	unsigned char	m_uchBG;		//"��֤�¹�"
	unsigned char	m_uchJJ;		//"��֤����"
	unsigned char	m_uchZQ;		//"��֤ծȯ"
	unsigned char	m_uchZZ;	//"��֤תծ"
	unsigned char	m_uchHG;	//	"��֤�ع�"
	unsigned char	m_uchETF;	//"��֤ETF"
	unsigned char	m_uchJJT;	//	"����ͨ"
	unsigned char	m_uchQZ;		//"��֤Ȩ֤"
	unsigned char	m_uchYX;		//"��֤����"
	unsigned char	m_uchQT;	//		"��֤����"
	unsigned char	m_uchKFJJ;	//	"���Ż���"
	unsigned char	m_uchFXJS;//		"���վ�ʾ"
	unsigned char	m_uchSZTS;//		"��֤����"
	unsigned char	m_uchFJZS;//		"����ָ��"
};




#endif






