/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */
#include <QFocusEvent>

#include "utils.h"
#include "totextview.h"


bool toTextView::editSave(bool)
{
    QString fn = toSaveFilename(QString::null, QString::fromLatin1("*.html"), this);
    if (!fn.isEmpty())
    {
        if (!toWriteFile(fn, toPlainText()))
            return false;
        return true;
    }
    return false;
}

void toTextView::editCopy(void)
{
    copy();
}

void toTextView::editSelectAll(void)
{
    selectAll();
}

void toTextView::focusInEvent (QFocusEvent *e)
{
    receivedFocus();
    QTextBrowser::focusInEvent(e);
}
