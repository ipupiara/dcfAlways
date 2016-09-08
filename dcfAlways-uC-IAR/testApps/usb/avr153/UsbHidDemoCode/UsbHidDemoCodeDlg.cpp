// UsbHidDemoCodeDlg.cpp : implementation file
//
////////////////////////////////////////////////////////////////////////////////
//  This demo program exercises the functions of the AtUsbHid library.
//  The library functions prototypes can be found in the last part of the
//   AtUsbHid.h file.
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UsbHidDemoCode.h"
#include "UsbHidDemoCodeDlg.h"
#include <winuser.h>
#include <windows.h>
#include <dbt.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



/////////////////////////////////////////////////////////////////////////////
// Include Atmel Hid Usb
#include "AtUsbHid.h"

#define DEFAULT_VID 0x03EB
#define DEFAULT_PID 0x2013


/*---------------------------------------------------------------------------

FUNCTION: handleError

PURPOSE: Call when an error is return by a function call

PARMATERS: DWORD errorCode - error code that represent the error
  	
RETURN:
   
COMMENTS:
  
---------------------------------------------------------------------------*/
void handleError(DWORD errorCode)
{
    switch(errorCode)
    {
    case ERROR_MOD_NOT_FOUND:
		AfxMessageBox( "Could not find Atmel USB HID DLL: AtUsbHid.dll\nPlease update the PATH variable.\n", MB_ICONSTOP,0);
		exit(-1);        
        break;

    case ERROR_USB_DEVICE_NOT_FOUND:
        OutputDebugString("Error: Could not open the device.\n");
        break;

    case ERROR_USB_DEVICE_NO_CAPABILITIES:
        OutputDebugString("Error: Could not get USB device capabilities.\n");
        break;

    case ERROR_WRITE_FAULT:
        OutputDebugString("Error: Could not write.\n");
        break;

    case ERROR_READ_FAULT:
        OutputDebugString("Error: Could not read.\n");
        break;

    default:
        OutputDebugString("Error: Unknown error code.\n");
    }
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUsbHidDemoCodeDlg dialog

CUsbHidDemoCodeDlg::CUsbHidDemoCodeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CUsbHidDemoCodeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUsbHidDemoCodeDlg)
	m_PID = _T("2402");
	m_VID = _T("03EB");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);


}

void CUsbHidDemoCodeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUsbHidDemoCodeDlg)
	DDX_Control(pDX, IDC_FW_UPGRADE, m_FwUpgrade);
	DDX_Control(pDX, IDC_LIST, m_RecievedData);
	DDX_Control(pDX, IDC_STATUS_TEXT, m_Status);
	DDX_Control(pDX, IDC_LED4, m_Led4);
	DDX_Control(pDX, IDC_LED3, m_Led3);
	DDX_Control(pDX, IDC_LED2, m_Led2);
	DDX_Control(pDX, IDC_LED1, m_Led1);
	DDX_Text(pDX, IDC_PID, m_PID);
	DDV_MaxChars(pDX, m_PID, 4);
	DDX_Text(pDX, IDC_VID, m_VID);
	DDV_MaxChars(pDX, m_VID, 4);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CUsbHidDemoCodeDlg, CDialog)
	//{{AFX_MSG_MAP(CUsbHidDemoCodeDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_LED1, OnLed1)
	ON_BN_CLICKED(IDC_LED2, OnLed2)
	ON_BN_CLICKED(IDC_LED3, OnLed3)
	ON_BN_CLICKED(IDC_LED4, OnLed4)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_PW_UPGRADE, OnFwUpgrade)
	ON_BN_CLICKED(IDC_BUTTON_VID_PID, OnButtonVidPid)
	//}}AFX_MSG_MAP
	ON_WM_DEVICECHANGE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUsbHidDemoCodeDlg message handlers

BOOL CUsbHidDemoCodeDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	

	// Set default Vid and Pid
	Vid  = DEFAULT_VID;	
	Pid  = DEFAULT_PID;
	

	// Disable All Led Button until connection
	DisableLedButton();
	
	// Explicitely load the AtUsbHid library.
	hLib = LoadLibrary(AT_USB_HID_DLL);
    if (hLib == NULL)        
    {
        handleError(GetLastError());
        return 0;
    }

    // Get USB HID library functions addresses.    
    if (loadFuncPointers(hLib)==NULL) {        
		AfxMessageBox( "Could not get USB HID library functions addresses",MB_ICONSTOP,0);
        return 0;
	}

	// try to connect Device
    ConnectDevice();

	DYNCALL(hidRegisterDeviceNotification)((m_hWnd));	

	SetTimer(1, 200, 0);

	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CUsbHidDemoCodeDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CUsbHidDemoCodeDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CUsbHidDemoCodeDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

