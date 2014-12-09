//============================================================================
// Generic thread class
// (C)2006 Bert vt Veer
//============================================================================

#ifndef THREAD_H
#define THREAD_H

// Platform-specific stuff
#ifdef _MSC_VER

#pragma once
#include <windows.h>
#include <process.h>

#define THREAD_MUTEX_T CRITICAL_SECTION
#define THREAD_ID_T HANDLE
//unsigned long
#define THREAD_TYPE_T void
#define THREAD_RET_T

#define LOCK_MUTEX EnterCriticalSection(&mutex)
#define UNLOCK_MUTEX LeaveCriticalSection(&mutex)
#define START_THREAD(f,p) InitializeCriticalSection(&mutex); id=(HANDLE)_beginthread(f, 0, p)

#else // assuming Linux or anything supporting pthreads

#include <pthread.h>

#define THREAD_MUTEX_T pthread_mutex_t
#define THREAD_ID_T pthread_t
#define THREAD_TYPE_T void*
#define THREAD_RET_T 0

#define LOCK_MUTEX pthread_mutex_lock(&mutex)
#define UNLOCK_MUTEX pthread_mutex_unlock(&mutex)
#define START_THREAD(f,p) pthread_create(&id, NULL, f, p)

#endif

//============================================================================
// Implementation
//============================================================================

namespace pro {


// Main Thread class
class Thread 
{
	private:
	protected:
		// typedefs
		typedef void (*user_func_t)(void);
		typedef THREAD_TYPE_T (*fptr_t)(void *);
		typedef THREAD_TYPE_T type_t;
		enum e_threadstate { error,	init, running, stopped, finished };

		struct s_descriptor {
			Thread* self;
			user_func_t userfunc;
			s_descriptor() {}
			s_descriptor(Thread* pSelf, user_func_t pUserFunc)
				: self(pSelf), userfunc(pUserFunc) {}
		} descriptor;

		// housekeeping
		THREAD_MUTEX_T mutex;
		THREAD_ID_T id;
		e_threadstate state;

		// dispatch function (actual thread routine)
		friend static type_t dispatch(void* pDesc) {
			s_descriptor _desc = *(s_descriptor*)pDesc;
			while (_desc.self->state == running) (*(_desc.userfunc))();
			_desc.self->state = finished;
			return THREAD_RET_T;
		}

	public:
		// construct/destruct
		Thread() : state(init) {}
		Thread(user_func_t pFunc) { Run(pFunc); }
		virtual ~Thread() {
			if (state == running) {	End(); }
		}

		// user functions
		void Lock() { LOCK_MUTEX; }
		void Unlock() { UNLOCK_MUTEX; }
		void Run(user_func_t pFunc) {
			//static s_descriptor _desc(this, pFunc);
			descriptor = s_descriptor(this, pFunc);
			state = running;
			START_THREAD(dispatch, (void*)&descriptor);
			//SetThreadPriority(id, THREAD_PRIORITY_HIGHEST);//// Windows only!!
		}
		void End() { 
			state = stopped;
			//while (state!=finished); // wait
		}
};


}; // namespace pro
#endif // THREAD_H

