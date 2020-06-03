// tscvc6Dlg.cpp : implementation file
//

#include "stdafx.h"
#include "tscvc6.h"
#include "tscvc6Dlg.h"
#include "uart.h"
#include "crc32.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define UPDATE_START 1
#define UPDATE_SEND  2
#define UPDATE_DONE  3
#define UPDATE_ERR   4
#define UPDATE_OK    5

CWinThread*    Update_Thread=0;//声明线程
UINT StartUpdateThread(void *param);//声明线程函数

char UpdateFileName[256]={0};
UINT UpdateFileMode=0;
UINT UpdateFileIndex=0;
UINT UpdateFileAddr=0;
UINT UpdateFileBlockSize=0;

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
// CTscvc6Dlg dialog

CTscvc6Dlg::CTscvc6Dlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTscvc6Dlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTscvc6Dlg)
	m_UpdateFile = _T("");
	m_ScriptFile = _T("");
	m_UpdateFileIndex = 0;
	m_UpdateFileAddr = _T("");
	m_UpdateFileBlockSize = 0;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTscvc6Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTscvc6Dlg)
	DDX_Control(pDX, IDC_COMBO5, m_ComBoParity);
	DDX_Control(pDX, IDC_COMBO4, m_ComBoStopBit);
	DDX_Control(pDX, IDC_COMBO2, m_ComBoBaud);
	DDX_Control(pDX, IDC_COMBO3, m_ComBoData);
	DDX_Control(pDX, IDC_COMBO1, m_ComBoCom);
	DDX_Text(pDX, IDC_EDIT3, m_UpdateFile);
	DDX_Text(pDX, IDC_EDIT7, m_ScriptFile);
	DDX_Text(pDX, IDC_EDIT2, m_UpdateFileIndex);
	DDX_Text(pDX, IDC_EDIT8, m_UpdateFileAddr);
	DDX_Text(pDX, IDC_EDIT5, m_UpdateFileBlockSize);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CTscvc6Dlg, CDialog)
	//{{AFX_MSG_MAP(CTscvc6Dlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	ON_BN_CLICKED(IDC_BUTTON2, OnButton2)
	ON_BN_CLICKED(IDC_BUTTON3, OnButton3)
	ON_BN_CLICKED(IDC_BUTTON5, OnButton5)
	ON_BN_CLICKED(IDC_BUTTON6, OnButton6)
	ON_BN_CLICKED(IDC_BUTTON7, OnButton7)
	ON_BN_CLICKED(IDC_BUTTON8, OnButton8)
	ON_BN_CLICKED(IDC_BUTTON10, OnButton10)
	ON_BN_CLICKED(IDC_BUTTON4, OnButton4)
	ON_BN_CLICKED(IDC_BUTTON11, OnButton11)
	ON_BN_CLICKED(IDC_BUTTON9, OnButton9)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WN_SHOW_UPDATE,ShowUpdate)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTscvc6Dlg message handlers

