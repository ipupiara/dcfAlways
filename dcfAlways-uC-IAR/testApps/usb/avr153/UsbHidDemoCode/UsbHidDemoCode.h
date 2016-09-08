// UsbHidDemoCode.h : main header file for the USBHIDDEMOCODE application
//

#if !defined(AFX_USBHIDDEMOCODE_H__DD281565_9143_4401_93E0_87831D1AF790__INCLUDED_)
#define AFX_USBHIDDEMOCODE_H__DD281565_9143_4401_93E0_87831D1AF790__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CUsbHidDemoCodeApp:
// See UsbHidDemoCode.cpp for the implementation of this class
//

class CUsbHidDemoCodeApp : public CWinApp
{
public:
	CUsbHidDemoCodeApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUsbHidDemoCodeApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CUsbHidDemoCodeApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_USBHIDDEMOCODE_H__DD281565_9143_4401_93E0_87831D1AF790__INCLUDED_)
