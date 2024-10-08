#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/reboot.h>

#include <string>
#include <vector>

#include "support.h"

// Reference:
// ----------
//  - https://github.com/vanvught/rpidmx512/blob/master/lib-hal/src/linux/hardware.cpp
//  - https://github.com/jonathanspw/fastfetch/blob/master/src/modules/host.c
//  - https://github.com/ve3wwg/raspberry_pi2/blob/master/librpi2/mtop.cpp
//  - https://github.com/epsilonrt/piduino/blob/master/utils/pinfo/main.cpp
//  * https://github.com/HeroCC/moos-ivp-cc/blob/master/src/uRaspiMon/RaspiMon.cpp
//  - https://github.com/markwkm/pg_top/blob/master/pg_top.c
//  * https://github.com/mrdotx/cinfo/blob/master/cinfo.c

// struct sysinfo {
// ----------------
//    long uptime;                            -- Seconds since boot
//    unsigned long loads[3];                 -- 1, 5, and 15 minute load averages
//    unsigned long totalram;                 -- Total usable main memory size
//    unsigned long freeram;                  -- Available memory size
//    unsigned long sharedram;                -- Amount of shared memory
//    unsigned long bufferram;                -- Memory used by buffers
//    unsigned long totalswap;                -- Total swap space size
//    unsigned long freeswap;                 -- swap space still available
//    unsigned short procs;                   -- Number of current processes
//    unsigned long totalhigh;                -- Total high memory size
//    unsigned long freehigh;                 -- Available high memory size
//    unsigned int mem_unit;                  -- Memory unit size in bytes
//    char _f[20-2*sizeof(long)-sizeof(int)]; -- Padding for libc5

static const char RASPBIAN_LED_INIT[] = "echo gpio | sudo tee /sys/class/leds/led0/trigger";
static const char RASPBIAN_LED_OFF[] = "echo 0 | sudo tee /sys/class/leds/led0/brightness";
static const char RASPBIAN_LED_ON[] = "echo 1 | sudo tee /sys/class/leds/led0/brightness";
static const char RASPBIAN_LED_HB[] = "echo heartbeat | sudo tee /sys/class/leds/led0/trigger";
static const char RASPBIAN_LED_FLASH[] = "echo timer | sudo tee /sys/class/leds/led0/trigger";

class VCGenCmd {

    std::string execVcgenCommand(std::string command, std::string vcPath="/usr/bin/vcgencmd");
    std::vector<std::string> getValue(const std::string &vcOutput);

    std::string hexStrToBinaryStr(const std::string& hex);
    const char* hexCharToBinary(char c);

public:
    bool sanityCheck();
    float getTemperature();
    float getVoltage();
    long getClockSpeed(std::string clock="arm");
    std::string getThrottleHex();
    std::string getThrottleBinary();
};

static std::string exec_cmd(const std::string &cmd, bool *err = NULL) {
    std::string output;
    output.resize(1024*10);
    bool ret = exec_cmd(cmd.c_str(), &output[0], output.size());
    if (err)
        *err = ret;
    return output;
}

/// RPi revisions decode

#define RPI_PERIPHERAL_BASE_UNKNOWN 0
#define RPI_BROADCOM_2835_PERIPHERAL_BASE 0x20000000
#define RPI_BROADCOM_2836_PERIPHERAL_BASE 0x3F000000
#define RPI_BROADCOM_2837_PERIPHERAL_BASE 0x3F000000
#define RPI_BROADCOM_2711_PERIPHERAL_BASE 0xFE000000
#define RPI_BROADCOM_2712_PERIPHERAL_BASE 0x00000010

typedef enum {
    RPI_MEMORY_UNKNOWN = -1,
    RPI_256MB = 256,
    RPI_512MB = 512,
    RPI_1024MB = 1024,
    RPI_2048MB = 2048,
    RPI_4096MB = 4096,
    RPI_8192MB = 8192,
} RASPBERRY_PI_MEMORY_T;

typedef enum {
    RPI_PROCESSOR_UNKNOWN = -1,
    RPI_BROADCOM_2835 = 2835,
    RPI_BROADCOM_2836 = 2836,
    RPI_BROADCOM_2837 = 2837,
    RPI_BROADCOM_2711 = 2711,
    RPI_BROADCOM_2712 = 2712,
} RASPBERRY_PI_PROCESSOR_T;

typedef enum {
    RPI_I2C_DEVICE_UNKNOWN = -1,
    RPI_I2C_0 = 0,
    RPI_I2C_1 = 1
} RASPBERRY_PI_I2C_DEVICE_T;

