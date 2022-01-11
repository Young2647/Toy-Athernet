#ifndef _ICMP_H_
#define _ICMP_H_

#include "defines.h"
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

bool ParseCmd(std::string cmd, std::vector<int8_t>& cmd_data, std::string& dst_addr);
std::vector<int8_t> StringtoVector(std::string cmd);

#endif

