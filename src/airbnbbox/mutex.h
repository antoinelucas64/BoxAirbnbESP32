#ifndef MUTEX_AETA

#define MUTEX_AETA

#include <esp_pthread.h>
#include <sys/types.h>
namespace aa {

class Mutex {
public:
  Mutex(String name_p, int delay = 0):
    isLock(false),
    mutex(PTHREAD_MUTEX_INITIALIZER),
    locker(0),
    lastAccess(0),
    delayBetweenAccess(delay),
    name(name_p)
  {
  }

  inline void lock() {
    pthread_mutex_lock(&mutex);
    int current = millis();
    if(delayBetweenAccess>0 &&  current  - lastAccess < delayBetweenAccess){
      int wait = delayBetweenAccess - current + lastAccess;
      if(wait > delayBetweenAccess) delayBetweenAccess = wait;
      Serial.print("Wait ");
      Serial.println(wait);
      delay(wait);
    }
    //locker = pthread_self();
  }

  inline void unlock() {
    locker = 0;
    lastAccess= millis();
    pthread_mutex_unlock(&mutex);
  }

  /** return thread that works currently on this mutex. */
  inline pthread_t getLocker() {
    return locker;
  }

  inline const String & getName(){
    return name;
  }

private:
  bool isLock = false;
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  pthread_t locker = 0;
  int lastAccess;
  int delayBetweenAccess;
  String name;

};
};
class LockMutex {
  
public:
  LockMutex(aa::Mutex* mutex_p) {
    /* if(!pthread_equal(pthread_self(), mutex_p->getLocker())){*/
    Serial.print("try to lock ");
    Serial.println(mutex_p->getName());
    mutex_p->lock();
    mutex = mutex_p;
    Serial.println("lock");
    /*} else{
      // lock twice from same thread
      mutex = 0;
   }
*/
  }

  ~LockMutex() {
    Serial.print("unlock ");
    Serial.println(mutex->getName());
    if (mutex != 0) mutex->unlock();
  }

private:
  aa::Mutex* mutex;
};





#endif