#ifndef __OPENDNP3_GENERATED_LOGLEVEL_H_
#define __OPENDNP3_GENERATED_LOGLEVEL_H_

#include <string>
#include <cstdint>

namespace openpal {

/**
  Enumeration for log levels
*/
enum class LogLevel : int
{
  Event = 0x1,
  Error = 0x2,
  Warning = 0x4,
  Info = 0x8,
  Interpret = 0x10,
  Comm = 0x20,
  Debug = 0x40
};

std::string LogLevelToString(LogLevel arg);
int LogLevelToType(LogLevel arg);
LogLevel LogLevelFromType(int arg);

}

#endif
