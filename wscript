#!/usr/bin/env python
# encoding: utf-8

from waflib.Configure import conf
from waflib import Options
from waflib.Errors import ConfigurationError
from waflib.Task import Task
from waflib.Utils import subprocess, is_win32, to_list
from waflib import Logs as log
from waflib import Utils
import sys
import os
from os import popen
import shutil

top     = '.'
VERSION = '1.4.4svn'
APPNAME = 'tora'
out     = 'build'

if sys.platform == 'darwin':
    APPNAME = 'TOra'

def options(opt):
    opt.load('compiler_cxx qt4')
    opt.add_option('--without-oracle', action='store_true', help='Disable Oracle support')
    opt.add_option('--with-qscintilla', action='store', help='Set Qscintilla dir')
    opt.add_option('--with-oracle', action='store', help='Set Oracle home dir (defaults to ORACLE_HOME)')
    opt.add_option('--with-pgconfig', action='store', help='Path to PostgreSQL\'s pg_config')
    opt.add_option('--with-pg-libpath', action='store', help='Path to Postgre lib dir')

@conf
def check_postgres(conf):
    pg_config = Options.options.with_pgconfig
    if not pg_config:
        pg_config = conf.find_program('pg_config', var='PG_CONFIG', mandatory=False)

    if pg_config:
        pg_libdir = popen("%s --libdir" % pg_config).readline().strip()
        conf.env.append_value("LIBPATH_PG", pg_libdir)
        conf.env.LIB_PG = "pq"
        pg_includedir = popen("%s --includedir" % pg_config).readline().strip()
        conf.env.append_value("INCLUDES_PG", pg_includedir)

        conf.msg('Found Postgres libraries', pg_libdir)
        conf.msg('Found Postgres includes', pg_includedir)
        
    if Options.options.with_pg_libpath:
        conf.env.LIBPATH_PG = Options.options.with_pg_libpath
        
    if is_win32:
        conf.env.LIB_PG = "libpq"
        if is_msvc(conf.env):
            conf.env.append_value('LIB_PG', 'Secur32')
            conf.env.append_value('LIB_PG', 'UserEnv')
            conf.env.append_value('LIB_PG', 'Advapi32')
            conf.env.append_value('LIB_PG', 'Shell32')

    conf.check_cxx(
        uselib='PG',
        header_name='libpq-fe.h',
        define_name='HAVE_POSTGRESQL_LIBPQ_FE_H',
        msg='Checking for libpq-fe.h',
        mandatory=True)

    conf.check_cxx(
        use='PG',
        define_name='LIBPQ_DECL_CANCEL',
        execute=False,
        fragment='''#include <libpq-fe.h>
                    int main() { PQcancel; }''',
        msg='Checking libpq defines PQcancel',
        mandatory=False)

@conf
def check_oracle(conf):
    have_oracle = True
    if conf.options.without_oracle:
        conf.msg('Checking for Oracle', 'no', 'YELLOW')
        conf.define('TO_NO_ORACLE', 1)
        have_oracle = False
    else:
        pass

    conf.HAVE_ORACLE = have_oracle

@conf
def check_qscintilla(conf):
    includes = []
    libpath  = []
    
    if conf.options.with_qscintilla:
        qdir = conf.options.with_qscintilla
        if os.path.exists(os.path.join(qdir, 'Qt4')):
            qdir = os.path.join(qdir, 'Qt4')
        includes = [qdir]
        libpath  = [qdir]

    try:
        conf.check(
            features='cxx cxxprogram',
            header_name='Qsci/qscilexersql.h',
            uselib_store='QSCINTILLA2',
            lib=['qscintilla2'],
            use='QTCORE QTGUI',
            libpath=libpath,
            includes=includes,
            fragment='''
#include <Qsci/qscilexersql.h>
int main() { QsciLexerSQL sqlLexer(0); }
''',
            msg='Checking for Qscintilla',
            mandatory=True)
    except ConfigurationError:
        log.error('''Couldn't compile a simple QScintilla application.
See config.log or specify its location with --with-qscintilla=/path/to/qscintilla

You can download qscintilla here:
http://www.riverbankcomputing.co.uk/software/qscintilla/

Or, alternatively see if your OS offers a QScintilla 2 package with development headers.''')
        raise


