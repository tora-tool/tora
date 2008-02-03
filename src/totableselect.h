#ifndef TOTABLESELECT_H
#define TOTABLESELECT_H

#include "config.h"
#include "totool.h"

#include <QGroupBox>

class toConnection;
class toResultCombo;

class toTableSelect : public QGroupBox
{
    Q_OBJECT

    toResultCombo *Schema;
    toResultCombo *Table;

    QString SelectedTable;
public:
    toTableSelect(QWidget *parent, const char *name = 0);
    virtual ~toTableSelect()
    { }

    void setTable(const QString &table);
signals:
    void selectTable(const QString &table);
private slots:
    void setup(void);
    void changeSchema(void);
    void changeTable(void);
};


#endif
