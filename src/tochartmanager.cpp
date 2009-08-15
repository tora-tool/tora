
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

#include "tochartmanager.h"
#include "utils.h"

#include "tobarchart.h"
#include "toconf.h"
#include "toconnection.h"
#include "tolinechart.h"
#include "tomain.h"
#include "toresultview.h"
#include "tosmtp.h"
#include "totool.h"

#include <ctype.h>

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qvalidator.h>
#include <QMdiArea>

#include <QString>
#include <QPixmap>

#include "icons/chart.xpm"
#include "icons/fileopen.xpm"
#include "icons/refresh.xpm"

class toChartTool : public toTool
{
protected:
    toChartHandler *Handler;
    toChartManager *Window;
public:
    toChartTool()
            : toTool(930, "Chart Manager")
    {
        Window = NULL;
    }
    virtual QWidget *toolWindow(QWidget *, toConnection &)
    {
        if (!Handler)
            return NULL;

        if (Window)
        {
            Window->setFocus(Qt::OtherFocusReason);
            // already opened, prevent creating new sub window
            return 0;
        }

        Window = new toChartManager(toMainWidget()->workspace());
        Window->setWindowTitle(qApp->translate("toChartTool", "Chart Manager"));
        Window->setWindowIcon(QPixmap(const_cast<const char**>(chart_xpm)));
        Window->refresh();
        return Window;
    }
    virtual void customSetup()
    {
        toMainWidget()->getEditMenu()->addAction(
            QIcon(chart_xpm),
            qApp->translate("toChartTool", "Chart Manager..."),
            this,
            SLOT(createWindow()));

        Handler = new toChartHandler();
    }
    void closeWindow(void)
    {
        Window = NULL;
    }
    void closeHandler(void)
    {
        Handler = NULL;
    }

    toChartHandler *handler()
    {
        return Handler;
    }
    toChartManager *manager()
    {
        return Window;
    }

    virtual bool canHandle(toConnection &)
    {
        return true;
    }
    virtual void closeWindow(toConnection &connection){};
};

static toChartTool ChartTool;

toChartAlarm::toChartAlarm(QWidget *parent,
                           const char *name,
                           bool modal,
                           Qt::WFlags fl)
        : QDialog(parent)
{
    setupUi(this);
}

void toChartAlarm::changeValue(int val)
{
    if (val == 1)
        ExtraLabel->setText(qApp->translate("toChartAlarm", "Email"));
    else
        ExtraLabel->setText(QString::null);

    Extra->setEnabled(val == 1);
}

toChartSetup::toChartSetup(toLineChart *chart,
                           QWidget* parent,
                           const char* name,
                           bool modal)
        : QDialog(parent)
{

    setupUi(this);
    Chart = chart;

    connect(BrowseButton, SIGNAL(clicked()), this, SLOT(browseFile()));
    connect(AddButton, SIGNAL(clicked()), this, SLOT(addAlarm()));
    connect(RemoveButton, SIGNAL(clicked()), this, SLOT(removeAlarm()));
    connect(ModifyButton, SIGNAL(clicked()), this, SLOT(modifyAlarm()));
}


void toChartSetup::browseFile()
{
    QString str = toOpenFilename(Filename->text(), QString::fromLatin1("*.csv"), this);
    if (!str.isEmpty())
        Filename->setText(str);
}


