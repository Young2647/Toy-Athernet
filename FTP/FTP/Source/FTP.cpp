/*
  ==============================================================================

    FTP.cpp
    Created: 1 Jan 2022 9:53:05pm
    Author:  zhaoyb

  ==============================================================================
*/

#include "FTP.h"

using namespace std;
COMMMAND ParseCmd(std::string cmd, std::vector<uint8_t>& cmd_data)
{
    istringstream in(cmd);
    vector<string> temp;
    string t;
    while (getline(in, t, ' '))
    {
        temp.push_back(t);
    }
    
    if (temp.empty()) return WRNG; // the input is wrong

    string instruct_cmd = temp[0];
    transform(instruct_cmd.begin(), instruct_cmd.end(), instruct_cmd.begin(), ::toupper);

    string information_cmd = "";
    if (temp.size() > 1)
    {
        information_cmd = temp[1];
    }

    if (instruct_cmd == "CONT")
    {
        if (information_cmd != "")
            cmd_data = StringtoVector(information_cmd);
        else
        {
            cmd_data = StringtoVector(DEFAULT_HOST);
        }
        return CONT;
    }
    if (instruct_cmd == "USER")
    {
        if (information_cmd != "")
            cmd_data = StringtoVector(information_cmd);
        else
            cmd_data = StringtoVector("anonymous");
        return USER;
    }
    else if (instruct_cmd == "PASS")
    {
        if (information_cmd != "")
            cmd_data = StringtoVector(information_cmd);
        else
            cmd_data = StringtoVector("");
        return PASS;
    }
    else if (instruct_cmd == "PWD")
    {
        if (information_cmd != "")
        {
            printf("Too many arguments for PWD command.\n");
            return WRNG;
        }
        else
        {
            cmd_data = StringtoVector("");
            return PWD;
        }
    }
    else if (instruct_cmd == "CWD")
    {
        if (information_cmd != "")
            cmd_data = StringtoVector(information_cmd);
        else
        {
            printf("Missing argument for directory path.\n");
            return WRNG;
        }
        return CWD;
    }
    else if (instruct_cmd == "PASV")
    {
        if (information_cmd != "")
        {
            printf("Too many arguments for PASV command.\n");
            return WRNG;
        }
        else
        {
            cmd_data = StringtoVector("");
            return PASV;
        }
    }
    else if (instruct_cmd == "LIST")
    {
        if (information_cmd != "")
            cmd_data = StringtoVector(information_cmd);
        else
            cmd_data = StringtoVector("");
        return LIST;
    }
    else if (instruct_cmd == "RETR")
    {
        if (temp.size() > 2)
        {
            cmd_data = StringtoVector(information_cmd);
            cmd_data.push_back((uint8_t)' ');
            vector<uint8_t> local_path = StringtoVector(temp[2]);
            cmd_data.insert(cmd_data.end(), local_path.begin(), local_path.end());
        }
        else if (information_cmd != "")
            cmd_data = StringtoVector(information_cmd);
        else
        {
            printf("Missing argument for directory path.\n");
            return WRNG;
        }
        return RETR;
    }
    else if (instruct_cmd == "QUIT")
    {
        if (information_cmd != "")
        {
            printf("Too many arguments for QUIT command.\n");
            return WRNG;
        }
        else
        {
            cmd_data = StringtoVector("");
            return QUIT;
        }
    }
    else
    {
        printf("Invalid Command.\n");
        return WRNG;
    }
}

std::vector<uint8_t> StringtoVector(std::string cmd)
{
    vector<uint8_t> temp;
    for (auto data : cmd) temp.push_back(data);
    return temp;
}
