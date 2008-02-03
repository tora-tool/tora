/*****
*
* TOra - An Oracle Toolkit for DBA's and developers
* Copyright (C) 2003-2005 Quest Software, Inc
* Portions Copyright (C) 2005 Other Contributors
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
*      these libraries without written consent from Quest Software, Inc.
*      Observe that this does not disallow linking to the Qt Free Edition.
*
*      You may link this product with any GPL'd Qt library such as Qt/Free
*
* All trademarks belong to their respective owners.
*
*****/

#include "utils.h"

#include "toconf.h"
#include "toconnection.h"
#include "toglobalsetting.h"
#include "tomain.h"
#include "toresultview.h"
#include "tosql.h"
#include "totool.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qfontdialog.h>
#include <qlabel.h>
#include <qdir.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qtextcodec.h>
#include <qtooltip.h>
#include <qvalidator.h>
#include <qvariant.h>
#include <qcolordialog.h>

#include <QString>
#include <QFileDialog>


toGlobalSetting::toGlobalSetting(QWidget *parent, const char *name, Qt::WFlags fl)
    : QWidget(parent, fl), toSettingTab("preferences.html#global")
{
    if(name)
        setObjectName(name);

    setupUi(this);

    SavePassword->setChecked(!toConfigurationSingle::Instance().globalConfig(CONF_SAVE_PWD, "").isEmpty());
    DesktopAware->setChecked(!toConfigurationSingle::Instance().globalConfig(CONF_DESKTOP_AWARE, "Yes").isEmpty());
    ToolsLeft->setChecked(!toConfigurationSingle::Instance().globalConfig(CONF_TOOLS_LEFT, "").isEmpty());
    toRefreshCreate(OptionGroup, TO_TOOLBAR_WIDGET_NAME, QString::null, Refresh);
    DefaultSession->setText(toConfigurationSingle::Instance().globalConfig(CONF_DEFAULT_SESSION, DEFAULT_SESSION));
    Status->setValue(toConfigurationSingle::Instance().globalConfig(CONF_STATUS_MESSAGE,
                     DEFAULT_STATUS_MESSAGE).toInt());
    HistorySize->setValue(toConfigurationSingle::Instance().globalConfig(CONF_STATUS_SAVE,
                          DEFAULT_STATUS_SAVE).toInt());
    IncludeDB->setChecked(!toConfigurationSingle::Instance().globalConfig(CONF_DB_TITLE, "Yes").isEmpty());
    MaximizeMain->setChecked(!toConfigurationSingle::Instance().globalConfig(CONF_MAXIMIZE_MAIN, "Yes").isEmpty());
    Statusbar->setChecked(!toConfigurationSingle::Instance().globalConfig(CONF_MESSAGE_STATUSBAR, "").isEmpty());
    RestoreSession->setChecked(!toConfigurationSingle::Instance().globalConfig(CONF_RESTORE_SESSION, "").isEmpty());
    HelpDirectory->setText(toHelpPath());
    ChangeConnection->setChecked(!toConfigurationSingle::Instance().globalConfig(CONF_CHANGE_CONNECTION, "Yes").isEmpty());
    ConnectHistory->setValue(toConfigurationSingle::Instance().globalConfig(CONF_CONNECT_SIZE, DEFAULT_CONNECT_SIZE).toInt());
    int samples = toConfigurationSingle::Instance().globalConfig(CONF_CHART_SAMPLES, DEFAULT_CHART_SAMPLES).toInt();
    if (samples < 0)
    {
        UnlimitedSamples->setChecked(true);
        ChartSamples->setValue(QString::fromLatin1(DEFAULT_CHART_SAMPLES).toInt());
    }
    else
        ChartSamples->setValue(samples);
    samples = toConfigurationSingle::Instance().globalConfig(CONF_DISPLAY_SAMPLES, DEFAULT_DISPLAY_SAMPLES).toInt();
    if (samples < 0)
    {
        AllSamples->setChecked(true);
        DisplaySamples->setValue(ChartSamples->value());
    }
    else
        DisplaySamples->setValue(samples);
    DefaultFormat->setCurrentItem(toConfigurationSingle::Instance().globalConfig(CONF_DEFAULT_FORMAT, "").toInt());
    ToadBindings->setChecked(!toConfigurationSingle::Instance().globalConfig(CONF_TOAD_BINDINGS, DEFAULT_TOAD_BINDINGS).isEmpty());
    DisplayGrid->setChecked(!toConfigurationSingle::Instance().globalConfig(CONF_DISPLAY_GRIDLINES, DEFAULT_DISPLAY_GRIDLINES).isEmpty());

    QString typ = toConfigurationSingle::Instance().globalConfig(CONF_SIZE_UNIT, DEFAULT_SIZE_UNIT);
    if (typ == "KB")
        SizeUnit->setCurrentItem(1);
    else if (typ == "MB")
        SizeUnit->setCurrentItem(2);

#ifdef ENABLE_QT_XFT

    AntialiaseFonts->setChecked(toConfigurationSingle::Instance().globalConfig(CONF_QT_XFT, DEFAULT_QT_XFT) == "true");
#else

    AntialiaseFonts->hide();
#endif

#ifdef ENABLE_STYLE

    Style->insertStringList(toGetSessionTypes());
    QString str = toGetSessionType();
    for (int i = 0;i < Style->count();i++)
    {
        if (str == Style->text(i))
        {
            Style->setCurrentItem(i);
            break;
        }
    }
#else
    Style->hide();
    StyleLabel->hide();
#endif

#ifdef Q_OS_WIN32
    DesktopAware->hide();
#endif

    DockToolbar->hide();

    if (toMonolithic())
    {
        PluginLabel->hide();
        PluginDirectory->hide();
        PluginBrowse->hide();
    }
    else
    {
        PluginDirectory->setText(toConfigurationSingle::Instance().globalConfig(CONF_PLUGIN_DIR,
                                 DEFAULT_PLUGIN_DIR));
    }

    /** disk caching options
     */

    CacheDirectory->setText(toConnection::cacheDir());
    DiskCaching->setChecked(!toConfigurationSingle::Instance().globalConfig(CONF_CACHE_DISK, DEFAULT_CACHE_DISK).isEmpty());

    CustomSQL->setText(toConfigurationSingle::Instance().globalConfig(CONF_SQL_FILE,
                       DEFAULT_SQL_FILE));
    Locale->setText(toConfigurationSingle::Instance().globalConfig(CONF_LOCALE, QTextCodec::locale()));

    SmtpServer->setText(toConfigurationSingle::Instance().globalConfig(
                            CONF_SMTP, DEFAULT_SMTP));
    SmtpPort->setValue(toConfigurationSingle::Instance().globalConfig(
                           CONF_SMTP_PORT, DEFAULT_SMTP_PORT).toInt());
}

