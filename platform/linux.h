#ifndef LINUX_H
#define LINUX_H

#include <string>

// OS informations

std::string get_full_name();
std::string get_name();
std::string get_version();
std::string get_kernel();
bool get_is64_bit();

#endif // LINUX_H
