#include <sys/stat.h>
#include <sys/utsname.h>

#include <fstream>
#include <sstream>
#include <string>

namespace utils {

// replace the std::string::starts_with function only available in C++20 and above.
template <typename string_type, typename prefix_type>
inline bool starts_with(const string_type& str, const prefix_type& prefix) {
#ifdef __cpp_lib_starts_ends_with
  return str.starts_with(prefix);
#else
  return str.rfind(prefix, 0) == 0;
#endif
}
} // utils


// Unix OS version info:
// ---------------------
//                                 // from /etc/os-release         older /etc/lsb-release         // redhat /etc/redhat-release         // debian /etc/debian_version
//  product type                   // $ID                          $DISTRIB_ID                    // single line file containing:       // Debian
//  product version                // $VERSION_ID                  $DISTRIB_RELEASE               // <Vendor_ID release Version_ID>     // single line file <Release_ID/sid>
//  pretty name                    // $PRETTY_NAME                 $DISTRIB_DESCRIPTION

static std::string unquote(const char *begin, const char *end) {
  if (*begin == '"') {
    return std::string(begin + 1, end - begin - 2);
  }
  return std::string(begin, end - begin);
}

static std::string unquote(const std::string::iterator &begin, const std::string::iterator &end) {
  if (*begin == '"') {
    return std::string(begin + 1, end - 2);
  }
  return std::string(begin, end);
}

static bool read_etc_file(const char *filename, const std::string &id_key, const std::string &version_key, const std::string &pretty_name_key, std::string &id_val, std::string &version_val, std::string &pretty_name_val) {
  std::string line;
  std::ifstream stream("/etc/os-release");
  if (!stream) {
    return false;
  }
  while (getline(stream, line)) {
    if (utils::starts_with(line, id_key)) {
      line = line.substr(line.find('=') + 1, line.length());
      id_val = unquote(line.begin(), line.end());
    }
    if (utils::starts_with(line, version_key)) {
      line = line.substr(line.find('=') + 1, line.length());
      version_val = unquote(line.begin(), line.end());
    }
    if (utils::starts_with(line, pretty_name_key)) {
      line = line.substr(line.find('=') + 1, line.length());
      pretty_name_val = unquote(line.begin() + 1, line.end());
    }
  }
  return true;
}

static bool read_os_release() {
  static const std::string ID = "ID=";
  static const std::string VERSIONID = "VERSION_ID=";
  static const std::string PRETTYNAME = "PRETTY_NAME=";

  std::string id, versionId, prettyName;

  // man os-release(5) says:
  // The file /etc/os-release takes precedence over /usr/lib/os-release.
  // Applications should check for the former, and exclusively use its data
  // if it exists, and only fall back to /usr/lib/os-release if it is missing.
  return read_etc_file("/etc/os-release", ID, VERSIONID, PRETTYNAME, id, versionId, prettyName) || read_etc_file("/usr/lib/os-release", ID, VERSIONID, PRETTYNAME, id, versionId, prettyName);
}

/// Linux system informations

std::string get_full_name() {
  std::string line;
  std::ifstream stream("/etc/os-release");
  if (!stream) {
    return "Linux <unknown version>";
  }
  while (getline(stream, line)) {
    if (utils::starts_with(line, "PRETTY_NAME")) {
      line = line.substr(line.find('=') + 1, line.length());
      return {line.begin() + 1, line.end() - 1}; // remove \" at begin and end of the substring result
    }
  }
  stream.close();
  return "Linux <unknown version>";
}

std::string get_name() {
  std::string line;
  std::ifstream stream("/etc/os-release");
  if (!stream) {
    return "Linux";
  }
  while (getline(stream, line)) {
    if (utils::starts_with(line, "NAME")) {
      line = line.substr(line.find('=') + 1, line.length());
      // remove \" at begin and end of the substring result
      return {line.begin() + 1, line.end() - 1};
    }
  }
  stream.close();
  return "Linux";
}

std::string get_version() {
  std::string line;
  std::ifstream stream("/etc/os-release");
  if (!stream) {
    return "<unknown version>";
  }
  while (getline(stream, line)) {
    if (utils::starts_with(line, "VERSION_ID")) {
      line = line.substr(line.find('=') + 1, line.length());
      // remove \" at begin and end of the substring result
      return {line.begin() + 1, line.end() - 1};
    }
  }
  stream.close();
  return "<unknown version>";
}

std::string get_kernel() {
  static utsname info;
  if (uname(&info) == 0) {
    return info.release;
  }
  return "<unknown kernel>";
}

bool get_is64_bit() {
  struct stat buffer {};
  return (stat("/lib64/ld-linux-x86-64.so.2", &buffer) == 0);
}