typedef enum {
    RPI_MODEL_UNKNOWN = -1,
    RPI_MODEL_A,
    RPI_MODEL_B,
    RPI_MODEL_A_PLUS,
    RPI_MODEL_B_PLUS,
    RPI_MODEL_B_PI_2,
    RPI_MODEL_ALPHA,
    RPI_COMPUTE_MODULE,
    RPI_MODEL_ZERO,
    RPI_MODEL_B_PI_3,
    RPI_COMPUTE_MODULE_3,
    RPI_MODEL_ZERO_W,
    RPI_MODEL_B_PI_3_PLUS,
    RPI_MODEL_A_PI_3_PLUS,
    RPI_COMPUTE_MODULE_3_PLUS,
    RPI_MODEL_B_PI_4,
    RPI_MODEL_400,
    RPI_COMPUTE_MODULE_4,
    RPI_MODEL_ZERO_2_W,
    RPI_COMPUTE_MODULE_4S,
    RPI_MODEL_PI_5,
} RASPBERRY_PI_MODEL_T;

typedef enum {
    RPI_MANUFACTURER_UNKNOWN = -1,
    RPI_MANUFACTURER_SONY_UK,
    RPI_MANUFACTURER_EGOMAN,
    RPI_MANUFACTURER_QISDA,
    RPI_MANUFACTURER_EMBEST,
    RPI_MANUFACTURER_SONY_JAPAN,
    RPI_MANUFACTURER_STADIUM,
} RASPBERRY_PI_MANUFACTURER_T;

//-------------------------------------------------------------------------

typedef struct {
    RASPBERRY_PI_MEMORY_T memory;
    RASPBERRY_PI_PROCESSOR_T processor;
    RASPBERRY_PI_I2C_DEVICE_T i2cDevice;
    RASPBERRY_PI_MODEL_T model;
    RASPBERRY_PI_MANUFACTURER_T manufacturer;
    int pcbRevision;
    int warrantyBit;
    int revisionNumber;
    uint32_t peripheralBase;
} RASPBERRY_PI_INFO_T;


// getRaspberryPiInformation()
//
// return - 0 - failed to get revision from /proc/cpuinfo
//          1 - found classic revision number
//          2 - found Pi 2 style revision number

int getRaspberryPiInformation(RASPBERRY_PI_INFO_T *info);
int getRaspberryPiInformationForRevision( int revision, RASPBERRY_PI_INFO_T *info);
int getRaspberryPiRevision(void);
const char *raspberryPiMemoryToString(RASPBERRY_PI_MEMORY_T memory);
const char *raspberryPiProcessorToString(RASPBERRY_PI_PROCESSOR_T processor);
const char *raspberryPiI2CDeviceToString(RASPBERRY_PI_I2C_DEVICE_T i2cDevice);
const char *raspberryPiModelToString(RASPBERRY_PI_MODEL_T model);
const char *raspberryPiManufacturerToString(RASPBERRY_PI_MANUFACTURER_T manufacturer);

/// System information

bool hwGetSysInfo(float &cpuload, uint64_t &totalram, uint64_t &procs, uint64_t &uptime) {
    struct sysinfo sys_info;
    if(sysinfo(&sys_info) != 0) {
        return false;
    }

    cpuload = ((float)sys_info.loads[0])/(1<<SI_LOAD_SHIFT); // cpu info
    totalram = sys_info.freeram / 1024 / 1024; // freeram
    procs = sys_info.procs; // processes
    uptime = sys_info.uptime; // uptime

    return true;
}

float hwGetCoreTemperature() {
    const char cmd[] = "vcgencmd measure_temp|egrep \"[0-9.]{4,}\" -o";
    char result[8];

    exec_cmd(cmd, result, sizeof(result));

    return atof(result);
}

// Returns a double with the temp in deg C and the temp in a text string
float hwGetTemperatureInfo(char *tempstring) {
    FILE *fin = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
    if (fin != NULL) {
        long millideg;
        fscanf(fin, "%ld", &millideg);

        float cputemp_degc = millideg / 1000.0;
        float cputemp_degf = ((cputemp_degc * (9.0/5.0)) + 32);

        if (tempstring != NULL) {
            sprintf(tempstring, "CPU temp is %3.3f deg C / %3.3f deg F", cputemp_degc, cputemp_degf);
        }
        fclose(fin);
        return(cputemp_degc);
    }
    return -1;
}

