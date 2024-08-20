#ifndef RPI_H
#define RPI_H

#include "device_info.h"

#include <stdint.h>

bool hwGetSysInfo(float &cpuload, uint64_t &totalram, uint64_t &procs, uint64_t &uptime);
float hwGetTemperatureInfo(char *tempstring);

bool hwReboot();
bool hwPowerOff();
float hwGetCoreTemperature();

#endif // RPI_H
