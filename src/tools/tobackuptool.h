#ifndef TOBACKUPTOOL_H
#define TOBACKUPTOOL_H

#include "core/totool.h"
#include <map>

class QWidget;
class toConnection;

//
// C++ Interface: tobackuptool
//
// Description:
//
//
// Author: Thomas Porschberg, core <pberg@porschberg>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//

class toBackupTool : public toTool
{
        Q_OBJECT;
    public:
        toBackupTool();
        virtual const char *menuItem();
        virtual toToolWidget* toolWindow(QWidget *parent, toConnection &connection);
        void closeWindow(toConnection &connection);
    protected:
        std::map<toConnection *, QWidget *> Windows;
        virtual const char **pictureXPM(void);
};

#endif