size_t getDeviceSummary(char *tempstring, size_t tempstring_size) {
    std::string output;

    output += "\n";
    output += exec_cmd("cat /sys/firmware/devicetree/base/model");
    output += "\n";
    output += "\n";
    output += exec_cmd("cat /etc/os-release | head -4");
    output += exec_cmd("\n");
    output += exec_cmd("uname -a");
    output += "\n";
    output += exec_cmd("cat /proc/cpuinfo | tail -3");
    output += "Throttled flag  : " + exec_cmd("vcgencmd get_throttled");
    output += "Camera          : " + exec_cmd("vcgencmd get_camera");

    if (tempstring && tempstring_size) {
        strncpy(tempstring, output.c_str(), tempstring_size);
    }

    return output.size();
}

bool hwReboot() {
    if(geteuid() == 0) {
        sync();

        if (reboot(RB_AUTOBOOT) == 0) {
            return true;
        }

        perror("Call to reboot(RB_AUTOBOOT) failed.\n");
    }
    printf("Only the superuser may call reboot(RB_AUTOBOOT).\n");
    return false;
}

bool hwPowerOff() {
    if(geteuid() == 0) {
        sync();

        if (reboot(RB_POWER_OFF) == 0) {
            return true;
        }

        perror("Call to reboot(RB_POWER_OFF) failed.\n");
        return false;
    }

    printf("Only the superuser may call reboot(RB_POWER_OFF).\n");
    return false;
}

char *hwGetCpuInfoRevision(char *revision) {
    FILE *fp;
    char buffer[1024];

    if ((fp = fopen("/sys/firmware/devicetree/base/model", "r")) == NULL)
        return NULL;

    char *r = fgets(buffer, sizeof(buffer) , fp);
    fclose(fp);

    if (!r) return NULL;

    if (strncmp(buffer, "Raspberry",9) != 0)
        return NULL;

    if ((fp = fopen("/proc/cpuinfo", "r")) == NULL)
        return NULL;

    while(!feof(fp)) {
        if (fgets(buffer, sizeof(buffer) , fp)) {
            sscanf(buffer, "Revision  : %s", revision);
        }
    }
    fclose(fp);

    return revision;
}

/// RPi revisions decode


