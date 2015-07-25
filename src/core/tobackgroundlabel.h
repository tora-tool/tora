//
// C++ Interface: toBackgroundLabel
//
// Copyright: See COPYING file that comes with this distribution
//
#ifndef TOBACKGROUNDLABEL_H
#define TOBACKGROUNDLABEL_H

#include <QLabel>

class QWidget;
class QMouseEvent;
class QMovie;

class toBackgroundLabel : public QLabel
{
        Q_OBJECT;

    public:
        toBackgroundLabel(QWidget *parent);
        virtual ~toBackgroundLabel();
        virtual void mouseReleaseEvent(QMouseEvent* e);
        void unpause();
        void pause();
        void setTip(const QString &tip);
        void setSpeed(int Running);
    private:
        static QMovie* Animation;
};


#endif
