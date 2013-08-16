
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

#include "core/toglobalsetting.h"
#include "core/utils.h"
#include "core/toconf.h"
#include "core/toconfiguration.h"
#include "core/tocache.h"
#include "core/toqvalue.h"

#include <QtGui/QColorDialog>
#include <QtCore/QString>
#include <QtGui/QFileDialog>

ConnectionColorsDialog::ConnectionColorsDialog(QWidget * parent)
    : QDialog(parent)
{
    setupUi(this);

    ConnectionColorsIterator it(toConfigurationSingle::Instance().connectionColors());
    while (it.hasNext())
    {
        it.next();
        newItem(it.key(), it.value());
    }
    treeWidget->hideColumn(2);

    connect(addButton, SIGNAL(clicked()), this, SLOT(addItem()));
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteItem()));
}

void ConnectionColorsDialog::newItem(const QString & color, const QString & desc)
{
    QTreeWidgetItem * item = new QTreeWidgetItem(treeWidget);
    item->setIcon(0, Utils::connectionColorPixmap(color));
    item->setText(1, desc);
    item->setText(2, color);
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
}

void ConnectionColorsDialog::addItem()
{
    QColor c(QColorDialog::getColor());
    if (!c.isValid())
        return;

    newItem(c.name(), c.name());
}

void ConnectionColorsDialog::deleteItem()
{
    int index = treeWidget->indexOfTopLevelItem(treeWidget->currentItem());
    delete treeWidget->takeTopLevelItem(index);
}

void ConnectionColorsDialog::accept()
{
    ConnectionColors newMap;
    QTreeWidgetItemIterator it(treeWidget);
    while (*it)
    {
        newMap[(*it)->text(2)] = (*it)->text(1);
        ++it;
    }
    toConfigurationSingle::Instance().setConnectionColors(newMap);
    QDialog::accept();
}



toGlobalSetting::toGlobalSetting(QWidget *parent, const char *name, Qt::WFlags fl)
    : QWidget(parent, fl)
    , toSettingTab("preferences.html#global")
{
    if (name)
        setObjectName(name);

    setupUi(this);
#if QT_VERSION < 0x040400
    TabbedTools->setVisible(false);
#endif

    SavePassword->setChecked(toConfigurationSingle::Instance().savePassword());
    Utils::toRefreshCreate(OptionGroup, "toRefreshCreate", QString::null, Refresh);
    DefaultSession->setText(toConfigurationSingle::Instance().defaultSession());
    Status->setValue(toConfigurationSingle::Instance().statusMessage());
    HistorySize->setValue(toConfigurationSingle::Instance().statusSave());
    IncludeDB->setChecked(toConfigurationSingle::Instance().dbTitle());
    Statusbar->setChecked(toConfigurationSingle::Instance().messageStatusbar());
    MultiLineResults->setChecked(toConfigurationSingle::Instance().multiLineResults());
    ColorizedConnections->setChecked(toConfigurationSingle::Instance().colorizedConnections());
    connect(ColorizedConnectionsConfigure, SIGNAL(clicked()),
            this, SLOT(ColorizedConnectionsConfigure_clicked()));
    RestoreSession->setChecked(toConfigurationSingle::Instance().restoreSession());
    HelpDirectory->setText(toConfigurationSingle::Instance().helpPath()/*toHelpPath()*/);
    ChangeConnection->setChecked(toConfigurationSingle::Instance().changeConnection());
    int samples = toConfigurationSingle::Instance().chartSamples();
    if (samples < 0)
    {
        UnlimitedSamples->setChecked(true);
        ChartSamples->setValue(DEFAULT_CHART_SAMPLES);
    }
    else
        ChartSamples->setValue(samples);
    samples = toConfigurationSingle::Instance().displaySamples();
    if (samples < 0)
    {
        AllSamples->setChecked(true);
        DisplaySamples->setValue(ChartSamples->value());
    }
    else
        DisplaySamples->setValue(samples);
    DefaultFormat->setCurrentIndex(toConfigurationSingle::Instance().defaultFormat());
    ToadBindings->setChecked(toConfigurationSingle::Instance().toadBindings());
    DisplayGrid->setChecked(toConfigurationSingle::Instance().displayGridlines());

    QString typ(toConfigurationSingle::Instance().sizeUnit());
    if (typ == "KB")
        SizeUnit->setCurrentIndex(1);
    else if (typ == "MB")
        SizeUnit->setCurrentIndex(2);

    Style->addItems(Utils::toGetSessionTypes());
    QString str = Utils::toGetSessionType();
    for (int i = 0; i < Style->count(); i++)
    {
        if (str == Style->itemText(i))
        {
            Style->setCurrentIndex(i);
            break;
        }
    }

#ifdef Q_OS_WIN
    MySQLHome->setEnabled(true);
    MySQLHomeBrowse->setEnabled(true);
    PgSQLHome->setEnabled(true);
    PgSQLHomeBrowse->setEnabled(true);
#endif
    OracleHome->setText(toConfigurationSingle::Instance().oracleHome());
    MySQLHome->setText(toConfigurationSingle::Instance().mysqlHome());
    PgSQLHome->setText(toConfigurationSingle::Instance().pgsqlHome());

    /** disk caching options
     */

    CacheDirectory->setText(toCache::cacheDir().absolutePath());
    DiskCaching->setChecked(toConfigurationSingle::Instance().cacheDisk());

    CustomSQL->setText(toConfigurationSingle::Instance().sqlFile());
    Locale->setText(toConfigurationSingle::Instance().locale());
}

