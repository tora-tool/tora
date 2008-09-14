/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#include "config.h"

#include "toworksheettext.h"

#include "utils.h"
#include "toworksheet.h"

#include <QMessageBox>
#include <QFileInfo>
#include <QDir>


toWorksheetText::toWorksheetText(toWorksheet *worksheet,
                                 QWidget *parent,
                                 const char *name)
        : toHighlightedText(parent, name), Worksheet(worksheet) { }


bool toWorksheetText::editSave(bool askfile)
{
    bool ret = toHighlightedText::editSave(askfile);
    Worksheet->setCaption();
    return ret;
}


bool toWorksheetText::editOpen(QString suggestedFile)
{
    int ret = 1;
    if (isModified())
    {
        // grab focus so user can see file and decide to save
        setFocus(Qt::OtherFocusReason);

        ret = TOMessageBox::information(
                  this,
                  qApp->translate("toWorksheetText", "Save changes?"),
                  qApp->translate(
                      "toWorksheetText",
                      "The editor has been changed. Do you want to save them,\n"
                      "discard changes or open file in new worksheet?"),
                  qApp->translate("toWorksheetText", "&Save"),
                  qApp->translate("toWorksheetText", "&Discard"),
                  qApp->translate("toWorksheetText", "&New worksheet"), 0);

        if (ret < 0)
            return false;
        else if (ret == 0)
        {
            if (!editSave(false))
                return false;
        }
    }

    QString fname;
    if (suggestedFile != QString::null)
        fname = suggestedFile;
    else
    {
        QFileInfo file(filename());
        fname = toOpenFilename(file.dir().path(), QString::null, this);
    }
    if (fname.isEmpty())
        return false;
    try
    {
        if (ret == 2)
            toWorksheet::fileWorksheet(fname);
        else
        {
            openFilename(fname);
            Worksheet->setCaption();
        }

        emit fileOpened();
        return true;
    }
    TOCATCH;

    return false;
}