void toGlobalSetting::pluginBrowse(void)
{
	QString str = TOFileDialog::getExistingDirectory(this, tr("Open Plugin Directory"), PluginDirectory->text());
    if (!str.isEmpty())
        PluginDirectory->setText(str);
}

void toGlobalSetting::sqlBrowse(void)
{
    QString str = toOpenFilename(CustomSQL->text(), QString::null, this);
    if (!str.isEmpty())
        CustomSQL->setText(str);
}

void toGlobalSetting::sessionBrowse(void)
{
    QString str = toOpenFilename(DefaultSession->text(), QString::fromLatin1("*.tse"), this);
    if (!str.isEmpty())
        DefaultSession->setText(str);
}

void toGlobalSetting::helpBrowse(void)
{
    QString str = toOpenFilename(HelpDirectory->text(), QString::fromLatin1("toc.htm*"), this);
    if (!str.isEmpty())
        HelpDirectory->setText(str);
}

void toGlobalSetting::cacheBrowse(void)
{
    QString str = toOpenFilename(HelpDirectory->text(), QString::fromLatin1(".tora_cache"), this);
    if (!str.isEmpty())
        CacheDirectory->setText(str);
}

void toGlobalSetting::saveSetting(void)
{
    if (!toMonolithic())
        toConfigurationSingle::Instance().globalSetConfig(CONF_PLUGIN_DIR, PluginDirectory->text());
    toConfigurationSingle::Instance().globalSetConfig(CONF_CACHE_DISK, DiskCaching->isChecked() ? "Yes" : "");
    toConfigurationSingle::Instance().globalSetConfig(CONF_CACHE_DIR, CacheDirectory->text());
    toConfigurationSingle::Instance().globalSetConfig(CONF_SQL_FILE, CustomSQL->text());
    toConfigurationSingle::Instance().globalSetConfig(CONF_DEFAULT_SESSION, DefaultSession->text());
    toConfigurationSingle::Instance().globalSetConfig(CONF_REFRESH, Refresh->currentText());
    toConfigurationSingle::Instance().globalSetConfig(CONF_SAVE_PWD, SavePassword->isChecked() ? "Yes" : "");
    toConfigurationSingle::Instance().globalSetConfig(CONF_DESKTOP_AWARE, DesktopAware->isChecked() ? "Yes" : "");
    toConfigurationSingle::Instance().globalSetConfig(CONF_STATUS_MESSAGE, QString::number(Status->value()));
    toConfigurationSingle::Instance().globalSetConfig(CONF_STATUS_SAVE, QString::number(HistorySize->value()));
    toConfigurationSingle::Instance().globalSetConfig(CONF_CHART_SAMPLES, QString::number(ChartSamples->value()));
    toConfigurationSingle::Instance().globalSetConfig(CONF_CONNECT_SIZE, QString::number(ConnectHistory->value()));
    toConfigurationSingle::Instance().globalSetConfig(CONF_MAXIMIZE_MAIN, MaximizeMain->isChecked() ? "Yes" : "");
    toConfigurationSingle::Instance().globalSetConfig(CONF_MESSAGE_STATUSBAR, Statusbar->isChecked() ? "Yes" : "");
    toConfigurationSingle::Instance().globalSetConfig(CONF_RESTORE_SESSION, RestoreSession->isChecked() ? "Yes" : "");
    toConfigurationSingle::Instance().globalSetConfig(CONF_TOOLS_LEFT, ToolsLeft->isChecked() ? "Yes" : "");
    toConfigurationSingle::Instance().globalSetConfig(CONF_DEFAULT_FORMAT,
            QString::number(DefaultFormat->currentItem()));
    toConfigurationSingle::Instance().globalSetConfig(CONF_TOAD_BINDINGS, ToadBindings->isChecked() ? "Yes" : "");
    toConfigurationSingle::Instance().globalSetConfig(CONF_DISPLAY_GRIDLINES, DisplayGrid->isChecked() ? "Yes" : "");
    toConfigurationSingle::Instance().globalSetConfig(CONF_CHANGE_CONNECTION, ChangeConnection->isChecked() ? "Yes" : "");
    toConfigurationSingle::Instance().globalSetConfig(CONF_DB_TITLE, IncludeDB->isChecked() ? "Yes" : "");
    toConfigurationSingle::Instance().globalSetConfig(CONF_SIZE_UNIT, SizeUnit->currentText());
    toConfigurationSingle::Instance().globalSetConfig(CONF_HELP_PATH, HelpDirectory->text());
#ifdef ENABLE_STYLE

    toConfigurationSingle::Instance().globalSetConfig(CONF_STYLE, Style->currentText());
    toSetSessionType(Style->currentText());
#endif
#ifdef ENABLE_QT_XFT

    toConfigurationSingle::Instance().globalSetConfig(CONF_QT_XFT, AntialiaseFonts->isChecked() ? "true" : "false");
#endif

    if (AllSamples->isChecked())
        toConfigurationSingle::Instance().globalSetConfig(CONF_DISPLAY_SAMPLES, "-1");
    else
        toConfigurationSingle::Instance().globalSetConfig(CONF_DISPLAY_SAMPLES, QString::number(DisplaySamples->value()));
    if (UnlimitedSamples->isChecked())
        toConfigurationSingle::Instance().globalSetConfig(CONF_CHART_SAMPLES, "-1");
    else
        toConfigurationSingle::Instance().globalSetConfig(CONF_CHART_SAMPLES, QString::number(ChartSamples->value()));

    toConfigurationSingle::Instance().globalSetConfig(CONF_LOCALE, Locale->text());

    toConfigurationSingle::Instance().globalSetConfig(CONF_SMTP, SmtpServer->text());
    toConfigurationSingle::Instance().globalSetConfig(CONF_SMTP_PORT, QString::number(SmtpPort->value()));
}

