CMAKE help and hints
--------------------------------------------------------------

1. On Linux use build Tora aside from sources:

1.1 Debug Build:
  mkdir -p tora3/DBUILD
  cd tora3/DBUILD
  cmake -DBOOST_ROOT=/usr/include -DCMAKE_BUILD_TYPE=Debug ..
  make tora poracle parsing

1.2.
  Release Build:
  mkdir -p tora3/RBUILD
  cd tora3/RBUILD
  cmake -DBOOST_ROOT=/usr/include -DCMAKE_BUILD_TYPE=Release ..
  make tora poracle parsing

2. Options:
  BOOST_ROOT for some unknown reason the CMAKE's FindBoots module
  does not search in /usr/include. You must provide the path manually

  Cmake installs TOra into /usr/local as default. You can change it
  for your location:
  -DCMAKE_INSTALL_PREFIX:PATH=/home/username/toradevelopment
  If you want to set special paths for help files or whatever see
  "Paths for installation" below.

  Note for Windows: if you don't specify install prefix, you can get
  compile-time errorrs in some cases. It looks like some MSVC
  environments don't like spaces in its default paths
  (C:\Program oieeek, error here).

3. QScintilla2
  TOra requires QScintilla2. QScintilla2 is the new Qt4 port of old
  Qt3 release with new API. It's strongly recomended to use standalone
  package of QScintilla2. Unfortunatelly it's sometimes missing in
  older distros. You can use TOra internal copy of QScintilla2 with:
  -DWANT_INTERNAL_QSCINTILLA=1

4. Oracle client libs
  TOra can be built against various Oracle clients. Some clients
  (e.g. instant-client) do not provide ORACLE_HOME settings. Cmake will
  warn you in this case so you can specify required paths manually:
  -DORACLE_PATH_INCLUDES=/usr/include/oracle/10.2.0.3/client
  -DORACLE_PATH_LIB=/usr/lib/oracle/10.2.0.3/client/lib

  Or set ORACLE_HOME before runnning cmake. The main program Tora no more depends
  on libclnt.so. Only the DB provider plugin poracle.so Needs it.

5. PgSQL (support not finished yet)  
  TOra requires PostgreSQL development environment to enable
  full PgSQL support (native cancel etc.).
  This environment shoud be found automatically on UNIXes. But you can
  specify path for alternative location and/or for Windows too.
  -DPOSTGRESQL_PATH_INCLUDES=/home/devel/pgsql/include
  -DPOSTGRESQL_PATH_INCLUDES=d:\pgsql\include
  -DPOSTGRESQL_PATH_LIB=/home/devel/pgsql/lib
  Windows note: Sometimes (dunno why) it doesn't find pgsql libs.
              You can use "set LIB=%LIB%;pathtopgsqllibs" then.
  etc.
  Native PostgreSQL support can be disabled with:
  -DENABLE_PGSQL=0


