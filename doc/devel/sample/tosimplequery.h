#ifndef TO_SAMPLE_TOOL_H
#define TO_SAMPLE_TOOL_H

#include "core/totool.h"

class toResultView;
class QLineEdit;
class toConnection;

class toSample : public toToolWidget
{
    Q_OBJECT;

public:
    toSample(/*toTool *tool,*/ QWidget *parent, toConnection &connection);
    virtual ~toSample() {};
    virtual void slotWindowActivated(toToolWidget*){};

private slots:
    void execute(void);

private:
    toResultView *Result;
    QLineEdit *Statement;
    QAction *updateAct;
};

#endif

