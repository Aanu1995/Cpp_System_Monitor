#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"

#define KB2MB 1024

using std::stof;
using std::string;
using std::to_string;
using std::vector;
using std::ifstream;
using std::istringstream;


long ProcessPID(int pid, const string search_key) {
  string filename = LinuxParser::kProcDirectory + to_string(pid) + LinuxParser::kStatusFilename;
  
  string line;
  string key;
  string value;
  ifstream filestream(filename);

  if (filestream.is_open()) {
    while (getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      
      istringstream linestream(line);

      while (linestream >> key >> value) {
        if (key == search_key) return stol(value);
      }
    }
  }
  
  filestream.close();
  return 0;
}

vector<string> ProcessStatusInfo(string filename) {
  vector<string> data {};
  string line;
  string value;
  ifstream filestream(filename);

  if (filestream.is_open()) {
    getline(filestream, line);

    istringstream linestream(line);
    while (linestream >> value) data.push_back(value);
  }
  
  filestream.close();
  return data;
}


// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value = "";
  ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      
      istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  
  filestream.close();
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os;
  string kernel;
  string version;
  string line;
  ifstream filestream(kProcDirectory + kVersionFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  
  filestream.close();
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids {};
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  
  closedir(directory);
  return pids;
}

// Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  
  float memTotal = 0.0;
  float memAvail = 0.0;
  string line;
  string key;
  string value;

  // read file /proc/meminfo and look for MemTotal and MemFree
  ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      
      istringstream linestream(line);
      while (linestream >> key >> value) {
        // search for key memTotal
        if (key == "MemTotal") {
          memTotal = std::stof(value);
        }
        // search for key MemAvailable
        else if (key == "MemAvailable") {
          memAvail = std::stof(value);
          break;
        }
      }
    }
  }

  // Memory Utilization = (Total Memory - Available Memory) / Total Memory
  return (memTotal - memAvail) / memTotal;
}

// Read and return the system uptime
long LinuxParser::UpTime() {
  string line;
  string wholeTime;
  long sys_uptime = 0;

  ifstream filestream(kProcDirectory + kUptimeFilename);

  if (filestream.is_open()) {
    getline(filestream, line);
    istringstream linestream(line);
    linestream >> wholeTime;
    sys_uptime = stol(wholeTime);
  }
  
  filestream.close();
  return sys_uptime;
}

// Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  return (ActiveJiffies() - IdleJiffies());
}

// Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) {
  string filename = kProcDirectory + to_string(pid) + kStatFilename;
  vector<string> statusInfo = ProcessStatusInfo(filename);
  
  // To get start time, the order of the process in the file data is 22
  // since vector count starts from 0 then the order will be 21
  // https://man7.org/linux/man-pages/man5/proc.5.html
  
  int kStarttime = 21;

  if ((int) statusInfo.size() >= kStarttime){
    // Amount of time that this process has been scheduled in user mode
    long utime = stol(statusInfo[13]);
    // Amount of time that this process has been scheduled in kernel mode
    long stime = stol(statusInfo[14]);
    // Amount of time that this process's waited-for children have been scheduled in user mode
    long cutime = stol(statusInfo[15]);
    // Amount of time that this process's waited-for children have been scheduled in kernel mode
    long cstime = stol(statusInfo[16]);
    
    return (utime + stime + cutime + cstime);
  }
    

  return 0;
}

// Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  vector<string> cpu_stats = CpuUtilization();
  long user = stol(cpu_stats[kUser_]);
  long nice = stol(cpu_stats[kNice_]);
  long system = stol(cpu_stats[kSystem_]);
  long idle = stol(cpu_stats[kIdle_]);
  long iowait = stol(cpu_stats[kIOwait_]);
  long irq = stol(cpu_stats[kIRQ_]);
  long softirq = stol(cpu_stats[kSoftIRQ_]);
  long steal = stol(cpu_stats[kSteal_]);

  return (user + nice + system + idle + iowait + irq + softirq + steal);
}

// Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  vector<string> cpu_stats = CpuUtilization();
  long idle = stol(cpu_stats[kIdle_]);
  long iowait = stol(cpu_stats[kIOwait_]);

  return (idle + iowait);
}

// Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  vector<string> data {};
  string line;
  string value;

  ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    // gets only the first line in the stream of data
    getline(filestream, line);

    istringstream linestream(line);
    linestream >> value;

    for (int cpuState = 0; cpuState <= kGuestNice_; cpuState++) {
      linestream >> value;
      data.push_back(value);
    }
  }
  
  filestream.close();
  return data;
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  string line;
  string key;
  string value;

  ifstream filestream(kProcDirectory + kStatFilename);

  if (filestream.is_open()) {
    while (getline(filestream, line)) {
      
      istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "processes") return stoi(value);
      }
    }
  }
  
  filestream.close();
  return 0;
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  string line;
  string key;
  string value;
  
  ifstream filestream(kProcDirectory + kStatFilename);

  if (filestream.is_open()) {
    while (getline(filestream, line)) {
      istringstream linestream(line);
      
      while (linestream >> key >> value) {
        if (key == "procs_running") return stoi(value);
      }
    }
  }

  filestream.close();
  return 0;
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  string line = "";

  ifstream filestream(kProcDirectory + std::to_string(pid) + kCmdlineFilename);

  if (filestream.is_open()){
    getline(filestream, line);
  }

  filestream.close();
  return line;
}

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) {
  const long memoryInMb = ProcessPID(pid, "VmRSS") / KB2MB;
  return to_string(memoryInMb);
}

// Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
  long userId = ProcessPID(pid, "Uid");
  return to_string(userId);
}

// Read and return the user associated with a process
string LinuxParser::User(int pid) {
  // read the user ID for this process
  string userId = Uid(pid);
  string line;
  string key;
  string user = "";

  ifstream filestream(kPasswordPath);

  if (filestream.is_open()) {
    while (getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::replace(line.begin(), line.end(), 'x', ' ');
      
      istringstream linestream(line);

      while (linestream >> user >> key) {
        if (key == userId) return user;
      }
    }
  }

  filestream.close();
  return user;
}

// Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
  string filename = kProcDirectory + to_string(pid) + kStatFilename;
  vector<string> statusInfo = ProcessStatusInfo(filename);
  
  // To get start time, the order of the process in the file data is 22
  // since vector count starts from 0 then the order will be 21
  // https://man7.org/linux/man-pages/man5/proc.5.html
  
  int kStarttime = 21;

  if ((int) statusInfo.size() >= kStarttime) {
    // start time is the time the process started after system boot.
    long startTime = stol(statusInfo[kStarttime]);
    return startTime / sysconf(_SC_CLK_TCK);
  }

  return 0;
}