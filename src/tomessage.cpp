/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#include <QPushButton>
#include <QDateTime>
#include <QSettings>

#include "tomessage.h"


toMessage::toMessage(QWidget * parent, Qt::WindowFlags f)
    : QDialog(parent, f)
{
    setupUi(this);
    setModal(false);
    Message->setReadOnly(true);
    buttonBox->button(QDialogButtonBox::Close)->setFocus(Qt::OtherFocusReason);

    QSettings s;
    s.beginGroup("toMessage");
    restoreGeometry(s.value("geometry", QByteArray()).toByteArray());
    s.endGroup();

    connect(buttonBox->button(QDialogButtonBox::Close),
            SIGNAL(clicked()), this, SLOT(hide()));
    connect(buttonBox->button(QDialogButtonBox::Reset),
            SIGNAL(clicked()), Message, SLOT(clear()));
}

void toMessage::appendText(const QString & text)
{
    if (!isVisible())
        show();

    if (Message->lines() != 1)
        Message->append("\n\n");
    Message->append(QDateTime::currentDateTime().toString(Qt::TextDate) + "\n");
    Message->append(text);
    Message->ensureLineVisible(Message->lines());
}

void toMessage::hideEvent(QHideEvent * event)
{
    QSettings s;
    s.beginGroup("toMessage");
    s.setValue("geometry", saveGeometry());
    s.endGroup();
    event->accept();
}
