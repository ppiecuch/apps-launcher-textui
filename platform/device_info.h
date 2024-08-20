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
    int getBatteryPercentage() = 0;    // 0 - 100
    int getSystemLoadPercentage() = 0; // 0 -100
    bool getCpuInfo(int *cpu_num) = 0;
};

#endif // DEVICE_INFO_H
