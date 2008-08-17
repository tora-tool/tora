//
// C++ Interface: toBackgroundLabel
//
// Copyright: See COPYING file that comes with this distribution
//
#ifndef TOBACKGROUNDLABEL_H
#define TOBACKGROUNDLABEL_H

#include "config.h"

#include <qmovie.h>
#include <qwidget.h>
#include <qlabel.h>
//Added by qt3to4:
#include <QMouseEvent>

class toBackgroundLabel : public QLabel
{
    Q_OBJECT;

public:
    //static toBackgroundLabel createToBackgroundLabel();
    virtual void mouseReleaseEvent(QMouseEvent* e);
    virtual void unpause();
    virtual void pause();
    virtual void setTip(QString tip);
    virtual void setSpeed(int Running);
    toBackgroundLabel(QWidget *parent);
private:
    static QMovie* Animation;
};


#endif
