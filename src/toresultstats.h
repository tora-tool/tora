/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TORESULTSTATS_H
#define TORESULTSTATS_H

#include "config.h"
#include "tobackground.h"
#include "toresultview.h"

class toNoBlockQuery;

#define TO_STAT_BLOCKS 10
#define TO_STAT_MAX 500

/** This widget will displays information about statistics in either a database or a session.
 */

class toResultStats : public toResultView
{
    Q_OBJECT
    /** Session ID to get statistics for.
     */
    int SessionID;
    /** Number of rows of statistics
     */
    int Row;
    /** Get information about if only changed items are to be displayed.
     */
    bool OnlyChanged;
    /** Display system statistics.
     */
    bool System;
    /** Last read values, used to calculate delta values.
     */
    double LastValues[TO_STAT_MAX + TO_STAT_BLOCKS];

    bool Reset;
    toNoBlockQuery *Query;
    toNoBlockQuery *SessionIO;
    toBackground Poll;

    /** Setup widget.
     */
    void setup();
    /** Add value
     */
    void addValue(bool reset, int id, const QString &name, double value);
public:
    /** Create statistics widget for session statistics.
     * @param OnlyChanged Only display changed items.
     * @param ses Session ID to display info about.
     * @param parent Parent widget.
     * @param name Name of widget.
     */
    toResultStats(bool OnlyChanged, int ses, QWidget *parent, const char *name = NULL);
    /** Create statistics widget for the current session statistics.
     * @param OnlyChanged Only display changed items.
     * @param parent Parent widget.
     * @param name Name of widget.
     */
    toResultStats(bool OnlyChanged, QWidget *parent, const char *name = NULL);
    /** Create statistics widget for the current database statistics.
     * @param parent Parent widget.
     * @param name Name of widget.
     */
    toResultStats(QWidget *parent, const char *name = NULL);

    /** Destroy object.
     */
    ~toResultStats();

    /**
     * close this object
     */
    bool close(void);

    /** Reset statistics. Read in last values without updating widget data.
     */
    void resetStats(void);
    /** Support Oracle
     */
    virtual bool canHandle(toConnection &conn);
signals:
    /** Emitted when session is changed.
     * @param ses New session ID.
     */
    void sessionChanged(int ses);
    /** Emitted when session is changed.
     * @param ses New session ID as string.
     */
    void sessionChanged(const QString &);
public slots:
    /** Change the session that the current query will run on.
     * @param query Query to check connection for.
     */
    void changeSession(toQuery &conn);
    /** Change session to specified id.
     * @param ses Session ID to change to.
     */
    void changeSession(int ses);
    /** Update the statistics.
     * @param reset Set delta to current values. This means that the next time this widget
     *              is updated the delta will be from the new values.
     */
    void refreshStats(bool reset = true);
private slots:
    void poll(void);
};

#endif
