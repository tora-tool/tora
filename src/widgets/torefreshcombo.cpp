#include "widgets/torefreshcombo.h"
#include "core/toconfiguration.h"
#include "core/toglobalconfiguration.h"

toRefreshCombo::toRefreshCombo(QWidget *parent, const QString& def)
	: QComboBox(parent)
	, m_timer(new QTimer(this))
{
	this->setObjectName("toRefreshCombo");
	this->setEditable(false);

	addItem(tr("None"));
	addItem(tr("2 seconds"));
	addItem(tr("5 seconds"));
	addItem(tr("10 seconds"));
	addItem(tr("30 seconds"));
	addItem(tr("1 min"));
	addItem(tr("5 min"));
	addItem(tr("10 min"));
	QString str;
	if (!def.isNull())
		str = def;
	else
		str = toConfigurationNewSingle::Instance().option(ToConfiguration::Global::RefreshInterval).toString();
	if (str == "2 seconds")
		setCurrentIndex(1);
	else if (str == "5 seconds")
		setCurrentIndex(2);
	else if (str == "10 seconds")
		setCurrentIndex(3);
	else if (str == "30 seconds")
		setCurrentIndex(4);
	else if (str == "1 min")
		setCurrentIndex(5);
	else if (str == "5 min")
		setCurrentIndex(6);
	else if (str == "10 min")
		setCurrentIndex(7);
	else
		setCurrentIndex(0);
}

void toRefreshCombo::setRefreshInterval(QString const& interval)
{
#if QT_VERSION < 0x050000
	int index = findText(interval);
	setCurrentIndex(index);
#else
	setCurrentText(interval);
#endif	
}


QTimer* toRefreshCombo::timer()
{
	return m_timer;
}

int toRefreshCombo::refreshParse()
{
#if 0
    QString t = currentText();
    if (t.isEmpty())
        t = toConfigurationNewSingle::Instance().option(ToConfiguration::Global::RefreshInterval).toString();

    if (t == tr("toRefreshCreate", "None") || t == "None")
        timer->stop();
    else if (t == tr("toRefreshCreate", "2 seconds") || t == "2 seconds")
        timer->start(2 * 1000);
    else if (t == tr("toRefreshCreate", "5 seconds") || t == "5 seconds")
        timer->start(5 * 1000);
    else if (t == tr("toRefreshCreate", "10 seconds") || t == "10 seconds")
        timer->start(10 * 1000);
    else if (t == tr("toRefreshCreate", "30 seconds") || t == "30 seconds")
        timer->start(30 * 1000);
    else if (t == tr("toRefreshCreate", "1 min") || t == "1 min")
        timer->start(60 * 1000);
    else if (t == tr("toRefreshCreate", "5 min") || t == "5 min")
        timer->start(300 * 1000);
    else if (t == tr("toRefreshCreate", "10 min") || t == "10 min")
        timer->start(600 * 1000);
    else
        throw qApp->translate("toRefreshParse", "Unknown timer value");
#endif
    return 0;
}