QString toChartSetup::modifyAlarm(const QString &str, bool &persistent)
{
    toChartManager::chartAlarm alarm(str, persistent);
    toChartAlarm diag(this, NULL, true);
    diag.Operation->setCurrentIndex((int)alarm.Operation);
    diag.Comparison->setCurrentIndex((int)alarm.Comparison);
    diag.Action->setCurrentIndex((int)alarm.Action);
    diag.Value->setText(QString::number(alarm.Value));
    diag.changeValue((int)alarm.Action);
    diag.Value->setValidator(new QDoubleValidator(diag.Value));
    diag.Extra->setText(alarm.Extra);
    std::list<int>::iterator sel = alarm.Columns.begin();
    diag.Charts->addColumn(qApp->translate("toChartSetup", "Charts"));
    diag.Charts->setSelectionMode(toTreeWidget::Multi);
    toTreeWidgetItem *item = NULL;
    std::list<QString>::iterator lab = Chart->labels().begin();
    for (int i = 0;lab != Chart->labels().end();i++, lab++)
    {
        item = new toTreeWidgetItem(diag.Charts, item, *lab);
        if (sel != alarm.Columns.end() && *sel == i)
        {
            item->setSelected(true);
            sel++;
        }
    }
    diag.Persistent->setChecked(persistent);
    if (diag.exec())
    {
        persistent = diag.Persistent->isChecked();
        int i = 0;
        alarm.Columns.clear();
        for (toTreeWidgetItem *item = diag.Charts->firstChild();item;item = item->nextSibling())
        {
            if (item->isSelected())
                alarm.Columns.insert(alarm.Columns.end(), i);
            i++;
        }
        alarm.Action = (toChartManager::action)diag.Action->currentIndex();
        alarm.Operation = (toChartManager::chartAlarm::operation)diag.Operation->currentIndex();
        alarm.Comparison = (toChartManager::chartAlarm::comparison)diag.Comparison->currentIndex();
        alarm.Value = diag.Value->text().toDouble();
        alarm.Extra = diag.Extra->text();
        return alarm.toString();
    }
    return QString::null;
}

void toChartSetup::addAlarm()
{
    toChartManager::chartAlarm alarm;
    QString str = modifyAlarm(alarm.toString(), alarm.Persistent);
    if (!str.isEmpty())
        new toTreeWidgetItem(Alarms, str, alarm.Persistent ? qApp->translate("toChartSetup", "Persistent") : qApp->translate("toChartSetup", "Temporary"));
}

void toChartSetup::modifyAlarm()
{
    toTreeWidgetItem *item = Alarms->selectedItem();
    if (item)
    {
        bool pers = item->text(1) == qApp->translate("toChartSetup", "Persistent");
        QString str = modifyAlarm(item->text(0), pers);
        if (!str.isEmpty())
        {
            item->setText(0, str);
            item->setText(1, pers ? qApp->translate("toChartSetup", "Persistent") : qApp->translate("toChartSetup", "Temporary"));
        }
    }
}

void toChartSetup::removeAlarm()
{
    delete Alarms->selectedItem();
}

toChartReceiver::toChartReceiver(toChartHandler *parent, toLineChart *chart)
        : QObject(parent)
{
    Parent = parent;
    Chart = chart;
    Result = NULL;
    connect(Chart, SIGNAL(valueAdded(std::list<double> &, const QString &)),
            this, SLOT(valueAdded(std::list<double> &, const QString &)));
}

toResult *toChartReceiver::result(void)
{
    if (!Result)
        Result = dynamic_cast<toResult *>(Chart);
    return Result;
}

QString toChartReceiver::name(void)
{
    if (!Result || Result->sqlName().isEmpty())
        return QString::null;
    try
    {
        LastName = Result->connection().description(false) + QString(":") + QString(Result->sqlName());
    }
    catch (...)
        {}
    return LastName;
}

void toChartReceiver::valueAdded(std::list<double> &value, const QString &xValues)
{
    Parent->valueAdded(Chart, name(), value, xValues);
}

