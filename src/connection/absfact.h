#ifndef __CF_H__
#define __CF_H__

#include "core/tora_export.h"
#include "core/toconnectionprovider.h"
#include "loki/Factory_alt.h"

class toConnectionProvider; // See toconnectionprovider.h

/** Declare datatype for Connection provider factory */
TORA_EXPORT typedef Util::GenericFactory<toConnectionProvider, LOKI_TYPELIST_1(toConnectionProviderFinder::ConnectionProvirerParams const&)> ConnectionProvirerFact;
/** This singleton represents the only instance of the factory */
class TORA_EXPORT  ConnectionProvirerFactory: public Loki::SingletonHolder<ConnectionProvirerFact> {};

#endif
