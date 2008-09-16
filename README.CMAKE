CMAKE help and hints
--------------------------------------------------------------
There is an alternative build method since the TOra's Qt4 port.
You can use cmake for it (cmake.org).

Standard usage is very similar to configure/make/make install:
cmake [options] build_dir

where "build_dir" can be e.g.: . for TOra source root directory
or manually created directory to keep build files out of SVN copy.

Options:

Cmake installs TOra into /usr/local as default. You can change it
for your location:
-DCMAKE_INSTALL_PREFIX:PATH=/home/username/toradevelopment
If you want to set special paths for help files or whatever see
"Paths for installation" below.

Note for Windows: if you don't specify install prefix, you can get
compile-time errorrs in some cases. It looks like some MSVC
environments don't like spaces in its default paths
(C:\Program oieeek, error here).


Enable compilation with debugging symbols:
-DWANT_DEBUG=1


TOra requires QScintilla2. QScintilla2 is the new Qt4 pord of old
Qt3 release with new API. It's strongly recomended to use standalone
package of QScintilla2. Unfortunatelly it's sometimes missing in
older distros. You can use TOra internal copy of QScintilla2 with:
-DWANT_INTERNAL_QSCINTILLA=1


TOra can be built against various Oracle clients. Some clients
(e.g. instant-client) do not provide ORACLE_HOME settings. Cmake will
warn you in this case so you can specify required paths manually:
-DORACLE_PATH_INCLUDES=/usr/include/oracle/10.2.0.3/client
-DORACLE_PATH_LIB=/usr/lib/oracle/10.2.0.3/client/lib


You have to specify -DORACLE_OCI_VERSION for one of values:
[8, 8I, 9I, 10G, 10G_R2]. Example:
-DORACLE_OCI_VERSION=10G_R2


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


Search following for more info:
http://cmake.org
http://docs.scribus.net/index.php?lang=en&page=install4


Paths for installation
The base install prefix is set with -DCMAKE_INSTALL_PREFIX (see above).
You can specify installation of additional files with some variables.
Changing these variables will affects TOra compilation too.
List of the settings:
-DTORA_PLUGIN_DIR=path
    It sets the path for functionality addons (binary plugins (if it's
    used) or template description files etc.)
    The default value is CMAKE_INSTALL_PREFIX/share/tora
-DTORA_DOC_DIR=path
    It sets the path for basic system-wide documentation fles (README,
    AUTHORS, LICENSE etc.). It is not read by anybody propably but it's
    pretty standard for Linux packages ;)
    The default value is CMAKE_INSTALL_PREFIX/share/doc/tora
-DTORA_HELP_DIR=path
    It sets the location of help files (manual and API docs).
    The default value is TORA_PLUGIN_DIR/help
