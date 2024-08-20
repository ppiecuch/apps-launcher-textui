#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stdbool.h>

#if defined(__x86_64__) || defined(_M_X64)
#define ARCH_X86 1
#elif defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_IX86)
#define ARCH_X86 1
#elif defined(__ARM_ARCH_2__)
#define ARCH_ARM 1
#elif defined(__ARM_ARCH_3__) || defined(__ARM_ARCH_3M__)
#define ARCH_ARM 1
#elif defined(__ARM_ARCH_4T__) || defined(__TARGET_ARM_4T)
#define ARCH_ARM 1
#elif defined(__ARM_ARCH_5_) || defined(__ARM_ARCH_5E_)
#define ARCH_ARM 1
#elif defined(__ARM_ARCH_6T2_) || defined(__ARM_ARCH_6T2_)
#define ARCH_ARM 1
#elif defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_6J__) || defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6Z__) || defined(__ARM_ARCH_6ZK__)
#define ARCH_ARM 1
#elif defined(__ARM_ARCH_7__) || defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7S__)
#define ARCH_ARM 1
#elif defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7S__)
#define ARCH_ARM 1
#elif defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7S__)
#define ARCH_ARM 1
#elif defined(__ARM_ARCH_7M__)
#define ARCH_ARM 1
#elif defined(__ARM_ARCH_7S__)
#define ARCH_ARM 1
#elif defined(__aarch64__) || defined(_M_ARM64)
#define ARCH_ARM 1
#elif defined(mips) || defined(__mips__) || defined(__mips)
#define ARCH_MIPS 1
#elif defined(__sh__)
#define ARCH_SH 1
#elif defined(__powerpc) || defined(__powerpc__) || defined(__powerpc64__) || defined(__POWERPC__) || defined(__ppc__) || defined(__PPC__) || defined(_ARCH_PPC)
#define ARCH_PPC 1
#elif defined(__PPC64__) || defined(__ppc64__) || defined(_ARCH_PPC64)
#define ARCH_PPC 1
#define ARCH_PPC64 1
#elif defined(__sparc__) || defined(__sparc)
#define ARCH_SPARC 1
#elif defined(__riscv) || defined(__riscv__)
#define ARCH_RISCV 1
#elif defined(__m68k__)
#define ARCH_M68K 1
#else
#error Undetected architecture.
#endif

#define UNKNOWN_DATA -1

typedef int32_t VENDOR;

enum {
    // ARCH_X86
    CPU_VENDOR_INTEL,
    CPU_VENDOR_AMD,
    CPU_VENDOR_HYGON,
    // ARCH_ARM
    CPU_VENDOR_ARM,
    CPU_VENDOR_APPLE,
    CPU_VENDOR_BROADCOM,
    CPU_VENDOR_CAVIUM,
    CPU_VENDOR_NVIDIA,
    CPU_VENDOR_APM,
    CPU_VENDOR_QUALCOMM,
    CPU_VENDOR_HUAWEI,
    CPU_VENDOR_SAMSUNG,
    CPU_VENDOR_MARVELL,
    CPU_VENDOR_PHYTIUM,
    // ARCH_RISCV
    CPU_VENDOR_RISCV,
    CPU_VENDOR_SIFIVE,
    CPU_VENDOR_THEAD,
    // OTHERS
    CPU_VENDOR_UNKNOWN,
    CPU_VENDOR_INVALID
};


struct frequency {
  int32_t base;
  int32_t max;
  bool measured; // Indicates if max frequency was measured
};

struct hypervisor {
  bool present;
  char* hv_name;
  VENDOR hv_vendor;
};

struct cach {
  int32_t size;
  uint8_t num_caches;
  bool exists;
  // plenty of more properties to include in the future...
};


struct cache {
  struct cach* L1i;
  struct cach* L1d;
  struct cach* L2;
  struct cach* L3;
  struct cach** cach_arr;
  uint8_t max_cache_level;
};

struct topology {
    int32_t total_cores;
    struct cache* cach;
#if defined(ARCH_X86) || defined(ARCH_PPC)
    int32_t physical_cores;
    int32_t logical_cores;
    uint32_t sockets;
    uint32_t smt_supported; // Number of SMT that CPU supports (equal to smt_available if SMT is enabled)
#ifdef ARCH_X86
    uint32_t smt_available; // Number of SMT that is currently enabled
    int32_t total_cores_module; // Total cores in the current module (only makes sense in hybrid archs, like ADL)
    struct apic* apic;
#endif
#endif
};


struct features {
    bool AES; // Must be the first field of features struct!
#ifdef ARCH_X86
    bool AVX;
    bool AVX2;
    bool AVX512;
    bool SSE;
    bool SSE2;
    bool SSE3;
    bool SSSE3;
    bool SSE4a;
    bool SSE4_1;
    bool SSE4_2;
    bool FMA3;
    bool FMA4;
    bool SHA;
#elif ARCH_PPC
    bool altivec;
#elif ARCH_ARM
    bool NEON;
    bool SHA1;
    bool SHA2;
    bool CRC32;
    bool SVE;
    bool SVE2;
    uint64_t cntb;
#endif
};

struct extensions {
    char* str;
    uint64_t mask;
};

struct cpu_info {
    VENDOR cpu_vendor;
    struct uarch* arch;
    struct hypervisor* hv;
    struct frequency* freq;
    struct cache* cach;
    struct topology* topo;
    int64_t peak_performance;

    // Similar but not exactly equal to struct features
#ifdef ARCH_RISCV
    struct extensions* ext;
#else
    struct features* feat;
#endif

#if defined(ARCH_X86) || defined(ARCH_PPC)
    char* cpu_name; // CPU name from model
#endif

#ifdef ARCH_X86
    uint32_t maxLevels; //  Max cpuids levels
    uint32_t maxExtendedLevels; // Max cpuids extended levels
    bool topology_extensions; // Topology Extensions (AMD only)
    bool hybrid_flag; // Hybrid Flag (Intel only)
    uint32_t core_type; // Core Type (P/E)
#elif ARCH_PPC
    uint32_t pvr;
#elif ARCH_ARM
    uint32_t midr; // Main ID register
#endif

#if defined(ARCH_ARM) || defined(ARCH_RISCV)
    struct system_on_chip* soc;
#endif

#if defined(ARCH_X86) || defined(ARCH_ARM)
    // If SoC contains more than one CPU and they are different,
    // the others will be stored in the next_cpu field
    struct cpu_info* next_cpu;
    uint8_t num_cpus;
#ifdef ARCH_X86
    uint32_t first_core_id; // The index of the first core in the module
#endif
#endif
};

#endif // CPU_H
