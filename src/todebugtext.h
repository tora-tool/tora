/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TODEBUGTEXT_H
#define TODEBUGTEXT_H

#include "config.h"
#include "tohighlightedtext.h"

#include <totreewidget.h>
//Added by qt3to4:
#include <QString>
#include <QMouseEvent>

#include <algorithm>

class toConnection;
class toDebug;

class toBreakpointItem : public toTreeWidgetItem
{
    int Line;
    int Namespace;
public:
    toBreakpointItem(toTreeWidget *parent, toTreeWidgetItem *after,
                     const QString &schema, const QString &type, const QString &object, int line);
    void setBreakpoint(void);
    void clearBreakpoint(void);
    void disableBreakpoint(void);
    void enableBreakpoint(void);
    int line()
    {
        return Line;
    }
};

class toDebugText : public toHighlightedText
{
    Q_OBJECT
    QString Schema;
    QString Object;
    QString Type;
    int LastX;
    toDebug *Debugger;

    toTreeWidget *Breakpoints;
    bool NoBreakpoints;
    toBreakpointItem *FirstItem;
    toBreakpointItem *CurrentItem;

    bool checkItem(toBreakpointItem *item);
    bool hasBreakpoint(int row);
    int breakMarker;
    int disabledBreakMarker;
    static int ID;
public:
    toDebugText(toTreeWidget *breakpoints,
                QWidget *parent,
                toDebug *debugger);

    void toggleBreakpoint(int row = -1, bool enable = false);

    void setData(const QString &schema, const QString &type, const QString &data);
    const QString &schema(void) const
    {
        return Schema;
    }
    const QString &object(void) const
    {
        return Object;
    }
    void setType(const QString &type)
    {
        setData(Schema, type, Object);
    }
    void setSchema(const QString &schema)
    {
        setData(schema, Type, Object);
    }
    const QString &type(void) const
    {
        return Type;
    }
    void clear(void);

    bool readData(toConnection &connection, toTreeWidget *);
    bool readErrors(toConnection &connection);
    bool compile(void);

    virtual void exportData(std::map<QString, QString> &data, const QString &prefix);
    virtual void importData(std::map<QString, QString> &data, const QString &prefix);
protected slots:
    virtual void toggleBreakpoint(int margin, int line, Qt::KeyboardModifiers state);
protected:
    virtual void mouseMoveEvent (QMouseEvent *me);

};

#endif
