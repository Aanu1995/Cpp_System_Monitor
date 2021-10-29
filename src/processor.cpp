#include "processor.h"
#include "linux_parser.h"

// Return the aggregate CPU utilization
float Processor::Utilization() {
  return static_cast<float>(LinuxParser::Jiffies()) /
         static_cast<float>(LinuxParser::ActiveJiffies());
}