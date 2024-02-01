#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_NO_EXCEPTIONS_BUT_WITH_ALL_ASSERTS

#include <MockLibraries.hpp>
#include <doctest.h>
#include <string>
#include <SensorInterfaceLib/Inc/SensorInterface.hpp>
#include <MCUInterfaceLib/Inc/MCUInterface.hpp>
#include <MainLib/Inc/settings.hpp>

namespace sensorLib = VehicleControlUnit::SensorInterfaceLib;
namespace mcuLib = VehicleControlUnit::MCUInterfaceLib;
namespace utilsLib = VehicleControlUnit::UtilsLib;
namespace dataLib = VehicleControlUnit::DataStoreLib;
namespace settings = VehicleControlUnit::MainLib::Settings;

TEST_CASE("SensorInterface ReadThrottleSignal Test Cases") {
    utilsLib::ADCManager adcManager;
    dataLib::DataStore dataStore;
    utilsLib::Logger logger;

    const uint8_t ThrottleSignalADCIndex0 = 0;
    const uint8_t ThrottleSignalADCIndex1 = 1;
    settings::SensorInterfaceParameters sensorInterfaceParams;
    sensorInterfaceParams.ThrottleMinPin0 = 500;
    sensorInterfaceParams.ThrottleMaxPin0 = 1000;
    sensorInterfaceParams.ThrottleMinPin1 = 1500;
    sensorInterfaceParams.ThrottleMaxPin1 = 2000;
    sensorInterfaceParams.MaxTorque = 500;
    sensorInterfaceParams.ThrottleSignalOutOfRangeThreshold = 20;
    sensorInterfaceParams.ThrottleSignalDeviationThreshold = 50;

    sensorLib::SensorInterface sensorInterface(logger, dataStore, adcManager, ThrottleSignalADCIndex0, ThrottleSignalADCIndex1, sensorInterfaceParams);

    SUBCASE("WHEN there exists a throttle sensor that gives value that is out of the threshold THEN error is set in datastore correctly")
    {
        SUBCASE("Pin 0 bottom out of range")
        {
            adcManager.buffer[ThrottleSignalADCIndex0] = 475;
            adcManager.buffer[ThrottleSignalADCIndex1] = 1500;

            uint16_t checkADC1 = 0;
            uint16_t checkADC2 = 0;
            adcManager.GetBufferByIndex(ThrottleSignalADCIndex0, checkADC1);
            adcManager.GetBufferByIndex(ThrottleSignalADCIndex1, checkADC2);
            REQUIRE(checkADC1 == 475);
            REQUIRE(checkADC2 == 1500);

            sensorInterface.ReadADC();
            CHECK(dataStore.mDrivingInputDataStore.GetError());
            CHECK(dataStore.mDrivingInputDataStore.GetTorque() == 0);
        }

        SUBCASE("Pin 1 bottom out of range")
        {
            adcManager.buffer[ThrottleSignalADCIndex0] = 500;
            adcManager.buffer[ThrottleSignalADCIndex1] = 1475;

            sensorInterface.ReadADC();
            CHECK(dataStore.mDrivingInputDataStore.GetError());
            CHECK(dataStore.mDrivingInputDataStore.GetTorque() == 0);
        }

        SUBCASE("Pin 0 top out of range")
        {
            adcManager.buffer[ThrottleSignalADCIndex0] = 1025;
            adcManager.buffer[ThrottleSignalADCIndex1] = 2000;

            sensorInterface.ReadADC();
            CHECK(dataStore.mDrivingInputDataStore.GetError());
            CHECK(dataStore.mDrivingInputDataStore.GetTorque() == 0);
        }

        SUBCASE("Pin 1 top out of range")
        {
            adcManager.buffer[ThrottleSignalADCIndex0] = 1000;
            adcManager.buffer[ThrottleSignalADCIndex1] = 2025;

            sensorInterface.ReadADC();
            CHECK(dataStore.mDrivingInputDataStore.GetError());
            CHECK(dataStore.mDrivingInputDataStore.GetTorque() == 0);
        }

        SUBCASE("Both out of range")
        {
            adcManager.buffer[ThrottleSignalADCIndex0] = 475;
            adcManager.buffer[ThrottleSignalADCIndex1] = 1475;

            sensorInterface.ReadADC();
            CHECK(dataStore.mDrivingInputDataStore.GetError());
            CHECK(dataStore.mDrivingInputDataStore.GetTorque() == 0);
        }
    }

    SUBCASE("WHEN throttle is slight out of range but inside the out of range threshold THEN the correct torque is set in datastore")
    {
        SUBCASE("zero percent")
        {
            adcManager.buffer[ThrottleSignalADCIndex0] = 485;
            adcManager.buffer[ThrottleSignalADCIndex1] = 1485;

            sensorInterface.ReadADC();
            CHECK(!dataStore.mDrivingInputDataStore.GetError());
            CHECK(dataStore.mDrivingInputDataStore.GetTorque() == 0);
        }

        SUBCASE("100 percent")
        {
            adcManager.buffer[ThrottleSignalADCIndex0] = 1015;
            adcManager.buffer[ThrottleSignalADCIndex1] = 2015;

            sensorInterface.ReadADC();
            CHECK(!dataStore.mDrivingInputDataStore.GetError());
            CHECK(dataStore.mDrivingInputDataStore.GetTorque() == sensorInterfaceParams.MaxTorque);
        }
    }

    SUBCASE("WHEN the out of range threshold is higher than ThrottleMin THEN data is stored to datastore correctly")
    {
        settings::SensorInterfaceParameters sensorInterfaceParamsSpecial;
        sensorInterfaceParamsSpecial.ThrottleMinPin0 = 10;
        sensorInterfaceParamsSpecial.ThrottleMaxPin0 = 510;
        sensorInterfaceParamsSpecial.ThrottleMinPin1 = 1500;
        sensorInterfaceParamsSpecial.ThrottleMaxPin1 = 2000;
        sensorInterfaceParamsSpecial.MaxTorque = 500;
        sensorInterfaceParamsSpecial.ThrottleSignalOutOfRangeThreshold = 20;
        sensorInterfaceParamsSpecial.ThrottleSignalDeviationThreshold = 50;
        sensorLib::SensorInterface sensorInterfaceSpecial(logger, dataStore, adcManager, ThrottleSignalADCIndex0, ThrottleSignalADCIndex1, sensorInterfaceParams);

        SUBCASE("WHEN only pin 1 out of bottom range THEN error is set")
        {
            adcManager.buffer[ThrottleSignalADCIndex0] = 0;
            adcManager.buffer[ThrottleSignalADCIndex1] = 1475;

            sensorInterfaceSpecial.ReadADC();
            CHECK(dataStore.mDrivingInputDataStore.GetError());
            CHECK(dataStore.mDrivingInputDataStore.GetTorque() == 0);
        }
    }

    SUBCASE("WHEN both throttle sensors are at a certain percentage THEN correct torque is set in datastore")
    {
        SUBCASE("50 percent")
        {
            adcManager.buffer[ThrottleSignalADCIndex0] = 750;
            adcManager.buffer[ThrottleSignalADCIndex1] = 1750;

            sensorInterface.ReadADC();
            CHECK(!dataStore.mDrivingInputDataStore.GetError());
            CHECK(dataStore.mDrivingInputDataStore.GetTorque() == sensorInterfaceParams.MaxTorque/2);
        }

        SUBCASE("25 percent")
        {
            adcManager.buffer[ThrottleSignalADCIndex0] = 625;
            adcManager.buffer[ThrottleSignalADCIndex1] = 1625;

            sensorInterface.ReadADC();
            CHECK(!dataStore.mDrivingInputDataStore.GetError());
            CHECK(dataStore.mDrivingInputDataStore.GetTorque() == 125);
        }
        
        SUBCASE("75 percent")
        {
            adcManager.buffer[ThrottleSignalADCIndex0] = 875;
            adcManager.buffer[ThrottleSignalADCIndex1] = 1875;

            sensorInterface.ReadADC();
            CHECK(!dataStore.mDrivingInputDataStore.GetError());
            CHECK(dataStore.mDrivingInputDataStore.GetTorque() == 375);
        }
    }

    SUBCASE("WHEN the throttle sensors has less than 10 percent deviation THEN the one with lower value is set as torque in datastore")
    {
        adcManager.buffer[ThrottleSignalADCIndex0] = 740; // Torque = 240
        adcManager.buffer[ThrottleSignalADCIndex1] = 1760; // Torque = 260

        sensorInterface.ReadADC();
        CHECK(!dataStore.mDrivingInputDataStore.GetError());
        CHECK(dataStore.mDrivingInputDataStore.GetTorque() == 240);
    }

    SUBCASE("WHEN the throttle sensors has more than 10 percent deviation THEN error is set in datastore")
    {
        adcManager.buffer[ThrottleSignalADCIndex0] = 700; // Torque = 200
        adcManager.buffer[ThrottleSignalADCIndex1] = 1755; // Torque = 255

        sensorInterface.ReadADC();
        CHECK(dataStore.mDrivingInputDataStore.GetError());
        CHECK(dataStore.mDrivingInputDataStore.GetTorque() == 0); 
    }
}

TEST_CASE("MCUInterface driving input")
{
    utilsLib::CANManager canManager;
    dataLib::DataStore dataStore;
    utilsLib::Logger logger;
    settings::MCUInterfaceParameters mcuInterfaceParams;

    mcuLib::MCUInterface mcuInterface(logger, dataStore, canManager, mcuInterfaceParams);

    SUBCASE("WHEN persisted implausibility is set in data store THEN error command message is sent via CAN Manager")
    {

    }
}