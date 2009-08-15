
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2009 Numerous Other Contributors
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation;  only version 2 of
 * the License is valid for this program.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 *      As a special exception, you have permission to link this program
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 * 
 *      Specifically you are not permitted to link this program with the
 *      Qt/UNIX, Qt/Windows or Qt Non Commercial products of TrollTech.
 *      And you are not permitted to distribute binaries compiled against
 *      these libraries. 
 * 
 *      You may link this product with any GPL'd Qt library.
 * 
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "utils.h"

#include "toabout.h"
#include "toconf.h"
#include "toconnection.h"
#include "tohighlightedtext.h"
#include "tomain.h"
#include "tosql.h"
#include "totool.h"

// qt4 in via the qtranslator
// #include "tora_toad.h"

#ifndef Q_OS_WIN32
#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <QDateTime>

#include <memory>

#include <qapplication.h>
#include <qmessagebox.h>
#include <qtextcodec.h>

#include <QString>
#include <QTranslator>
#include <QStyleFactory>
#include <QFont>
#include <QTextCodec>

#ifndef TOMONOLITHIC
#include <dlfcn.h>


#include <tosplash.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qlabel.h>
#include <QProgressBar>
#endif

bool toMonolithic(void)
{
#ifdef TOMONOLITHIC
    return true;
#else

    return false;
#endif
}

// void toUpdateIndicateEmpty(void);

