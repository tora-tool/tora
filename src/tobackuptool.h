#ifndef TOBACKUPTOOL_H
#define TOBACKUPTOOL_H

#include <map>

#include "totool.h"

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
protected:
    std::map<toConnection *, QWidget *> Windows;
    virtual const char **pictureXPM(void);
public:
    toBackupTool();
    virtual const char *menuItem();
    virtual QWidget *toolWindow(QWidget *parent, toConnection &connection);
    void closeWindow(toConnection &connection);
};

#endif
