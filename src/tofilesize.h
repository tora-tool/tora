/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOFILESIZE_H
#define TOFILESIZE_H

#include "config.h"

#include <QGroupBox>

class QRadioButton;
class QSpinBox;

/**
 * A widget to get a size value for Oracle objects.
 */
class toFilesize : public QGroupBox
{
    Q_OBJECT

    /**
     * @internal
     * Value of widget
     */
    QSpinBox *Value;
    /**
     * @internal
     * MB unit indicator button
     */
    QRadioButton *MBSize;
    /**
     * @internal
     * KB unit indicator button
     */
    QRadioButton *KBSize;
    /**
     * Setup this widget
     */
    void setup(void);
private slots:
    /**
     * @internal
     * Change the unit type callback.
     */
    void changeType(bool);
    /**
     * @internal
     * Size value changed callback.
     */
    void changedSize(void)
    {
        emit valueChanged();
    }
signals:
    /**
     * Emitted when value was changed in the widget.
     */
    void valueChanged(void);
public:
    /**
     * Create a filesize widget.
     * @param parent Parent widget.
     * @param name Name of widget.
     */
    toFilesize(QWidget* parent = 0, const char* name = 0);
    /**
     * Create a filesize widget.
     * @param title Title of size selection frame.
     * @param parent Parent widget.
     * @param name Name of widget.
     */
    toFilesize(const QString &title, QWidget* parent = 0, const char* name = 0);
    /**
     * Set the value of the widget in KB.
     * @param sizeInKB Size in KB.
     */
    void setValue(int sizeInKB);
    /**
     * Get value in KB.
     * @return Size selected in KB.
     */
    int value(void);
    /**
     * Get an oracle size specification string.
     * @return String describing size in valid Oracle format.
     */
    QString sizeString(void);
};

#endif
