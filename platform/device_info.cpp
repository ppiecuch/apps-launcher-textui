#include "device_info.h"

#ifdef __rpi__
# include "rpi.h"
#endif

#include "linux.h"
#include "support.h"

class GenericDeviceInfo : public DeviceInfo {
    std::string pname, pdescr;
public:
    const char *getPlatformName() { return pname.c_str(); }
    const char *getPlatformDescription() { return pdescr.c_str(); }

    BatteryStatus getBatteryStatus() { return BSTATUS_UNKNOWN; }
    int getBatteryPercentage() { return 0; }

    int getSystemLoadPercentage() { return 0; }
    bool getCpuInfo(int *cpu_num) { return false; }

    GenericDeviceInfo() {
        pdescr = f_ssprintf("Generic %s %s", get_is64_bit() ? "64b" : "32b", get_full_name().c_str());
        pname = "Generic Linux";
    }
};

static DeviceInfo *_device  = nullptr;

DeviceInfo *buildDeviceInfo() {
    if (!_device) {
        _device = new GenericDeviceInfo;
    }
    return _device;
}