def configure(conf):
    if sys.platform == 'darwin':
        # must build i386 on mac due to an unfixed bug in Oracle's x64 client
        conf.env.ARCH = ['i386']

    conf.load('compiler_cxx qt4')
    conf.load('slow_qt4')
    
    # preserve these values for later
    if hasattr(Options.options, 'qtdir'):
        conf.env.QTDIR = Options.options.qtdir
    if hasattr(Options.options, 'qt_lib'):
        conf.env.QTDIR = Options.options.qt_lib
    if hasattr(Options.options, 'qt_bin'):
        conf.env.QTDIR = Options.options.qt_bin

    if is_msvc(conf.env):
        # add exception support
        conf.env.append_value('CXXFLAGS', ['/EHsc'])
        conf.env.append_value('LIB_TORAMSVC', 'ws2_32')
        # provides a main entry point for windows
        conf.env.append_value('LIB_TORAMSVC', 'qtmain')
        conf.env.append_value('CXXFLAGS', conf.env.CFLAGS_CRT_MULTITHREADED_DLL)
        conf.env.append_value('LINKFLAGS_TORAMSVC', conf.env.CFLAGS_WINDOWS)

    conf.check_qscintilla()
    conf.check_postgres()
    conf.check_oracle()

    conf.define('VERSION', VERSION)
    conf.define('SCI_LEXER', 1)
    conf.define('TOMONOLITHIC', 1)
    conf.define('DEFAULT_PLUGIN_DIR', conf.env.LIBDIR)

    conf.write_config_header('config.h')
    # expected on the command line
    conf.define('TOVERSION', VERSION)
    if is_msvc(conf.env):
        conf.define('QSCINTILLA_DLL', 1)

    conf.env.TORA_QTLIBS = to_list('QTCORE QTGUI QTSQL QTNETWORK QTXML')
    libs = conf.env.TORA_QTLIBS + ['QSCINTILLA2', 'PG', 'TORAMSVC']
    if conf.env.HAVE_ORACLE:
        libs.append(['clntsh', 'ociei'])
    conf.env.TORA_LIBS = libs

def build(bld):
    # make sure the source tree is clean
    if bld.path.ant_glob('**/moc_*.cpp **/ui_*.h'):
        raise ConfigurationError('Old build files found. Remove any moc_*.cpp or ui_*.h ' \
                                     'files (or checkout a clean copy) before building with waf.')

    conditional_oracle_sources = bld.path.ant_glob(
        'src/tooracleconnection.cpp '
            'src/tooracleconnection_trotl.cpp '
            'src/tooracleextract.cpp '
            'src/tooraclepreload.cpp '
            'src/tooraclesettingui.ui ')
    conditional_teradata_sources = bld.path.ant_glob('src/toteradataconnection.cpp')
    simple_query_sources = bld.path.ant_glob('src/*simplequery*')
    oci_stub_sources = bld.path.ant_glob('src/toocistub.cpp src/toociwrapper.h')
    other_weird_stuff = bld.path.ant_glob('src/tofyracleconnection.cpp src/tosqlparsetest.cpp')

    sources = set(bld.path.ant_glob(
            'src/*.cpp src/tora.qrc src/*.ui '
                'src/loki/src/*.cpp '
                'src/migratetool/*.cpp '
                'src/shortcuteditor/*.cpp src/shortcuteditor/*.ui '
                'src/docklets/toviewconnections.cpp src/docklets/toviewdirectory.cpp'))
    sources -= set(conditional_oracle_sources +
                   conditional_teradata_sources +
                   simple_query_sources +
                   oci_stub_sources +
                   other_weird_stuff)

    if is_win32:
        sources = sources.union(bld.path.ant_glob('src/windows/tora.rc'))
                   
    # Write build/build.env. This is useful for debugging.
    env_file = open(os.path.join(bld.bldnode.abspath(), 'build.env'), 'w')
    env_file.write(unicode(bld.env) + '\n')
    env_file.close()

    params = dict(
        features = 'qt4 cxx cxxprogram',
        uselib   = bld.env.TORA_LIBS,
        source   = sources,
        target   = APPNAME,
        includes = ['.', 'src', 'src/loki/include', 'src/migratetool', 'src/shortcuteditor', 'src/docklets'],
        defines  = ['HAVE_CONFIG_H', 'WAF'],
        lang     = bld.path.ant_glob('src/i18n/*.ts'),
        )

    if sys.platform == 'darwin':
        params.update(
            dict(
                mac_app=True,
                mac_plist='osx_tools/Info.plist',
                mac_resources='src/icons/tora.icns',
                ))

    # adds tasks for the main program build
    bld(**params)

    if sys.platform == 'darwin':
        # add additional install files so after the 'install' command
        # finishes, the bundle will launch correctly. See c_osx.py for
        # examples.

        inst_to   = getattr(bld, 'install_path', '/Applications') + '/TOra.app/Contents/Frameworks'
        sources   = []

        # generate install time tasks to copy libraries
        for lib in ['QSCINTILLA2', 'PG']:
            libname = 'lib' + to_list(bld.env['LIB_%s' % lib])[0] + '.dylib'
            path = os.path.join(to_list(bld.env['LIBPATH_%s' % lib])[0], libname)
            source = bld.root.find_node(path)
            bld.install_files(inst_to, source, chmod=Utils.O755)
            sources.append(source)

        # generate build task to run install_name_tool and macdeployqt on build/TOra.app
        bld.env.INSTALL_NAME_TOFIX = sources
        bld.add_post_fun(fixup_bundle)
    
    if is_win32:
        # add tasks for windows to create build/release, a folder from where
        # you can run tora.exe without manually copying DLLs in place.
        qtdir = qtdir_node(bld)
        for qtlib in bld.env.TORA_QTLIBS:
            dll_name = bld.env['LIB_%s' % qtlib][0]
            bld().create_task('copy_win_release',
                              qtdir.find_node('bin/%s.dll' % dll_name),
                              bld.path.find_or_declare('release/%s.dll' % dll_name))
        # installs to lib, not bin
        bld().create_task('copy_win_release',
                          qtdir.find_node('lib/qscintilla2.dll'),
                          bld.path.find_or_declare('release/qscintilla2.dll'))
        sqldriver_node = bld.path.find_or_declare('release/sqldrivers')
        for plugin in qtdir.ant_glob('plugins/sqldrivers/*.dll'):
            bld().create_task('copy_win_release',
                              plugin,
                              sqldriver_node.find_or_declare(plugin.name))
        bld().create_task('copy_win_release',
                          bld.bldnode.find_node(APPNAME + '.exe'),
                          bld.bldnode.find_or_declare('release/%s.exe' % APPNAME))

