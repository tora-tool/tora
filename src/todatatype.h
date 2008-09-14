/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TO_DATATYPE
#define TO_DATATYPE

#include "config.h"

#include <list>
#include <QWidget>

#include "toextract.h"

class QComboBox;
class QLabel;
class QLineEdit;
class QSpinBox;
class toConnection;

/**
 * Widget used to represent a datatype selection for a column.
 *
 */
class toDatatype : public QWidget
{
    Q_OBJECT;

    std::list<toExtract::datatype> Datatypes;

    QComboBox *Type;
    QLabel    *LeftParenthesis;
    QSpinBox  *Size;
    QLabel    *Comma;
    QSpinBox  *Precision;
    QLabel    *RightParenthesis;
    QLineEdit *Custom;
    bool       PreferCustom;

    void setupLabels();
    void setup(toConnection &conn);

public:
    /**
     * Create the widget and specify connection and the datatype.
     *
     * @param conn Connection to specify datatype for.
     * @param def Initial datatype to use.
     * @param parent The parent widget.
     * @param name Name of widget.
     */
    toDatatype(toConnection &conn,
               const QString &def,
               QWidget *parent,
               const char *name = NULL);

    /**
     * Create the widget and specify connection.
     *
     * @param conn Connection to specify datatype for.
     * @param parent The parent widget.
     * @param name Name of widget.
     */
    toDatatype(toConnection &conn, QWidget *parent, const char *name = NULL);

    /**
     * Get a string representation of the type.
     */
    QString type() const;

public slots:
    /**
     * Change the type.
     *
     * @param type The entire type specification (Including posible
     * size and precision)
     */
    void setType(const QString &type);

    /**
     * Indicate if you would prefer to enter a custom datatype or if
     * posible use the simpler controls.
     *
     * @param prefer If true always use custom controls.
     */
    void setCustom(bool prefer);

private slots:
    void changeType(int);
};

#endif
