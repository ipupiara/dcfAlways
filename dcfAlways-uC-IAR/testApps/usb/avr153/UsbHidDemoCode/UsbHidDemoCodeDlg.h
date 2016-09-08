// UsbHidDemoCodeDlg.h : header file
//

#if !defined(AFX_USBHIDDEMOCODEDLG_H__F9AA5915_C6E4_4CF0_95B7_2C6EB1964BAB__INCLUDED_)
#define AFX_USBHIDDEMOCODEDLG_H__F9AA5915_C6E4_4CF0_95B7_2C6EB1964BAB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CUsbHidDemoCodeDlg dialog

class CUsbHidDemoCodeDlg : public CDialog
{
// Construction
public:
	void AddRecievedData(CString NewData);
	
	void ConnectDevice();
	void DisableLedButton();
	void EnableLedButton();
	boolean Led1;
	boolean Led2;
	boolean Led3;
	boolean Led4;
	boolean IsConnected;
	HINSTANCE hLib;  
	int Vid;
	int Pid;

	CUsbHidDemoCodeDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CUsbHidDemoCodeDlg)
	enum { IDD = IDD_USBHIDDEMOCODE_DIALOG };
	CButton	m_FwUpgrade;
	CListBox	m_RecievedData;
	CStatic	m_Status;
	CButton	m_Led4;
	CButton	m_Led3;
	CButton	m_Led2;
	CButton	m_Led1;
	CString	m_PID;
	CString	m_VID;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUsbHidDemoCodeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CUsbHidDemoCodeDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnLed1();
	afx_msg void OnLed2();
	afx_msg void OnLed3();
	afx_msg void OnLed4();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnFwUpgrade();
	virtual void OnCancel();
	afx_msg void OnButtonVidPid();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	BOOL OnDeviceChange(UINT nEventType, DWORD dwData);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_USBHIDDEMOCODEDLG_H__F9AA5915_C6E4_4CF0_95B7_2C6EB1964BAB__INCLUDED_)