//-------------------------------------------------------------------------
//
// The file /proc/cpuinfo contains a line such as:-
//
// Revision    : 0003
//
// that holds the revision number of the Raspberry Pi.
// Known revisions (prior to the Raspberry Pi 2) are:
//
//     +----------+---------+---------+--------+--------------+
//     | Revision |  Model  | PCB Rev | Memory | Manufacturer |
//     +----------+---------+---------+--------+--------------+
//     |   0000   |         |         |        |              |
//     |   0001   |         |         |        |              |
//     |   0002   |    B    |    1    | 256 MB |   Egoman     |
//     |   0003   |    B    |    1    | 256 MB |   Egoman     |
//     |   0004   |    B    |    2    | 256 MB |   Sony UK    |
//     |   0005   |    B    |    2    | 256 MB |   Qisda      |
//     |   0006   |    B    |    2    | 256 MB |   Egoman     |
//     |   0007   |    A    |    2    | 256 MB |   Egoman     |
//     |   0008   |    A    |    2    | 256 MB |   Sony UK    |
//     |   0009   |    A    |    2    | 256 MB |   Qisda      |
//     |   000a   |         |         |        |              |
//     |   000b   |         |         |        |              |
//     |   000c   |         |         |        |              |
//     |   000d   |    B    |    2    | 512 MB |   Egoman     |
//     |   000e   |    B    |    2    | 512 MB |   Sony UK    |
//     |   000f   |    B    |    2    | 512 MB |   Egoman     |
//     |   0010   |    B+   |    1    | 512 MB |   Sony UK    |
//     |   0011   | compute |    1    | 512 MB |   Sony UK    |
//     |   0012   |    A+   |    1    | 256 MB |   Sony UK    |
//     |   0013   |    B+   |    1    | 512 MB |   Embest     |
//     |   0014   | compute |    1    | 512 MB |   Embest     |
//     |   0015   |    A+   |    1    | 256 MB |   Embest     |
//     |   0015   |    A+   |    1    | 512 MB |   Embest     |
//     +----------+---------+---------+--------+--------------+
//
// If the Raspberry Pi has been over-volted (voiding the warranty) the
// revision number will have 100 at the front. e.g. 1000002.
//
//-------------------------------------------------------------------------
//
// With the release of the Raspberry Pi 2, there is a new encoding of the
// Revision field in /proc/cpuinfo. The bit fields are as follows
//
//     +----+----+----+----+----+----+----+----+
//     |FEDC|BA98|7654|3210|FEDC|BA98|7654|3210|
//     +----+----+----+----+----+----+----+----+
//     |    |    |    |    |    |    |    |AAAA|
//     |    |    |    |    |    |BBBB|BBBB|    |
//     |    |    |    |    |CCCC|    |    |    |
//     |    |    |    |DDDD|    |    |    |    |
//     |    |    | EEE|    |    |    |    |    |
//     |    |    |F   |    |    |    |    |    |
//     |    |   G|    |    |    |    |    |    |
//     |    |  H |    |    |    |    |    |    |
//     +----+----+----+----+----+----+----+----+
//     |1098|7654|3210|9876|5432|1098|7654|3210|
//     +----+----+----+----+----+----+----+----+
//
// +---+-------+--------------+--------------------------------------------+
// | # | bits  |   contains   | values                                     |
// +---+-------+--------------+--------------------------------------------+
// | A | 00-03 | PCB Revision | (the pcb revision number)                  |
// | B | 04-11 | Model name   |                                            |
// | C | 12-15 | Processor    | BCM2835, BCM2836, BCM2837, BCM2711,        |
// |   |       |              | BCM2712                                    |
// | D | 16-19 | Manufacturer | Sony, Egoman, Embest, Sony Japan, Embest,  |
// |   |       |              | Stadium                                    |
// | E | 20-22 | Memory size  | 256 MB, 512 MB, 1024 MB, 2048 MB, 4096 MB, |
// |   |       |              | 8192 MB                                    |
// | F | 23-23 | encoded flag | (if set, revision is a bit field)          |
// | G | 24-24 | waranty bit  | (if set, warranty void - Pre Pi2)          |
// | H | 25-25 | waranty bit  | (if set, warranty void - Post Pi2)         |
// +---+-------+--------------+--------------------------------------------+
//
// Also, due to some early issues the warranty bit has been move from bit
// 24 to bit 25 of the revision number (i.e. 0x2000000). It is also possible
// that both bits may be set (i.e. 0x3000000).
//
// e.g.
//
// Revision    : A01041
//
// A - PCB Revision - 1 (first revision)
// B - Model Name - 4 (Model B Pi 2)
// C - Processor - 1 (BCM2836)
// D - Manufacturer - 0 (Sony)
// E - Memory - 2 (1024 MB)
// F - Endcoded flag - 1 (encoded cpu info)
//
// Revision    : A21041
//
// A - PCB Revision - 1 (first revision)
// B - Model Name - 4 (Model B Pi 2)
// C - Processor - 1 (BCM2836)
// D - Manufacturer - 2 (Embest)
// E - Memory - 2 (1024 MB)
// F - Endcoded flag - 1 (encoded cpu info)
//
// Revision    : 900092
//
// A - PCB Revision - 2 (second revision)
// B - Model Name - 9 (Model Zero)
// C - Processor - 0 (BCM2835)
// D - Manufacturer - 0 (Sony)
// E - Memory - 1 (512 MB)
// F - Endcoded flag - 1 (encoded cpu info)
//
// Revision    : A02082
//
// A - PCB Revision - 2 (first revision)
// B - Model Name - 8 (Model B Pi 3)
// C - Processor - 2 (BCM2837)
// D - Manufacturer - 0 (Sony)
// E - Memory - 2 (1024 MB)
// F - Endcoded flag - 1 (encoded cpu info)
//
// Revision    : A52082
//
// A - PCB Revision - 2 (second revision)
// B - Model Name - 8 (Model B Pi 3)
// C - Processor - 2 (BCM2837)
// D - Manufacturer - 5 (Stadium)
// E - Memory - 2 (1024 MB)
// F - Endcoded flag - 1 (encoded cpu info)
//
// Revision    : 03A01041
//
// A - PCB Revision - 1 (second revision)
// B - Model Name - 4 (Model B Pi 2)
// C - Processor - 1 (BCM2836)
// D - Manufacturer - 0 (Sony UK)
// E - Memory - 2 (1024 MB)
// F - Endcoded flag - 1 (encoded cpu info)
// G - Pre-Pi2 Warranty - 1 (void)
// H - Post-Pi2 Warranty - 1 (void)
//
// Revision    : B03111
//
// A - PCB Revision - 1 (first revision)
// B - Model Name - 17 (Model B Pi 4)
// C - Processor - 3 (BCM2711)
// D - Manufacturer - 0 (Sony UK)
// E - Memory - 32 (2048 MB)
// F - Endcoded flag - 1 (encoded cpu info)

//-------------------------------------------------------------------------

