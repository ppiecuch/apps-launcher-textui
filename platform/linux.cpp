#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/utsname.h>

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

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

// split input string at delimiter and return result
std::vector<std::string> split(const std::string& input, const std::string& delimiter) {
  std::vector<std::string> result;
  size_t shift = 0;
  while (true) {
    size_t match = input.find(delimiter, shift);
    result.emplace_back(input.substr(shift, match - shift));
    if (match == std::string::npos) {
      break;
    }
    shift = match + delimiter.size();
  }
  return result;
}

// remove all white spaces (' ', '\t', '\n') from start and end of input
void strip(std::string& input) {
  if (input.empty()) {
    return;
  }
  // optimization for input size == 1
  if (input.size() == 1) {
    if (input[0] == ' ' || input[0] == '\t' || input[0] == '\n') {
      input = "";
      return;
    } else {
      return;
    }
  }
  size_t start_index = 0;
  while (true) {
    char c = input[start_index];
    if (c != ' ' && c != '\t' && c != '\n') {
      break;
    }
    start_index++;
  }
  size_t end_index = input.size() - 1;
  while (true) {
    char c = input[end_index];
    if (c != ' ' && c != '\t' && c != '\n') {
      break;
    }
    end_index--;
  }
  if (end_index < start_index) {
    input.assign("");
    return;
  }
  input.assign(input.begin() + start_index, input.begin() + end_index + 1);
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

/// Memory info

void get_from_sysconf(long &total, long &available) {
  long pages = sysconf(_SC_PHYS_PAGES);
  long available_pages = sysconf(_SC_AVPHYS_PAGES);
  long page_size = sysconf(_SC_PAGESIZE);
  if (pages > 0 && page_size > 0) {
    total = pages * page_size;
  }
  if (available_pages > 0 && page_size > 0) {
    available = available_pages * page_size;
  }
}

void set_value(std::string& line, long &dst) {
  auto split_line = utils::split(line, ":");
  if (split_line.size() == 2) {
    auto& value = split_line[1];
    utils::strip(value);
    auto space = value.find(' ');
    if (space != std::string::npos) {
      auto a = std::string(value.begin(), value.begin() + static_cast<long>(space));
      dst = std::stoll(a) * 1024;
    }
  }
}

std::vector<long> parse_meminfo() {
  std::vector<long> mi{-1, -1, -1}; // total,free,available
  std::ifstream f_meminfo("/proc/meminfo");
  if (!f_meminfo) {
    get_from_sysconf(mi[0], mi[2]);
  } else {
    while (mi[0] == -1 || mi[1] == -1 || mi[2] == -1) {
      std::string line;
      if (!std::getline(f_meminfo, line)) {
        if (mi[0] == -1 || mi[2] == -1) {
          get_from_sysconf(mi[0], mi[2]);
        }
        return mi;
      }
      if (utils::starts_with(line, "MemTotal")) {
        set_value(line, mi[0]);
      } else if (utils::starts_with(line, "MemFree")) {
        set_value(line, mi[1]);
      } else if (utils::starts_with(line, "MemAvailable")) {
        set_value(line, mi[2]);
      }
    }
  }
  return mi;
}

/// Process info

int get_count_processes() { // Function to count the number of directories (process IDs) in /proc
    struct dirent *entry;
    int count = 0;
    DIR *dir = opendir("/proc");
    if (dir == NULL) {
        perror("opendir failed");
        return -1;
    }
    while ((entry = readdir(dir)) != NULL) {
        if (isdigit(*entry->d_name)) {
            count++;
        }
    }
    closedir(dir);
    return count;
}

int get_count_threads(int pid) { // Function to count the number of threads for a given process ID
    char path[256];
    sprintf(path, "/proc/%d/task", pid);

    struct dirent *entry;
    int count = 0;

    DIR *dir = opendir(path);
    if (dir == NULL) {
        perror("opendir failed");
        return -1;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (isdigit(*entry->d_name)) {
            count++;
        }
    }

    closedir(dir);
    return count;
}
