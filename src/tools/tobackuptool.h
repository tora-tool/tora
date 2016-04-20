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

        const char *menuItem() override;
        toToolWidget* toolWindow(QWidget *parent, toConnection &connection) override;
        bool canHandle(const toConnection &conn) override;
        void closeWindow(toConnection &connection) override;

    protected:
        std::map<toConnection *, QWidget *> Windows;
        const char **pictureXPM(void) override;
};

#endif
