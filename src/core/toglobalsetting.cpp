
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2013 Numerous Other Contributors
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
 * along with this program as the file COPYING.txt; if not, please see
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt.
 * 
 *      As a special exception, you have permission to link this program
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 * 
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "core/toglobalsetting.h"
#include "core/utils.h"
#include "core/toconf.h"
#include "core/toconfenum.h"
#include "core/toconfiguration_new.h"
#include "core/tomainwindow.h"
#include "core/tocache.h"
#include "core/toqvalue.h"
#include "ts_log/ts_log_utils.h"

#include <QtGui/QColorDialog>
#include <QtCore/QString>
#include <QtGui/QFileDialog>

#include <QtCore/QDebug>

ToConfiguration::Global s_global;

QVariant ToConfiguration::Global::defaultValue(int option) const
{
	switch(option)
	{
	// Paths
	case CustomSQL:			return QVariant(QDir::homePath() + QDir::separator() + ".torasql");
	case HelpDirectory:		return QVariant(QString("qrc:/help/toc.html"));
	case DefaultSession:		return QVariant(QDir::homePath() + QDir::separator() + ".tora.tse" );
	case CacheDirectory:		return QVariant(QString(""));
	case OracleHomeDirectory:	return QVariant(QString(""));
	case MysqlHomeDirectory:	return QVariant(QString(""));
	case PgsqlHomeDirectory:	return QVariant(QString(""));
	case GraphvizHomeDirectory:
	{
		QString defaultGvHome;
#if defined(Q_OS_WIN32)
		defaultGvHome = "C:/Program Files/Graphviz 2.28/bin";
#elif defined(Q_OS_WIN64)
		defaultGvHome = "C:/Program Files(x86)/Graphviz 2.28/bin";
#else
		defaultGvHome = "/usr/bin";
#endif
		return QVariant(defaultGvHome);
	}
	// Options: (1st column)
	case ChangeConnectionBool:	return QVariant((bool)true);
	case SavePasswordBool:		return QVariant((bool)false);
	case IncludeDbCaptionBool:	return QVariant((bool)true);
	case RestoreSessionBool:	return QVariant((bool)false);
	case ToadBindingsBool:		return QVariant((bool)false);
	case CacheDiskBool:		return QVariant((bool)true);
	case DisplayGridlinesBool:	return QVariant((bool)true);
	case MultiLineResultsBool:	return QVariant((bool)false);
	case MessageStatusbarBool:	return QVariant((bool)false);
	case ColorizedConnectionsBool:	return QVariant((bool)true);
	case ColorizedConnectionsMap:
	{
		QMap<QString, QVariant> retval;
		retval["#FF0000"] = QVariant("Production");
		retval["#00FF00"] = QVariant("Development");
		retval["#0000FF"] = QVariant("Testing");
		return retval;
	}
	// Options: (2nd column)
	case StatusMessageInt:		return QVariant((int)5);
	case HistorySizeInt:		return QVariant((int)10);
	case ChartSamplesInt:		return QVariant((int)100);
	case DisplaySamplesInt:     return QVariant((int)-1);
	case SizeUnit:			return QVariant(QString("MB"));
	case RefreshIntervalInt:	return QVariant((int)0);  // None
	case DefaultListFormat:		return QVariant((int)0);
	case Style:			return QVariant(QString(""));
	case Translation:		return QVariant(QLocale().name());
	default:
		Q_ASSERT_X( false, qPrintable(__QHERE__), qPrintable(QString("Context Editor un-registered enum value: %1").arg(option)));
		return QVariant();
	}
};


