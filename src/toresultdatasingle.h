/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TORESULTDATASINGLE_H
#define TORESULTDATASINGLE_H

#include "config.h"
#include "toresultmodel.h"

#include <QWidget>
#include <QList>
#include <QPointer>

class QScrollArea;
class QCheckBox;
class QLineEdit;


/**
 * This widget is used for single record view in the content editor.
 *
 */
class toResultDataSingle : public QWidget
{
    Q_OBJECT;

    int Row;
    QScrollArea *Container;
    QList<QCheckBox *> Null;
    QList<QLineEdit *> Value;

    QPointer<toResultModel> Model;

public:
    toResultDataSingle(QWidget *parent);

    void changeSource(toResultModel *, int row);
    void changeRow(int row);

    inline int currentRow(void) const {
        return Row;
    }

private slots:
    virtual void showMemo(int col);
    void saveRow(void);
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
};

#endif
