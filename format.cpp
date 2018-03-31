struct CFfexFtdcOrderField
{
	///交易日，未使用的保留字段，其值为空
  ///char[9]
	TFfexFtdcDateType	TradingDay;
	///结算组代码，未使用的保留字段，其值为空
  ///char[9]
	TFfexFtdcSettlementGroupIDType	SettlementGroupID;
	///结算编号，未使用的保留字段，其值为空
  ///int
	TFfexFtdcSettlementIDType	SettlementID;
	///报单编号
  ///char[13]
	TFfexFtdcOrderSysIDType	OrderSysID;
	///会员代码
  ///char[11]
	TFfexFtdcParticipantIDType	ParticipantID;
	///客户代码
  ///char[11]
	TFfexFtdcClientIDType	ClientID;
	///交易用户代码
  ///char[16]
	TFfexFtdcUserIDType	UserID;
	///合约代码
  ///char[31]
	TFfexFtdcInstrumentIDType	InstrumentID;
	///报单价格条件
  ///char
	TFfexFtdcOrderPriceTypeType	OrderPriceType;
	///买卖方向
  ///char
	TFfexFtdcDirectionType	Direction;
	///组合开平标志
  ///char[5]
	TFfexFtdcCombOffsetFlagType	CombOffsetFlag;
	///组合投机套保标志
  ///char[5]
	TFfexFtdcCombHedgeFlagType	CombHedgeFlag;
	///价格
  ///double
	TFfexFtdcPriceType	LimitPrice;
	///数量
  ///int
	TFfexFtdcVolumeType	VolumeTotalOriginal;
	///有效期类型
  ///char
	TFfexFtdcTimeConditionType	TimeCondition;
	///GTD日期
  ///char[9]
	TFfexFtdcDateType	GTDDate;
	///成交量类型
  ///char
	TFfexFtdcVolumeConditionType	VolumeCondition;
	///最小成交量
  ///int
	TFfexFtdcVolumeType	MinVolume;
	///触发条件
  ///char
	TFfexFtdcContingentConditionType	ContingentCondition;
	///止损价
  ///double
	TFfexFtdcPriceType	StopPrice;
	///强平原因
  ///char
	TFfexFtdcForceCloseReasonType	ForceCloseReason;
	///本地报单编号
  ///char[13]
	TFfexFtdcOrderLocalIDType	OrderLocalID;
	///自动挂起标志
  ///int
	TFfexFtdcBoolType	IsAutoSuspend;
	///报单来源，未使用的保留字段，其值为空
  ///char
	TFfexFtdcOrderSourceType	OrderSource;
	///报单状态
  ///char
	TFfexFtdcOrderStatusType	OrderStatus;
	///报单类型，未使用的保留字段，其值为空
  ///char
	TFfexFtdcOrderTypeType	OrderType;
	///今成交数量，未使用的保留字段，其值为空
  ///int
	TFfexFtdcVolumeType	VolumeTraded;
	///剩余数量
  ///int
	TFfexFtdcVolumeType	VolumeTotal;
	///报单日期
  ///char[9]
	TFfexFtdcDateType	InsertDate;
	///插入时间，未使用的保留字段，其值为空
  ///char[9]
	TFfexFtdcTimeType	InsertTime;
	///激活时间，未使用的保留字段，其值为空
  ///char[9]
	TFfexFtdcTimeType	ActiveTime;
	///挂起时间，未使用的保留字段，其值为空
  ///char[9]
	TFfexFtdcTimeType	SuspendTime;
	///最后修改时间
  ///char[9]
	TFfexFtdcTimeType	UpdateTime;
	///撤销时间，未使用的保留字段，其值为空
  ///char[9]
	TFfexFtdcTimeType	CancelTime;
	///最后修改交易用户代码
  ///char[16]
	TFfexFtdcUserIDType	ActiveUserID;
	///优先权，未使用的保留字段，其值为空
  ///int
	TFfexFtdcPriorityType	Priority;
	///按时间排队的序号，未使用的保留字段，其值为空
  ///int
	TFfexFtdcTimeSortIDType	TimeSortID;
  ///结算会员编号，未使用的保留字段，其值为空
  ///char[11]
  TFfexFtdcParticipantIDType	ClearingPartID;
	///业务单元，未使用的保留字段，其值为空
	TFfexFtdcBusinessUnitType	BusinessUnit;
};