ConnectionColorsDialog::ConnectionColorsDialog(QWidget * parent)
    : QDialog(parent)
{
    setupUi(this);

    const QMap<QString, QVariant> ColorsMap(toConfigurationNewSingle::Instance().option(ToConfiguration::Global::ColorizedConnectionsMap).toMap());
    QMapIterator<QString, QVariant> it(ColorsMap);
    while (it.hasNext())
    {
        it.next();
        newItem(it.key(), it.value().toString());
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
    toConfigurationNewSingle::Instance().setOption(ToConfiguration::Global::ColorizedConnectionsMap, newMap);
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

	//CustomSQL->setText(toConfigurationNewSingle::Instance().option(ToConfiguration::Global::customSQL());
	//HelpDirectory->setText(toConfigurationNewSingle::Instance().option(ToConfiguration::Global::helpDirectory());
    //DefaultSession->setText(toConfigurationNewSingle::Instance().option(ToConfiguration::Global::defaultSession());
	CacheDirectory->setText(toCache::cacheDir().absolutePath());
#ifdef Q_OS_WIN
	MysqlHomeDirectory->setEnabled(true);
    MySQLHomeBrowse->setEnabled(true);
    PgsqlHomeDirectory->setEnabled(true);
    PgSQLHomeBrowse->setEnabled(true);
    GraphvizHomeDirectory->setEnabled(true);
    GraphvizHomeBrowse->setEnabled(true);
#endif
    //OracleHomeDirectory->setText(toConfigurationNewSingle::Instance().option(ToConfiguration::Global::oracleHome());
    //MysqlHomeDirectory->setText(toConfigurationNewSingle::Instance().option(ToConfiguration::Global::mysqlHome());
    //PgsqlHomeDirectory->setText(toConfigurationNewSingle::Instance().option(ToConfiguration::Global::pgsqlHome());
    //GraphvizHomeDirectory->setText(toConfigurationNewSingle::Instance().option(ToConfiguration::Global::graphvizHome());

    ////ChangeConnectionBool->setChecked(ToConfiguration::Global::changeConnection());
	//SavePasswordBool->setChecked(toConfigurationNewSingle::Instance().option(ToConfiguration::Global::savePassword());
    //IncludeDbCaptionBool->setChecked(toConfigurationNewSingle::Instance().option(ToConfiguration::Global::includeDbCaption());
    //RestoreSessionBool->setChecked(toConfigurationNewSingle::Instance().option(ToConfiguration::Global::restoreSession());
	//ToadBindingsBool->setChecked(toConfigurationNewSingle::Instance().option(ToConfiguration::Global::toadBindings());
	////CacheDiskBool->setChecked(toConfigurationNewSingle::Instance().option(ToConfiguration::Global::cacheDisk());
    ////DisplayGridlinesBool->setChecked(toConfigurationNewSingle::Instance().option(ToConfiguration::Global::displayGridlines());
    ////MultiLineResultsBool->setChecked(toConfigurationNewSingle::Instance().option(ToConfiguration::Global::multiLineResults());
    ////MessageStatusbarBool->setChecked(toConfigurationNewSingle::Instance().option(ToConfiguration::Global::messageStatusbar());
    ////ColorizedConnectionsBool->setChecked(toConfigurationNewSingle::Instance().option(ToConfiguration::Global::colorizedConnections());
    connect(ColorizedConnectionsMap, SIGNAL(clicked()),
            this, SLOT(ColorizedConnectionsConfigure_clicked()));

    ////StatusMessageInt->setValue(toConfigurationNewSingle::Instance().option(ToConfiguration::Global::statusMessage());
    ////HistorySizeInt->setValue(toConfigurationNewSingle::Instance().option(ToConfiguration::Global::historySize());
    int samples = toConfigurationNewSingle::Instance().option(ToConfiguration::Global::ChartSamplesInt).toInt();
    if (samples < 0)
    {
        UnlimitedSamples->setChecked(true);
        ChartSamplesInt->setValue(DEFAULT_CHART_SAMPLES);
    }
    else
        ChartSamplesInt->setValue(samples);
    samples = toConfigurationNewSingle::Instance().option(ToConfiguration::Global::DisplaySamplesInt).toInt();
    if (samples < 0)
    {
        AllSamples->setChecked(true);
        DisplaySamplesInt->setValue(ChartSamplesInt->value());
    }
    else
        DisplaySamplesInt->setValue(samples);
    // SizeUnit
    QString typ(toConfigurationNewSingle::Instance().option(ToConfiguration::Global::SizeUnit).toString());
    if (typ == "KB")
        SizeUnit->setCurrentIndex(1);
    else if (typ == "MB")
        SizeUnit->setCurrentIndex(2);
    // Refresh
    Utils::toRefreshCreate(OptionGroup, "toRefreshCreate", QString::null, RefreshIntervalInt);
    // DefaultFormat
    DefaultListFormat->setCurrentIndex(toConfigurationNewSingle::Instance().option(ToConfiguration::Global::DefaultListFormat).toInt());
    // style
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
    // Translation
    Translation->setText(toConfigurationNewSingle::Instance().option(ToConfiguration::Global::Translation).toString());

    toSettingTab::loadSettings(this);
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
    QString str = TOFileDialog::getExistingDirectory(this, tr("Select ORACLE_HOME"), OracleHomeDirectory->text());
    if (!str.isEmpty())
    	OracleHomeDirectory->setText(str);
}

void toGlobalSetting::mysqlBrowse(void)
{
    QString str = TOFileDialog::getExistingDirectory(this, tr("MySQL client installation"), MysqlHomeDirectory->text());
    if (str.isEmpty())
    	return;
    QFileInfo libmysqlopt(str + QDir::separator() + "lib" + QDir::separator() + "opt", "libmysql.dll");
    QFileInfo libmysql(str + QDir::separator() + "lib", "libmysql.dll");
    if( Utils::toLibrary::isValidLibrary(libmysqlopt))
    	MysqlHomeDirectory->setText(libmysqlopt.absolutePath());
    else if( Utils::toLibrary::isValidLibrary(libmysql))
    	MysqlHomeDirectory->setText(libmysql.absolutePath());
    else
        TOMessageBox::warning(
        		toMainWindow::lookup(),
        		QT_TRANSLATE_NOOP("toLibraryErrorA", "Library error"),
        		QT_TRANSLATE_NOOP("toLibraryErrorA", QString("Couldn't validate library file: %1").arg(libmysqlopt.absoluteFilePath()))
        		);
}

void toGlobalSetting::pqsqlBrowse(void)
{
    QString str = TOFileDialog::getExistingDirectory(this, tr("PgSQL client installation"), PgsqlHomeDirectory->text());
    if (str.isEmpty())
    	return;
    QFileInfo libpq(str + QDir::separator() + "lib", "libpq.dll");
    if( Utils::toLibrary::isValidLibrary(libpq))
    	PgsqlHomeDirectory->setText(str);
    else
        TOMessageBox::warning(
        		toMainWindow::lookup(),
        		QT_TRANSLATE_NOOP("toLibraryErrorB", "Library error"),
        		QT_TRANSLATE_NOOP("toLibraryErrorB", QString("Couldn't validate library file: %1").arg(libpq.absoluteFilePath()))
        		);
}

#ifdef Q_OS_WIN
#define DOT "dot.exe"
#else
#define DOT "dot"
#endif

void toGlobalSetting::graphvizBrowse(void)
{
	QString defaultGvHome;
#if defined(Q_OS_WIN32)
	defaultGvHome = "C:/Program Files/Graphviz 2.28/bin";
#elif defined(Q_OS_WIN64)
	defaultGvHome = "C:/Program Files(x86)/Graphviz 2.28/bin";
#else
	defaultGvHome = "/usr/bin";
#endif

	QDir gvDir(GraphvizHomeDirectory->text());
	if(GraphvizHomeDirectory->text().isEmpty() || !gvDir.exists() || !gvDir.isReadable())
		gvDir = defaultGvHome;
    QString str = TOFileDialog::getExistingDirectory(this, tr("Graphviz installation"), gvDir.absolutePath());
    if (str.isEmpty())
    	return;
    QFileInfo bindot(str + QDir::separator() + "bin", DOT);
    QFileInfo dot(str + QDir::separator(), DOT);
    if( bindot.isExecutable() && bindot.isFile())
    	GraphvizHomeDirectory->setText(bindot.absoluteDir().absolutePath());
    else if( dot.isExecutable() && dot.isFile())
    	GraphvizHomeDirectory->setText(dot.absoluteDir().absolutePath());
    else
        TOMessageBox::warning(
        		toMainWindow::lookup(),
        		QT_TRANSLATE_NOOP("toLibraryErrorB", "Executable error"),
        		QT_TRANSLATE_NOOP("toLibraryErrorB", QString("Couldn't validate executable file: %1").arg(bindot.absoluteFilePath()))
        		);
}

void toGlobalSetting::ColorizedConnectionsConfigure_clicked()
{
    ConnectionColorsDialog dia(this);
    dia.exec();
}

void toGlobalSetting::saveSetting(void)
{
	toSettingTab::saveSettings(this);
	////toConfigurationNewSingle::Instance().option(ToConfiguration::Global::setCustomSQL(CustomSQL->text());
    ////toConfigurationNewSingle::Instance().option(ToConfiguration::Global::setHelpDirectory(HelpDirectory->text());
	////toConfigurationNewSingle::Instance().option(ToConfiguration::Global::setDefaultSession(DefaultSession->text());
	////toConfigurationNewSingle::Instance().option(ToConfiguration::Global::setCacheDir(CacheDirectory->text());
	////toConfigurationNewSingle::Instance().option(ToConfiguration::Global::setOracleHome(OracleHomeDirectory->text());
	////toConfigurationNewSingle::Instance().option(ToConfiguration::Global::setMysqlHome(MysqlHomeDirectory->text());
	////toConfigurationNewSingle::Instance().option(ToConfiguration::Global::setPgsqlHome(PgsqlHomeDirectory->text());
	////toConfigurationNewSingle::Instance().option(ToConfiguration::Global::setGraphvizHome(GraphvizHomeDirectory->text());

	////toConfigurationNewSingle::Instance().option(ToConfiguration::Global::setChangeConnection(ChangeConnectionBool->isChecked());
	////toConfigurationNewSingle::Instance().option(ToConfiguration::Global::setSavePassword(SavePasswordBool->isChecked());
	////toConfigurationNewSingle::Instance().option(ToConfiguration::Global::setIncludeDbCaption(IncludeDbCaptionBool->isChecked());
	////toConfigurationNewSingle::Instance().option(ToConfiguration::Global::setRestoreSession(RestoreSessionBool->isChecked());
	////toConfigurationNewSingle::Instance().option(ToConfiguration::Global::setToadBindings(ToadBindingsBool->isChecked());
	////toConfigurationNewSingle::Instance().option(ToConfiguration::Global::setCacheDisk(CacheDiskBool->isChecked());
	////toConfigurationNewSingle::Instance().option(ToConfiguration::Global::setDisplayGridlines(DisplayGridlinesBool->isChecked());
	////toConfigurationNewSingle::Instance().option(ToConfiguration::Global::setMultiLineResults(MultiLineResultsBool->isChecked());
	////toConfigurationNewSingle::Instance().option(ToConfiguration::Global::setMessageStatusbar(MessageStatusbarBool->isChecked());
	////toConfigurationNewSingle::Instance().option(ToConfiguration::Global::setColorizedConnections(ColorizedConnectionsBool->isChecked());

    ////toConfigurationNewSingle::Instance().option(ToConfiguration::Global::setStatusMessage(StatusMessageInt->value());
    ////toConfigurationNewSingle::Instance().option(ToConfiguration::Global::setHistorySize(HistorySizeInt->value());

    if (UnlimitedSamples->isChecked())
        toConfigurationNewSingle::Instance().setOption(ToConfiguration::Global::ChartSamplesInt, -1);
    else
        toConfigurationNewSingle::Instance().setOption(ToConfiguration::Global::ChartSamplesInt, ChartSamplesInt->value());

    if (AllSamples->isChecked())
        toConfigurationNewSingle::Instance().setOption(ToConfiguration::Global::DisplaySamplesInt, -1);
    else
        toConfigurationNewSingle::Instance().setOption(ToConfiguration::Global::DisplaySamplesInt, DisplaySamplesInt->value());

    ////toConfigurationNewSingle::Instance().option(ToConfiguration::Global::setSizeUnit(SizeUnit->currentText());
    ////toConfigurationNewSingle::Instance().option(ToConfiguration::Global::setRefresh(RefreshIntervalInt->currentText());
    ////toConfigurationNewSingle::Instance().option(ToConfiguration::Global::setDefaultFormat(DefaultListFormat->currentIndex());
    ////toConfigurationNewSingle::Instance().option(ToConfiguration::Global::setStyle(Style->currentText());
    Utils::toSetSessionType(Style->currentText());
    ////toConfigurationNewSingle::Instance().option(ToConfiguration::Global::setTranslation(Translation->text());
}

toToolSetting::toToolSetting(QWidget *parent, const char *name, Qt::WFlags fl)
    : QWidget(parent/*, name, fl*/)
    , toSettingTab("toolsetting.html")
{
    setupUi(this);

    Enabled->setSorting(0);
    QMap<QString, QVariant> tMap = toConfigurationNewSingle::Instance().option(ToConfiguration::Main::ToolsMap).toMap();
    // loop over all registered tool names and check if they are enabled
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
            item->setSelected(tMap[i.key()].toBool());
        }
    }

    // set the default tool to prevent overwriting when
    // user does not change this combo box
    QString defName(toConfigurationNewSingle::Instance().option(ToConfiguration::Main::DefaultTool).toString());
    int currIx = -1;
    if (!defName.isEmpty())
    {
        toTool *def = ToolsRegistrySing::Instance().value(toConfigurationNewSingle::Instance().option(ToConfiguration::Main::DefaultTool).toString());
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
    QMap<QString, QVariant> tMap = toConfigurationNewSingle::Instance().option(ToConfiguration::Main::ToolsMap).toMap();
    for (QTreeWidgetItemIterator it(Enabled); (*it); it++)
    {
        tMap[(*it)->text(2)] = (*it)->isSelected();
        if (DefaultTool->currentText() == (*it)->text(0))
            toConfigurationNewSingle::Instance().setOption(ToConfiguration::Main::DefaultTool, (*it)->text(2));

    }
    //toSettingTab::saveSettings(this);
    toConfigurationNewSingle::Instance().setOption(ToConfiguration::Main::ToolsMap, tMap);
}
