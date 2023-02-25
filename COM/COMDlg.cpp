
// COMDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "COM.h"
#include "COMDlg.h"
#include "afxdialogex.h"
#include "SerialWithCrc.h"	//使用串口类
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CSerialWithCrc comCrc;
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CCOMDlg 对话框



CCOMDlg::CCOMDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_COM_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCOMDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CCOMDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_RECV, &CCOMDlg::OnBnClickedBtnRecv)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON1, &CCOMDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BTN_OPEN, &CCOMDlg::OnBnClickedBtnOpen)
	ON_BN_CLICKED(IDC_BTN_CLOSE, &CCOMDlg::OnBnClickedBtnClose)
END_MESSAGE_MAP()


// CCOMDlg 消息处理程序

BOOL CCOMDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CCOMDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CCOMDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CCOMDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CCOMDlg::OnBnClickedBtnRecv()
{
	// TODO: 在此添加控件通知处理程序代码
	vector<char> recv;
	comCrc.ReadData(recv);
	CString strText,str;
	for (int i = 0; i < recv.size(); ++i)
	{
		str.Format(_T("%02x"), recv[i]&0xff);
		strText += str;
	}
	//输出接收结果
	CEdit* pEdt = (CEdit*)GetDlgItem(IDC_EDT_OUTPUT);
	pEdt->SetWindowText(strText);

	DWORD res = 0;
	if (recv.size() > 4)
	{
		int datasize = (recv[2]);
		char buf[4];
		datasize = min(sizeof(buf), datasize);
		for (int i = 0; i < datasize; ++i)
		{		
			res <<= 8;
			res |= recv[3 + i];

		}
	}

	strText.Format(_T("%u"), res);
	/// <summary>
	//输出编码数
	/// </summary>
	pEdt = (CEdit*)GetDlgItem(IDC_EDT_RESULT);
	pEdt->SetWindowText(strText);
	
}


void CCOMDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	comCrc.Close();
	CDialog::OnCancel();
	CDialogEx::OnClose();
}


void CCOMDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	CEdit* pEdt = (CEdit*)GetDlgItem(IDC_EDT_INPUT);
	CString str;
	pEdt->GetWindowText(str);

	char arr[] = { 0x06,0x00,0x08,0x00,0x01 };
	vector<char> sendArray= StringToHexChar(str);	//测试的发送，填的是字符串，所以需要转成16进制
	//sendArray.push_back(地址);
	//for (int i = 0; i < sizeof(arr); ++i)
	//{
	//	sendArray.push_back(arr[i]);
	//}
	//sendArray.push_back(0x53);	//举例
	int addr = GetDlgItemInt(IDC_EDT_ADDR);
	int funcIndex = GetDlgItemInt(IDC_EDT_FUNCINDEX);
	comCrc.SendData(addr, funcIndex, sendArray);
}


vector<char> CCOMDlg::StringToHexChar(CString str)
{
	vector<char> result;
	int len = str.GetLength();
	for (int i = 0; i < len;)
	{
		char hex = 0;
		for (int j = 0; j < 2; ++j)
		{
			hex <<= 8;
			char val=str.GetAt(i);
			if (val >= '0' && val < '9')
			{
				val -= '0';
			}
			else if (val >= 'a' && val < 'f')
			{
				val -= 87;
			}
			else if (val >= 'A' && val < 'F')
			{
				val -= 55;
			}
			hex |= val;
			++i;
		}
		
		result.push_back(hex);
	}



	return result;
}

void CCOMDlg::OnBnClickedBtnOpen()
{
	// TODO: 在此添加控件通知处理程序代码
	int comPort = GetDlgItemInt(IDC_EDT_COM_PORT);
	if (!comCrc.Open(comPort, 9600))
	{
		MessageBox(_T("打开串口失败!"));
	}
}


void CCOMDlg::OnBnClickedBtnClose()
{
	// TODO: 在此添加控件通知处理程序代码
	comCrc.Close();
}
