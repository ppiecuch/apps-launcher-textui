#ifndef CPU_X86_INFO_H
#define CPU_X86_INFO_H

#include "cpu_info.h"

#include <stdint.h>
#include <stdbool.h>

/// CPU ID

struct cpu_info* get_cpu_info(void);
struct cache* get_cache_info(struct cpu_info* cpu);
struct frequency* get_frequency_info(struct cpu_info* cpu);
struct topology* get_topology_info(struct cpu_info* cpu, struct cache* cach, int module);

char* get_str_avx(struct cpu_info* cpu);
char* get_str_sse(struct cpu_info* cpu);
char* get_str_fma(struct cpu_info* cpu);
char* get_str_topology(struct cpu_info* cpu, struct topology* topo, bool dual_socket);
char* get_str_cpu_name_abbreviated(struct cpu_info* cpu);

void print_debug(struct cpu_info* cpu);
void print_raw(struct cpu_info* cpu);

void free_topo_struct(struct topology* topo);

/// APIC

struct apic {
    uint32_t pkg_mask;
    uint32_t pkg_mask_shift;
    uint32_t core_mask;
    uint32_t smt_mask_width;
    uint32_t smt_mask;
    uint32_t* cache_select_mask;
    uint32_t* cache_id_apic;
};

bool get_topology_from_apic(struct cpu_info* cpu, struct topology* topo);
uint32_t is_smt_enabled_amd(struct topology* topo);

#ifdef __linux__
int get_total_cores_module(int total_cores, int module);
#endif

/// UARCH

struct uarch;

struct uarch* get_uarch_from_cpuid(struct cpu_info* cpu, uint32_t dump, uint32_t ef, uint32_t f, uint32_t em, uint32_t m, int s);
char* infer_cpu_name_from_uarch(struct uarch* arch);
bool vpus_are_AVX512(struct cpu_info* cpu);
bool is_knights_landing(struct cpu_info* cpu);
int get_number_of_vpus(struct cpu_info* cpu);
bool choose_new_intel_logo_uarch(struct cpu_info* cpu);
char* get_str_uarch(struct cpu_info* cpu);
char* get_str_process(struct cpu_info* cpu);
void free_uarch_struct(struct uarch* arch);

#endif // CPU_X86_INFO_H