BOOL CTscvc6Dlg::OnInitDialog()
{
		char strtmp[64];
		int i;
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
	
	// TODO: Add extra initialization here
	for(i=0;i<64;i++)
	{
	    _snprintf(strtmp,10,"COM%d",i);
		m_ComBoCom.AddString(strtmp);
	}
    m_ComBoCom.SetCurSel(0);

	m_ComBoBaud.AddString("115200");
	m_ComBoBaud.AddString("57600");
    m_ComBoBaud.AddString("38400");
	m_ComBoBaud.AddString("19200");
	m_ComBoBaud.AddString("9600");
    m_ComBoBaud.AddString("4800");
    m_ComBoBaud.SetCurSel(0);

	m_ComBoData.AddString("8");
    m_ComBoData.AddString("7");
    m_ComBoData.AddString("6");
    m_ComBoData.SetCurSel(0);

    m_ComBoStopBit.AddString("1");
    m_ComBoStopBit.AddString("1.5");
    m_ComBoStopBit.AddString("2");
    m_ComBoStopBit.SetCurSel(0);

    m_ComBoParity.AddString("无");
    m_ComBoParity.AddString("奇校验");
    m_ComBoParity.AddString("偶校验");
    m_ComBoParity.SetCurSel(0);

    m_UpdateFileIndex=0;
    m_UpdateFileAddr="0x00000000";
    m_UpdateFileBlockSize=1024;
	((CButton *)GetDlgItem(IDC_CHECK3))->SetCheck(FALSE);

	UpdateData(FALSE);  // 变量->控件
 
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CTscvc6Dlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CTscvc6Dlg::OnPaint() 
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
HCURSOR CTscvc6Dlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

static int ComOpenState = 0;
static char ComStr[64]={0};
static int ComBaud=0;
static int ComData=0;
static int ComStop=0;
static int ComParity=0;
static CString EditStr = _T("");
static BYTE gReadBuff[64]={0};
static char gReadStr[64]={0};
static char gWriteBuff[32]={(char)0xFF,(char)0x3C,(char)0x01,(char)0x00,(char)0x3D,(char)0x0D};
static char xPointStr[64];
static char yPointStr[64];
static DWORD gReadLen;
static char gPosition = 0;
static unsigned int gPrintNum =0;
static gRaiodSel = 1;
static int xOffset;
char DigToChar(unsigned char val)
{
	if((val>=0) && (val<=9))
	{
	    return (val+'0');
	}
	else if((val>9) && (val<=15))
	{
	    return (val-10 +'A');
	}
	else
	{
	    return ' ';
	}
}


void CTscvc6Dlg::OnButton1() 
{

}




void CTscvc6Dlg::OnButton2() 
{
	// TODO: Add your control notification handler code here
	int nIndex;
	if(0 == ComOpenState)
	{
    	ComOpenState = 1;
		nIndex = m_ComBoCom.GetCurSel();
		if(nIndex>=10)
		{
			_snprintf(ComStr,20,"\\\\.\\COM%d",nIndex);
		}
		else
		{
			_snprintf(ComStr,20,"COM%d",nIndex);
		}
		if(uart_open(ComStr) != 0)
		{
			MessageBox("打开失败");
		}
		else
		{
            nIndex = m_ComBoBaud.GetCurSel();
            switch(nIndex)
			{
			case 0:
				ComBaud = 115200;
				break;
			case 1:
				ComBaud = 57600;
				break;
			case 2:
				ComBaud = 38400;
				break;
			case 3:
				ComBaud = 19200;
				break;
			case 4:
				ComBaud = 9600;
				break;
			case 5:
				ComBaud = 4800;
				break;
			default:
				ComBaud = 9600;
				break;
			}
            nIndex = m_ComBoData.GetCurSel();
            switch(nIndex)
			{
			case 0:
				ComData = 8;
				break;
			case 1:
				ComData = 7;
				break;
			case 2:
				ComData = 6;
				break;
			default:
				ComData = 8;
				break;
			}
            nIndex = m_ComBoParity.GetCurSel();
            switch(nIndex)
			{
			case 0:
				ComParity = NOPARITY;
				break;
			case 1:
				ComParity = ODDPARITY;
				break;
			case 2:
				ComParity = EVENPARITY;
				break;
			default:
				ComParity = NOPARITY;
				break;
			}
            nIndex = m_ComBoStopBit.GetCurSel();
            switch(nIndex)
			{
			case 0:
				ComStop = ONESTOPBIT;
				break;
			case 1:
				ComStop = ONE5STOPBITS;
				break;
			case 2:
				ComStop = TWOSTOPBITS;
				break;
			default:
				ComStop = ONESTOPBIT;
				break;
			}
			if(uart_config(ComBaud,ComData,ComParity,ComStop,10000) != 0)
			{
				MessageBox("配置失败");
			}
			else
			{
				SetDlgItemText(IDC_BUTTON2,"关闭");
				((CButton *)GetDlgItem(IDC_RADIO1))->SetCheck(TRUE);
			}
		}
	}
	else
	{
    	ComOpenState = 0;
		if(uart_close(ComStr) != 0)
		{
			MessageBox("关闭失败");
		}
		else
		{
			SetDlgItemText(IDC_BUTTON2,"打开");
			((CButton *)GetDlgItem(IDC_RADIO1))->SetCheck(FALSE);
		}
	}
}

void CTscvc6Dlg::OnButton3() 
{
	// TODO: Add your control notification handler code here
	CString str = _T("");
   ((CEdit *)GetDlgItem(IDC_EDIT1))->SetWindowText(str);
}

BOOL CTscvc6Dlg::DestroyWindow() 
{
	// TODO: Add your specialized code here and/or call the base class
	return CDialog::DestroyWindow();
}

void CTscvc6Dlg::OnButton5() 
{
	// TODO: Add your control notification handler code here
	CString str;
	int len=0;
	char buff[64]={0};
	UpdateData(TRUE); //控件 -> 变量
	//len = m_FeedLen;
	str.Format(_T("FEED %d"),len);
	strcpy( buff, str.GetBuffer(str.GetLength()));
}

void CTscvc6Dlg::OnButton6() 
{
	// TODO: Add your control notification handler code here
	CString str;
	int len=0;
	char buff[64]={0};
	UpdateData(TRUE); //控件 -> 变量
	//len = m_UnFeedLen;
	str.Format(_T("BACKFEED %d"),len);
	strcpy( buff, str.GetBuffer(str.GetLength()));
}

void CTscvc6Dlg::OnButton7() 
{
	// TODO: Add your control notification handler code here
}

void CTscvc6Dlg::OnButton8() 
{
	// TODO: Add your control notification handler code here
    ShellExecute(NULL,"open","help.docx",NULL,NULL, SW_SHOWNORMAL); 
}

void CTscvc6Dlg::OnButton10() 
{
	// TODO: Add your control notification handler code here
	m_UpdateFile = this->SelectFile();
    UpdateData(FALSE);   //变量 -> 控件
}

CString CTscvc6Dlg::SelectFile()
{
    CString strFile = _T("");

    CFileDialog    dlgFile(TRUE, NULL, NULL, OFN_HIDEREADONLY, _T("Describe Files (*.bin)|*.bin|All Files (*.*)|*.*||"), NULL);

    if (dlgFile.DoModal())
    {
        strFile = dlgFile.GetPathName();
    }

    return strFile;
}

void CTscvc6Dlg::OnButton4() 
{
	// TODO: Add your control notification handler code here
	CString str = _T("");
   ((CEdit *)GetDlgItem(IDC_EDIT4))->SetWindowText(str);
}

void CTscvc6Dlg::OnButton11() 
{
	// TODO: Add your control notification handler code here
	m_ScriptFile = this->SelectFile();
    UpdateData(TRUE);   //  控件->变量
}

void CTscvc6Dlg::OnButton9() 
{
	// TODO: Add your control notification handler code here
	/*创建线程 传入文件名为参数*/
	CString filename;
    filename = m_UpdateFile;
    UpdateData(TRUE);   //控件->变量

   //创建线成,并传入主线成的事例
	if(0 == Update_Thread)
    {
		if(((CButton *)GetDlgItem(IDC_CHECK3))->GetCheck())
		{
			UpdateFileMode = 1;
		}
		else
		{
			UpdateFileMode = 0;
		}
		UpdateFileIndex = m_UpdateFileIndex;
		UpdateFileAddr = strtol(m_UpdateFileAddr,NULL,16);
		UpdateFileBlockSize = m_UpdateFileBlockSize;
		strcpy(UpdateFileName,m_UpdateFile.GetBuffer(m_UpdateFile.GetLength()));
		Update_Thread =  AfxBeginThread(StartUpdateThread, 
                                      (LPVOID)this,
                                      THREAD_PRIORITY_BELOW_NORMAL,
                                      5*1024*1024,
                                      0,
                                      NULL);    
		 ((CEdit *)GetDlgItem(IDC_BUTTON9))->SetWindowText("结束升级");
	}
	else
	{
		Update_Thread->PostThreadMessage(WN_EXIT_UPDATE,(WPARAM)0,0);
		//CloseHandle(Update_Thread);
		Update_Thread = 0;
		((CEdit *)GetDlgItem(IDC_BUTTON9))->SetWindowText("开始升级");
		((CEdit *)GetDlgItem(IDC_STATIC1))->SetWindowText("0/0");
	}
}

LRESULT CTscvc6Dlg::ShowUpdate(WPARAM wParam, LPARAM lParam)
{
	CString str("");
	CProgressCtrl* pProg = (CProgressCtrl*)GetDlgItem(IDC_PROGRESS1);
    pProg->SetRange(0, 100);
	switch(wParam)
	{
	case UPDATE_SEND:
		pProg->SetPos(lParam>100?100:lParam);
		str.Format(_T("%d%%"),lParam>100?100:lParam);
		((CEdit *)GetDlgItem(IDC_STATIC1))->SetWindowText(str);
		break;
	case UPDATE_ERR:
		pProg->SetPos(100);
		((CEdit *)GetDlgItem(IDC_STATIC1))->SetWindowText("100%");
		((CEdit *)GetDlgItem(IDC_BUTTON9))->SetWindowText("开始升级");
		Update_Thread=0;
		break;
	case UPDATE_OK:
		pProg->SetPos(100);
		((CEdit *)GetDlgItem(IDC_STATIC1))->SetWindowText("100%");
		((CEdit *)GetDlgItem(IDC_BUTTON9))->SetWindowText("开始升级");
		Update_Thread=0;
		break;
	}
	return 0;
}


UINT UpdateSendStart(UINT frames)
{
	int res=0;
	int retry;
	BYTE sendbuff[8] = {0xC6,0x50,0x02,0x00,0x00,0x00,0x00,0x00};
	BYTE readbuff[8] = {0};
	if(UpdateFileMode)
	{
	    sendbuff[1] = 0x51;
	}
	else
	{
	    sendbuff[1] = 0x50;
	}
	sendbuff[3] = UpdateFileIndex;
	sendbuff[4] = (frames>>16)&0xFF;
	sendbuff[5] = (frames>>8)&0xFF;
	sendbuff[6] = (frames>>0)&0xFF;
	sendbuff[7] = checksum8(sendbuff,7);
	retry = 3;
	do
	{
		res = uart_trans(sendbuff,8,readbuff,8);
		if(res)
		{
			if((readbuff[0]==0x99) && (readbuff[1]==0xFF))
			//if((readbuff[0]==0xC6) && (readbuff[1]==0x50))
			{
				res = 1;
			}
			else
			{
				res = 0;
			}
		}else{}
	}while((res==0) && ((retry--)>0));
    if(res)
    {
		return 0;
	}
	else
	{
		return 1;
	}
}

UINT UpdateSendData(BYTE* buff,UINT len,UINT frameindex,UINT startaddr,UINT endaddr)
{
	int res=0;
	int retry;
	UINT crc=0;
	UINT start;
	UINT end;
	BYTE sendbuff[4096+21] = {0};
	BYTE readbuff[8] = {0};
	if(len>4096)
	{
		return 1;
	}
	start = (frameindex-1)*UpdateFileBlockSize + startaddr;
    end = start + len;
	sendbuff[0]=((len+18)>>8) & 0xFF;
	sendbuff[1]=(len+18) & 0xFF;
	sendbuff[2]=0xC7;
	sendbuff[3]=0x02;
	sendbuff[4]=UpdateFileIndex;
	sendbuff[5] = (frameindex>>16)&0xFF;
	sendbuff[6] = (frameindex>>8)&0xFF;
	sendbuff[7] = (frameindex>>0)&0xFF;
	sendbuff[8] = (start>>24)&0xFF;
	sendbuff[9] = (start>>16)&0xFF;
	sendbuff[10] = (start>>8)&0xFF;
	sendbuff[11] = (start>>0)&0xFF;
	sendbuff[12] = (end>>24)&0xFF;
	sendbuff[13] = (end>>16)&0xFF;
	sendbuff[14] = (end>>8)&0xFF;
	sendbuff[15] = (end>>0)&0xFF;
	memcpy(&sendbuff[16],buff,len);
	crc = crc32(&sendbuff[2], len+14);
	sendbuff[len+16] = (crc>>24)&0xFF;
	sendbuff[len+17] = (crc>>16)&0xFF;
	sendbuff[len+18] = (crc>>8)&0xFF;
	sendbuff[len+19] = (crc>>0)&0xFF;
	sendbuff[len+20] = checksum8(sendbuff, len+20);

	retry = 3;
	do
	{
		res = uart_trans(sendbuff,len+21,readbuff,8);
		if(res)
		{
			if((readbuff[0]==0x98) && (readbuff[1]==0xFF))
			//if((readbuff[2]==0xC7) && (readbuff[3]==0x02))
			{
				res = 1;
			}
			else
			{
				res = 0;
			}
		}else{}
	}while((res==0) && ((retry--)>0));
    if(res)
    {
		return 0;
	}
	else
	{
		return 1;
	}
}

UINT UpdateSendDone(void)
{
	int res=0;
	int retry;
	BYTE sendbuff[8] = {0xC8,0x50,0x02,0x00,0xAA,0xAA,0xAA,0xAA};
	BYTE readbuff[8] = {0};
	if(UpdateFileMode)
	{
	    sendbuff[1] = 0x51;
	}
	else
	{
	    sendbuff[1] = 0x50;
	}
	sendbuff[3] = UpdateFileIndex;
	retry = 3;
	do
	{
		res = uart_trans(sendbuff,8,readbuff,8);
		if(res)
		{
			if((readbuff[0]==0x97) && (readbuff[1]==0xFF))
			//if((readbuff[0]==0xC8) && (readbuff[1]==0x50))
			{
				res = 1;
			}
			else
			{
				res = 0;
			}
		}else{}
	}while((res==0) && ((retry--)>0));
    if(res)
    {
		return 0;
	}
	else
	{
		return 1;
	}
}

UINT StartUpdateThread(void *param)
{
    CTscvc6Dlg* dlg;
	MSG msg;
	UINT filelen;
	dlg = (CTscvc6Dlg*)param;
	WPARAM frameindex = 0;
	LPARAM framenum = 100;
	CFile file;      // CFile对象 
    BYTE FileBuffer[1024*1024];      // 文件内容
	UINT udatestate=UPDATE_START;
	UINT sendfrmes = 0;
	UINT lastfrmenum = 0;
	/*文件处理*/
	if(file.Open(UpdateFileName, CFile::modeRead))
	{
		filelen=file.GetLength();
		if(filelen>1024*1024)
		{
			AfxMessageBox("文件不能超过1M");
			file.Close();
			dlg->PostMessage(WN_SHOW_UPDATE,UPDATE_ERR,0);
			Sleep(10);
			return 1;
		}
		file.SeekToBegin(); 
		UINT nRet = file.Read(FileBuffer, file.GetLength()); 
		if(nRet < filelen)
		{
			AfxMessageBox("read err",0);
			file.Close();
			dlg->PostMessage(WN_SHOW_UPDATE,UPDATE_ERR,0);
			Sleep(10);
			return 1;
		}
		else
		{
			sendfrmes = filelen/UpdateFileBlockSize;
			lastfrmenum = filelen%UpdateFileBlockSize;
			file.Close();
			while(1)
			{
				Sleep(10);
				/*结束处理*/
				if(PeekMessage(&msg,NULL, 0, 0, PM_REMOVE))
				{
					switch (msg.message)
					{
					case WN_EXIT_UPDATE:
						return 0;
						break;
					}
				}
		    	/*升级处理*/
				switch(udatestate)
				{
				case UPDATE_START:
					if(UpdateSendStart(sendfrmes + (lastfrmenum>0?1:0))==0)
					{
					    udatestate = UPDATE_SEND;
					}
					else
					{
                        AfxMessageBox("启动更新失败");
						dlg->PostMessage(WN_SHOW_UPDATE,UPDATE_ERR,0);
						Sleep(10);
						return 1;
					}
				break;
				case UPDATE_SEND:
					if(frameindex < sendfrmes)
					{

						if(UpdateSendData(&FileBuffer[UpdateFileBlockSize*frameindex],UpdateFileBlockSize,frameindex+1,UpdateFileAddr,UpdateFileAddr+filelen)==0)
						{
							frameindex++;
							dlg->PostMessage(WN_SHOW_UPDATE,UPDATE_SEND,(frameindex*100)/sendfrmes);
						}
						else
						{
							AfxMessageBox("发送失败");
							dlg->PostMessage(WN_SHOW_UPDATE,UPDATE_ERR,0);
							Sleep(10);
							return 1;
						}

					}
					else
					{
						if(lastfrmenum)
						{
							if(UpdateSendData(&FileBuffer[UpdateFileBlockSize*frameindex],lastfrmenum,frameindex+1,UpdateFileAddr,UpdateFileAddr+filelen)==0)
							{
								frameindex++;
								dlg->PostMessage(WN_SHOW_UPDATE,UPDATE_SEND,(frameindex*100)/sendfrmes);
							}
							else
							{
								AfxMessageBox("发送失败");
								dlg->PostMessage(WN_SHOW_UPDATE,UPDATE_ERR,0);
								Sleep(10);
								return 1;
							}
						}
						udatestate = UPDATE_DONE;
					}
				break;
				case UPDATE_DONE:
					if(UpdateSendDone() == 0)
					{
                        AfxMessageBox("启动成功");
						dlg->PostMessage(WN_SHOW_UPDATE,UPDATE_OK,0);
						Sleep(10);
						return 0;
					}
					else
					{
                        AfxMessageBox("启动失败");
						dlg->PostMessage(WN_SHOW_UPDATE,UPDATE_ERR,0);
						Sleep(10);
						return 1;
					}
				break;
				}
			}
		}
	}
	else
	{
		AfxMessageBox("打开文件失败",0);
		dlg->PostMessage(WN_SHOW_UPDATE,UPDATE_ERR,0);
		Sleep(10);
		AfxEndThread(0,1);
		return 1;
	}
	return 0;
}