toChartManager::toChartManager(QWidget *main)
        : QWidget(main), toHelpContext(QString::fromLatin1("chartmanager.html"))
{

    QVBoxLayout *vbox = new QVBoxLayout;

    QToolBar *toolbar = toAllocBar(this, tr("Chart Manager"));
    vbox->addWidget(toolbar);

    toolbar->addAction(QIcon(refresh_xpm),
                       tr("Refresh list"),
                       this,
                       SLOT(refresh()));

    toolbar->addSeparator();

    toolbar->addAction(QIcon(fileopen_xpm),
                       tr("Open tracker file"),
                       this,
                       SLOT(openChart()));
    toolbar->addAction(QIcon(chart_xpm),
                       tr("Setup chart"),
                       this,
                       SLOT(setupChart()));

    toolbar->addWidget(new toSpacer());

    List = new toListView(this);
    List->addColumn(tr("Connection"));
    List->addColumn(tr("Title"));
    List->addColumn(tr("ID"));
    List->addColumn(tr("Tracking"));
    List->addColumn(tr("Alarms"));
    List->setSorting(2);
    List->setSelectionMode(toTreeWidget::Single);
    vbox->addWidget(List);

    connect(&Refresh, SIGNAL(timeout()), this, SLOT(refresh()));

    setFocusProxy(List);

    vbox->setSpacing(0);
    vbox->setContentsMargins(0, 0, 0, 0);
    setLayout(vbox);
}

toChartManager::~toChartManager()
{
    ChartTool.closeWindow();
}

void toChartHandler::addChart(toLineChart *chart)
{
    Charts.insert(Charts.end(), new toChartReceiver(this, chart));
    toChartManager *manager = ChartTool.manager();
    if (manager)
    {
        manager->Refresh.setSingleShot(true);
        manager->Refresh.start(1);
    }
}

void toChartHandler::removeChart(toLineChart *chart)
{
    for (std::list<toChartReceiver *>::iterator i = Charts.begin();i != Charts.end();i++)
    {
        if ((*i)->chart() == chart)
        {
            QString name = (*i)->name();
            if (!name.isNull())
            {
                std::map<QString, std::list<toChartManager::chartAlarm> >::iterator fnda = Alarms.find(name);
                if (fnda != Alarms.end())
                {
                    bool any = false;
                    for (std::list<toChartManager::chartAlarm>::iterator j = (*fnda).second.begin();j != (*fnda).second.end();)
                    {
                        if (!(*j).Persistent)
                        {
                            (*fnda).second.erase(j);
                            j = (*fnda).second.begin();
                        }
                        else
                        {
                            j++;
                            any = true;
                        }
                    }
                    if (!any)
                        Alarms.erase(fnda);
                }

                std::map<QString, toChartManager::chartTrack>::iterator fndt = Files.find(name);
                if (fndt != Files.end())
                {
                    if (!(*fndt).second.Persistent)
                        Files.erase(fndt);
                }
            }

            delete *i;
            Charts.erase(i);
            toChartManager *manager = ChartTool.manager();
            if (manager)
            {
                manager->Refresh.setSingleShot(true);
                manager->Refresh.start(1);
            }
            return ;
        }
    }
}

toChartManager::alarmSignal::alarmSignal(void)
{
    Action = Ignore;
}

toChartManager::chartAlarm::chartAlarm(const QString &inp, bool pers)
{
    char oper[100];
    char cols[10000];
    char comp[100];
    char act[100];
    char extra[10000];
    Signal = false;
    Persistent = pers;

    // The \01 is just one character unlikely to be in an email address
    int ret = sscanf(inp.toUtf8(), "%s %s %s %lf %s %[^\01]", oper, cols, comp, &Value, act, extra);
    if (ret != 5 && ret != 6)
    {
        Operation = Any;
        Value = 0;
        Comparison = Equal;
        Action = Ignore;
        return ;
    }

    QString t = QString::fromLatin1(oper);
    if (t == QString::fromLatin1("min"))
        Operation = Min;
    else if (t == QString::fromLatin1("all"))
        Operation = All;
    else if (t == QString::fromLatin1("sum"))
        Operation = Sum;
    else if (t == QString::fromLatin1("average"))
        Operation = Average;
    else if (t == QString::fromLatin1("max"))
        Operation = Max;
    else
        Operation = Any;

    t = QString::fromUtf8(cols);
    if (t.length() > 2)
    {
        QStringList lst = t.mid(1, t.length() - 2).split(",");
        for (int i = 0;i < lst.count();i++)
            Columns.insert(Columns.end(), lst[i].toInt());
        Columns.sort();
    }

    t = QString::fromLatin1(comp);
    if (t == QString::fromLatin1("="))
    {
        Comparison = Equal;
    }
    else if (t == QString::fromLatin1("!="))
    {
        Comparison = NotEqual;
    }
    else if (t == QString::fromLatin1("<"))
    {
        Comparison = Less;
    }
    else if (t == QString::fromLatin1(">"))
    {
        Comparison = Greater;
    }
    else if (t == QString::fromLatin1("<="))
    {
        Comparison = LessEqual;
    }
    else if (t == QString::fromLatin1(">="))
    {
        Comparison = GreaterEqual;
    }
    else
    {
        Comparison = Equal;
    }

    t = QString::fromLatin1(act);
    if (t == QString::fromLatin1("StatusMessage"))
        Action = StatusMessage;
    else if (t == QString::fromLatin1("Email"))
        Action = Email;
    else
        Action = Ignore;
    if (ret == 6)
        Extra = QString::fromUtf8(extra);
}

