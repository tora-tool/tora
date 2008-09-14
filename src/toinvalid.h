/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOINVALID_H
#define TOINVALID_H

#include "config.h"
#include "totool.h"

class toResultExtract;
class toResultTableView;

class toInvalid : public toToolWidget
{
    Q_OBJECT;

    toResultTableView *Objects;
    toResultExtract *Source;

public:
    toInvalid(QWidget *parent, toConnection &connection);

public slots:
    virtual void changeSelection(void);
    virtual void refresh(void);
    virtual void recompileSelected(void);
};

#endif
