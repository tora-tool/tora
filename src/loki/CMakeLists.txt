INCLUDE_DIRECTORIES(
${CMAKE_SOURCE_DIR}/src/loki/
${CMAKE_SOURCE_DIR}/src/loki/include
${CMAKE_SOURCE_DIR}/src/loki/include/loki/
)

# SET(TORA_LOKI_MOC_CLASSES
# )

SET(TORA_LOKI_LIB_SOURCES

include/loki/AbstractFactory.h
include/loki/AssocVector.h
include/loki/CachedFactory.h
include/loki/ConstPolicy.h
include/loki/DataGenerators.h
include/loki/EmptyType.h
include/loki/Factory.h
include/loki/Function.h
include/loki/Functor.h
include/loki/HierarchyGenerators.h
include/loki/Key.h
include/loki/LockingPtr.h
include/loki/LokiExport.h
include/loki/LokiTypeInfo.h
include/loki/MultiMethods.h
include/loki/NullType.h
include/loki/OrderedStatic.h
include/loki/Pimpl.h
include/loki/RefToValue.h
include/loki/Register.h
include/loki/SPCachedFactory.h
include/loki/SafeFormat.h
include/loki/ScopeGuard.h
include/loki/Sequence.h
include/loki/Singleton.h
include/loki/SmallObj.h
include/loki/SmartPtr.h
include/loki/StrongPtr.h
include/loki/Threads.h
include/loki/Tuple.h
include/loki/TypeManip.h
include/loki/TypeTraits.h
include/loki/Typelist.h
include/loki/TypelistMacros.h
include/loki/Visitor.h

include/loki/flex/allocatorstringstorage.h
include/loki/flex/cowstringopt.h
include/loki/flex/flex_string.h
include/loki/flex/flex_string_details.h
include/loki/flex/flex_string_shell.h
include/loki/flex/simplestringstorage.h
include/loki/flex/smallstringopt.h
include/loki/flex/vectorstringstorage.h

include/loki/static_check.h

include/loki/yasli/platform.h
include/loki/yasli/random.h
include/loki/yasli/yasli_fill_iterator.h
include/loki/yasli/yasli_memory.h
include/loki/yasli/yasli_protocols.h
include/loki/yasli/yasli_traits.h
include/loki/yasli/yasli_vector.h

#src/OrderedStatic.cpp
#src/SafeFormat.cpp
src/Singleton.cpp
src/SmallObj.cpp
#src/SmartPtr.cpp
#src/StrongPtr.cpp
)

# QT4_WRAP_CPP(TORA_LOKI_MOC_SOURCES ${TORA_LOKI_MOC_CLASSES})

SET(TORA_LOKI_LIB "tora_loki_lib")
ADD_LIBRARY(${TORA_LOKI_LIB} STATIC ${TORA_LOKI_LIB_SOURCES} )#${TORA_LOKI_MOC_SOURCES})
