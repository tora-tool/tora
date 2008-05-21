/*****
*
* TOra - An Oracle Toolkit for DBA's and developers
* Copyright (C) 2003-2005 Quest Software, Inc
* Portions Copyright (C) 2005 Other Contributors
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation;  only version 2 of
* the License is valid for this program.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*      As a special exception, you have permission to link this program
*      with the Oracle Client libraries and distribute executables, as long
*      as you follow the requirements of the GNU GPL in regard to all of the
*      software in the executable aside from Oracle client libraries.
*
*      Specifically you are not permitted to link this program with the
*      Qt/UNIX, Qt/Windows or Qt Non Commercial products of TrollTech.
*      And you are not permitted to distribute binaries compiled against
*      these libraries without written consent from Quest Software, Inc.
*      Observe that this does not disallow linking to the Qt Free Edition.
*
*      You may link this product with any GPL'd Qt library such as Qt/Free
*
* All trademarks belong to their respective owners.
*
*****/

#ifndef TORUNNABLE_H
#define TORUNNABLE_H

#include <QObject>
#include <QEvent>
#include <QThread>


class toRunnableThread;


/**
 * Custom event class for deleting thread
 *
 */
class FinishedEvent : public QEvent {
    toRunnableThread *Thread;

public:
    FinishedEvent(toRunnableThread *t) : QEvent(QEvent::User) {
        Thread = t;
    }


    toRunnableThread* thread() const {
        return Thread;
    }
};


/**
 * An abstract class designed to be run from a toRunnableThread. This
 * allows you to send signals and use slots from a thread in an
 * intuitive fashion.
 *
 */
class toRunnable : public QObject {
    Q_OBJECT;

    toRunnableThread *Thread;

public:

    /**
     * Create new runnable object. Does not accept a QObject parent as
     * this class will be deleted by toRunnableThread.
     *
     */
    toRunnable() : QObject(0), Thread(0) {
    }

    virtual ~toRunnable() {
    }


    /**
     * Override to provide executable. Thread will exit when run does.
     *
     */
    virtual void run(void) = 0;


    /**
     * Set by toRunnableThread before run()
     *
     */
    void setThread(toRunnableThread *thread) {
        Thread = thread;
    }


    toRunnableThread* thread(void) const {
        return Thread;
    }
};


/**
 * A QThread subclass that uses toRunnable
 *
 */
class toRunnableThread : public QThread {
    Q_OBJECT;

    toRunnable *Runner;
    virtual void run(void);

public:

    /**
     * Creates new thread. Call start to begin.
     *
     */
    toRunnableThread(toRunnable *runner) : QThread(0) {
        Runner = runner;
        Runner->moveToThread(this);
    }


    /**
     * Redeclares public
     *
     */
    int exec() {
        return QThread::exec();
    }
};


/**
 * Can't delete thread from event handler
 *
 */
class toRunnableDeleter : public QObject {
    Q_OBJECT;

public:
    toRunnableDeleter() {
    }


protected:

    /**
     * Overrides qobject and redeclares non-virtual. It's not a good
     * idea to override this.
     *
     */
    void customEvent(QEvent *event);
};

#endif
