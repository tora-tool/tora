/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOSTORAGEDEFINITION_H
#define TOSTORAGEDEFINITION_H

#include "config.h"
#include "ui_tostoragedefinitionui.h"

#include <list>

#include <QWidget>
#include <QLabel>

class QCheckBox;
class Q3GroupBox;
class Q3GroupBox;
class QLabel;
class QSpinBox;
class toFilesize;
class toStorageDialog;

class toStorageDefinition : public QWidget, public Ui::toStorageDefinitionUI
{
public:
    toStorageDefinition(QWidget *parent = 0, const char *name = 0, Qt::WFlags fl = 0);

    void forRollback(bool val);

    std::list<QString> sql(void);

    friend class toStorageDialog;
};

#endif