toChartManager::chartAlarm::chartAlarm()
{
    Signal = false;
    Operation = Any;
    Comparison = Equal;
    Action = StatusMessage;
    Value = 0;
    Persistent = true;
}

toChartManager::chartAlarm::chartAlarm(operation oper, comparison comp, action act,
                                       double value, std::list<int> &cols, const QString &extra,
                                       bool persistent)
{
    Signal = false;
    Operation = oper;
    Comparison = comp;
    Action = act;
    Value = value;
    Columns = cols;
    Persistent = persistent;
    Extra = extra;
}

QString toChartManager::chartAlarm::toString(void)
{
    QString t;
    switch (Operation)
    {
    case Any:
        t = QString::fromLatin1("any");
        break;
    case All:
        t = QString::fromLatin1("all");
        break;
    case Sum:
        t = QString::fromLatin1("sum");
        break;
    case Average:
        t = QString::fromLatin1("average");
        break;
    case Max:
        t = QString::fromLatin1("max");
        break;
    case Min:
        t = QString::fromLatin1("min");
        break;
    }
    t += QString::fromLatin1(" (");
    bool first = true;
    for (std::list<int>::iterator i = Columns.begin();i != Columns.end();i++)
    {
        if (first)
            first = false;
        else
            t += QString::fromLatin1(",");
        t += QString::number(*i);
    }
    t += QString::fromLatin1(")");
    switch (Comparison)
    {
    case Equal:
        t += QString::fromLatin1(" = ");
        break;
    case NotEqual:
        t += QString::fromLatin1(" != ");
        break;
    case Less:
        t += QString::fromLatin1(" < ");
        break;
    case Greater:
        t += QString::fromLatin1(" > ");
        break;
    case LessEqual:
        t += QString::fromLatin1(" <= ");
        break;
    case GreaterEqual:
        t += QString::fromLatin1(" >= ");
        break;
    }
    t += QString::number(Value);
    switch (Action)
    {
    case StatusMessage:
        t += QString::fromLatin1(" StatusMessage");
        break;
    case Email:
        t += QString::fromLatin1(" Email");
        break;
    case Ignore:
        t += QString::fromLatin1(" Ignore");
        break;
    }
    if (!Extra.isEmpty())
    {
        t += QString::fromLatin1(" ");
        t += Extra;
    }
    return t;
}

bool toChartManager::chartAlarm::checkValue(double val)
{
    switch (Comparison)
    {
    case Equal:
        return val == Value;
    case NotEqual:
        return val != Value;
    case Less:
        return Value < val;
    case Greater:
        return Value > val;
    case LessEqual:
        return Value <= val;
    case GreaterEqual:
        return Value >= val;
    }
    return false;
}

