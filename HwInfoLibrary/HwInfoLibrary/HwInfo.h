#ifndef HWINFO_H
#define HWINFO_H

#include <Windows.h>
#include "Enums.h"
#include "Constants.h"

// ***************************************************************************************************************
//                                          HWiNFO Shared Memory Footprint
// ***************************************************************************************************************
//
//         |-----------------------------|-----------------------------------|-----------------------------------|
// Content |  HWiNFO_SENSORS_SHARED_MEM2 |  HWiNFO_SENSORS_SENSOR_ELEMENT[]  | HWiNFO_SENSORS_READING_ELEMENT[]  |
//         |-----------------------------|-----------------------------------|-----------------------------------|
// Pointer |<--0                         |<--dwOffsetOfSensorSection         |<--dwOffsetOfReadingSection        |
//         |-----------------------------|-----------------------------------|-----------------------------------|
// Size    |  dwOffsetOfSensorSection    |   dwSizeOfSensorElement           |    dwSizeOfReadingElement         |
//         |                             |      * dwNumSensorElement         |       * dwNumReadingElement       |
//         |-----------------------------|-----------------------------------|-----------------------------------|

#pragma pack(1)

struct HwInfoSensorElement
{
    DWORD dwSensorID;                                  // A unique Sensor ID
    DWORD dwSensorInst;                                // The instance of the sensor (together with dwSensorID forms a unique ID)
    char szSensorNameOrig[HWiNFO_SENSORS_STRING_LEN]; // Original sensor name
    char szSensorNameUser[HWiNFO_SENSORS_STRING_LEN]; // Sensor name displayed, which might have been renamed by user
};

struct HwInfoSensorReadingElement
{
    SensorReadingType tReading;                 // Type of sensor reading
    DWORD dwSensorIndex;                          // This is the index of sensor in the Sensors[] array to which this reading belongs to
    DWORD dwReadingID;                            // A unique ID of the reading within a particular sensor
    char szLabelOrig[HWiNFO_SENSORS_STRING_LEN]; // Original label (e.g. "Chassis2 Fan")
    char szLabelUser[HWiNFO_SENSORS_STRING_LEN]; // Label displayed, which might have been renamed by user
    char szUnit[HWiNFO_UNIT_STRING_LEN];          // e.g. "RPM"
    double Value;
    double ValueMin;
    double ValueMax;
    double ValueAvg;
};

struct HwInfoSensorsSharedMemory
{
    DWORD dwSignature;             // "HWiS" if active, 'DEAD' when inactive
    DWORD dwVersion;               // v1 current
    DWORD dwRevision;              // 0: Initial layout (HWiNFO ver <= 6.11)
                                    // 1: Added (HWiNFO v6.11-3917)
    __time64_t poll_time;          // last polling time

    // descriptors for the Sensors section
    DWORD dwOffsetOfSensorSection; // Offset of the Sensor section from beginning of HWiNFO_SENSORS_SHARED_MEM2
    DWORD dwSizeOfSensorElement;   // Size of each sensor element = sizeof( HWiNFO_SENSORS_SENSOR_ELEMENT )
    DWORD dwNumSensorElements;     // Number of sensor elements

    // descriptors for the Readings section
    DWORD dwOffsetOfReadingSection; // Offset of the Reading section from beginning of HWiNFO_SENSORS_SHARED_MEM2
    DWORD dwSizeOfReadingElement;   // Size of each Reading element = sizeof( HWiNFO_SENSORS_READING_ELEMENT )
    DWORD dwNumReadingElements;     // Number of Reading elements

    DWORD dwPollingPeriod;          // Current sensor polling period in HWiNFO. This variable is present since dwRevision=1 (HWiNFO v6.11) or later
};

#pragma pack()

inline HANDLE OpenHwInfoFileMapping()
{
    return OpenFileMapping(FILE_MAP_READ, FALSE, HWiNFO_SENSORS_MAP_FILE_NAME);
}

inline HwInfoSensorsSharedMemory* ReadSensorsSharedMemory(HANDLE handle)
{
    return (HwInfoSensorsSharedMemory*)MapViewOfFile(handle, FILE_MAP_READ, 0, 0, 0);
}

extern "C"
{
    __declspec(dllexport)HwInfoSensorElement* ReadAllSensors(DWORD& sizeOut)
    {
        HANDLE handle = OpenHwInfoFileMapping();
        HwInfoSensorsSharedMemory* hwInfoMemory = ReadSensorsSharedMemory(handle);
        if (!hwInfoMemory)
        {
            return nullptr;
        }

        HwInfoSensorElement* buffer = new HwInfoSensorElement[hwInfoMemory->dwNumSensorElements];

        // loop through all available sensors
        for (DWORD dwSensor = 0; dwSensor < hwInfoMemory->dwNumSensorElements; ++dwSensor)
        {
            DWORD offset = hwInfoMemory->dwOffsetOfSensorSection + hwInfoMemory->dwSizeOfSensorElement * dwSensor;
            HwInfoSensorElement* sensor = (HwInfoSensorElement*)((BYTE*)hwInfoMemory + offset);

            buffer[dwSensor] = *sensor;
        }

        // Set sizeOut param reference to let the caller know how many elements are stored in the buffer
        sizeOut = hwInfoMemory->dwNumSensorElements;

        // release memory
        UnmapViewOfFile(hwInfoMemory);
        CloseHandle(handle);

        return buffer;
    }

    __declspec(dllexport)HwInfoSensorReadingElement* ReadAllSensorReadings(DWORD& sizeOut)
    {
        HANDLE handle = OpenHwInfoFileMapping();
        HwInfoSensorsSharedMemory* hwInfoMemory = ReadSensorsSharedMemory(handle);
        if (!hwInfoMemory)
        {
            return nullptr;
        }

        HwInfoSensorReadingElement* buffer = new HwInfoSensorReadingElement[hwInfoMemory->dwNumReadingElements];

        // loop through all available readings
        for (DWORD dwReading = 0; dwReading < hwInfoMemory->dwNumReadingElements; ++dwReading)
        {
            DWORD offset = hwInfoMemory->dwOffsetOfReadingSection + hwInfoMemory->dwSizeOfReadingElement * dwReading;
            HwInfoSensorReadingElement* sensorReading = (HwInfoSensorReadingElement*)((BYTE*)hwInfoMemory + offset);

            buffer[dwReading] = *sensorReading;
        }

        // Set sizeOut param reference to let the caller know how many elements are stored in the buffer
        sizeOut = hwInfoMemory->dwNumReadingElements;

        // release memory
        UnmapViewOfFile(hwInfoMemory);
        CloseHandle(handle);

        return buffer;
    }

    __declspec(dllexport)void ReleasePtr(void* ptr) 
    {
        if (ptr) 
        {
            delete[] ptr;
        }
    }
}

#endif
