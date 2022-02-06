
set(ERMODEL_SRCS
  canvasedge.cpp
  canvaselement.cpp
  canvasnode.cpp
  canvassubgraph.cpp
  dot2qtconsts.cpp
  FontsCache.cpp
  graphedge.cpp
  graphelement.cpp
  graphexporter.cpp
  graphnode.cpp
  graphsubgraph.cpp
  dotgrammar.cpp
  dotgraphview.cpp
  pannerview.cpp
  loadagraphthread.cpp
  dotgraph.cpp
  dotgrammar.cpp
  DotGraphParsingHelper.cpp
  ##simpleprintingcommand.cpp
  layoutagraphthread.cpp
)

SET(ERMODEL_MOC_CLASSES
  canvasedge.h
  canvaselement.h
  canvasnode.h
  graphelement.h
  dotgraphview.h
  pannerview.h
  dotgraph.h
)

IF(USE_QT6)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)

  QT6_WRAP_CPP(ERMODEL_MOC_SOURCES ${ERMODEL_MOC_CLASSES})
  INCLUDE_DIRECTORIES(${QT_INCLUDES})
ELSE()
  QT5_WRAP_CPP(ERMODEL_MOC_SOURCES ${ERMODEL_MOC_CLASSES})
ENDIF()

INCLUDE_DIRECTORIES(${graphviz_INCLUDE_DIRECTORIES})

IF(Boost_FOUND)
  INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
ENDIF()

ADD_LIBRARY(ermodel STATIC ${ERMODEL_SRCS} ${ERMODEL_MOC_SOURCES})
TARGET_LINK_LIBRARIES(ermodel ${graphviz_LIBRARIES} ${QT_LIBRARIES})
