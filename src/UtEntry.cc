#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include "UtEntry.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#include "Dbg.h"

using namespace std;

string UtEntry::to_string() const
{
    stringstream str;
    if (ut_type == EMPTY)
    {
        str << "Invalid entry";
        return str.str();
    }
    str << get_time_str(false) << "  ";
    str << "User '" << ut_user << "' ";
    switch(ut_type)
    {
    case LOGIN_PROCESS:
        str << "logged in";
        break;
    case DEAD_PROCESS:
        str << "logged out";
        break;
    case USER_PROCESS:
        str << "started process [" << ut_pid << "]";
        break;
    case INIT_PROCESS:
        str << "is the father of all processes";
        break;
    case BOOT_TIME:
        str << "set boot time";
        break;
    case NEW_TIME:
        str << "set new time";
        break;
    case OLD_TIME:
        str << "this was old time";
        break;
    default:
        str << "did something(" << ut_type << ")";
        break;
    }
    if(*ut_host)
    {
        str << " from " << ut_host;
        str << "(" << get_IP_str() << ")";
    }
    str << " on " << ut_line;
    return str.str();
}

string UtEntry::get_IP_str() const
{
    stringstream str;
    
    if (ut_addr_v6[0])
    {
        if (ut_addr_v6[1])
        {
            str << hex << ut_addr_v6[0] << ":" << ut_addr_v6[1] << ":" << ut_addr_v6[2] << ":" << ut_addr_v6[3];
        }
        else 
        {
            in_addr addr = { ut_addr_v6[0] };
            str << inet_ntoa(addr);
        }
    }
    return str.str();
}

string UtEntry::get_time_str(bool with_usec) const
{
    stringstream str;
    time_t time = ut_tv.tv_sec;
    tm tbs;
    if(localtime_r(&time, &tbs)) 
    {
        str << setfill('0');
        str << 1900 + tbs.tm_year << "-" << setw(2) << tbs.tm_mon << "-" << setw(2) << tbs.tm_mday << " ";
        str << setw(2) << tbs.tm_hour << ":" << setw(2) << tbs.tm_min << ":" << setw(2) << tbs.tm_sec;
        if (with_usec)
        {
            str << "." << setw(6) << ut_tv.tv_usec;
        }
    }
    return str.str();
}

int UtEntry::compare(const UtEntry& entry) const
{
    string user(ut_line);
    DBG(cout << "UtEntry::compare('" << user << "', '" << entry.ut_line << "') = " << user.compare(entry.ut_line) << endl);
    return user.compare(entry.ut_line);
}

UtEntry::IgnoredUsers UtEntry::ignored_users;
const int ignored_initialized = UtEntry::init_ignored_users();
