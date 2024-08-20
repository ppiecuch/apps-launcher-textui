#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/reboot.h>

#include <string>

#include "support.h"

// Reference:
// ----------
//  - https://github.com/vanvught/rpidmx512/blob/master/lib-hal/src/linux/hardware.cpp
//  - https://github.com/jonathanspw/fastfetch/blob/master/src/modules/host.c


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

static std::string exec_cmd(const std::string &cmd, bool *err = NULL) {
    std::string output;
    output.resize(1024*10);
    bool ret = exec_cmd(cmd.c_str(), &output[0], output.size());
    if (err)
        *err = ret;
    return output;
}

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

uint64_t getDeviceSummary(char *tempstring, uint64_t tempstring_size) {
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
        if (fgets(buffer, sizeof(buffer) , fp)){
            sscanf(buffer, "Revision  : %s", revision);
        }
    }
    fclose(fp);

    return revision;
}


int hwGetRpiRevision(void) {
    char revision[1024] = {'\0'};

    if (hwGetCpuInfoRevision(revision) == NULL)
        return -1;

    if ((strcmp(revision, "0002") == 0) ||
        (strcmp(revision, "1000002") == 0 ) ||
        (strcmp(revision, "0003") == 0) ||
        (strcmp(revision, "1000003") == 0 ))
        return 1;
    else if ((strcmp(revision, "0004") == 0) ||
            (strcmp(revision, "1000004") == 0 ) ||
            (strcmp(revision, "0005") == 0) ||
            (strcmp(revision, "1000005") == 0 ) ||
            (strcmp(revision, "0006") == 0) ||
            (strcmp(revision, "1000006") == 0 ))
        return 2;
    else if ((strcmp(revision, "a01040") == 0) ||   /* Raspberry Pi 2B */
            (strcmp(revision, "a01041") == 0) ||
            (strcmp(revision, "a02042") == 0) ||
            (strcmp(revision, "a21041") == 0) ||
            (strcmp(revision, "a22042") == 0))
        return 3;
    else if ((strcmp(revision, "a02082") == 0) ||   /* Raspberry Pi 3B */
            (strcmp(revision, "a22082") == 0) ||
            (strcmp(revision, "a32082") == 0) ||
            (strcmp(revision, "a52082") == 0) ||
            (strcmp(revision, "a22083") == 0) ||
            (strcmp(revision, "a020d3") == 0))     /* Raspberry Pi 3B+ */
        return 4;
    else if ((strcmp(revision, "a03111") == 0) ||   /* Raspberry Pi 4B  rev. 1.1, 1.2, 1.4, 1.5 */
            (strcmp(revision, "b03111") == 0) ||
                    (strcmp(revision, "c03111") == 0) ||
            (strcmp(revision, "b03112") == 0) ||
                    (strcmp(revision, "c03112") == 0) ||
            (strcmp(revision, "b03114") == 0) ||
            (strcmp(revision, "c03114") == 0) ||
                    (strcmp(revision, "d03114") == 0) ||
                    (strcmp(revision, "b03115") == 0) ||
            (strcmp(revision, "c03115") == 0) ||
            (strcmp(revision, "d03115") == 0))
        return 5;
    else if ((strcmp(revision, "c03130") == 0) ||   /* Raspberry Pi 400 */
            (strcmp(revision, "c03131") == 0))
        return 6;
    else                                            /* assume rev 7 */
        return 7;
}