void toChartManager::chartAlarm::valueAdded(toChartHandler *handler,
        const QString &str,
        std::list<double> &value,
        const QString &xValue)
{
    std::list<double> vals;
    std::list<double>::iterator i = value.begin();
    std::list<int>::iterator j = Columns.begin();
    for (int id = 0;j != Columns.end() && i != value.end();i++, id++)
    {
        if (id == *j)
        {
            vals.insert(vals.end(), *i);
            j++;
        }
    }
    bool sig = false;
    switch (Operation)
    {
    case Any:
    {
        for (std::list<double>::iterator i = vals.begin();i != vals.end();i++)
            if (checkValue(*i))
            {
                sig = true;
                break;
            }
    }
    break;
    case All:
        sig = true;
        {
            for (std::list<double>::iterator i = vals.begin();i != vals.end();i++)
                if (!checkValue(*i))
                {
                    sig = false;
                    break;
                }
        }
        break;
    case Sum:
    {
        double val = 0;
        for (std::list<double>::iterator i = vals.begin();i != vals.end();i++)
            val += *i;
        sig = checkValue(val);
    }
    break;
    case Average:
    {
        double val = 0;
        for (std::list<double>::iterator i = vals.begin();i != vals.end();i++)
            val += *i;
        sig = checkValue(val / vals.size());
    }
    break;
    case Max:
    {
        bool first = true;
        double val = 0;
        for (std::list<double>::iterator i = vals.begin();i != vals.end();i++)
        {
            if (first)
            {
                first = false;
                val = *i;
            }
            else
                val = std::max(*i, val);
        }
        sig = checkValue(val);
    }
    break;
    case Min:
    {
        bool first = true;
        double val = 0;
        for (std::list<double>::iterator i = vals.begin();i != vals.end();i++)
        {
            if (first)
            {
                first = false;
                val = *i;
            }
            else
                val = std::min(*i, val);
        }
        sig = checkValue(val);
    }
    break;
    }
    if (sig)
    {
        if (Signal)
            return ;
        handler->SignalAlarms.insert(handler->SignalAlarms.end(),
                                     alarmSignal(Action, xValue, str, toString(), Extra));
        handler->Timer.setSingleShot(true);
        handler->Timer.start(1);
        Signal = true;
    }
    else
        Signal = false;
}

void toChartHandler::saveSettings(void)
{
    {
//         int num = 0;
        ChartsMap c;
        for (std::map<QString, toChartManager::chartTrack>::iterator i = Files.begin();i != Files.end();i++)
        {
            if ((*i).second.Persistent)
            {
//                 num++;
//                 QString name = QString("Files:") + QString::number(num).toLatin1();
//                 ChartTool.setConfig(name + ":Name", (*i).first);
//                 ChartTool.setConfig(name + ":Spec", (*i).second.File.fileName());
                c[(*i).first] = (*i).second.File.fileName();
            }
        }
//         ChartTool.setConfig("FilesCount", QString::number(num));
        toConfigurationSingle::Instance().setChartFiles(c);
    }
    {
        ChartsMap c;
//         int num = 0;
        for (std::map<QString, std::list<toChartManager::chartAlarm> >::iterator i = Alarms.begin();
                i != Alarms.end();i++)
        {
            for (std::list<toChartManager::chartAlarm>::iterator j = (*i).second.begin();j != (*i).second.end();j++)
            {
                if ((*j).Persistent)
                {
//                     num++;
//                     QString name = QString("Alarms:") + QString::number(num).toLatin1();
//                     ChartTool.setConfig(name + ":Name", (*i).first);
//                     ChartTool.setConfig(name + ":Spec", (*j).toString());
                    c[(*i).first] = (*j).toString();
                }
            }
        }
//         ChartTool.setConfig("AlarmCount", QString::number(num));
        toConfigurationSingle::Instance().setChartAlarms(c);
    }
    toConfigurationSingle::Instance().saveConfig();
}