void toDatabaseSetting::numberFormatChange()
{
    Decimals->setEnabled(NumberFormat->currentItem() == 2);
}

void toDatabaseSetting::IndicateEmptyColor_clicked()
{
    QColor c = QColorDialog::getColor(IndicateEmptyColor->paletteBackgroundColor(), this, "IndicateEmptyColorDialog");
    if (c.isValid())
        IndicateEmptyColor->setPaletteBackgroundColor(c);
}

toDatabaseSetting::toDatabaseSetting(QWidget *parent, const char *name, Qt::WFlags fl)
        : QWidget(parent/*, name, fl*/), toSettingTab("database.html")
{
    setupUi(this);

    MaxColDisp->setText(toConfigurationSingle::Instance().globalConfig(CONF_MAX_COL_DISP,
                        DEFAULT_MAX_COL_DISP));
    QString str = toConfigurationSingle::Instance().globalConfig(CONF_MAX_NUMBER, DEFAULT_MAX_NUMBER);
    if (str.toInt() <= 0)
        ReadAll->setChecked(true);
    else
        InitialFetch->setText(str);

    str = toConfigurationSingle::Instance().globalConfig(CONF_MAX_CONTENT, DEFAULT_MAX_CONTENT);
    if (str.toInt() <= 0)
    {
        MaxContent->setText(InitialFetch->text());
        UnlimitedContent->setChecked(true);
    }
    else
        MaxContent->setText(str);

    MaxColDisp->setValidator(new QIntValidator(MaxColDisp));
    InitialFetch->setValidator(new QIntValidator(InitialFetch));
    MaxContent->setValidator(new QIntValidator(InitialFetch));

    Decimals->setValue(toConfigurationSingle::Instance().globalConfig(CONF_NUMBER_DECIMALS, DEFAULT_NUMBER_DECIMALS).toInt());
    NumberFormat->setCurrentItem(toConfigurationSingle::Instance().globalConfig(CONF_NUMBER_FORMAT, DEFAULT_NUMBER_FORMAT).toInt());
    if (NumberFormat->currentItem() == 2)
        Decimals->setEnabled(true);

    AutoCommit->setChecked(!toConfigurationSingle::Instance().globalConfig(CONF_AUTO_COMMIT, "").isEmpty());
    DontReread->setChecked(!toConfigurationSingle::Instance().globalConfig(CONF_DONT_REREAD, "Yes").isEmpty());
    ObjectCache->setCurrentItem(toConfigurationSingle::Instance().globalConfig(CONF_OBJECT_CACHE, DEFAULT_OBJECT_CACHE).toInt());
    BkgndConnect->setChecked(!toConfigurationSingle::Instance().globalConfig(CONF_BKGND_CONNECT, "").isEmpty());
    IndicateEmpty->setChecked(!toConfigurationSingle::Instance().globalConfig(CONF_INDICATE_EMPTY, "").isEmpty());

    QColor nullColor;
    nullColor.setNamedColor(toConfigurationSingle::Instance().globalConfig(CONF_INDICATE_EMPTY_COLOR, "#f2ffbc"));
    IndicateEmptyColor->setPaletteBackgroundColor(nullColor);

    int val = toConfigurationSingle::Instance().globalConfig(CONF_AUTO_LONG, "0").toInt();
    AutoLong->setChecked(val);
    MoveAfter->setValue(val);
    KeepAlive->setChecked(!toConfigurationSingle::Instance().globalConfig(CONF_KEEP_ALIVE, "").isEmpty());
}

