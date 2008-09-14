/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TORESULTCOLSCOMMENT_H
#define TORESULTCOLSCOMMENT_H

#include "config.h"

#include <QString>
#include <QLineEdit>
#include <QFocusEvent>

class QWidget;


/** This widget is used for single record view in the content
 * editor. Only for internal use.
 *
 * @internal
 */
class toResultColsComment : public QLineEdit
{
    Q_OBJECT;

    bool    Changed;
    bool    Table;
    QString Name;

private slots:
    void saveUnchanged();

public:
    toResultColsComment(QWidget *parent);
    void setComment(bool table, const QString &name, const QString &comment);

public slots:
    void commentChanged();
};

#endif
