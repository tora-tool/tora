#ifndef TOMONOLITHIC

#include <qcstring.h>

#include <dlfcn.h>
#include <stdlib.h>

class toInit {
public:
  toInit(void)
  {
    char *home=getenv("ORACLE_HOME");
    if (!home)
      return;
    QCString dlpath=home;
    dlpath+="/lib/libclntsh.so";
    if (!dlopen(dlpath,RTLD_NOW|RTLD_GLOBAL))
      return;
  }
};

static toInit Init;
#endif