/*---------------------------------------------------------------------------

FUNCTION: OnLed1

PURPOSE: Toggle Led 1 if Led1 button has been pressed

PARMATERS:

	
RETURN:
   

COMMENTS:
  
---------------------------------------------------------------------------*/
void CUsbHidDemoCodeDlg::OnLed1() 
{
	
	if(Led1 == false) {
		Led1 = true;
		m_Led1.SetWindowText(_T("LED 1 ON"));		
		DYNCALL( writeData )((UCHAR *)"11");
	}
	else {
		Led1 = false;
		m_Led1.SetWindowText(_T("LED 1 OFF"));				
		DYNCALL(writeData)((UCHAR *)"01");
	}
}

/*---------------------------------------------------------------------------

FUNCTION: OnLed2

PURPOSE: Toggle Led 2 if Led2 button has been pressed

PARMATERS:

	
RETURN:
   

COMMENTS:
  
---------------------------------------------------------------------------*/
void CUsbHidDemoCodeDlg::OnLed2() 
{
	if(Led2 == false) {
		Led2 = true;
		m_Led2.SetWindowText(_T("LED 2 ON"));		
		DYNCALL(writeData)((UCHAR *)"12");
	}
	else {
		Led2 = false;
		m_Led2.SetWindowText(_T("LED 2 OFF"));		
		DYNCALL(writeData)((UCHAR *)"02");
	}
	
}

/*---------------------------------------------------------------------------

FUNCTION: OnLed3

PURPOSE: Toggle Led 3 if Led3 button has been pressed

PARMATERS:

	
RETURN:
   

COMMENTS:
  
---------------------------------------------------------------------------*/
void CUsbHidDemoCodeDlg::OnLed3() 
{
	
	if(Led3 == false) {
		Led3 = true;
		m_Led3.SetWindowText(_T("LED 3 ON"));		
		DYNCALL(writeData)((UCHAR *)"13");
	}
	else {
		Led3 = false;
		m_Led3.SetWindowText(_T("LED 3 OFF"));	
		DYNCALL(writeData)((UCHAR *)"03");
	}

}


/*---------------------------------------------------------------------------

FUNCTION: OnLed4

PURPOSE: Toggle Led 4 if Led4 button has been pressed

PARMATERS:

	
RETURN:
   

COMMENTS:
  
---------------------------------------------------------------------------*/
void CUsbHidDemoCodeDlg::OnLed4() 
{

	if(Led4 == false) {
		Led4 = true;
		m_Led4.SetWindowText(_T("LED 4 ON"));		
		DYNCALL(writeData)((UCHAR*)"14");
	}
	else {
		Led4 = false;
		m_Led4.SetWindowText(_T("LED 4 OFF"));		
		DYNCALL(writeData)((UCHAR*)"04");
	}

}


/*---------------------------------------------------------------------------

FUNCTION: DisableLedButton

PURPOSE: Disable All led button
		 Disbale Firmware Upgrade button 
         Change Led button text
		 Change Current Status to Diconnected
         Stop Read Function timer

PARMATERS:

	
RETURN:
   

COMMENTS:
  
---------------------------------------------------------------------------*/
void CUsbHidDemoCodeDlg::DisableLedButton()
{
	// Set All Led To Off
	Led1 = false ;
	Led2 = false ;
	Led3 = false ;
	Led4 = false ;

	// Disable All Led button
	m_Led1.EnableWindow(false);
	m_Led2.EnableWindow(false);
	m_Led3.EnableWindow(false);
	m_Led4.EnableWindow(false);

	// Disable Firmware Upgrade
	m_FwUpgrade.EnableWindow(false);

	// Change Led Text
	m_Led1.SetWindowText(_T("LED 1"));
	m_Led2.SetWindowText(_T("LED 2"));
	m_Led3.SetWindowText(_T("LED 3"));
	m_Led4.SetWindowText(_T("LED 4"));
	
	m_Status.SetWindowText(_T("Disconnected"));

	CDialog::KillTimer(1);

	IsConnected = false ;
}

/*---------------------------------------------------------------------------

FUNCTION: ConnectDevice

PURPOSE: Connect Device using Current Vid and Pid
         if connection is succefull, change status to Connected
		 if connection fail Status is set to disconnected

PARMATERS:

	
RETURN:
   

COMMENTS:
  
---------------------------------------------------------------------------*/
void CUsbHidDemoCodeDlg::ConnectDevice()
{
	// Open our USB device.
	if (DYNCALL(findHidDevice)(Vid, Pid)) {
		EnableLedButton();		
	}
    else {
		// Can't access leds button if not connected
        DisableLedButton();
    }
}


