/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOSCRIPT_H
#define TOSCRIPT_H

#include "config.h"
#include "tohelp.h"
#include "ui_toscriptui.h"

#include <list>
#include <algorithm>

class toExtract;
class toListView;
class toWorksheet;


/*! \brief DB objects exporter/comparator.
It's using toscriptui.ui layout.
\warning Beware of removing confContainer widget in toscriptui.ui file.
It's mandatory for compilation and it can be lost very easily during
designer's editations.
*/
class toScript : public toToolWidget
{
    Q_OBJECT;

    Ui::toScriptUI *ScriptUI;
    //! Additional widget/Tab widget. DDL scripts container.
    toWorksheet *Worksheet;
    //! Result lists for its action
    toListView *DropList;
    toListView *CreateList;
    toListView *SearchList;
    //! Text report summary widget/Tab
    QTextBrowser *Report;

    /*! Setup the selected (referenced) toExtract depending
    on GUI widgets. Including Resize tab.
    */
    void setupExtract(toExtract &);

    /*! Create commin string list with all selected objects
    for given objects selection
    */
    std::list<QString> createObjectList(QItemSelectionModel * selections);

    void fillDifference(std::list<QString> &objects, toTreeWidget *list);

    //! \brief Create separated strings for exporter
    struct PrefixString
    {
        public:
            PrefixString(std::list<QString> & l, QString s);
            void operator()(QString& tmp);
        private:
            std::list<QString> & _l;
            QString _s;
    };

protected:
    void closeEvent(QCloseEvent *event);

public:
    toScript(QWidget *parent, toConnection &connection);
    virtual ~toScript();

public slots:
    //! Perform requested action (depending on the mode)
    void execute(void);

    //! Enable/disable GUI widgets depending on mode specified
    void changeMode(int);

    void keepOn(toTreeWidgetItem *item);

    //! Size tab - add new item
    void newSize(void);
    //! Size tab - remove current item
    void removeSize(void);

    /*! Ask user for output location. A file or a directory.
    It's stored in ScriptUI->Filename then
    */
    void browseFile(void);
};

inline void toScript::PrefixString::operator()(QString& txt)
{
    _l.push_back(_s + txt);
}

inline toScript::PrefixString::PrefixString(std::list<QString> & l, QString s) : _l(l), _s(s)
{}

#endif
