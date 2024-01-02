#pragma once

#include <BMSInterfaceLib/Inc/BMSInterface.hpp>
#include <DataStoreLib/Inc/DataStore.hpp>
#include <UtilsLib/Inc/ADCManager.hpp>
#include <UtilsLib/Inc/Logger.hpp>
#include <UtilsLib/Inc/CANManager.hpp>
#include <MainLib/Inc/setttings.hpp>
#include <MCUInterfaceLib/Inc/MCUErrorManager.hpp>
#include <MCUInterfaceLib/Inc/MCUInterface.hpp>

#include <stm32f1xx.h>

namespace VehicleControlUnit { namespace MainLib {

class Main
{

public:
	Main(CAN_HandleTypeDef& canHandler, ADC_HandleTypeDef& adcHandler) :
		mLogger{Settings::spamLoggingEnabled,
				Settings::infoLoggingEnabled,
				Settings::errorLoggingEnabled},
		mDataStore(),
		mADCManager(mLogger, adcHandler, "ADC1", Settings::ADCDMABufferLength),
		mCanManagerForBMSAndMCU(mLogger, canHandler, "CAN1", mBMSInterface, mMCUInterface),
		mBMSInterface(mLogger, mDataStore),
		mMCUInterface(mLogger, mDataStore, mCanManagerForBMSAndMCU),
		mMCUErrorManager(mLogger, mDataStore, Settings::implausibleThresholdInterval)
	{}

	void Setup();
	void Loop();

	void CANMessageReceiveHandlerFIFO0(const CAN_RxHeaderTypeDef& header, const uint8_t message[8]);

private:
	UtilsLib::Logger mLogger;
	DataStoreLib::DataStore mDataStore;

	UtilsLib::ADCManager mADCManager;
	UtilsLib::CANManangerForBMSAndMCU mCanManagerForBMSAndMCU;

	BMSInterfaceLib::BMSInterface mBMSInterface;
	MCUInterfaceLib::MCUInterface mMCUInterface;
	MCUInterfaceLib::MCUErrorManager mMCUErrorManager;


};

}} // namespace VehicleControlUnit::MainLib