void toUpdateIndicateEmpty(void);

void toDatabaseSetting::saveSetting(void)
{
    toConfigurationSingle::Instance().globalSetConfig(CONF_MAX_COL_DISP, MaxColDisp->text());
    if (ReadAll->isChecked())
        toConfigurationSingle::Instance().globalSetConfig(CONF_MAX_NUMBER, "-1");
    else
        toConfigurationSingle::Instance().globalSetConfig(CONF_MAX_NUMBER, InitialFetch->text());
    if (UnlimitedContent->isChecked())
        toConfigurationSingle::Instance().globalSetConfig(CONF_MAX_CONTENT, "-1");
    else
    {
        int num = InitialFetch->text().toInt();
        int maxnum = MaxContent->text().toInt();
        if (num < 0)
            maxnum = num;
        else if (num >= maxnum)
            maxnum = num + 1;
        if (maxnum != MaxContent->text().toInt())
            TOMessageBox::information(this, tr("Invalid values"),
                                      tr("Doesn't make sense to have max content less than initial\n"
                                         "fetch size. Will adjust value to be higher."),
                                      tr("&Ok"));
        toConfigurationSingle::Instance().globalSetConfig(CONF_MAX_CONTENT, QString::number(maxnum));
    }
    toConfigurationSingle::Instance().globalSetConfig(CONF_AUTO_COMMIT, AutoCommit->isChecked() ? "Yes" : "");
    toConfigurationSingle::Instance().globalSetConfig(CONF_DONT_REREAD, DontReread->isChecked() ? "Yes" : "");
    toConfigurationSingle::Instance().globalSetConfig(CONF_OBJECT_CACHE, QString::number(ObjectCache->currentItem()));
    toConfigurationSingle::Instance().globalSetConfig(CONF_BKGND_CONNECT, BkgndConnect->isChecked() ? "Yes" : "");
    toConfigurationSingle::Instance().globalSetConfig(CONF_AUTO_LONG,
            AutoLong->isChecked() ? MoveAfter->cleanText() : QString::fromLatin1("0"));
    toConfigurationSingle::Instance().globalSetConfig(CONF_INDICATE_EMPTY, IndicateEmpty->isChecked() ? "Yes" : "");
    toConfigurationSingle::Instance().globalSetConfig(CONF_INDICATE_EMPTY_COLOR, IndicateEmptyColor->paletteBackgroundColor().name());
    toConfigurationSingle::Instance().globalSetConfig(CONF_KEEP_ALIVE, KeepAlive->isChecked() ? DEFAULT_KEEP_ALIVE : "");

    toConfigurationSingle::Instance().globalSetConfig(CONF_NUMBER_FORMAT, QString::number(NumberFormat->currentItem()));
    toConfigurationSingle::Instance().globalSetConfig(CONF_NUMBER_DECIMALS, QString::number(Decimals->value()));
    toQValue::setNumberFormat(NumberFormat->currentItem(), Decimals->value());

    toMainWidget()->updateKeepAlive();
    toUpdateIndicateEmpty();
}

