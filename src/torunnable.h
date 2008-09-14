/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

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
