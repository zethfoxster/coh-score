// Job.cpp: implementation of the CJob class.
//
//////////////////////////////////////////////////////////////////////

#include "Job.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CJob job;


DWORD WINAPI IOOBjectTimerCallback( LPVOID param ){
	
	CIOObject *pIOObject;
	
	pIOObject = (CIOObject *)param;
	pIOObject->OnTimerCallback();

	return 0;
}

void CJob::RunTimer()
{
_BEFORE
    HANDLE events[2];
    events[0] = m_terminateEvent;
    events[1] = m_timerEvent;

    for( ; ; ) {
		DWORD tick = GetTickCount();
	    LONG waitTime = (LONG)(m_topTick - tick);
	    if(waitTime <= 0) {
		    m_lock.Enter();
		    CIOTimer &top = m_timerQueue.top();
		    CIOObject *pObject = top.m_pObject;
		    for( ; ; ) {
			    m_timerQueue.pop();
				pObject->OnTimerCallback();
			    top = m_timerQueue.top();
			    waitTime = (LONG)(top.m_tick - tick);
			    if(waitTime > 0) {
				    m_topTick = top.m_tick;
				    break;
			    }
			    pObject = top.m_pObject;
		    }
		    m_lock.Leave();
	    }

		DWORD waitResult = WaitForMultipleObjects(2, events, FALSE, waitTime);
		if ( m_terminating ) {
			m_lock.Enter();
			for( ; ; ) {
				if(m_timerQueue.empty())
					break;
				m_timerQueue.top().m_pObject->ReleaseRef();
				m_timerQueue.pop();
			}
			m_lock.Leave();
			return;
		}
	}
_AFTER_FIN
}

void CJob::RunEvent()
{
 _BEFORE
    for( ; ; ) {
		DWORD waitResult = WaitForMultipleObjects((DWORD)m_handles.size(), &(m_handles.front()), FALSE, INFINITE);

        if(m_terminating) {
			std::vector<CIOObject *>::iterator it;
			for( it = m_objects.begin(); it != m_objects.end();) {
				it = m_objects.erase(it);
			}
			return;
		}
		if(waitResult >= WAIT_OBJECT_0 && 
          waitResult < WAIT_OBJECT_0 + m_handles.size()) {
			waitResult -= WAIT_OBJECT_0;
			m_objects[waitResult]->OnEventCallback();
		}
	}

 _AFTER_FIN
}

int CJob::PushTimer(CIOObject *pObject, DWORD milisec, BOOL setEvent)
{
    milisec += GetTickCount();

    m_lock.Enter();

    if(m_terminating) {
        m_lock.Leave();
        return FALSE;
    }

    m_timerQueue.push(CIOTimer(pObject, milisec));

	if((LONG)+(m_topTick - milisec) > 0) {
		m_topTick = milisec;
		m_lock.Leave();
        if(setEvent) {
		    SetEvent(m_timerEvent);
        }
    } else {
        m_lock.Leave();
    }

	return TRUE;
}

BOOL CJob::PushEvent(HANDLE handle, CIOObject *pObject)
{
    m_lock.Enter();

    if(m_terminating) {
        m_lock.Leave();
        return FALSE;
    }

    if(m_handles.size() >= MAXIMUM_WAIT_OBJECTS) {
        m_lock.Leave();
        return FALSE;
    } else {
	    m_handles.push_back(handle);
	    m_objects.push_back(pObject);
        m_lock.Leave();
        size_t k=m_objects.size();
		return TRUE;
    }
}

CJob::CJob()
{

    m_terminating = FALSE;

    m_timerEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    m_terminateEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    pIOTopTimer = new CIOTopTimer();
	//pIOTopTimer->AddRef();
	m_topTick = TOP_TICK_EXPIRE + GetTickCount();
    m_timerQueue.push(CIOTimer(pIOTopTimer, m_topTick));
    m_handles.push_back(m_terminateEvent);
    m_objects.push_back(pIOTopTimer);
	
}

CJob::~CJob()
{ 
	CloseHandle( m_timerEvent );
	CloseHandle( m_terminateEvent );
	//TODO - TBROWN - Hmmm - sometimes pIOTopTimer leaks if not deleted here, and other times it's been deleted by the time it gets here.
}

void CJob::SetTerminate()
{
    m_terminating = TRUE;
    SetEvent(m_terminateEvent);
	SetEvent(m_timerEvent);
}

void CIOTopTimer::OnTimerCallback(void)
{
    if(RegisterTimer(TOP_TICK_EXPIRE)) {
        ReleaseRef();
    }
	return;
}