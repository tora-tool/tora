/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOWORKINGWIDGET_H
#define TOWORKINGWIDGET_H

#include <QWidget>

class QLabel;
class QPushButton;


/*! \brief Cool widget displayign the "waiting" status for queries.
See its usage in toResultTableView or toScriptSchemaWidget for example.
*/
class toWorkingWidget : public QWidget
{
    Q_OBJECT

    public:
        /*! Interactive allows user to stop running action
        (it has to be handled in code). NonInteractive disables
        the stop widget. */
        enum WorkingWidgetType
        {
            Interactive,
            NonInteractive
        };

        toWorkingWidget(QWidget * parent = 0);

        //! \brief Set the text for display label
        void setText(const QString & text);
        //! \brief Set the widget behaviour. See WorkingWidgetType enum.
        void setType(WorkingWidgetType type = Interactive);

    signals:
        //! \brief This is emitted when user requested "stop/cancel" action.
        void stop();

    public slots:
        /*! \brief Show this widget.
        It just prepare itself for showing if it's in the
        WorkingWidgetType Interactive mode (CurrentType). There has to be
        a forceShow() call connected to QTimer shot to show all
        internal widgets (see toResultTableView for example).
        The widget is shown normally if it's in the NonInteractive mode.
        */
        void show();
        /*! \brief Really show all subwidgets.
        See show() docs.
        */
        void forceShow();

    private:
        // Remember current internal type
        WorkingWidgetType CurrentType;

        // horizontal container for WorkingLabel and WorkingStop
        QWidget *HWorking;

        // label displayed by Working
        QLabel *WorkingLabel;

        // stop button displayed by Working
        QPushButton *WorkingStop;

    private slots:
        //! Handle the stop/cancel button press.
        void stopWorking();
};

#endif
