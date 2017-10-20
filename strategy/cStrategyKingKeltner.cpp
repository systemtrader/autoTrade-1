#include "cStrategyKingKeltner.h"


cStrategyKingKeltner::cStrategyKingKeltner(void)
{
}


cStrategyKingKeltner::~cStrategyKingKeltner(void)
{

}

void cStrategyKingKeltner::init(){
	if (m_close.size() == 0) {
		// Start Time 
		this->m_marketData->loadSeriesHistory(m_oneMinuteDataDir, m_startDate, m_endDate, m_open, m_high, m_low, m_close, m_volume);
	}
	this->m_pMdSpi->SubscribeMarketData(m_inst);// trade 1801

}

void cStrategyKingKeltner::unInit(){



}

void cStrategyKingKeltner::run(){
	if (this->m_marketData->GetMarketDataHandle(m_inst) && isTradeTime()) {
		CThostFtdcDepthMarketDataField lastData = this->m_marketData->GetMarketDataHandle(m_inst)->getLastMarketData();
		int tickMinute = cDateTime(cSystem::GetCurrentTimeBuffer().c_str()).Minute();
		// new Candle
		if (tickMinute != m_candleMinute) {
			if (m_candleMinute != -1) {
				//add
				m_open.push_back(m_lastOpen);
				m_high.push_back(m_lastHigh);
				m_low.push_back(m_lastLow);
				m_close.push_back(m_lastClose);
				m_volume.push_back(m_lastVolume);
				//erase
				m_open.erase(m_open.begin());
				m_high.erase(m_high.begin());
				m_low.erase(m_low.begin());
				m_close.erase(m_close.begin());
				m_volume.erase(m_volume.begin());
			}
			on5MBar();
			m_lastOpen = lastData.LastPrice;
			m_lastHigh = lastData.LastPrice;
			m_lastLow = lastData.LastPrice;
			m_lastClose = lastData.LastPrice;
			m_lastVolume = lastData.Volume;
			m_candleMinute = tickMinute;// update current candle Minute

		}
		else {
			m_lastHigh = max(m_lastHigh, lastData.LastPrice);
			m_lastLow = min(m_lastLow, lastData.LastPrice);
			m_lastClose = lastData.LastPrice;
			m_lastVolume += lastData.Volume;

		}
	}

}



void cStrategyKingKeltner::on5MBar(){
	// =================================================================  指标计算 =================================================
	int outBegIdx[100] = {};
	int outNBElement[100] = {};
	double outReal[100] = {};
	TA_RSI(m_close.size() - 14, m_close.size() - 1, &m_close[0], 14, outBegIdx, outNBElement, outReal);
	
	//=============================================================取消前面所有未成交单 ==============================================
	this->m_pTradeSpi->cancleMyPendingOrder();

	// ==============================================================日志输出========================================================
	double rsiValue = outReal[0];
	cout << cSystem::GetCurrentTimeBuffer() << " RSI: " << rsiValue << " " << m_lastOpen << " " << m_lastHigh << " " << m_lastLow << " " << m_lastClose << endl;
	// ===========================================================下单逻辑============================================================
	int longPos = this->m_pPositionC.get()->getHolding_long(m_inst);
	int shortPos = this->m_pPositionC.get()->getHolding_short(m_inst);

	int netPos = longPos - shortPos;

	if (netPos == 0) {
		if (rsiValue>90) {
			this->m_pTradeSpi->insertOrder(m_inst, DIRECTION::sell, OFFSETFLAG::open, 1, 0, 1);
		}
		if (rsiValue<10) {
			this->m_pTradeSpi->insertOrder(m_inst, DIRECTION::buy, OFFSETFLAG::open, 1, 0, 1);
		}
	}
	else if (netPos > 0) {
		if (rsiValue>50) {
			this->m_pTradeSpi->insertOrder(m_inst, DIRECTION::sell, OFFSETFLAG::close, 1, 0, 1);
		}
	}
	else if (netPos < 0) {
		if (rsiValue<50) {
			this->m_pTradeSpi->insertOrder(m_inst, DIRECTION::buy, OFFSETFLAG::close, 1, 0, 1);
		}
	}
}


bool cStrategyKingKeltner::isTradeTime() {
	DateTimeFormat s0900 = 900, s1015 = 1015, s1030 = 1030, s1130 = 1130, s1330 = 1330, s1500 = 1500, s2100 = 2100, s2330 = 2330;
	cDateTime nowDateTime = cDateTime(cSystem::GetCurrentTimeBuffer().c_str());
	DateTimeFormat hour = nowDateTime.Hour();
	DateTimeFormat min = nowDateTime.Minute();

	DateTimeFormat nowTime = hour * 100 + min;
	bool newState;
	if ((nowTime>s0900 && nowTime <s1015) ||
		(nowTime>s1030 && nowTime <s1130) ||
		(nowTime>s1330 && nowTime <s1500) ||
		(nowTime>s2100 && nowTime <s2330)
		)
	{

		newState = true;
	}
	else {

		newState = false;
	}
	//if(m_oldState && newState){
	//	cout << "tradeState open" << endl;
	//}else{
	//	cout << "tradeState open" << endl;

	//}
	return newState;

}

bool cStrategyKingKeltner::keltner(vector<double> inData, double kkLength, double kkDev, double& kkUp,double &kkDown) {
	double mid = 0, atr = 0;
	int outBegIdx_SMA[100] = {};
	int outNBElement_SMA[100] = {};
	double outReal_SMA[100] = {};

	int outBegIdx_ATR[100] = {};
	int outNBElement_ATR[100] = {};
	double outReal_ATR[100] = {};

	TA_SMA(inData.size() - kkLength, inData.size(), &m_close[0], kkLength, outBegIdx_SMA, outNBElement_SMA, outReal_SMA);
	
	TA_ATR(inData.size() - kkLength, inData.size(), &m_high[0], &m_low[0], &m_close[0], kkLength, outBegIdx_ATR, outNBElement_ATR, outReal_ATR);



}