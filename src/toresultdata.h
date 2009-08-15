
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2009 Numerous Other Contributors
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 *      As a special exception, you have permission to link this program
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 * 
 *      Specifically you are not permitted to link this program with the
 *      Qt/UNIX, Qt/Windows or Qt Non Commercial products of TrollTech.
 *      And you are not permitted to distribute binaries compiled against
 *      these libraries. 
 * 
 *      You may link this product with any GPL'd Qt library.
 * 
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#ifndef TORESULTDATA_H
#define TORESULTDATA_H

#include "config.h"
#include "toresulttableviewedit.h"
#include "tosql.h"
#include "toresult.h"
#include "toconnection.h"
#include "toresultmodel.h"

#include <QWidget>
#include <QList>
#include <QMap>

class QAction;
class toResultDataSingle;
class QCloseEvent;


class toResultData : public QWidget, public toResult
{
    Q_OBJECT;

    // toolbar actions
    QAction *filterAct;
    QAction *removeAct;
    QAction *refreshAct;
    QAction *addAct;
    QAction *saveAct;
    QAction *discardAct;
    QAction *deleteAct;
    QAction *duplicateAct;
    QAction *firstAct;
    QAction *previousAct;
    QAction *nextAct;
    QAction *lastAct;
    QAction *singleAct;

    // editable QTableView
    toResultTableViewEdit *Edit;

    // single record edit form
    toResultDataSingle *Form;

    // Owner of table.
    QString Owner;

    // Table name
    QString Table;

    // SQL select statement
    QString SQL;

    // ---------------------------------------- filter settings

    // Use filter for all tables.
    bool AllFilter;

    // Filter selection criteria
    QMap<QString, QString> Criteria;

    // Filter retrieve order
    QMap<QString, QString> Order;

    // Current filter name in map
    QString FilterName;

    // generate filtername
    QString filterName();

    // true if user has elected to discard current changes.
    // should be reset on every query
    bool Discard;

public:
    toResultData(QWidget *parent = 0,
                 const char *name = 0,
                 Qt::WindowFlags f = 0);


    /**
     * Reimplemented from toResult. First parameter is ignored.
     *
     */
    virtual void query(const QString &, const toQList &params);


    /**
     * Reimplemented from toResult. Can handle all connections.
     *
     */
    virtual bool canHandle(toConnection &)
    {
        return true;
    }


    /**
     * Export data to a map.
     *
     * @param data A map that can be used to recreate the data of a chart.
     * @param prefix Prefix to add to the map.
     */
    virtual void exportData(std::map<QString, QString> &data,
                            const QString &prefix);


    /**
     * Import data
     *
     * @param data Data to read from a map.
     * @param prefix Prefix to read data from.
     */
    virtual void importData(std::map<QString, QString> &data,
                            const QString &prefix);


    /**
     * Prompt to save if modified.
     *
     */
    bool maybeSave(void);

signals:
    /**
     * Emitted when changes are saved.
     *
     */
    void changesSaved(void);

protected:
    virtual void closeEvent(QCloseEvent *);

protected slots:
    void navigate(QAction *);
    void changeFilter(bool);
    void removeFilter(bool);

public slots:
    void singleRecordForm(bool display);
    void updateForm(void);
    void save(void);
    void addRecord(void);
    void refreshWarn(void);
};


#endif
