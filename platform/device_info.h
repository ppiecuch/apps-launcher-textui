#ifndef DEVICE_INFO_H
#define DEVICE_INFO_H

enum BatteryStatus {
    BSTATUS_UNKNOWN,
    BSTATUS_CHARGING,
    BSTATUS_DISCHARGING,
    BSTATUS_NOTCHARGING,
};

class DeviceInfo {
public:
    virtual const char *getPlatformName() = 0;
    virtual const char *getPlatformDescription() = 0;
    virtual const char *getKernelInfo() = 0;

    virtual bool getBatteryStatus(BatteryStatus &ret) = 0;
    virtual bool getBatteryPercentage(int &ret) = 0;    // 0 - 100

    virtual bool getUptime(float &ret) = 0; // seconds
    virtual bool getMemoryInfo(long &total, long &available, long &free) = 0; // bytes
    virtual bool getMemoryUsagePercentage(int &ret) = 0; // 0 - 100
    virtual bool getNumProcs(long &ret) = 0;
    virtual bool getSystemLoadPercentage(int &ret) = 0; // 0 - 100
    virtual bool getCpuInfo(int *cpu_num) = 0;
};

DeviceInfo *buildDeviceInfo();

#endif // DEVICE_INFO_H