void toChartHandler::loadSettings(void)
{
//     {
//         for (int num = ChartTool.config("FilesCount", "0").toInt();num > 0;num--)
//         {
//             QString name = QString("Files:") + QString::number(num).toLatin1();
//             QString t = ChartTool.config(name + ":Name", "");
//             QString s = ChartTool.config(name + ":Spec", "");
//             if (!t.isEmpty() && !s.isEmpty())
//                 Files[t] = toChartManager::chartTrack(s, true);
//         }
//     }
    ChartsMapIterator i(toConfigurationSingle::Instance().chartFiles());
    while (i.hasNext())
    {
        i.next();
        if (!i.key().isEmpty() && !i.value().isEmpty())
            Files[i.key()] = toChartManager::chartTrack(i.value(), true);
    }
//     {
//         for (int num = ChartTool.config("AlarmCount", "0").toInt();num > 0;num--)
//         {
//             QString name = QString("Alarms:") + QString::number(num).toLatin1();
//             QString t = ChartTool.config(name + ":Name", "");
//             QString s = ChartTool.config(name + ":Spec", "");
//             if (!t.isEmpty() && !s.isEmpty())
//                 Alarms[t].insert(Alarms[t].end(), toChartManager::chartAlarm(s, true));
//         }
//     }
    ChartsMapIterator i1(toConfigurationSingle::Instance().chartAlarms());
    while (i1.hasNext())
    {
        i1.next();
        if (!i1.key().isEmpty() && !i1.value().isEmpty())
            Alarms[i1.key()].insert(Alarms[i1.key()].end(), toChartManager::chartAlarm(i1.value(), true));
    }
}

void toChartHandler::alarm(void)
{
    while (!SignalAlarms.empty())
    {
        toChartManager::alarmSignal signal = toShift(SignalAlarms);
        if (signal.Action == toChartManager::StatusMessage)
            toStatusMessage(tr("ALARM:") + signal.Chart + QString::fromLatin1(": ") + signal.Alarm + QString::fromLatin1(": ") + signal.xValue);
        else if (signal.Action == toChartManager::Email)
            new toSMTP(QString::fromLatin1(TOAPPNAME " <noreply@localhost>"),
                       signal.Extra,
                       tr("TOra alert:") + " " + signal.Chart,
                       tr("A defined alert value was detected:\n\n%1\n\nAt: %2").arg(signal.Alarm).arg(signal.xValue));
    }
}

void toChartHandler::valueAdded(toLineChart *chart,
                                const QString &chartName,
                                std::list<double> &value,
                                const QString &xValue)
{
    std::map<QString, std::list<toChartManager::chartAlarm> >::iterator fnda = Alarms.find(chartName);
    if (fnda != Alarms.end())
    {
        std::list<toChartManager::chartAlarm> &alarms = (*fnda).second;
        for (std::list<toChartManager::chartAlarm>::iterator i = alarms.begin();i != alarms.end();i++)
            (*i).valueAdded(this, chartName, value, xValue);
    }

    std::map<QString, toChartManager::chartTrack>::iterator fndt = Files.find(chartName);
    if (fndt != Files.end())
    {
        QFile &file = (*fndt).second.File;
        bool header = false;
        if (!file.isOpen())
        {
            if (!file.exists())
                header = true;
            file.open(QIODevice::Unbuffered | QIODevice::WriteOnly | QIODevice::Append);
        }
        if (file.isOpen())
        {
            static QRegExp quote(QString::fromLatin1("\""));
            QString out = "\"";
            if (header)
            {
                QString t = chart->title();
                t.replace(quote, QString::fromLatin1("\"\""));
                out += t.toUtf8();
                std::list<QString> labels = chart->labels();
                for (std::list<QString>::iterator i = labels.begin();i != labels.end();i++)
                {
                    out += "\";\"";
                    QString t = (*i);
                    t.replace(quote, QString::fromLatin1("\"\""));
                    out += t.toUtf8();
                }
                out += "\"\n\"";
            }
            QString t = xValue;
            t.replace(quote, QString::fromLatin1("\"\""));
            out += t.toUtf8();
            for (std::list<double>::iterator i = value.begin();i != value.end();i++)
            {
                out += "\";\"";
                out += QString::number(*i);
            }
            out += "\"\n";
            file.write(out.toUtf8());
        }
    }
}