void toGlobalSetting::sqlBrowse(void)
{
    QString str = Utils::toOpenFilename(CustomSQL->text(), QString::null, this);
    if (!str.isEmpty())
        CustomSQL->setText(str);
}

void toGlobalSetting::sessionBrowse(void)
{
    QString str = Utils::toOpenFilename(DefaultSession->text(), QString::fromLatin1("*.tse"), this);
    if (!str.isEmpty())
        DefaultSession->setText(str);
}

void toGlobalSetting::helpBrowse(void)
{
    QString str = Utils::toOpenFilename(HelpDirectory->text(), QString::fromLatin1("toc.htm*"), this);
    if (!str.isEmpty())
        HelpDirectory->setText(str);
}

void toGlobalSetting::cacheBrowse(void)
{
    QString str = Utils::toOpenFilename(CacheDirectory->text(), QString::fromLatin1(".tora_cache"), this);
    if (!str.isEmpty())
        CacheDirectory->setText(str);
}

void toGlobalSetting::oracleBrowse(void)
{
    QString str = TOFileDialog::getExistingDirectory(this, tr("Select ORACLE_HOME"), OracleHome->text());
    if (!str.isEmpty())
    	OracleHome->setText(str);
}

void toGlobalSetting::mysqlBrowse(void)
{
    QString str = TOFileDialog::getExistingDirectory(this, tr("MySQL client installation"), MySQLHome->text());
    if (str.isEmpty())
    	return;
    QFileInfo libmysql(str + QDir::separator() + "lib" + QDir::separator() + "opt", "libmysql.dll");
    if( Utils::toLibrary::isValidLibrary(libmysql))
    	MySQLHome->setText(libmysql.absolutePath());
    else
        TOMessageBox::warning(
        		Utils::toQMainWindow(),
        		QT_TRANSLATE_NOOP("toLibraryErrorA", "Library error"),
        		QT_TRANSLATE_NOOP("toLibraryErrorA", QString("Couldn't validate library file: %1").arg(libmysql.absoluteFilePath()))
        		);
}

void toGlobalSetting::pqsqlBrowse(void)
{
    QString str = TOFileDialog::getExistingDirectory(this, tr("PgSQL client installation"), PgSQLHome->text());
    if (str.isEmpty())
    	return;
    QFileInfo libpq(str + QDir::separator() + "lib", "libpq.dll");
    if( Utils::toLibrary::isValidLibrary(libpq))
    	PgSQLHome->setText(str);
    else
        TOMessageBox::warning(
        		Utils::toQMainWindow(),
        		QT_TRANSLATE_NOOP("toLibraryErrorB", "Library error"),
        		QT_TRANSLATE_NOOP("toLibraryErrorB", QString("Couldn't validate library file: %1").arg(libpq.absoluteFilePath()))
        		);
}

void toGlobalSetting::ColorizedConnectionsConfigure_clicked()
{
    ConnectionColorsDialog dia(this);
    dia.exec();
}

