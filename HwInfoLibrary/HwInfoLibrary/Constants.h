#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <windows.h>
#include <cstdint>
#include <string>

// Name of the file mapping object that needs to be opened using OpenFileMapping Function:
const static TCHAR HWiNFO_SENSORS_MAP_FILE_NAME[] = TEXT("Global\\HWiNFO_SENS_SM2");
const static TCHAR HWiNFO_SENSORS_MAP_FILE_NAME_REMOTE[] = TEXT("Global\\HWiNFO_SENS_SM2_REMOTE_");

// Name of the global mutex which is acquired when accessing the Shared Memory space. Release as quick as possible !
const static std::string HWiNFO_SENSORS_SM2_MUTEX = "Global\\HWiNFO_SM2_MUTEX";

const static uint32_t HWiNFO_SENSORS_STRING_LEN = 128;
const static uint32_t HWiNFO_UNIT_STRING_LEN = 16;

#endif