static RASPBERRY_PI_MEMORY_T revisionToMemory[] = {
    RPI_MEMORY_UNKNOWN, //  0
    RPI_MEMORY_UNKNOWN, //  1
    RPI_256MB,          //  2
    RPI_256MB,          //  3
    RPI_256MB,          //  4
    RPI_256MB,          //  5
    RPI_256MB,          //  6
    RPI_256MB,          //  7
    RPI_256MB,          //  8
    RPI_256MB,          //  9
    RPI_MEMORY_UNKNOWN, //  A
    RPI_MEMORY_UNKNOWN, //  B
    RPI_MEMORY_UNKNOWN, //  C
    RPI_512MB,          //  D
    RPI_512MB,          //  E
    RPI_512MB,          //  F
    RPI_512MB,          // 10
    RPI_512MB,          // 11
    RPI_256MB,          // 12
    RPI_512MB,          // 13
    RPI_512MB,          // 14
    RPI_512MB           // 15
};

static RASPBERRY_PI_MEMORY_T bitFieldToMemory[] = {
    RPI_256MB,  // 0
    RPI_512MB,  // 1
    RPI_1024MB, // 2
    RPI_2048MB, // 3
    RPI_4096MB, // 4
    RPI_8192MB, // 5
};

static RASPBERRY_PI_PROCESSOR_T bitFieldToProcessor[] = {
    RPI_BROADCOM_2835, // 0
    RPI_BROADCOM_2836, // 1
    RPI_BROADCOM_2837, // 2
    RPI_BROADCOM_2711, // 3
    RPI_BROADCOM_2712, // 4
};

static RASPBERRY_PI_I2C_DEVICE_T revisionToI2CDevice[] = {
    RPI_I2C_DEVICE_UNKNOWN, //  0
    RPI_I2C_DEVICE_UNKNOWN, //  1
    RPI_I2C_0,              //  2
    RPI_I2C_0,              //  3
    RPI_I2C_1,              //  4
    RPI_I2C_1,              //  5
    RPI_I2C_1,              //  6
    RPI_I2C_1,              //  7
    RPI_I2C_1,              //  8
    RPI_I2C_1,              //  9
    RPI_I2C_DEVICE_UNKNOWN, //  A
    RPI_I2C_DEVICE_UNKNOWN, //  B
    RPI_I2C_DEVICE_UNKNOWN, //  C
    RPI_I2C_1,              //  D
    RPI_I2C_1,              //  E
    RPI_I2C_1,              //  F
    RPI_I2C_1,              // 10
    RPI_I2C_1,              // 11
    RPI_I2C_1,              // 12
    RPI_I2C_1,              // 13
    RPI_I2C_1,              // 14
    RPI_I2C_1               // 15
};

static RASPBERRY_PI_MODEL_T bitFieldToModel[] = {
    RPI_MODEL_A,               //  0
    RPI_MODEL_B,               //  1
    RPI_MODEL_A_PLUS,          //  2
    RPI_MODEL_B_PLUS,          //  3
    RPI_MODEL_B_PI_2,          //  4
    RPI_MODEL_ALPHA,           //  5
    RPI_COMPUTE_MODULE,        //  6
    RPI_MODEL_UNKNOWN,         //  7
    RPI_MODEL_B_PI_3,          //  8
    RPI_MODEL_ZERO,            //  9
    RPI_COMPUTE_MODULE_3,      //  A
    RPI_MODEL_UNKNOWN,         //  B
    RPI_MODEL_ZERO_W,          //  C
    RPI_MODEL_B_PI_3_PLUS,     //  D
    RPI_MODEL_A_PI_3_PLUS,     //  E
    RPI_MODEL_UNKNOWN,         //  F
    RPI_COMPUTE_MODULE_3_PLUS, // 10
    RPI_MODEL_B_PI_4,          // 11
    RPI_MODEL_ZERO_2_W,        // 12
    RPI_MODEL_400,             // 13
    RPI_COMPUTE_MODULE_4,      // 14
    RPI_COMPUTE_MODULE_4S,     // 15
    RPI_MODEL_UNKNOWN,         // 16
    RPI_MODEL_PI_5,            // 17
};

static RASPBERRY_PI_MODEL_T revisionToModel[] = {
    RPI_MODEL_UNKNOWN,  //  0
    RPI_MODEL_UNKNOWN,  //  1
    RPI_MODEL_B,        //  2
    RPI_MODEL_B,        //  3
    RPI_MODEL_B,        //  4
    RPI_MODEL_B,        //  5
    RPI_MODEL_B,        //  6
    RPI_MODEL_A,        //  7
    RPI_MODEL_A,        //  8
    RPI_MODEL_A,        //  9
    RPI_MODEL_UNKNOWN,  //  A
    RPI_MODEL_UNKNOWN,  //  B
    RPI_MODEL_UNKNOWN,  //  C
    RPI_MODEL_B,        //  D
    RPI_MODEL_B,        //  E
    RPI_MODEL_B,        //  F
    RPI_MODEL_B_PLUS,   // 10
    RPI_COMPUTE_MODULE, // 11
    RPI_MODEL_A_PLUS,   // 12
    RPI_MODEL_B_PLUS,   // 13
    RPI_COMPUTE_MODULE, // 14
    RPI_MODEL_A_PLUS    // 15
};

