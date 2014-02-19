
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
#include "core/toconfiguration.h"
#include "core/toconfiguration_new.h"
#include "core/tomainwindow.h"
#include "core/tocache.h"
#include "core/toqvalue.h"

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
	case DefaultListFormat:		return QVariant(QString(""));
	case Style:			return QVariant(QString(""));
	case Translation:		return QVariant(QLocale().name());
	}
};


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

	CustomSQL->setText(toConfigurationSingle::Instance().customSQL());
	HelpDirectory->setText(toConfigurationSingle::Instance().helpDirectory());
    DefaultSession->setText(toConfigurationSingle::Instance().defaultSession());
	CacheDirectory->setText(toCache::cacheDir().absolutePath());
#ifdef Q_OS_WIN
	MysqlHome->setEnabled(true);
    MySQLHomeBrowse->setEnabled(true);
    PgsqlHome->setEnabled(true);
    PgSQLHomeBrowse->setEnabled(true);
    GraphvizHome->setEnabled(true);
    GraphvizHomeBrowse->setEnabled(true);
#endif
    OracleHomeDirectory->setText(toConfigurationSingle::Instance().oracleHome());
    MysqlHomeDirectory->setText(toConfigurationSingle::Instance().mysqlHome());
    PgsqlHomeDirectory->setText(toConfigurationSingle::Instance().pgsqlHome());
    GraphvizHomeDirectory->setText(toConfigurationSingle::Instance().graphvizHome());

    ChangeConnectionBool->setChecked(toConfigurationSingle::Instance().changeConnection());
	SavePasswordBool->setChecked(toConfigurationSingle::Instance().savePassword());
    IncludeDbCaptionBool->setChecked(toConfigurationSingle::Instance().includeDbCaption());
    RestoreSessionBool->setChecked(toConfigurationSingle::Instance().restoreSession());
	ToadBindingsBool->setChecked(toConfigurationSingle::Instance().toadBindings());
	CacheDiskBool->setChecked(toConfigurationSingle::Instance().cacheDisk());
    DisplayGridlinesBool->setChecked(toConfigurationSingle::Instance().displayGridlines());
    MultiLineResultsBool->setChecked(toConfigurationSingle::Instance().multiLineResults());
    MessageStatusbarBool->setChecked(toConfigurationSingle::Instance().messageStatusbar());
    ColorizedConnectionsBool->setChecked(toConfigurationSingle::Instance().colorizedConnections());
    connect(ColorizedConnectionsMap, SIGNAL(clicked()),
            this, SLOT(ColorizedConnectionsConfigure_clicked()));

    StatusMessageInt->setValue(toConfigurationSingle::Instance().statusMessage());
    HistorySizeInt->setValue(toConfigurationSingle::Instance().historySize());
    int samples = toConfigurationSingle::Instance().chartSamples();
    if (samples < 0)
    {
        UnlimitedSamples->setChecked(true);
        ChartSamplesInt->setValue(DEFAULT_CHART_SAMPLES);
    }
    else
        ChartSamplesInt->setValue(samples);
    samples = toConfigurationSingle::Instance().displaySamples();
    if (samples < 0)
    {
        AllSamples->setChecked(true);
        DisplaySamplesInt->setValue(ChartSamplesInt->value());
    }
    else
        DisplaySamplesInt->setValue(samples);
    // SizeUnit
    QString typ(toConfigurationSingle::Instance().sizeUnit());
    if (typ == "KB")
        SizeUnit->setCurrentIndex(1);
    else if (typ == "MB")
        SizeUnit->setCurrentIndex(2);
    // Refresh
    Utils::toRefreshCreate(OptionGroup, "toRefreshCreate", QString::null, RefreshIntervalInt);
    // DefaultFormat
    DefaultListFormat->setCurrentIndex(toConfigurationSingle::Instance().defaultFormat());
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
    Translation->setText(toConfigurationSingle::Instance().translation());

    {
    	static QRegExp any(".*");
    	QList<QWidget*> lst = findChildren<QWidget*>(any);
    	Q_FOREACH(QWidget *w, lst)
    	{
    		qDebug() << w->objectName();
    		if (w->objectName() == "qt_spinbox_lineedit") // internal widget inside QSpinBox
    			continue;
    		if (QComboBox *combo = qobject_cast<QComboBox*>(w))
    		{
    			try
    			{
    				QVariant v = toConfigurationNewSingle::Instance().option(combo->objectName());
    			} catch (...) {
    				qDebug() << w->objectName() << '*';
    				combo->setDisabled(true);
    			}
    		} else if (QSpinBox *spin = qobject_cast<QSpinBox*>(w))
    		{
    			try
    			{
    				QVariant v = toConfigurationNewSingle::Instance().option(spin->objectName());
    			} catch (...) {
    				qDebug() << w->objectName() << '#';
    				spin->setDisabled(true);
    			}
    		} else if (QLineEdit *edit = qobject_cast<QLineEdit*>(w))
    		{
    			try
    			{
    				QVariant v = toConfigurationNewSingle::Instance().option(edit->objectName());
    			} catch (...) {
    				qDebug() << w->objectName() << '&';
    				edit->setDisabled(true);
    			}
    		} else if (QCheckBox *checkbox = qobject_cast<QCheckBox*>(w))
    		{
    			try
    			{
    				QVariant v = toConfigurationNewSingle::Instance().option(checkbox->objectName());
    			} catch (...) {
    				qDebug() << w->objectName() << '%';
    				checkbox->setDisabled(true);
    			}
    		}

    	}
    }
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
	toConfigurationSingle::Instance().setCustomSQL(CustomSQL->text());
    toConfigurationSingle::Instance().setHelpDirectory(HelpDirectory->text());
    toConfigurationSingle::Instance().setDefaultSession(DefaultSession->text());
    toConfigurationSingle::Instance().setCacheDir(CacheDirectory->text());
    toConfigurationSingle::Instance().setOracleHome(OracleHomeDirectory->text());
    toConfigurationSingle::Instance().setMysqlHome(MysqlHomeDirectory->text());
    toConfigurationSingle::Instance().setPgsqlHome(PgsqlHomeDirectory->text());
    toConfigurationSingle::Instance().setGraphvizHome(GraphvizHomeDirectory->text());

    toConfigurationSingle::Instance().setChangeConnection(ChangeConnectionBool->isChecked());
    toConfigurationSingle::Instance().setSavePassword(SavePasswordBool->isChecked());
    toConfigurationSingle::Instance().setIncludeDbCaption(IncludeDbCaptionBool->isChecked());
    toConfigurationSingle::Instance().setRestoreSession(RestoreSessionBool->isChecked());
    toConfigurationSingle::Instance().setToadBindings(ToadBindingsBool->isChecked());
    toConfigurationSingle::Instance().setCacheDisk(CacheDiskBool->isChecked());
    toConfigurationSingle::Instance().setDisplayGridlines(DisplayGridlinesBool->isChecked());
    toConfigurationSingle::Instance().setMultiLineResults(MultiLineResultsBool->isChecked());
    toConfigurationSingle::Instance().setMessageStatusbar(MessageStatusbarBool->isChecked());
    toConfigurationSingle::Instance().setColorizedConnections(ColorizedConnectionsBool->isChecked());

    toConfigurationSingle::Instance().setStatusMessage(StatusMessageInt->value());
    toConfigurationSingle::Instance().setHistorySize(HistorySizeInt->value());

    if (UnlimitedSamples->isChecked())
        toConfigurationSingle::Instance().setChartSamples(-1);
    else
        toConfigurationSingle::Instance().setChartSamples(ChartSamplesInt->value());

    if (AllSamples->isChecked())
        toConfigurationSingle::Instance().setDisplaySamples(-1);
    else
        toConfigurationSingle::Instance().setDisplaySamples(DisplaySamplesInt->value());

    toConfigurationSingle::Instance().setSizeUnit(SizeUnit->currentText());
    toConfigurationSingle::Instance().setRefresh(RefreshIntervalInt->currentText());
    toConfigurationSingle::Instance().setDefaultFormat(DefaultListFormat->currentIndex());
    toConfigurationSingle::Instance().setStyle(Style->currentText());
    Utils::toSetSessionType(Style->currentText());
    toConfigurationSingle::Instance().setTranslation(Translation->text());
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
    int mxNumber = toConfigurationSingle::Instance().initialFetch();
    if (mxNumber <= 0)
        FetchAll->setChecked(true);
    else
        InitialFetch->setValue(mxNumber);

    int mxContent = toConfigurationSingle::Instance().initialEditorContent();
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
    CachedConnections->setValue(toConfigurationSingle::Instance().cachedConnections());
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
    toConfigurationSingle::Instance().setObjectCache((toConfiguration::ObjectCacheEnum)ObjectCache->currentIndex());

    toConfigurationSingle::Instance().setAutoCommit(AutoCommit->isChecked());
    toConfigurationSingle::Instance().setFirewallMode(FirewallMode->isChecked());
    toConfigurationSingle::Instance().setConnTestInterval(ConnTestInterval->value());
    toConfigurationSingle::Instance().setCachedConnections(CachedConnections->value());


    if (FetchAll->isChecked())
        toConfigurationSingle::Instance().setInitialFetch(-1);
    else
        toConfigurationSingle::Instance().setInitialFetch(InitialFetch->value());

    if (UnlimitedContent->isChecked())
        toConfigurationSingle::Instance().setInitialEditorContent(-1);
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
        toConfigurationSingle::Instance().setInitialEditorContent(maxnum);
    }

    toConfigurationSingle::Instance().setMaxColDisp(MaxColDisp->value());

//     toConfigurationSingle::Instance().setDontReread(DontReread->isChecked());

//     toConfigurationSingle::Instance().setBkgndConnect(BkgndConnect->isChecked());
//     toConfigurationSingle::Instance().setAutoLong(AutoLong->isChecked() ? MoveAfter->value() : 0);
    toConfigurationSingle::Instance().setIndicateEmpty(IndicateEmpty->isChecked());
    toConfigurationSingle::Instance().setIndicateEmptyColor(IndicateEmptyColor->palette().color(IndicateEmptyColor->backgroundRole()).name());
//     toConfigurationSingle::Instance().setKeepAlive(KeepAlive->isChecked() ? DEFAULT_KEEP_ALIVE : -1); //FIXME: there was ""



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
