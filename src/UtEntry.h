// -*- C++ -*-

#ifndef _UTENTRY_H_
#define _UTENTRY_H_

#include <string>
#include <utmp.h>


class UtEntry : private utmp
{

public:
    UtEntry() : utmp () {};
    UtEntry(const utmp &entry) : utmp(entry) {};
    ~UtEntry() {  };
    int compare(const UtEntry& entry) const;

    bool is_login_process() const { return (ut_type == LOGIN_PROCESS); };
    bool is_dead_process() const { return (ut_type == DEAD_PROCESS); };
    bool is_user_process() const { return (ut_type == USER_PROCESS); };

    std::string& get_user() const { return *(new std::string(ut_user)); }; 
    pid_t        get_pid() const { return ut_pid; };
    std::string& get_line() const { return *(new std::string(ut_line)); };
    short int    get_type() const { return ut_type; };

    std::string& to_string() const;
    std::string& get_time_str(bool with_usec = true) const;
    std::string& get_IP_str() const;
    std::string& get_host() const { return *(new std::string(ut_host)); }

}; // class UtEntry

inline bool operator==(const UtEntry& lhe, const UtEntry& rhe)
{
    return lhe.compare(rhe) == 0;
}

inline bool operator<(const UtEntry& lhe, const UtEntry& rhe)
{
    return (lhe.compare(rhe) < 0);
}


struct UtComp {
    bool operator() (const UtEntry& lhe, const UtEntry& rhe) const
        {
            return lhe.compare(rhe) < 0;
        };
};

#endif  // _UTENTRY_H_
