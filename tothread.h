#ifndef __TOTHREAD_H
#define __TOTHREAD_H

#include <semaphore.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>

class toSemaphore {
private:
  sem_t			Semaphore;
  void			init(int val);

public:
  toSemaphore();
  toSemaphore(const toSemaphore &);
  toSemaphore(int val);
  ~toSemaphore();

  void up();
  void down();
  bool tryDown();
  int getValue();
};

class toTask {
public:
  virtual ~toTask() { }
  virtual void run(void) = 0;
};

class toLock {
private:
  pthread_mutex_t Mutex;
public:
  toLock(void);
  toLock(const toLock &);
  ~toLock();

  void lock(void);
  void unlock(void);
  bool tryLock(void);

  operator pthread_mutex_t *()
  { return &Mutex; }
};

class toLocker {
private:
  toLock &Lock;
public:
  toLocker(toLock &lock)
    : Lock(lock)
  { Lock.lock(); }
  toLocker(const toLocker &);
  ~toLocker()
  { Lock.unlock(); }
};

class toThread {
private:
  pthread_t		Thread;
  pthread_attr_t	ThreadAttr;
  toTask		*Task;
  toSemaphore		StartSemaphore;
  void			initAttr(void);
  friend void		*toThreadStartWrapper(void*);
  
  toThread(const toThread &);
public:
  toThread(toTask *);
  ~toThread();
  
  void start(void);
  void startAsync(void);

  operator pthread_t()
  { return Thread; };

  static pthread_t getID(void)
  { return pthread_self(); }
  static void yield(void)
  { sched_yield(); }
  static void sleep(struct timespec time);
};

#endif
