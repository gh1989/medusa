/*
Taken from Leela Chess
*/

#pragma once

#include <deque>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>

#include "utils/mutex.h"

namespace Medusa {

class Logging {
 public:
  static Logging& Get();

  // Sets the name of the log. Empty name disables logging.
  void SetFilename(const std::string& filename);

 private:
  // Writes line to the log, and appends new line character.
  void WriteLineRaw(const std::string& line);

  Mutex mutex_;
  std::string filename_ GUARDED_BY(mutex_);
  std::ofstream file_ GUARDED_BY(mutex_);
  std::deque<std::string> buffer_ GUARDED_BY(mutex_);

  Logging() = default;
  friend class LogMessage;
};

class LogMessage : public std::ostringstream {
 public:
  LogMessage(const char* file, int line);
  ~LogMessage();
};

class StderrLogMessage : public std::ostringstream {
 public:
  StderrLogMessage(const char* file, int line);
  ~StderrLogMessage();

 private:
  LogMessage log_;
};

std::chrono::time_point<std::chrono::system_clock> SteadyClockToSystemClock(
    std::chrono::time_point<std::chrono::steady_clock> time);

std::string FormatTime(std::chrono::time_point<std::chrono::system_clock> time);
} 

#define LOGFILE ::Medusa::LogMessage(__FILE__, __LINE__)
#define CERR ::Medusa::StderrLogMessage(__FILE__, __LINE__)