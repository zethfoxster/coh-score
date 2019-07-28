//This was taken from the code project.

#ifndef REDIRECT_H_INCLUDED__
#define REDIRECT_H_INCLUDED__

class CRedirect
{
public:

	//--------------------------------------------------------------------------
	//	constructor
	//--------------------------------------------------------------------------
	CRedirect
		(
		LPCTSTR		szCommand,
		CEdit		*pEdit,
		CProgressCtrl *pProgress,
		LPCTSTR		szCurrentDirectory = NULL
		);

	//--------------------------------------------------------------------------
	//	destructor
	//--------------------------------------------------------------------------
	virtual ~CRedirect();

	//--------------------------------------------------------------------------
	//	public member functions
	//--------------------------------------------------------------------------
	virtual void		Run();
	virtual	void		Stop();
	virtual double		PercentDone(){return m_done;};

protected:

	//--------------------------------------------------------------------------
	//	member functions
	//--------------------------------------------------------------------------
	void				AppendText(LPCTSTR Text);
	void				PeekAndPump();
	void				SetSleepInterval(DWORD dwMilliseconds);
	void				ShowLastError(LPCTSTR szText);
	void				updatePercentDone(double done);

	//--------------------------------------------------------------------------
	//	member data
	//--------------------------------------------------------------------------
	bool				m_bStopped;
	DWORD				m_dwSleepMilliseconds;
	CEdit				*m_pEdit;
	CProgressCtrl		*m_Progress;
	LPCTSTR				m_szCommand;
	LPCTSTR				m_szCurrentDirectory;
	double				m_done;

};

#endif	// REDIRECT_H_INCLUDED__
