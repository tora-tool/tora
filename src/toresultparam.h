/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TORESULTPARAM_H
#define TORESULTPARAM_H

#include "config.h"
#include "toresult.h"

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

    virtual bool canHandle(toConnection &conn);

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
    virtual void query(const QString &sql, const toQList &param);
    /** Reimplemented for internal reasons.
     */
    virtual void refresh(void)
    {
        toResult::refresh();
    }
    /** Reimplemented for internal reasons.
     */
    virtual void changeParams(const QString &Param1)
    {
        toResult::changeParams(Param1);
    }
    /** Reimplemented For internal reasons.
     */
    virtual void changeParams(const QString &Param1, const QString &Param2)
    {
        toResult::changeParams(Param1, Param2);
    }
    /** Reimplemented for internal reasons.
     */
    virtual void changeParams(const QString &Param1, const QString &Param2, const QString &Param3)
    {
        toResult::changeParams(Param1, Param2, Param3);
    }
};

#endif