static RASPBERRY_PI_MANUFACTURER_T bitFieldToManufacturer[] = {
    RPI_MANUFACTURER_SONY_UK,    // 0
    RPI_MANUFACTURER_EGOMAN,     // 1
    RPI_MANUFACTURER_EMBEST,     // 2
    RPI_MANUFACTURER_SONY_JAPAN, // 3
    RPI_MANUFACTURER_EMBEST,     // 4
    RPI_MANUFACTURER_STADIUM     // 5
};

static RASPBERRY_PI_MANUFACTURER_T revisionToManufacturer[] = {
    RPI_MANUFACTURER_UNKNOWN, //  0
    RPI_MANUFACTURER_UNKNOWN, //  1
    RPI_MANUFACTURER_EGOMAN,  //  2
    RPI_MANUFACTURER_EGOMAN,  //  3
    RPI_MANUFACTURER_SONY_UK, //  4
    RPI_MANUFACTURER_QISDA,   //  5
    RPI_MANUFACTURER_EGOMAN,  //  6
    RPI_MANUFACTURER_EGOMAN,  //  7
    RPI_MANUFACTURER_SONY_UK, //  8
    RPI_MANUFACTURER_QISDA,   //  9
    RPI_MANUFACTURER_UNKNOWN, //  A
    RPI_MANUFACTURER_UNKNOWN, //  B
    RPI_MANUFACTURER_UNKNOWN, //  C
    RPI_MANUFACTURER_EGOMAN,  //  D
    RPI_MANUFACTURER_SONY_UK, //  E
    RPI_MANUFACTURER_EGOMAN,  //  F
    RPI_MANUFACTURER_SONY_UK, // 10
    RPI_MANUFACTURER_SONY_UK, // 11
    RPI_MANUFACTURER_SONY_UK, // 12
    RPI_MANUFACTURER_EMBEST,  // 13
    RPI_MANUFACTURER_EMBEST,  // 14
    RPI_MANUFACTURER_EMBEST   // 15
};

static int revisionToPcbRevision[] = {
    0, //  0
    0, //  1
    1, //  2
    1, //  3
    2, //  4
    2, //  5
    2, //  6
    2, //  7
    2, //  8
    2, //  9
    0, //  A
    0, //  B
    0, //  C
    2, //  D
    2, //  E
    2, //  F
    1, // 10
    1, // 11
    1, // 12
    1, // 13
    1, // 14
    1  // 15
};

// Remove leading and trailing whitespace from a string.

static char *trimWhiteSpace(char *string) {
    if (string == NULL)
        return NULL;
    while (isspace(*string))
        string++;
    if (*string == '\0')
        return string;
    char *end = string;
    while (*end)
        ++end;
    --end;
    while ((end > string) && isspace(*end))
        end--;
    *(end + 1) = 0;
    return string;
}

int getRaspberryPiRevision() {
    int raspberryPiRevision = 0;
    FILE *fp = fopen("/proc/cpuinfo", "r");
    if (fp == NULL) {
        perror("/proc/cpuinfo");
        return raspberryPiRevision;
    }
    char entry[80];
    while (fgets(entry, sizeof(entry), fp) != NULL) {
        char* saveptr = NULL;
        char *key = trimWhiteSpace(strtok_r(entry, ":", &saveptr));
        char *value = trimWhiteSpace(strtok_r(NULL, ":", &saveptr));
        if (strcasecmp("Revision", key) == 0) {
            raspberryPiRevision = strtol(value, NULL, 16);
        }
    }
    fclose(fp);
    return raspberryPiRevision;
}

int getRaspberryPiInformation(RASPBERRY_PI_INFO_T *info) {
    int revision = getRaspberryPiRevision();
    return getRaspberryPiInformationForRevision(revision, info);
}

