#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"
#include "process.h"

using namespace std;

// constructor
Process::Process(const int pid) : pid_(pid){};

// Return this process's ID
int Process::Pid() { return pid_; }

// Return this process's CPU utilization
float Process::CpuUtilization() {
  float result = ((float)LinuxParser::ActiveJiffies(pid_) / sysconf(_SC_CLK_TCK));

  cpu_utilization_ = result / UpTime();
  return this->cpu_utilization_;
}

// Return the command that generated this process
string Process::Command() { return LinuxParser::Command(pid_); }

// Return this process's memory utilization
string Process::Ram() { return LinuxParser::Ram(pid_); }

// Return the user (name) that generated this process
string Process::User() { return LinuxParser::User(pid_); }

// Return the age of this process (in seconds)
long int Process::UpTime() { return LinuxParser::UpTime(pid_); }

// Overload the "less than" comparison operator for Process objects
bool Process::operator<(Process const& a) const {
  return cpu_utilization_ > a.cpu_utilization_; 
}