static QString ReadCSV(const QString &data, int &pos, bool &nl)
{
    QString ret;
    if (data.at(pos) != '\"')
        throw qApp->translate("toChartManager", "Initial value didn't start with \" in CSV file");
    pos++;
    while (pos < data.length() &&
            (data.at(pos) != '\"' || (pos + 1 < data.length() &&
                                      data.at(pos + 1) == '\"')))
    {
        ret += data.at(pos);
        if (data.at(pos) == '\"')
            pos++;
        pos++;
    }
    if (pos >= data.length())
        throw qApp->translate("toChartManager", "Missing closing \" in CSV file");
    pos++;
    nl = false;
    while (pos < data.length() && (data.at(pos).isSpace() || data.at(pos) == ',' || data.at(pos) == ';'))
    {
        if (data.at(pos) == '\n')
            nl = true;
        pos++;
    }
    return ret;
}

void toChartManager::openChart(void)
{
    QString name = toOpenFilename(QString::null, QString::fromLatin1("*.csv"), this);
    if (!name.isEmpty())
    {
        try
        {
            QString data = toReadFile(name);
            int pos = 0;

            toLineChart *chart;
            switch (TOMessageBox::information(toMainWidget(),
                                              tr("Chart format"),
                                              tr("Select format of the chart to display"),
                                              tr("Barchart"),
                                              tr("Linechart"),
                                              tr("Cancel")))
            {
            case 0:
                chart = new toBarChart(toMainWidget()->workspace(), NULL);
                chart->setAttribute(Qt::WA_DeleteOnClose);
                break;
            case 1:
                chart = new toLineChart(toMainWidget()->workspace(), NULL);
                chart->setAttribute(Qt::WA_DeleteOnClose);
                break;
            default:
                return ;
            }
            chart->show();
            chart->setSamples( -1);
            bool eol;
            chart->setTitle(ReadCSV(data, pos, eol));
            std::list<QString> lab;
            while (!eol && pos < data.length())
                lab.insert(lab.end(), ReadCSV(data, pos, eol));
            chart->setLabels(lab);
            while (pos < data.length())
            {
                QString t = ReadCSV(data, pos, eol);
                std::list<double> val;
                while (!eol && pos < data.length())
                    val.insert(val.end(), ReadCSV(data, pos, eol).toDouble());
                chart->addValues(val, t);
            }
        }
        TOCATCH
    }
}

void toChartManager::setupChart(void)
{
    toChartReceiver *chart = selectedChart();
    if (chart && ChartTool.handler())
        ChartTool.handler()->setupChart(chart->chart());
}

toChartReceiver *toChartManager::selectedChart(void)
{
    try
    {
        toTreeWidgetItem *item = List->selectedItem();
        if (item && ChartTool.handler())
        {
            for (std::list<toChartReceiver *>::iterator i = ChartTool.handler()->Charts.begin();i != ChartTool.handler()->Charts.end();i++)
            {
                toResult *result = (*i)->result();
                if (result)
                {
                    if (item->text(0) == result->connection().description(false) &&
                            item->text(2) == result->sqlName())
                        return *i;
                }
            }
        }
    }
    TOCATCH
    return NULL;
}

