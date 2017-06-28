#ifndef __CTP_QUOTATION_PROTOCAL_CTP_DL_H__
#define	__CTP_QUOTATION_PROTOCAL_CTP_DL_H__
#pragma pack(1)


typedef struct
{
	char						Code[20];					///< 合约代码
	char						Name[64];					///< 合约名称
	unsigned char				Kind;						///< 证券类型
	char						PreName[4];					//深交所前缀[仅深圳市场，上海市场保留]
	unsigned int				LotSize;					///< 一手等于几张合约
	char						UnderlyingCode[20];			///< 标的证券代码
	unsigned int				ContractMult;				///< 合约乘数
	unsigned int				XqPrice;					///< 行权价格[*放大倍数]
	unsigned int				StartDate;					///< 首个交易日(YYYYMMDD)
	unsigned int				EndDate;					///< 最后交易日(YYYYMMDD)
	unsigned int				XqDate;						///< 行权日(YYYYMM)
	unsigned int				DeliveryDate;				///< 交割日(YYYYMMDD)
	unsigned int				ExpireDate;					///< 到期日(YYYYMMDD)
	unsigned short				TypePeriodIdx;				///< 分类交易时间段位置
	unsigned char				EarlyNightFlag;             ///< 日盘or夜盘标志 1：日盘 2：夜盘 
	unsigned int				PriceTick;					///< 最小变动价位
} tagSHL1ReferenceData_LF100;


typedef struct
{
	char						Code[20];					//名称代码表
	unsigned short				MKind;						//币种
	unsigned char				CType;						//详细类别，委托使用的分类
	unsigned char				BType;						//所属类别，A，B股等
	unsigned char				SFlag;						//停牌标记，是否可交易，需要推送
	unsigned char				PLimit;						//涨、跌停是否有效
	unsigned int				Worth;						//每股面值，*放大倍率10000
	unsigned int				ExTts;						//交易单位
	unsigned int				MaxExVol;					//最大交易数量
	unsigned int				MinExVol;					//最小交易数量
} tagSHL1ReferenceExtension_LF101;


typedef struct
{
	char						Code[20];					///< 合约代码
	unsigned int				Open;						///< 开盘价[*放大倍数]
	unsigned int				Close;						///< 今收价[*放大倍数]
	unsigned int				High;						///< 当日涨停价格[*放大倍数], 0表示无限制
	unsigned int				Low;						///< 当日跌停价格[*放大倍数], 0表示无限制
} tagSHL1SnapData_LF102;


typedef struct
{
	char						Code[20];					///< 合约代码
	unsigned int				Now;						///< 最新价[*放大倍数]
	unsigned int				Voip;						///< 基金模拟净值、权证溢价[放大1000倍][ETF、深圳LOF、权证该值有效]
	double						Amount;						///< 总成交金额[元]
	unsigned __int64			Volume;						///< 总成交量[股/张]
} tagSHL1SnapData_HF103;


typedef struct
{
	unsigned int				Price;						///< 委托价格[* 放大倍数]
	unsigned __int64			Volume;						///< 委托量[股]
} tagBuySellItem;


typedef struct
{
	char						Code[20];					///< 合约代码
	tagBuySellItem				Buy[5];						///< 买五档
	tagBuySellItem				Sell[5];					///< 卖五档
} tagSHL1SnapBuySell_HF104;


typedef struct
{
	char						Key[32];					///< 索引键值
	unsigned int				MarketDate;					///< 市场日期
	unsigned int				MarketTime;					///< 市场时间
	unsigned char				MarketStatus;				///< 市场状态[0初始化 1行情中]
} tagSHL1MarketStatus_HF105;


typedef struct
{
	char						Key[32];					///< 索引键值
	unsigned int				MarketID;					///< 市场编号
	unsigned int				KindCount;					///< 类别数量
	unsigned int				WareCount;					///< 商品数量
	unsigned int				PeriodsCount;				///< 交易时段信息列表长度
	unsigned int				MarketPeriods[8][2];		///< 交易时段描述信息列表
} tagSHL1MarketInfo_LF106;


typedef struct
{
	char						Key[20];					///< 索引键值
	char						KindName[64];				///< 类别的名称
	unsigned int				PriceRate;					///< 价格放大倍数[10的多少次方]
	unsigned int				LotSize;					///< “手”比率
	unsigned int				WareCount;					///< 该分类的商品数量
} tagSHL1KindDetail_LF107;



#pragma pack()
#endif