/*---------------------------------------------------------------------------

FUNCTION: EnableLedButton

PURPOSE: Enable all Led Button
         Enable Firmware upgrade
		 Set all led status to Off
		 Send Led status Off to device
		 Set Read function timer

PARMATERS:

	
RETURN:
   

COMMENTS:
  
---------------------------------------------------------------------------*/
void CUsbHidDemoCodeDlg::EnableLedButton()
{
	
	CString inReport;
	CString outReport;
	CString featureReport;

	// Enable All Led button
	m_Led1.EnableWindow(true);
	m_Led2.EnableWindow(true);
	m_Led3.EnableWindow(true);
	m_Led4.EnableWindow(true);

	// Enable  Firmware Upgrade
	m_FwUpgrade.EnableWindow(true);

	// Change Led Text
	m_Led1.SetWindowText(_T("LED 1 OFF"));
	m_Led2.SetWindowText(_T("LED 2 OFF"));
	m_Led3.SetWindowText(_T("LED 3 OFF"));
	m_Led4.SetWindowText(_T("LED 4 OFF"));


	// Be sure that all led are disbale
	DYNCALL(writeData)((UCHAR*)"01");
	DYNCALL(writeData)((UCHAR*)"02");	
	DYNCALL(writeData)((UCHAR*)"03");
	DYNCALL(writeData)((UCHAR*)"04");
	

	inReport.Format(     "%s: %dByte"," In",DYNCALL(getInputReportLength()));
	outReport.Format(    "%s: %dByte"," Out",DYNCALL(getOutputReportLength()));
	featureReport.Format("%s: %dByte"," Feature",DYNCALL(getFeatureReportLength()));

	CString text = "Connected\t\t"+inReport+","+outReport+","+featureReport;
		
	
	m_Status.SetWindowText(text);

	SetTimer(1,50,0);

	IsConnected = true ;
}

/*---------------------------------------------------------------------------

FUNCTION: OnDeviceChange

PURPOSE: This function is call each time a status change for a device using 
            ON_WM_DEVICECHANGE()
         The function will check if this our device change it status :
		 There is 2 important type of event :
			DBT_DEVICEARRIVAL : in this case, we try to connect a device using 
			   current VID and PID
			DBT_DEVICEREMOVECOMPLETE : if our device as been deconnected,
			  we close the device properly using closeDevice()

		if OnDeviceChange is called by another device nothing is done


PARMATERS: UINT nEventType : Event Id
           DWORD dwData : data associated to the Event

RETURN:

COMMENTS:
  
---------------------------------------------------------------------------*/
BOOL CUsbHidDemoCodeDlg::OnDeviceChange(UINT nEventType, DWORD dwData)
{
	
	int isOurDevice;
	switch(nEventType)
		{		
			case DBT_DEVICEARRIVAL :
				isOurDevice=DYNCALL(isMyDeviceNotification(dwData));
				if(isOurDevice&&IsConnected) {
					OutputDebugString(">>> Our Device Already Connected.\n");	
				}
				else {
					// Connect Only if status is disconnected
					OutputDebugString(">>> A device has been inserted and is now available.\n");
					ConnectDevice();
				}
				
				break;
				
			case DBT_DEVICEREMOVECOMPLETE :
				isOurDevice=DYNCALL(isMyDeviceNotification(dwData));
				if(IsConnected&&isOurDevice) {
					// Close Connection only once
					DisableLedButton();
					DYNCALL(closeDevice());
					OutputDebugString(">>> A device has been removed.\n");
				}
				
				break;			
			default :				
				OutputDebugString(">>> OnDeviceChange : default\n");
				break;
		}		
	return TRUE;
}

/*---------------------------------------------------------------------------

FUNCTION: AddRecievedData

PURPOSE: This function add new message to m_RecievedData CList and remove the 
           100th oldest message to avoid list to be too big.

PARMATERS:  
		CString NewData - New strinf to display in CList

RETURN:

COMMENTS:
  
---------------------------------------------------------------------------*/
void CUsbHidDemoCodeDlg::AddRecievedData(CString NewData)
{
	
	m_RecievedData.AddString( NewData );

	// display only last 100 messages recieved
	if(m_RecievedData.GetCount()>100) {
		m_RecievedData.DeleteString(0);
	}

	// Set Focus on Last Element
	int nCount = m_RecievedData.GetCount(); 	
	if (nCount > 0) m_RecievedData.SetCurSel(nCount-1);
	
}