void toGlobalSetting::saveSetting(void)
{
    toConfigurationSingle::Instance().setOracleHome(OracleHome->text());
    toConfigurationSingle::Instance().setMysqlHome(MySQLHome->text());
    toConfigurationSingle::Instance().setPgsqlHome(PgSQLHome->text());

    toConfigurationSingle::Instance().setCacheDisk(DiskCaching->isChecked());
    toConfigurationSingle::Instance().setCacheDir(CacheDirectory->text());
    toConfigurationSingle::Instance().setSqlFile(CustomSQL->text());
    toConfigurationSingle::Instance().setDefaultSession(DefaultSession->text());
    toConfigurationSingle::Instance().setRefresh(Refresh->currentText());
    toConfigurationSingle::Instance().setSavePassword(SavePassword->isChecked());
    toConfigurationSingle::Instance().setStatusMessage(Status->value());
    toConfigurationSingle::Instance().setStatusSave(HistorySize->value());
    toConfigurationSingle::Instance().setChartSamples(ChartSamples->value());
    toConfigurationSingle::Instance().setMessageStatusbar(Statusbar->isChecked());
    toConfigurationSingle::Instance().setMultiLineResults(MultiLineResults->isChecked());
    toConfigurationSingle::Instance().setColorizedConnections(ColorizedConnections->isChecked());
    toConfigurationSingle::Instance().setRestoreSession(RestoreSession->isChecked());
    toConfigurationSingle::Instance().setDefaultFormat(DefaultFormat->currentIndex());
    toConfigurationSingle::Instance().setToadBindings(ToadBindings->isChecked());
    toConfigurationSingle::Instance().setDisplayGridlines(DisplayGrid->isChecked());
    toConfigurationSingle::Instance().setChangeConnection(ChangeConnection->isChecked());
    toConfigurationSingle::Instance().setDbTitle(IncludeDB->isChecked());
    toConfigurationSingle::Instance().setSizeUnit(SizeUnit->currentText());
    toConfigurationSingle::Instance().setHelpPath(HelpDirectory->text());

    toConfigurationSingle::Instance().setStyle(Style->currentText());
    Utils::toSetSessionType(Style->currentText());

    if (AllSamples->isChecked())
        toConfigurationSingle::Instance().setDisplaySamples(-1);
    else
        toConfigurationSingle::Instance().setDisplaySamples(DisplaySamples->value());
    if (UnlimitedSamples->isChecked())
        toConfigurationSingle::Instance().setChartSamples(-1);
    else
        toConfigurationSingle::Instance().setChartSamples(ChartSamples->value());

    toConfigurationSingle::Instance().setLocale(Locale->text());
}

void toDatabaseSetting::numberFormatChange()
{
    Decimals->setEnabled(NumberFormat->currentIndex() == 2);
}

void toDatabaseSetting::IndicateEmptyColor_clicked()
{
    QPalette palette = IndicateEmptyColor->palette();
    QColor c = QColorDialog::getColor(
                   palette.color(IndicateEmptyColor->backgroundRole()),
                   this);

    if (c.isValid())
    {
        palette.setColor(IndicateEmptyColor->backgroundRole(), c);
        IndicateEmptyColor->setPalette(palette);
    }
}

toDatabaseSetting::toDatabaseSetting(QWidget *parent, const char *name, Qt::WFlags fl)
    : QWidget(parent, fl)
    , toSettingTab("database.html")
{
    if (name)
        setObjectName(name);
    setupUi(this);

    MaxColDisp->setValue(toConfigurationSingle::Instance().maxColDisp());
    int mxNumber = toConfigurationSingle::Instance().maxNumber();
    if (mxNumber <= 0)
        ReadAll->setChecked(true);
    else
        InitialFetch->setValue(mxNumber);

    int mxContent = toConfigurationSingle::Instance().maxContent();
    if (mxContent <= 0)
    {
        MaxContent->setValue(InitialFetch->value());
        UnlimitedContent->setChecked(true);
    }
    else
        MaxContent->setValue(mxContent);

//     MaxColDisp->setValidator(new QIntValidator(MaxColDisp));
//     InitialFetch->setValidator(new QIntValidator(InitialFetch));
//     MaxContent->setValidator(new QIntValidator(InitialFetch));q

    NumberFormat->setCurrentIndex(toConfigurationSingle::Instance().numberFormat());

    Decimals->setValue(toConfigurationSingle::Instance().numberDecimals());
    if (NumberFormat->currentIndex() == 2)
        Decimals->setEnabled(true);

    AutoCommit->setChecked(toConfigurationSingle::Instance().autoCommit());
//     DontReread->setChecked(toConfigurationSingle::Instance().dontReread());
    ObjectCache->setCurrentIndex(toConfigurationSingle::Instance().objectCache());
//     BkgndConnect->setChecked(toConfigurationSingle::Instance().bkgndConnect());
    IndicateEmpty->setChecked(toConfigurationSingle::Instance().indicateEmpty());
    FirewallMode->setChecked(toConfigurationSingle::Instance().firewallMode());
    ConnTestInterval->setValue(toConfigurationSingle::Instance().connTestInterval());

    QColor nullColor;
    nullColor.setNamedColor(toConfigurationSingle::Instance().indicateEmptyColor());
    QPalette palette = IndicateEmptyColor->palette();
    palette.setColor(IndicateEmptyColor->backgroundRole(), nullColor);
    IndicateEmptyColor->setPalette(palette);

//     int val = toConfigurationSingle::Instance().autoLong();
//     AutoLong->setChecked(val);
//     MoveAfter->setValue(val);
//     KeepAlive->setChecked(toConfigurationSingle::Instance().keepAlive());

    connect(IndicateEmpty, SIGNAL(clicked(bool)),
            IndicateEmptyColor, SLOT(setEnabled(bool)));
}