int getRaspberryPiInformationForRevision(int revision, RASPBERRY_PI_INFO_T *info) {
    int result = 0;

    if (info != NULL) {
        info->memory = RPI_MEMORY_UNKNOWN;
        info->processor = RPI_PROCESSOR_UNKNOWN;
        info->i2cDevice = RPI_I2C_DEVICE_UNKNOWN;
        info->model = RPI_MODEL_UNKNOWN;
        info->manufacturer = RPI_MANUFACTURER_UNKNOWN;
        info->pcbRevision = 0;
        info->warrantyBit = 0;
        info->revisionNumber = revision;
        info->peripheralBase = RPI_PERIPHERAL_BASE_UNKNOWN;

        if (revision != 0) {
            size_t maxOriginalRevision = (sizeof(revisionToModel) / sizeof(revisionToModel[0])) - 1;
            // remove warranty bit
            revision &= ~0x3000000;
            if (revision & 0x800000) {
                // Raspberry Pi2 style revision encoding
                result = 2;
                if (info->revisionNumber & 0x2000000) {
                    info->warrantyBit = 1;
                }
                int memoryIndex = (revision & 0x700000) >> 20;
                size_t knownMemoryValues = sizeof(bitFieldToMemory) / sizeof(bitFieldToMemory[0]);
                if (memoryIndex < knownMemoryValues) {
                    info->memory = bitFieldToMemory[memoryIndex];
                } else {
                    info->memory = RPI_MEMORY_UNKNOWN;
                }

                int processorIndex = (revision & 0xF000) >> 12;
                size_t knownProcessorValues = sizeof(bitFieldToProcessor) / sizeof(bitFieldToProcessor[0]);
                if (processorIndex < knownProcessorValues) {
                    info->processor = bitFieldToProcessor[processorIndex];
                } else {
                    info->processor = RPI_PROCESSOR_UNKNOWN;
                }

                // If some future firmware changes the Rev number of
                // older Raspberry Pis, then need to work out the i2c
                // device.
                info->i2cDevice = RPI_I2C_1;
                int modelIndex = (revision & 0xFF0) >> 4;
                size_t knownModelValues = sizeof(bitFieldToModel) / sizeof(bitFieldToModel[0]);
                if (modelIndex < knownModelValues) {
                    info->model = bitFieldToModel[modelIndex];
                } else {
                    info->model = RPI_MODEL_UNKNOWN;
                }

                int madeByIndex = (revision & 0xF0000) >> 16;
                size_t knownManufacturerValues = sizeof(bitFieldToManufacturer) / sizeof(bitFieldToManufacturer[0]);

                if (madeByIndex < knownManufacturerValues) {
                    info->manufacturer = bitFieldToManufacturer[madeByIndex];
                } else {
                    info->manufacturer = RPI_MANUFACTURER_UNKNOWN;
                }
                info->pcbRevision = revision & 0xF;
            } else if (revision <= maxOriginalRevision) {
                // Original revision encoding
                result = 1;
                if (info->revisionNumber & 0x1000000) {
                    info->warrantyBit = 1;
                }

                info->memory = revisionToMemory[revision];
                info->i2cDevice = revisionToI2CDevice[revision];
                info->model = revisionToModel[revision];
                info->manufacturer = revisionToManufacturer[revision];
                info->pcbRevision = revisionToPcbRevision[revision];

                if (info->model == RPI_MODEL_UNKNOWN) {
                    info->processor = RPI_PROCESSOR_UNKNOWN;
                } else {
                    info->processor = RPI_BROADCOM_2835;
                }
            }
        }

        switch (info->processor) {
            case RPI_PROCESSOR_UNKNOWN:
                info->peripheralBase = RPI_PERIPHERAL_BASE_UNKNOWN;
                break;
            case RPI_BROADCOM_2835:
                info->peripheralBase = RPI_BROADCOM_2835_PERIPHERAL_BASE;
                break;
            case RPI_BROADCOM_2836:
                info->peripheralBase = RPI_BROADCOM_2836_PERIPHERAL_BASE;
                break;
            case RPI_BROADCOM_2837:
                info->peripheralBase = RPI_BROADCOM_2837_PERIPHERAL_BASE;
                break;
            case RPI_BROADCOM_2711:
                info->peripheralBase = RPI_BROADCOM_2711_PERIPHERAL_BASE;
                break;
            case RPI_BROADCOM_2712:
                info->peripheralBase = RPI_BROADCOM_2712_PERIPHERAL_BASE;
                break;
            default:
                info->peripheralBase = RPI_PERIPHERAL_BASE_UNKNOWN;
                break;
        }
    }

    return result;
}

