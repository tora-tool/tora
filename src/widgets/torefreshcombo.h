#pragma once

#include <QComboBox>

class toRefreshCombo : public QComboBox
{
       Q_OBJECT
public:
	explicit toRefreshCombo(QWidget *parent, const QString& def = QString());

	void setRefreshInterval(QString const&);
	
	QTimer* timer();

	int refreshTime() const;

private slots:
        void slotRefreshUpdate();
private:
	QTimer *m_timer;
};
