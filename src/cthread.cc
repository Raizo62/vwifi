#include "cthread.h"
#include <condition_variable>

#include <signal.h>

namespace intthread {


/*********************************************************/
	/* Global variables and functions */
/********************************************************/

thread_local InterruptFlag this_thread_interrupt_flag ;

void interruption_point(){

	if(this_thread_interrupt_flag.is_set())
	{
		throw thread_interrupted();
	}

}


/*********************************************************/
	/* 	InterruptFlag class definitions */
/********************************************************/

void InterruptFlag::set(){

	std::lock_guard<std::mutex> guard(_mutex);
	_set = true ;

}

bool InterruptFlag::is_set() {

	std::lock_guard<std::mutex> guard(_mutex);
	return _set ;

}


/*********************************************************/
	/* InterruptibleThread class definitions */
/********************************************************/



int InterruptibleThread::number_thread = 0 ;

std::mutex  InterruptibleThread::_number_thread_mutex ;

InterruptibleThread::InterruptibleThread(){

	count_thread();


}


bool InterruptibleThread::started()
{

	return _started ;
}

void InterruptibleThread::interrupt(){

	if (_interrupt_flag != nullptr){
		_interrupt_flag->set();

	}
	_started = false ;

}

void InterruptibleThread::join(){

	_internal_thread.join();

}

pthread_t  InterruptibleThread::get_native_handle(){

	return _internal_thread.native_handle();
}


std::thread::id InterruptibleThread::get_id(){


	return _internal_thread.get_id();
}


void InterruptibleThread::count_thread(){

		std::unique_lock<std::mutex> lk(_number_thread_mutex);
		number_thread++;
}

void InterruptibleThread::uncount_thread(){

	std::unique_lock<std::mutex> lk(_number_thread_mutex);
	number_thread--;
}

bool InterruptibleThread::all_thread_interrupted(){

	std::unique_lock<std::mutex> lk(_number_thread_mutex);
	return (number_thread == 0);
}




/*********************************************************/
	/* AsyncTask class definitions */
/********************************************************/



AsyncTask::AsyncTask(){
}



void AsyncTask::dead()  {

	std::unique_lock<std::mutex> lk(_mutex_condition);
	InterruptibleThread::uncount_thread() ;
	_condition.notify_all();
	lk.unlock();

}

}
