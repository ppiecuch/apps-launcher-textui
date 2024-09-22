#ifndef LINUX_H
#define LINUX_H

#include <string>
#include <vector>

// OS informations

std::string get_full_name();
std::string get_name();
std::string get_version();
std::string get_kernel();

bool get_is64_bit();
int get_count_processes();

// HW informations

std::vector<long> parse_meminfo();

#endif // LINUX_H
