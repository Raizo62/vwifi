#ifndef _CTHREAD_H_
#define _CTHREAD_H_


#include <pthread.h>
#include <thread>
#include <future>
#include <iostream>
#include <mutex>
//#include <functional>

namespace intthread {


void interruption_point();

struct thread_interrupted{};

class InterruptFlag

{
	private:
		bool _set ;
		std::mutex  _mutex ;
	public:
		void set();
		bool is_set()  ;


};


class AsyncTask

{
	protected :

		std::condition_variable  _condition ;
		std::mutex  _mutex_condition ;

	public:
	
		AsyncTask();	
		void dead() ;

};



extern thread_local InterruptFlag this_thread_interrupt_flag ;

class InterruptibleThread


{
	private:

		InterruptFlag* _interrupt_flag {nullptr} ;
		std::thread _internal_thread ;
		static int number_thread ;
		static std::mutex _number_thread_mutex ;

	public:

		InterruptibleThread();

		pthread_t get_native_handle();

	
		/* include <functional> with this version */
		/*template<typename CLASS>
		void start(CLASS * obj , void  (CLASS::* f)()  ){

			std::function< void(void)> _f =  std::bind(f,*obj) ;

			std::promise<InterruptFlag*> p ;
			_internal_thread = std::thread([_f,&p]{
				p.set_value(&this_thread_interrupt_flag);
				_f();
			});
			_interrupt_flag = p.get_future().get();

		}*/

		template <typename OBJECT, typename FUNC>
		void start(OBJECT * obj , FUNC f  ){
			

			std::promise<InterruptFlag*> p ;
			_internal_thread = std::thread([obj,f,&p]{
				p.set_value(&this_thread_interrupt_flag);
				(obj->*f)();
				});
			_interrupt_flag = p.get_future().get();

		}



		void interrupt();
		void join();
		std::thread::id get_id();
		

		static bool all_thread_interrupted();
			
		static void count_thread();
			

		static void uncount_thread();
			





};

} // intthread space
#endif 
