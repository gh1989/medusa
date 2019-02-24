/*
Taken from Leela Chess 
*/


#include "utils/logging.h"
#include <iomanip>
#include <iostream>
#include <thread>

namespace medusa {

namespace {
size_t kBufferSizeLines = 200;
const char* kStderrFilename = "<stderr>";
}  // namespace

Logging& Logging::Get() {
  static Logging logging;
  return logging;
}

void Logging::WriteLineRaw(const std::string& line) {
  Mutex::Lock lock_(mutex_);
  if (filename_.empty()) {
    buffer_.push_back(line);
    if (buffer_.size() > kBufferSizeLines) buffer_.pop_front();
  } else {
    auto& file = (filename_ == kStderrFilename) ? std::cerr : file_;
    file << line << std::endl;
  }
}

void Logging::SetFilename(const std::string& filename) {
  Mutex::Lock lock_(mutex_);
  if (filename_ == filename) return;
  filename_ = filename;
  if (filename.empty() || filename == kStderrFilename) {
    file_.close();
  }
  if (filename.empty()) return;
  if (filename != kStderrFilename) file_.open(filename, std::ios_base::app);
  auto& file = (filename == kStderrFilename) ? std::cerr : file_;
  file << "\n\n============= Log started. =============" << std::endl;
  for (const auto& line : buffer_) file << line << std::endl;
  buffer_.clear();
}

LogMessage::LogMessage(const char* file, int line) {
  *this << FormatTime(std::chrono::system_clock::now()) << ' '
        << std::setfill(' ') << std::this_thread::get_id() << std::setfill('0')
        << ' ' << file << ':' << line << "] ";
}

LogMessage::~LogMessage() { Logging::Get().WriteLineRaw(str()); }

StderrLogMessage::StderrLogMessage(const char* file, int line)
    : log_(file, line) {}

StderrLogMessage::~StderrLogMessage() {
  std::cerr << str() << std::endl;
  log_ << str();
}

} 