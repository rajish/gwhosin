// -*- C++ -*-

#ifndef _DBG_H_
#define _DBG_H_

class DbgTrcFn
{
public:
    DbgTrcFn(std::string funct) : function(funct)
        {
            std::cout << "+++" << function << std::endl;
        }

    ~DbgTrcFn()
        {
            std::cout << "---" << function << std::endl;
        }
private:
    std::string function;
};



#ifdef DEBUG

#define DBG(x) x
#define TRACEFN DbgTrcFn(std::string(__FUNCTION__))

#else

#define DBG(x)
#define TRACEFN

#endif // DEBUG


#endif //
