/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>

#include "toworkingwidget.h"
#include "icons/stop.xpm"


toWorkingWidget::toWorkingWidget(QWidget * parent)
    : QWidget(parent)
{
    CurrentType = toWorkingWidget::Interactive;

    setAutoFillBackground(true);
    setPalette(QPalette(Qt::white));

    HWorking = new QWidget(this);
    HWorking->setAutoFillBackground(true);
    HWorking->setPalette(QPalette(QColor(241, 241, 169)));

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setSpacing(0);
    vbox->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *hbox = new QHBoxLayout;

    WorkingLabel = new QLabel(tr("Waiting..."), HWorking);
    WorkingLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    hbox->addWidget(WorkingLabel);

    WorkingStop = new QPushButton(QIcon(stop_xpm), tr("Stop"), HWorking);
    WorkingStop->setAutoFillBackground(true);
    WorkingStop->setBackgroundRole(QPalette::Window);
    WorkingStop->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    WorkingStop->setEnabled(false);

    connect(WorkingStop, SIGNAL(clicked()), this, SLOT(stopWorking()));

    hbox->addWidget(WorkingStop);

    HWorking->setLayout(hbox);

    vbox->addWidget(HWorking);
    vbox->addStretch(0);

    setLayout(vbox);
}

void toWorkingWidget::show()
{
    if (CurrentType == toWorkingWidget::Interactive)
        HWorking->hide();
    QWidget::show();
}

void toWorkingWidget::forceShow()
{
    HWorking->show();
}

void toWorkingWidget::stopWorking()
{
    emit stop();
}

void toWorkingWidget::setText(const QString & text)
{
    WorkingLabel->setText(text);
    WorkingStop->setEnabled(true);
}

void toWorkingWidget::setType(WorkingWidgetType type)
{
    CurrentType = type;
    bool e = (type == toWorkingWidget::Interactive);
    WorkingStop->setEnabled(e);
    WorkingStop->setVisible(e);
    WorkingStop->blockSignals(!e);
}
