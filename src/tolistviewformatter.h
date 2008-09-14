/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */
#ifndef TOLISTVIEWFORMATTER
#define TOLISTVIEWFORMATTER

#include <qstring.h>
#include <QModelIndexList>
#include <QVector>

class toListView;
class toResultModel;


/**
 * Class for storing settings for export.
 *
 */
class toExportSettings
{
public:
    bool    IncludeHeader;
    bool    OnlySelection;
    int     Type;
    QString Separator;
    QString Delimiter;

    QModelIndexList selected;

    toExportSettings(bool _header,
                     bool _selection,
                     int _type,
                     const QString &_sep,
                     const QString &_del)
    {
        IncludeHeader = _header;
        OnlySelection = _selection;
        Type          = _type;
        Separator     = _sep;
        Delimiter     = _del;
    }
};


class toListViewFormatter
{
public:
    toListViewFormatter();
    virtual ~toListViewFormatter();
    virtual QString getFormattedString(toListView& tListView) = 0;
    virtual QString getFormattedString(toExportSettings &settings,
                                       const toResultModel *model) = 0;

protected:
    virtual void endLine(QString &output);
    // build a vector of selected rows for easy searching
    virtual QVector<int> selectedList(const QModelIndexList &selected);
};

#endif
