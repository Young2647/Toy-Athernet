#include "defines.h"
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#pragma once
class FTPClient {
public:

};

COMMMAND ParseCmd(std::string cmd, std::vector<uint8_t> & cmd_data);
std::vector<uint8_t> StringtoVector(std::string cmd);