// void toUpdateIndicateEmpty(void);

void toDatabaseSetting::saveSetting(void)
{
    toConfigurationSingle::Instance().setMaxColDisp(MaxColDisp->value());
    if (ReadAll->isChecked())
        toConfigurationSingle::Instance().setMaxNumber(-1);
    else
        toConfigurationSingle::Instance().setMaxNumber(InitialFetch->value());
    if (UnlimitedContent->isChecked())
        toConfigurationSingle::Instance().setMaxContent(-1);
    else
    {
        int num = InitialFetch->value();
        int maxnum = MaxContent->value();
        if (num < 0)
            maxnum = num;
        else if (num >= maxnum)
            maxnum = num + 1;
        if (maxnum != MaxContent->text().toInt())
            TOMessageBox::information(this, tr("Invalid values"),
                                      tr("Doesn't make sense to have max content less than initial\n"
                                         "fetch size. Will adjust value to be higher."),
                                      tr("&Ok"));
        toConfigurationSingle::Instance().setMaxContent(maxnum);
    }
    toConfigurationSingle::Instance().setAutoCommit(AutoCommit->isChecked());
//     toConfigurationSingle::Instance().setDontReread(DontReread->isChecked());
    toConfigurationSingle::Instance().setObjectCache((toConfiguration::ObjectCache)ObjectCache->currentIndex());
//     toConfigurationSingle::Instance().setBkgndConnect(BkgndConnect->isChecked());
//     toConfigurationSingle::Instance().setAutoLong(AutoLong->isChecked() ? MoveAfter->value() : 0);
    toConfigurationSingle::Instance().setIndicateEmpty(IndicateEmpty->isChecked());
    toConfigurationSingle::Instance().setIndicateEmptyColor(IndicateEmptyColor->palette().color(IndicateEmptyColor->backgroundRole()).name());
//     toConfigurationSingle::Instance().setKeepAlive(KeepAlive->isChecked() ? DEFAULT_KEEP_ALIVE : -1); //FIXME: there was ""
    toConfigurationSingle::Instance().setFirewallMode(FirewallMode->isChecked());
    toConfigurationSingle::Instance().setConnTestInterval(ConnTestInterval->value());

    toConfigurationSingle::Instance().setNumberFormat(NumberFormat->currentIndex());
    toConfigurationSingle::Instance().setNumberDecimals(Decimals->value());
    toQValue::setNumberFormat(NumberFormat->currentIndex(), Decimals->value());

//     toUpdateIndicateEmpty();
}

toToolSetting::toToolSetting(QWidget *parent, const char *name, Qt::WFlags fl)
    : QWidget(parent/*, name, fl*/)
    , toSettingTab("toolsetting.html")
{
    setupUi(this);

    Enabled->setSorting(0);
    ToolsMap tMap(toConfigurationSingle::Instance().tools());
    for (ToolsRegistrySing::ObjectType::iterator i = ToolsRegistrySing::Instance().begin();
            i != ToolsRegistrySing::Instance().end();
            ++i)
    {
        toTool *pTool = i.value();
        if (pTool && pTool->menuItem())
        {
            QString menuName = qApp->translate("toTool", pTool->menuItem());
            DefaultTool->addItem(menuName);
            toTreeWidgetItem *item = new toTreeWidgetItem(Enabled,
                    menuName,
                    pTool->name(),
                    i.key());
            item->setSelected(tMap[i.key()]);
        }
    }

    // set the default tool to prevent overvritting when
    // user does not change this combo box
    QString defName(toConfigurationSingle::Instance().defaultTool());
    int currIx = -1;
    if (!defName.isEmpty())
    {
        toTool *def = ToolsRegistrySing::Instance().value(toConfigurationSingle::Instance().defaultTool());
        if (def)
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
    for (QTreeWidgetItemIterator it(Enabled); (*it); it++)
    {
        if ((*it)->isSelected())
        {
            DefaultTool->insertItem(id, (*it)->text(0));
            if ((*it)->text(0) == str)
                sel = id;
            id++;
        }
    }
    DefaultTool->setCurrentIndex(sel);
}

void toToolSetting::saveSetting(void)
{
    ToolsMap tMap(toConfigurationSingle::Instance().tools());
    for (QTreeWidgetItemIterator it(Enabled); (*it); it++)
    {
        tMap[(*it)->text(2)] = (*it)->isSelected();
        /*        QString str = (*it)->text(2).toLatin1();
                str += CONF_TOOL_ENABLE;
                toConfigurationSingle::Instance().globalSetConfig(str, (*it)->isSelected() ? "Yes" : "");
        */
        if (DefaultTool->currentText() == (*it)->text(0))
            toConfigurationSingle::Instance().setDefaultTool((*it)->text(2));

    }
    toConfigurationSingle::Instance().setTools(tMap);
}
