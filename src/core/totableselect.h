#ifndef TOTABLESELECT_H
#define TOTABLESELECT_H


#include "totool.h"

#include <QtGui/QGroupBox>

class toConnection;
class toResultSchema;
class toResultCombo;

class toTableSelect : public QGroupBox
{
    Q_OBJECT

    toResultSchema *Schema;
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
