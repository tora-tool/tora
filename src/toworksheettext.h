/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOWORKSHEET_TEXT_H
#define TOWORKSHEET_TEXT_H

#include "config.h"

#include "tohighlightedtext.h"
#include "toworksheet.h"

class toWorksheetText : public toHighlightedText
{
    toWorksheet *Worksheet;

public:
    toWorksheetText(toWorksheet *worksheet,
                    QWidget *parent,
                    const char *name = NULL);

    virtual bool editSave(bool askfile);
    virtual bool editOpen(QString suggestedFile);
};


#endif
