/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOSTORAGE_H
#define TOSTORAGE_H

#include "config.h"
#include "ui_tostoragedatafileui.h"
#include "ui_tostoragedialogui.h"
#include "ui_tostoragetablespaceui.h"
#include "ui_todroptablespaceui.h"
#include "totool.h"
#include "toresultstorage.h"

#include <list>

#include <qdialog.h>
#include <QLabel>

class QCheckBox;
class QLabel;
class QLineEdit;
class QMenu;
class QPushButton;
class QRadioButton;
class QSpinBox;
class QSplitter;
class QTabWidget;
class toConnection;
class toFilesize;
class toResultStorage;
class toResultView;
class toStorageDefinition;
class toStorageDialog;
class QTableView;


class toStorageTablespace : public QWidget, public Ui::toStorageTablespaceUI
{
    Q_OBJECT

    bool Modify;
    bool LoggingOrig;
    bool OnlineOrig;
    bool PermanentOrig;

public:
    toStorageTablespace(QWidget *parent = 0, const char *name = 0, Qt::WFlags fl = 0);

    std::list<QString> sql(void);
    bool allowStorage(void);

    friend class toStorageDialog;
signals:
    void allowStorage(bool);
    void tempFile(bool);

public slots:
    virtual void permanentToggle(bool);
    virtual void dictionaryToggle(bool);
    virtual void uniformToggle(bool);
    virtual void allowDefault(bool);
};

class toDropTablespace : public QWidget, public Ui::toDropTablespaceUI
{
    Q_OBJECT
public:
    toDropTablespace(QWidget *parent = 0, const char *name = 0, Qt::WFlags fl = 0);
    std::list<QString> sql(void);
signals:
    void validContent(bool);
};

class toStorageDatafile : public QWidget, public Ui::toStorageDatafileUI
{
    Q_OBJECT

    bool Modify;
    int InitialSizeOrig;
    int NextSizeOrig;
    int MaximumSizeOrig;
    QString FilenameOrig;
    bool Tempfile;

public:
    toStorageDatafile(bool tempFile, bool dispName,
                      QWidget *parent = 0, const char *name = 0, Qt::WFlags fl = 0);

    QString getName(void);
    std::list<QString> sql();

    friend class toStorageDialog;
signals:
    void validContent(bool);

public slots:
    virtual void browseFile(void);
    virtual void autoExtend(bool);
    virtual void setTempFile(bool temp)
    {
        Tempfile = temp;
    }

    virtual void maximumSize(bool);
    virtual void valueChanged(const QString &);
};

class toStorageDialog : public QDialog, public Ui::toStorageDialogUI
{
    Q_OBJECT
    enum
    {
        NewTablespace,
        NewDatafile,
        ModifyTablespace,
        ModifyDatafile,
        DropTablespace
    } Mode;
    QString TablespaceOrig;
    void Setup(void);
public:
    toStorageDatafile *Datafile;
    toStorageTablespace *Tablespace;
    toStorageDefinition *Default;
    toDropTablespace *Drop;

    toStorageDialog(toConnection &conn, const QString &Tablespace, QWidget *parent);
    toStorageDialog(toConnection &conn, const QString &Tablespace,
                    const QString &file, QWidget *parent);
    toStorageDialog(const QString &tablespace, QWidget *parent, bool drop = false);

    std::list<QString> sql(void);
public slots:
    void validContent(bool val);
    void allowStorage(bool val);
    virtual void displaySQL(void);
};


/*! \brief Display Tablespace content.
This class is used in Qt4 MVC architecture as a replacement
for too much slow toListView item based widget - it's about
70% faster.
toStorageObjectModel is read only model - there are no user
iteractions required.
See toStorage Objects and ObjectsModel.
See Qt4 docs to understand its MVC architecture.
\author Petr Vanek <petr@scribus.info>
*/
class toStorageObjectModel : public QAbstractTableModel
{
    Q_OBJECT

    public:
        toStorageObjectModel(QObject * parent = 0);
        ~toStorageObjectModel();

        /*! Set new data to display. Model is re-validated too.
        Values are converted from std::list to QList due the faster
        access with at() */
        void setValues(std::list<toStorageExtent::extentTotal> values);
        //! Current values
        QList<toStorageExtent::extentTotal> values() { return m_values; };

        int rowCount(const QModelIndex & parent = QModelIndex()) const;
        int columnCount(const QModelIndex & parent = QModelIndex()) const;

        //! Handle DisplayRole and AlignRole.
        QVariant data(const QModelIndex & index,
                      int role = Qt::DisplayRole) const;
        //! Readonly flags are set here.
        Qt::ItemFlags flags(const QModelIndex & index) const;
        //! Heading for horizontal and verticals labels. See HeaderData.
        QVariant headerData(int section,
                            Qt::Orientation orientation,
                            int role = Qt::DisplayRole) const;

    private:
        //! Summary of extents etc.
        QList<toStorageExtent::extentTotal> m_values;
        //! Strings to display as horizontal headers.
        QStringList HeaderData;
};


class toStorage : public toToolWidget
{
    Q_OBJECT

    toResultStorage *Storage;

    QTableView *Objects;
    toStorageObjectModel * ObjectsModel;
    toStorageExtent *Extents;
    QSplitter *ExtentParent;

    QMenu *ToolMenu;

    QAction * UpdateAct;
    QAction *OnlineAct;
    QAction *OfflineAct;
    QAction *CoalesceAct;
    QAction *LoggingAct;
    QAction *EraseLogAct;
    QAction *ModTablespaceAct;
    QAction *ReadOnlyAct;
    QAction *ReadWriteAct;
    QAction *NewFileAct;
    QAction *MoveFileAct;
    QAction *ModFileAct;
    QAction *ExtentAct;
    QAction *TablespaceAct;
    QAction *DropTablespaceAct;
    QAction * CreateTablespaceAct;

public:
    toStorage(QWidget *parent, toConnection &connection);

public slots:
    void refresh(void);

    void coalesce(void);

    void online(void);
    void offline(void);

    void logging(void);
    void noLogging(void);

    void readWrite(void);
    void readOnly(void);

    void newDatafile(void);
    void newTablespace(void);
    void modifyTablespace(void);
    void modifyDatafile(void);
    void moveFile(void);
    void dropTablespace(void);

    void showExtent(bool);
    void showTablespaces(bool);
    void selectionChanged(void);
    void selectObject(const QModelIndex & current, const QModelIndex &);
    void windowActivated(QMdiSubWindow *widget);
};

#endif
