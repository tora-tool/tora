
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 *
 * Shared/mixed copyright is held throughout files in this product
 *
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2013 Numerous Other Contributors
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation;  only version 2 of
 * the License is valid for this program.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program as the file COPYING.txt; if not, please see
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt.
 *
 *      As a special exception, you have permission to link this program
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 *
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#ifndef TORESULTPARAM_H
#define TORESULTPARAM_H

#include "core/toresult.h"

#include <map>

#include <QWidget>

class QLineEdit;
class QToolButton;
class toResultView;

/**
 * This widget displays the parameters of a session and lets you edit
 * them.
 */
class toResultParam : public QWidget, public toResult
{
        Q_OBJECT;

        QString LastItem;
        QString LastValue;
        std::map<QString, QString> NewValues;

        toResultView *Params;
        QLineEdit *Value;
        QToolButton *Toggle;

        void saveChange();
    public:
        /** Create the widget.
         * @param parent Parent widget.
         * @param name Name of widget.
         */
        toResultParam(QWidget *parent, const char *name = NULL);

        virtual bool canHandle(const toConnection &conn);

    private slots:
        void generateFile(void);
        void applySession(void);
        void applySystem(void);
        void applyChanges(void);
        void changeItem(void);
        void changedData(int row, int col, const QString &data);
        void dropChanges(void);
        void done(void);
        void showHidden(bool);
        void showGlobal(bool);

    public slots:
        virtual void query(const QString &sql, const toQueryParams &param);
};

#endif
