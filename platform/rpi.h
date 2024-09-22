#ifndef RPI_H
#define RPI_H

#include "device_info.h"

#include <stdint.h>
#include <stdlib.h>

size_t getDeviceSummary(char *tempstring, size_t tempstring_size);

const char *getDeviceBaseModel();

bool hwGetSysInfo(float &cpuload, uint64_t &totalram, uint64_t &procs, uint64_t &uptime);

float hwGetTemperatureInfo(char *tempstring);
float hwGetCoreTemperature();

bool hwReboot();
bool hwPowerOff();

class RPiDeviceInfo : public DeviceInfo {
    std::string pname, pdescr;
public:
    const char *getPlatformName() { return pname.c_str(); }
    const char *getPlatformDescription() { return pdescr.c_str(); }
    const char *getKernelInfo() { }

    bool getBatteryStatus(BatteryStatus &ret) { return false; }
    bool getBatteryPercentage(int &ret) { return false; }

    bool getUptime(float &ret) { return false; }
    bool getMemoryInfo(long &total, long &available, long &free) { return false; }
    bool getMemoryUsagePercentage(int &ret) { return false; }
    bool getSystemLoadPercentage(int &ret) { return false; }
    bool getCpuInfo(int *cpu_num) { return false; }

    RPiDeviceInfo() {
        pdescr = f_ssprintf("Generic %s %s", get_is64_bit() ? "64b" : "32b", get_full_name().c_str());
        pname = "Generic Linux";
    }
};

#endif // RPI_H
