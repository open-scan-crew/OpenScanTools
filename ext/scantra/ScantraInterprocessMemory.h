#pragma once

#include <atomic>

#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>

using namespace boost::interprocess;

class ScantraInterprocessMemory;

class ScantraInterprocessObserver
{
private:

	std::atomic<int>  return_value_;
   std::atomic<bool> ready_;

public:

   ScantraInterprocessMemory* shared_memory_;;

   int index_;

   bool registered_;

	enum Message
	{
      noMessage,
      hasConnected,
      hasDisconnected,
      logText,
      stationNew,
      stationChanged,
      intersectionPlane,
      stationColor,
      newAdjustmentResult,
      projectPath,
      selectStation,
      deselectStation,
      panoramicView,
      createProject,
      hasCreatedProject,
      openProject,
      hasOpenedProject
	};

	Message message_;

	// Condition waits until there is a new message
	bool new_message_;
	boost::interprocess::interprocess_condition  cond_;

   ScantraInterprocessObserver() 
   : shared_memory_(0), registered_(false), index_(-1), message_(noMessage), new_message_(false), ready_(true), return_value_(-99) 
   {}

   void update( Message message )
   {
      message_ = message;
      ready_ = false;
      new_message_ = true;
      cond_.notify_one();
   }
   
   void setReturn( int value );

   int getReturn() { return return_value_; }

   bool getReady() { return ready_; }
};



class ScantraInterprocessMemory
{
private:

   ScantraInterprocessObserver a_observer[5];

public: 

	// Mutex to protect access to the queue
	boost::interprocess::interprocess_mutex mutex;

	// This mutex is acquired until all updates have been completed
	boost::interprocess::interprocess_mutex      mutex_ready;
	boost::interprocess::interprocess_condition  cond_ready;

	wchar_t  w_array[50][256]; 
	unsigned n_w;
	unsigned n_w_max;

	bool name_eof;
	
	int      i_array[50];      
	unsigned n_i;
	
	double   d_array[50];      
	unsigned n_d;


	ScantraInterprocessMemory() 
   : n_w(0), n_w_max(50), name_eof(true), n_i(0), n_d(0)
   {
      for ( int i = 0; i < 5; i++ )
         a_observer[i].index_ = i;
   }

   ScantraInterprocessObserver* registerObserver()
   {
      scoped_lock<interprocess_mutex> lock(mutex);

      for ( unsigned i = 0; i < 5; i++ )
      {
         if ( ! a_observer[i].registered_ )
         {
            a_observer[i].registered_ = true;
            return &(a_observer[i]);
         }
      }
      
      return 0;
   }

   void deregisterObserver( ScantraInterprocessObserver* obs )
   {
      if ( ! obs )
         return;

      scoped_lock<interprocess_mutex> lock(mutex);

      int i_obs = obs->index_;
      if ( i_obs < 0 || i_obs > 4 )
         return;

      a_observer[i_obs].registered_ = false;
   }

   unsigned countObserver()
   {
      unsigned n_obs = 0;
      for ( unsigned i = 0; i < 5; i++ )
      {
         if ( a_observer[i].registered_ )
            n_obs++;
      }
      return n_obs;
   }

   bool ready()
   {
      for ( unsigned i = 0; i < 5; i++ )
      {
         if ( ! a_observer[i].registered_ )
            continue;

         if ( ! a_observer[i].getReady() )
            return false;            
      }

      return true;
   }

   int update( ScantraInterprocessObserver* sender )
   {
      if ( ! sender )
         return -2;

      int i_sender = sender->index_;
      if ( i_sender < 0 || i_sender > 4 )
         return 1;

      // --------------------------------------------
      // All observers except the sender are notified
      // --------------------------------------------
      {
         scoped_lock<interprocess_mutex> lock(mutex);

         for ( int i_obs = 0; i_obs < i_sender; i_obs++ )
         { 
            if ( a_observer[i_obs].registered_ ) 
               a_observer[i_obs].update( sender->message_ );
         }

         for ( int i_obs = i_sender+1; i_obs < 5; i_obs++ )
         { 
            if ( a_observer[i_obs].registered_ ) 
               a_observer[i_obs].update( sender->message_ );
         }
      }

      scoped_lock<interprocess_mutex> lock_ready(mutex_ready);
      
//      while ( ! ready() )
//         cond_ready.wait(lock_ready);

      // -------------------------------------------
      // Waits until all updated observers are ready
      // -------------------------------------------
      boost::posix_time::ptime tin = boost::posix_time::microsec_clock::universal_time();
      boost::posix_time::ptime waitTime = tin + boost::posix_time::milliseconds(2000);

      while (!ready()) 
      {
         if (!cond_ready.timed_wait(lock_ready, waitTime))
         {
            // -------------------------------------------------------------
            // In the event of a timeout, all other observers are logged off
            // -------------------------------------------------------------
            for ( int i_obs = 0; i_obs < i_sender; i_obs++ )
               a_observer[i_obs].registered_ = false; 

            for ( int i_obs = i_sender+1; i_obs < 5; i_obs++ )
               a_observer[i_obs].registered_ = false; 

            return -1;
         }
      } 

      return 0;
   }
};



inline void ScantraInterprocessObserver::setReturn( int value )
{
   return_value_ = value;
   ready_ = true;
   shared_memory_->cond_ready.notify_one();
}