/*---------------------------------------------------------------------------

FUNCTION: OnTimer

PURPOSE: This function allow us to call the check if a new data as been recieved
          if yes, the buffer imfarmation are display using AddRecievedData
		The Timer for this function must be killed if Connection 
		 is lost uasing function : CDialog::KillTimer(1);
		If a device is connected, the timer must be set using :
		  SetTimer(1,50,0);

PARMATERS:
  nIDEvent	  -	   Timer identifier
  	
RETURN:
   
COMMENTS:
  
---------------------------------------------------------------------------*/
void CUsbHidDemoCodeDlg::OnTimer(UINT nIDEvent) 
{
	UCHAR sbuffer[255];
	if(DYNCALL(readData(sbuffer))!=0) {
		if(sbuffer[0]==1) {
//			AddRecievedData("Button as been pressed");
		}
		else {
//			AddRecievedData("Button as been released");
		}
		//if there is data to display
		
		AddRecievedData(sbuffer);
	}
	
	CDialog::OnTimer(nIDEvent);
}

/*---------------------------------------------------------------------------

FUNCTION: OnFwUpgrade

PURPOSE: Call when Firmware Upgrade Button is pressed.
         This function set the deivce in Firmware upgarde mode using startBootLoader
		 Once bootloader mode as been sent, the device is close properly.
		 You have the to lauch Flip to load a new firmeare

PARMATERS:
  	
RETURN:
   
COMMENTS:
  
---------------------------------------------------------------------------*/
void CUsbHidDemoCodeDlg::OnFwUpgrade() 
{
	if(IsConnected) {//if our device is attached
		UCHAR* outputReport = new UCHAR[DYNCALL(getFeatureReportLength())];
        
		outputReport[0]=0x55;
		outputReport[1]=0xAA;
		outputReport[2]=0x55;
		outputReport[3]=0xAA;

		if(!DYNCALL(setFeature(outputReport))) {
			// Fail to run bootLoader
			AfxMessageBox( "Can not start Device in Boot Loader mode", MB_ICONSTOP,0);
		}		
		DisableLedButton();
		DYNCALL(closeDevice());//close all handles				
	}
	
}

/*---------------------------------------------------------------------------

FUNCTION: OnCancel

PURPOSE: On one cancel if device is connected, this one is disconnected.
		 the application is Unregister from the device notification table using
		  hidUnregisterDeviceNotification(m_hWnd)

PARMATERS:
  	
RETURN:
   
COMMENTS:
  
---------------------------------------------------------------------------*/
void CUsbHidDemoCodeDlg::OnCancel() 
{
	if(IsConnected) //if our device is attached
	{
		DYNCALL(closeDevice());//close all handles
		CDialog::KillTimer(1);//close the timer
	}
	DYNCALL(hidUnregisterDeviceNotification(m_hWnd));
	
	FreeLibrary(hLib);

	CDialog::OnCancel();
}


/*---------------------------------------------------------------------------

FUNCTION: OnButtonVidPid

PURPOSE: When clic on button VId PID
         Vendor ID en Product ID is taken from edit Box and new value 
		  are change.

PARMATERS: 

RETURN:

COMMENTS:
  
---------------------------------------------------------------------------*/
void CUsbHidDemoCodeDlg::OnButtonVidPid() 
{
	
	UpdateData();

	// Try To Convert in Hex
	char VidToConvert[10];
	char PidToConvert[10];
	char * pEnd;
	long newVid;
	long newPid;
	
	// Get New Vid
	strcpy(VidToConvert,"0x");
	strcat(VidToConvert,m_VID.GetBuffer(m_VID.GetLength()));
	newVid = strtol (VidToConvert,&pEnd,0);
	m_VID.Format("%X",newVid);
	
	// Get New Pid
	strcpy(PidToConvert,"0x");
	strcat(PidToConvert,m_PID.GetBuffer(m_PID.GetLength()));
	newPid = strtol (PidToConvert,&pEnd,0);
	m_PID.Format("%X",newPid);
	
	// Upadte 
	SetDlgItemText(IDC_VID , m_VID.GetBuffer(m_VID.GetLength()));
	SetDlgItemText(IDC_PID , m_PID.GetBuffer(m_PID.GetLength()));
	
	if((newVid!=Vid)||(newPid!=Pid)) {
		Vid=newVid;
		Pid=newPid;
		if(IsConnected) {
			// Close Connection only once
			DisableLedButton();
			DYNCALL(closeDevice());						
		}
		ConnectDevice();
	}
}
