/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */
#ifndef TOLISTVIEWFORMATTERFACTORY_H
#define TOLISTVIEWFORMATTERFACTORY_H

#include <loki/Factory.h>
#include <loki/Singleton.h>

using Loki::Factory;
using Loki::SingletonHolder;

typedef SingletonHolder< Factory<toListViewFormatter, int> > toListViewFormatterFactory;

#endif
