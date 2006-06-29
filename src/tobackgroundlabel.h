//
// C++ Interface: toBackgroundLabel
//
// Copyright: See COPYING file that comes with this distribution
//
#ifndef TOBACKGROUNDLABEL_H
#define TOBACKGROUNDLABEL_H

#include <qmovie.h>
#include <qwidget.h>
#include <qlabel.h>

class toBackgroundLabel : public QLabel{
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
        static const char data[];	
    
};


#endif