def is_msvc(env):
    return env.CC_NAME == 'msvc'
    
def qtdir_node(self):
    qtdir = getattr(self.env, 'QTDIR', None)
    if qtdir:
        return self.root.find_node(qtdir)
        
    qtsub = getattr(self.env, 'QT_BIN', None) or getattr(self.env, 'QT_LIB', None)
    if qtsub:
        return self.root.find_node(qtsub.parent)
    return self.root.find_node(self.env.LIBPATH_QTCORE).parent.parent

class AbsSourcePathMixin(object):
    def __str__(self):
        "string to display to the user. (Same as Task.__str__ but uses abspath for source files.)"
        env = self.env
        src_str = ' '.join([a.abspath() for a in self.inputs])
        tgt_str = ' '.join([a.nice_path(env) for a in self.outputs])
        if self.outputs: sep = ' -> '
        else: sep = ''
        return '%s: %s%s%s\n' % (self.__class__.__name__, src_str, sep, tgt_str)

def fixup_bundle(bld):
    app_dir = getattr(bld, 'install_path', '/Applications') + '/TOra.app' # path to build/TOra.app
    tora_bin = app_dir + '/Contents/MacOS/TOra'
    log.info('Fixing up install names in %s' % app_dir)

    for source in getattr(bld.env, 'INSTALL_NAME_TOFIX', []):
        # get the current install name for the library
        output = subprocess.check_output(['otool', '-D', source.abspath()])
        path = output.split('\n')[1].strip()

        # use mac tool to change the name in the new binary
        bld.exec_command(['install_name_tool',
                          '-change',
                          path,
                          '@executable_path/../Frameworks/%s' % source.name,
                          tora_bin])

    bld.exec_command(['chmod -R u+w', app_dir])
    # lastly, run qt's deployment tool. it will take care of the frameworks and whatnot.
    # would be good to find this bin in the configure phase.
    bld.exec_command(['macdeployqt', app_dir])

    # Resources folder is installed wrong, I belive this is a waf bug. todo.
    # fixed, i think
    # bld.exec_command(['mv', '-f', app_dir + '/Resources/tora.icns', app_dir + '/Contents/Resources'])

class copy_win_release(AbsSourcePathMixin, Task):
    """copy to build/release"""
    color = 'PINK'
    after = ['cxxprogram']

    def run(self):
        for source, dest in map(None, self.inputs, self.outputs):
            shutil.copy(source.abspath(), dest.abspath())


