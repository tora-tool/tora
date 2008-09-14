/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#include "tosplash.h"

#include <qpixmap.h>
#include <qlabel.h>
#include <qapplication.h>
#include <QDesktopWidget>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QPalette>

#include "icons/largelogo.xpm"

toSplash::toSplash(QWidget *parent, const char *name, Qt::WFlags f)
        : QWidget(parent, f)
{

    if (name)
        setObjectName(name);

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setSpacing(0);
    vbox->setContentsMargins(0, 0, 0, 0);
    setLayout(vbox);

    QPalette palette = this->palette();
    palette.setColor(backgroundRole(), Qt::white);
    setPalette(palette);

    QLabel *logo = new QLabel("Logo", this);
    vbox->addWidget(logo);
    logo->setPalette(palette);
    logo->setPixmap(QPixmap(QString::fromLatin1(":/icons/largelogo.xpm")));
    Label = new QLabel(tr("Loading plugins"), this);
    vbox->addWidget(Label);
    Label->setPalette(palette);
    Progress = new QProgressBar(this);
    vbox->addWidget(Progress);

    QDesktopWidget *d = QApplication::desktop();
    move((d->width() - width()) / 2, (d->height() - height()) / 2);
}

toSplash::~toSplash()
{
}