toToolSetting::toToolSetting(QWidget *parent, const char *name, Qt::WFlags fl)
        : QWidget(parent/*, name, fl*/), toSettingTab("toolsetting.html")
{
    setupUi(this);

    std::map<QString, toTool *> &tools = toTool::tools();
    Enabled->setSorting(0);
    for (std::map<QString, toTool *>::iterator i = tools.begin();i != tools.end();i++)
    {
        if ((*i).second->menuItem())
        {
            QString menuName = qApp->translate("toTool", (*i).second->menuItem());
			DefaultTool->insertItem(menuName);
            new toTreeWidgetItem(Enabled, menuName, (*i).second->name(), (*i).first);
        }
    }

    for(QTreeWidgetItemIterator it(Enabled); (*it); it++) {
        QString tmp = (*it)->text(2).latin1();
        tmp += CONF_TOOL_ENABLE;
        if (!toConfigurationSingle::Instance().globalConfig(tmp, "Yes").isEmpty())
            (*it)->setSelected(true);
    }

    // set the default tool to prevent overvritting when
    // user does not change this combo box
    QString defName(toConfigurationSingle::Instance().globalConfig(CONF_DEFAULT_TOOL, ""));
    int currIx = -1;
    if (!defName.isEmpty())
    {
        toTool *def = tools[toConfigurationSingle::Instance().globalConfig(CONF_DEFAULT_TOOL, "")];
        currIx = DefaultTool->findText(def->name());
    }
    DefaultTool->setCurrentIndex( (currIx == -1) ? 0 : currIx );
    changeEnable();
}

void toToolSetting::changeEnable(void)
{
    QString str = DefaultTool->currentText();

    DefaultTool->clear();
    int id = 0;
    int sel = 0;
    for(QTreeWidgetItemIterator it(Enabled); (*it); it++) {
        if ((*it)->isSelected())
        {
            DefaultTool->insertItem((*it)->text(0), id);
            if ((*it)->text(0) == str)
                sel = id;
            id++;
        }
    }
    DefaultTool->setCurrentItem(sel);
}

void toToolSetting::saveSetting(void)
{
    for(QTreeWidgetItemIterator it(Enabled); (*it); it++) {
        QString str = (*it)->text(2).latin1();
        str += CONF_TOOL_ENABLE;
        toConfigurationSingle::Instance().globalSetConfig(str, (*it)->isSelected() ? "Yes" : "");

        if (DefaultTool->currentText() == (*it)->text(0))
            toConfigurationSingle::Instance().globalSetConfig(CONF_DEFAULT_TOOL, (*it)->text(2));
    }
}