int main(int argc, char **argv)
{
    toConfiguration::setQSettingsEnv();

    /*! \warning: Keep the code before QApplication init as small
        as possible. There could be serious display issues when
        you construct some Qt classes before QApplication.
        It's the same for global static stuff - some instances can
        break it (e.g. qscintilla lexers etc.).
     */
    QApplication app(argc, argv);

    QString style(toConfigurationSingle::Instance().style());
    if (!style.isEmpty())
        QApplication::setStyle(QStyleFactory::create(style));

    // Set the default codec to use for QString
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8")); 

    try
    {

// qt4        if (getenv("LANG"))
//             qApp->setDefaultCodec(QTextCodec::codecForName(getenv("LANG")));

        QTranslator torats(0);
        torats.load(toConfigurationSingle::Instance().pluginDir()/*toPluginPath()*/ + "/" + QString("tora_") + toConfigurationSingle::Instance().locale(), ".");
        qApp->installTranslator(&torats);
        QTranslator toadbindings(0);
        if (toConfigurationSingle::Instance().toadBindings())
        {
            // qt4 - hot candidate for a builtin resource
//             if (!toadbindings.load(tora_toad, sizeof(tora_toad)))
//                 printf("Internal error, couldn't load TOAD bindings");
            toadbindings.load(toConfigurationSingle::Instance().pluginDir()/*toPluginPath()*/ + "/" + "tora_toad.qm");
            qApp->installTranslator(&toadbindings);
        }

#ifndef TOMONOLITHIC
        {
            toSplash splash(NULL, "About " TOAPPNAME, false);
            splash.show();
            std::list<QString> failed;
            QString dirPath = toConfigurationSingle::Instance().pluginDir();/*toPluginPath()*/;
            QDir d(dirPath, QString::fromLatin1("*.tso"), QDir::Name, QDir::Files);
            if (!d.exists())
            {
                fprintf(stderr,
                        "Couldn't find PluginDir, falling back to default: %s\n",
                        DEFAULT_PLUGIN_DIR);
                dirPath = DEFAULT_PLUGIN_DIR;
                d.cd(dirPath);
                if (d.exists())
                    toConfigurationSingle::Instance().setPluginDir(dirPath);
                else
                    fprintf(stderr, "Invalid PluginDir.\n");
            }

            for (unsigned int i = 0;i < d.count();i++)
            {
                failed.insert(failed.end(), d.filePath(d[i]));
            }
            QProgressBar *progress = splash.progress();
            QLabel *label = splash.label();
            progress->setTotalSteps(failed.size());
            progress->setProgress(1);
            qApp->processEvents();
            bool success;
            do
            {
                success = false;
                std::list<QString> current = failed;
                failed.clear();
                for (std::list<QString>::iterator i = current.begin();i != current.end();i++)
                {
                    if (!dlopen(*i, RTLD_NOW | RTLD_GLOBAL))
                    {
                        failed.insert(failed.end(), *i);
                    }
                    else
                    {
                        success = true;
                        progress->setProgress(progress->progress() + 1);
                        QFileInfo file(*i);
                        label->setText(qApp->translate("main", "Loaded plugin %1").arg(file.fileName()));
                        qApp->processEvents();
                    }
                }
            }
            while (failed.begin() != failed.end() && success);

            for (std::list<QString>::iterator i = failed.begin();i != failed.end();i++)
                if (!dlopen(*i, RTLD_NOW | RTLD_GLOBAL))
                    fprintf(stderr, "Failed to load %s\n  %s\n",
                            (const char *)(*i), dlerror());
        }
#endif

        try
        {
            toSQL::loadSQL(toConfigurationSingle::Instance().sqlFile());
        }
        catch (...)
            {}
        toConnectionProvider::initializeAll();

        {
            QString nls = getenv("NLS_LANG");
            if (nls.isEmpty())
                nls = "american_america.UTF8";
            else
            {
                int pos = nls.lastIndexOf('.');
                if (pos > 0)
                    nls = nls.left(pos);
                nls += ".UTF8";
            }
            toSetEnv("NLS_LANG", nls);
        }

        if (toConfigurationSingle::Instance().lastVersion() != TOVERSION)
        {
            std::auto_ptr<toAbout> about ( new toAbout(toAbout::About, NULL, "About " TOAPPNAME, true));
            if (!about->exec())
            {
                exit (2);
            }
            toConfigurationSingle::Instance().setLastVersion(TOVERSION);
        }


        if (toConfigurationSingle::Instance().firstInstall().isEmpty())
        {
            toConfigurationSingle::Instance().setFirstInstall(
                QDateTime::currentDateTime().toString());
        }

        toQValue::setNumberFormat(
            toConfigurationSingle::Instance().numberFormat(),
            toConfigurationSingle::Instance().numberDecimals());

        if (qApp->argc() > 2 || (qApp->argc() == 2 && qApp->argv()[1][0] == '-'))
        {
            printf("Usage:\n\n  tora [{X options}] [connectstring]\n\n");
            exit(2);
        }
        else if (qApp->argc() == 2)
        {
            QString connect = QString::fromLatin1(qApp->argv()[1]);
            QString user;
            int pos = connect.indexOf(QString::fromLatin1("@"));
            if (pos > -1)
            {
                user = connect.left(pos);
                connect = connect.right(connect.length() - pos - 1);
            }
            else
            {
                user = connect;
                if (getenv("ORACLE_SID"))
                    connect = QString::fromLatin1(getenv("ORACLE_SID"));
            }
            if (!connect.isEmpty())
                toConfigurationSingle::Instance().setDefaultDatabase(connect);
            pos = user.indexOf(QString::fromLatin1("/"));
            if (pos > -1)
            {
                toConfigurationSingle::Instance().setDefaultPassword(user.right(user.length() - pos - 1));
                user = user.left(pos);
            }
            if (!user.isEmpty())
                toConfigurationSingle::Instance().setDefaultUser(user);
        }

        toMarkedText::setDefaultTabWidth(toConfigurationSingle::Instance().tabStop());
        toMarkedText::setDefaultTabSpaces(toConfigurationSingle::Instance().tabSpaces());

//         toUpdateIndicateEmpty();

        new toMain;

        int ret = qApp->exec();
        return ret;
    }
    catch (const QString &str)
    {
        fprintf(stderr,
                "Unhandled exception:\n\n%s\n",
                (const char *) str.toLatin1());
        TOMessageBox::critical(NULL,
                               qApp->translate("main", "Unhandled exception"),
                               str,
                               qApp->translate("main", "Exit"));
    }
    return 1;
}