const char *raspberryPiMemoryToString(RASPBERRY_PI_MEMORY_T memory) {
    const char *string = "unknown";
    switch(memory) {
        case RPI_256MB:
            string = "256 MB";
            break;
        case RPI_512MB:
            string = "512 MB";
            break;
        case RPI_1024MB:
            string = "1024 MB";
            break;
        case RPI_2048MB:
            string = "2048 MB";
            break;
        case RPI_4096MB:
            string = "4096 MB";
            break;
        case RPI_8192MB:
            string = "8192 MB";
            break;
        default:
            break;
    }
    return string;
}

const char *raspberryPiProcessorToString(RASPBERRY_PI_PROCESSOR_T processor) {
    const char *string = "unknown";
    switch(processor) {
        case RPI_BROADCOM_2835:
            string = "Broadcom BCM2835";
            break;
        case RPI_BROADCOM_2836:
            string = "Broadcom BCM2836";
            break;
        case RPI_BROADCOM_2837:
            string = "Broadcom BCM2837";
            break;
        case RPI_BROADCOM_2711:
            string = "Broadcom BCM2711";
            break;
        case RPI_BROADCOM_2712:
            string = "Broadcom BCM2712";
            break;
        default:
            break;
    }
    return string;
}

const char *raspberryPiI2CDeviceToString(RASPBERRY_PI_I2C_DEVICE_T i2cDevice) {
    const char *string = "unknown";
    switch(i2cDevice) {
    case RPI_I2C_0:
        string = "/dev/i2c-0";
        break;
    case RPI_I2C_1:
        string = "/dev/i2c-1";
        break;
    default:
        break;
    }
    return string;
}

const char *raspberryPiModelToString(RASPBERRY_PI_MODEL_T model) {
    const char *string = "unknown";
    switch(model) {
        case RPI_MODEL_A:
            string = "Raspberry Pi Model A";
            break;
        case RPI_MODEL_B:
            string = "Raspberry Pi Model B";
            break;
        case RPI_MODEL_A_PLUS:
            string = "Raspberry Pi Model A Plus";
            break;
        case RPI_MODEL_B_PLUS:
            string = "Raspberry Pi Model B Plus";
            break;
        case RPI_MODEL_B_PI_2:
            string = "Raspberry Pi 2 Model B";
            break;
        case RPI_MODEL_ALPHA:
            string = "Raspberry Pi Alpha";
            break;
        case RPI_COMPUTE_MODULE:
            string = "Raspberry Pi Compute Module";
            break;
        case RPI_MODEL_ZERO:
            string = "Raspberry Pi Model Zero";
            break;
        case RPI_MODEL_B_PI_3:
            string = "Raspberry Pi 3 Model B";
            break;
        case RPI_COMPUTE_MODULE_3:
            string = "Raspberry Pi Compute Module 3";
            break;
        case RPI_MODEL_ZERO_W:
            string = "Raspberry Pi Model Zero W";
            break;
        case RPI_MODEL_B_PI_3_PLUS:
            string = "Raspberry Pi 3 Model B Plus";
            break;
        case RPI_MODEL_A_PI_3_PLUS:
            string = "Raspberry Pi 3 Model A Plus";
            break;
        case RPI_COMPUTE_MODULE_3_PLUS:
            string = "Raspberry Pi Compute Module 3 Plus";
            break;
        case RPI_MODEL_B_PI_4:
            string = "Raspberry Pi 4 Model B";
            break;
        case RPI_COMPUTE_MODULE_4:
            string = "Raspberry Pi Compute Module 4";
            break;
        case RPI_MODEL_ZERO_2_W:
            string = "Raspberry Pi Model Zero 2 W";
            break;
        case RPI_MODEL_400:
            string = "Raspberry Pi 400";
            break;
        case RPI_COMPUTE_MODULE_4S:
            string = "Raspberry Pi Compute Module 4S";
            break;
        case RPI_MODEL_PI_5:
            string = "Raspberry Pi 5";
            break;
        default:
            break;
    }
    return string;
}

const char *raspberryPiManufacturerToString(RASPBERRY_PI_MANUFACTURER_T manufacturer) {
    const char *string = "unknown";
    switch(manufacturer) {
        case RPI_MANUFACTURER_SONY_UK:
            string = "Sony UK";
            break;
        case RPI_MANUFACTURER_EGOMAN:
            string = "Egoman";
            break;
        case RPI_MANUFACTURER_QISDA:
            string = "Qisda";
            break;
        case RPI_MANUFACTURER_EMBEST:
            string = "Embest";
            break;
        case RPI_MANUFACTURER_SONY_JAPAN:
            string = "Sony Japan";
            break;
        case RPI_MANUFACTURER_STADIUM:
            string = "Stadium";
            break;
        default:
            break;
    }
    return string;
}