void toChartHandler::setupChart(toLineChart *chart)
{
    for (std::list<toChartReceiver *>::iterator i = Charts.begin();i != Charts.end();i++)
    {
        if ((*i)->chart() == chart)
        {
            QString name = (*i)->name();
            if (!name.isNull())
            {
                std::list<toChartManager::chartAlarm> alarm;
                std::map<QString, std::list<toChartManager::chartAlarm> >::iterator fnda = Alarms.find(name);
                if (fnda != Alarms.end())
                    alarm = (*fnda).second;

                toChartManager::chartTrack file;
                std::map<QString, toChartManager::chartTrack>::iterator fndt = Files.find(name);
                if (fndt != Files.end())
                    file = (*fndt).second;

                toChartSetup setup(chart, toMainWidget(), NULL, true);
                setup.Alarms->addColumn(tr("Alarms"));
                setup.Alarms->addColumn(tr("Persistent"));
                setup.Alarms->setSorting(0);
                setup.Alarms->setSelectionMode(toTreeWidget::Single);
                setup.Persistent->setChecked(file.Persistent);
                setup.Filename->setText(file.File.fileName());
                if (!setup.Filename->text().isEmpty())
                    setup.Enabled->setChecked(true);
                setup.ChartFrame->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,
                                                QSizePolicy::Preferred));

                toLineChart *t = chart->openCopy(setup.ChartFrame);
                t->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,
                                             QSizePolicy::Preferred));
                t->showLegend(false);
                t->showAxisLegend(false);
                setup.ChartFrame->layout()->addWidget(t);
                for (std::list<toChartManager::chartAlarm>::iterator j = alarm.begin();j != alarm.end();j++)
                    new toTreeWidgetItem(setup.Alarms,
                                         (*j).toString(),
                                         (*j).Persistent ? tr("Persistent") : tr("Temporary"));

                if (setup.exec())
                {
                    if (setup.Filename->text().isEmpty() || !setup.Enabled->isChecked())
                    {
                        if (fndt != Files.end())
                            Files.erase(fndt);
                    }
                    else
                    {
                        Files[name] = toChartManager::chartTrack(setup.Filename->text(),
                                      setup.Persistent->isChecked());
                    }
                    if (fnda != Alarms.end())
                        Alarms.erase(fnda);
                    alarm.clear();
                    for (toTreeWidgetItem *item = setup.Alarms->firstChild();
                            item;item = item->nextSibling())
                    {
                        alarm.insert(alarm.end(), toChartManager::chartAlarm(item->text(0),
                                     item->text(1) == tr("Persistent")));
                    }
                    if (!alarm.empty())
                        Alarms[name] = alarm;
                    saveSettings();
                }
            }
            break;
        }
    }
}

void toChartManager::refresh(void)
{
    if (!ChartTool.handler())
        return ;
    try
    {
        List->clear();
        for (std::list<toChartReceiver *>::iterator i = ChartTool.handler()->Charts.begin();i != ChartTool.handler()->Charts.end();i++)
        {
            toResult *result = (*i)->result();
            if (result)
            {
                toResultViewItem *item = new toResultViewMLine(List,
                        NULL,
                        result->connection().description(false));
                item->setText(1, (*i)->chart()->title());
                item->setText(2, result->sqlName());
                QString name = (*i)->name();
                if (!name.isNull())
                {
                    std::map<QString, std::list<chartAlarm> >::iterator fnda = ChartTool.handler()->Alarms.find(name);
                    if (fnda != ChartTool.handler()->Alarms.end())
                    {
                        QString t;
                        for (std::list<chartAlarm>::iterator j = (*fnda).second.begin();j != (*fnda).second.end();j++)
                        {
                            t += (*j).toString();
                            t += QString::fromLatin1("\n");
                        }
                        if (t.length() > 0)
                            item->setText(4, t.mid(0, t.length() - 1));
                    }

                    std::map<QString, chartTrack>::iterator fndt = ChartTool.handler()->Files.find(name);
                    if (fndt != ChartTool.handler()->Files.end())
                        item->setText(3, (*fndt).second.File.fileName());
                }
            }
        }
    }
    TOCATCH
}

toChartHandler::toChartHandler()
{
    connect(toMainWidget(), SIGNAL(chartAdded(toLineChart *)),
            this, SLOT(addChart(toLineChart *)));
    connect(toMainWidget(), SIGNAL(chartRemoved(toLineChart *)),
            this, SLOT(removeChart(toLineChart *)));
    connect(toMainWidget(), SIGNAL(chartSetup(toLineChart *)),
            this, SLOT(setupChart(toLineChart *)));
    connect(&Timer, SIGNAL(timeout()), this, SLOT(alarm()));
}

toChartHandler::~toChartHandler()
{
    ChartTool.closeHandler();
}
