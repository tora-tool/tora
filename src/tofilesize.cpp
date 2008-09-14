/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#include <stdio.h>

#include <qradiobutton.h>
#include <qspinbox.h>
#include <qlayout.h>

#include <QVBoxLayout>
#include <QHBoxLayout>

#include "tofilesize.h"


void toFilesize::setup()
{
    QVBoxLayout *vlayout = new QVBoxLayout(this);
    vlayout->addSpacing(15);
    setLayout(vlayout);

    QWidget *hcontainer = new QWidget(this);
    vlayout->addWidget(hcontainer);
    QHBoxLayout *layout = new QHBoxLayout(hcontainer);
    layout->setMargin(10);
    layout->setSpacing(10);
    vlayout->addSpacing(2);
    hcontainer->setLayout(layout);
    vlayout->addWidget(hcontainer);

    Value = new QSpinBox(this);
    Value->setSingleStep(1024);
    Value->setMaximum(2097151);
    Value->setMinimum(1);
    Value->setValue(1024);
    layout->addWidget(Value);
    layout->setStretchFactor(Value, 100);

    MBSize = new QRadioButton(this);
    MBSize->setText(QString::fromLatin1("MB"));
    MBSize->setChecked(true);
    layout->addWidget(MBSize);
    layout->setStretchFactor(MBSize, 1);
    connect(MBSize, SIGNAL(toggled(bool)), this, SLOT(changeType(bool)));

    KBSize = new QRadioButton(this);
    KBSize->setText(QString::fromLatin1("KB"));
    KBSize->setChecked(false);
    layout->addWidget(KBSize);
    layout->setStretchFactor(KBSize, 1);
    connect(Value, SIGNAL(valueChanged(int)), this, SLOT(changedSize(void)));
}

toFilesize::toFilesize(QWidget* parent, const char* name)
        : QGroupBox(parent)
{
    if (name)
        setObjectName(name);
    setup();
}

toFilesize::toFilesize(const QString &title, QWidget* parent, const char* name)
        : QGroupBox(parent)
{
    if (name)
        setObjectName(name);
    setup();
}

int toFilesize::value(void)
{
    if (MBSize->isChecked())
        return Value->value()*1024;
    else
        return Value->value();
}

void toFilesize::changeType(bool)
{
    if (MBSize->isChecked())
    {
        Value->setMaximum(2097151);
    }
    else
    {
        Value->setMaximum(2147483647);
    }
}

void toFilesize::setValue(int sizeInKB)
{
    if (sizeInKB % 1024 == 0)
    {
        MBSize->setChecked(true);
        KBSize->setChecked(false);
        Value->setValue(sizeInKB / 1024);
        Value->setMaximum(2097151);
    }
    else
    {
        MBSize->setChecked(false);
        KBSize->setChecked(true);
        Value->setMaximum(2147483647);
        Value->setValue(sizeInKB);
    }
}

QString toFilesize::sizeString(void)
{
    char buf[30];
    if (MBSize->isChecked())
        sprintf(buf, "%d K", Value->value()*1024);
    else
        sprintf(buf, "%d K", Value->value());
    return QString::fromLatin1(buf);
}
