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
    virtual int getBatteryPercentage() = 0;    // 0 - 100
    virtual int getSystemLoadPercentage() = 0; // 0 -100
    virtual bool getCpuInfo(int *cpu_num) = 0;
};

#endif // DEVICE_INFO_H
