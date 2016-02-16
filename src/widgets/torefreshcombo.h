#pragma once

#include <QComboBox>

class toRefreshCombo : public QComboBox
{
public:
	explicit toRefreshCombo(QWidget *parent, const QString& def = QString());

	QTimer* timer();

	int refreshTime() const;
	static int refreshParse();
private:
	QTimer *m_timer;
};
