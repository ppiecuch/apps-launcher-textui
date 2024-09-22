#include "device_info.h"

#ifdef __rpi__
# include "rpi.h"
#endif

#include "linux.h"
#include "support.h"

#include <string>
#include <vector>

class GenericDeviceInfo : public DeviceInfo {
    std::string pname, pdescr;
public:
    const char *getPlatformName() { return pname.c_str(); }
    const char *getPlatformDescription() { return pdescr.c_str(); }
    const char *getKernelInfo() { return get_kernel().c_str(); }

    bool getBatteryStatus(BatteryStatus &ret) { return false; }
    bool getBatteryPercentage(int &ret) { return false; }

    bool getUptime(float &ret) { return false; }
    bool getMemoryInfo(long &total, long &available, long &free) {
        std::vector<long> info = parse_meminfo();
        if (info[0] == -1 && info[1] == -1 && info[2] == -1)
            return false;
        total = info[0];
        free = info[1];
        available = info[2];
        return true;
    }
    bool getMemoryUsagePercentage(int &ret) {
        const std::vector<long> &info = parse_meminfo();
        if (info[0] == -1 && info[2] == -1)
            return false;
        ret = (double(info[2]) / double(info[0])) * 100;
        return true;
    }
    bool getNumProcs(long &ret) { return false; }
    bool getSystemLoadPercentage(int &ret) { return false; }
    bool getCpuInfo(int *cpu_num) { return false; }

    GenericDeviceInfo() {
        pdescr = f_ssprintf("Generic %s %s", get_is64_bit() ? "64b" : "32b", get_full_name().c_str());
        pname = "Generic Linux";
    }
};

static DeviceInfo *_device  = nullptr;

DeviceInfo *buildDeviceInfo() {
    if (!_device) {
#ifdef __rpi__
        _device = new RPiDeviceInfo;
#else
        _device = new GenericDeviceInfo;
#endif
    }
    return _device;
}
