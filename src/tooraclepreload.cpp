/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOMONOLITHIC

#include <dlfcn.h>
#include <stdlib.h>

class toInit
{
public:
    toInit(void)
    {
        char *home = getenv("ORACLE_HOME");
        if (!home)
            return ;
        QString dlpath = home;
        dlpath += "/lib/libclntsh.so";
        if (!dlopen(dlpath, RTLD_NOW | RTLD_GLOBAL))
            return ;
    }
};

static toInit Init;
#endif
