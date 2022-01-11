/*
  ==============================================================================

    ICMP.cpp
    Created: 11 Jan 2022 12:22:15pm
    Author:  A

  ==============================================================================
*/

#include "ICMP.h"

using namespace std;
bool ParseCmd(std::string cmd, std::vector<int8_t>& cmd_data, std::string & dst_addr)
{
    istringstream in(cmd);
    vector<string> temp;
    string t;
    while (getline(in, t, ' '))
    {
        temp.push_back(t);
    }

    if (temp.empty()) return false; // the input is wrong

    string instruct_cmd = temp[0];
    transform(instruct_cmd.begin(), instruct_cmd.end(), instruct_cmd.begin(), ::tolower);

    if (instruct_cmd != "ping") return false; //only process ping command

    if (temp.size() > 1)
    {
        dst_addr = temp[1];
    }
    else
    {
        printf("ERR: IP address needed!\n");
        return false;
    }

    if (temp.size() > 2)
    {
        if (temp[2] == "-p")
        {
            if (temp.size() > 3)
            {
                cmd_data = StringtoVector(temp[3]); // the payload specified
            }
            else
            {
                printf("Need specified payload for ¡®-p¡¯.\n");
                return false;
            }
        }
        else
        {
            printf("Wrong Command.\n");
            return false;
        }
    }
    return true;
}

std::vector<int8_t> StringtoVector(std::string cmd)
{
    vector<int8_t> temp;
    for (auto data : cmd) temp.push_back(data);
    return temp;
}
