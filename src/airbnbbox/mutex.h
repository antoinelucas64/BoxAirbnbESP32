#ifndef MUTEX_AETA

#define MUTEX_AETA

#include <pthread.h>
#include <sys/types.h>
namespace aa {

class Mutex{
public: 


  inline void lock(){
    pthread_mutex_lock(&mutex);
    //locker = pthread_self();
  }

  inline void unlock(){
    locker = 0;
    pthread_mutex_unlock(&mutex);    
  }

  /** return thread that works currently on this mutex. */
  inline pthread_t getLocker(){
    return locker;
  }

  private:
  bool isLock = false;
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  pthread_t locker = 0;

};
};
class LockMutex{

public:
  LockMutex(aa::Mutex * mutex_p){
   /* if(!pthread_equal(pthread_self(), mutex_p->getLocker())){*/
      mutex_p->lock();
      mutex = mutex_p;
      Serial.println("lock");
    /*} else{
      // lock twice from same thread
      mutex = 0;
   }
*/
  }

  ~LockMutex(){
    Serial.println("unlock");
    if(mutex != 0) mutex->unlock();
  }

private:

  aa::Mutex * mutex;
};





#endif