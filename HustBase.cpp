// HustBase.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "HustBase.h"

#include "MainFrm.h"
#include "HustBaseDoc.h"
#include "HustBaseView.h"
#include "TreeList.h"

#include "IX_Manager.h"
#include "PF_Manager.h"
#include "RM_Manager.h"
#include "SYS_Manager.h"
#include "str.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHustBaseApp

BEGIN_MESSAGE_MAP(CHustBaseApp, CWinApp)
	//{{AFX_MSG_MAP(CHustBaseApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_CREATEDB, OnCreateDB)
	ON_COMMAND(ID_OPENDB, OnOpenDB)
	ON_COMMAND(ID_DROPDB, OnDropDb)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHustBaseApp construction

CHustBaseApp::CHustBaseApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CHustBaseApp object

CHustBaseApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CHustBaseApp initialization
bool CHustBaseApp::pathvalue=false;

BOOL CHustBaseApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	// Change the registry key under which our settings are stored.
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CHustBaseDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CHustBaseView));
	AddDocTemplate(pDocTemplate);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it.
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	return TRUE;
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
		// No message handlers
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

// App command to run the dialog
void CHustBaseApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CHustBaseApp message handlers

void CHustBaseApp::OnCreateDB()
{
	CFileDialog dialog(TRUE);    //����Ҫ�趨Ĭ��·���Ȳ���
	dialog.m_ofn.lpstrTitle="��ѡ�����ݿ��λ��,���ڶԻ������������ݿ���";
	if(dialog.DoModal()==IDOK)
	{
		CString filePath,fileName,actualFilePath;
		filePath=dialog.GetPathName();
		fileName=dialog.GetFileTitle();
		int actualPathLength=filePath.GetLength()-fileName.GetLength()-1; //��1��Ϊ��ȥ����/��
		actualFilePath=filePath.Left(actualPathLength);  //��ò��������������ı��ļ��е��ļ��е�ַ
		//��CString��������ת��Ϊchar*��������
		//int path=actualFilePath.GetLength()+1;
		//int file=fileName.GetLength();
		//int a;
		//char filePathC[300];
		//strcpy(filePathC,actualFilePath);

		//char fileNameC[300];
		//strcpy(fileNameC,fileName);
		char *filePathC=(char *)malloc(sizeof(actualFilePath)+1);
		strcpy(filePathC,actualFilePath);
		char *fileNameC=(char *)malloc(sizeof(fileName)+1);
		strcpy(fileNameC,fileName);

		CreateDB(filePathC,fileNameC);
		//free(filePathC);
		//free(fileNameC);

	}
	//�����������ݿⰴť���˴�Ӧ��ʾ�û��������ݿ�Ĵ洢·�������ƣ�������CreateDB�����������ݿ⡣
}

void CHustBaseApp::OnOpenDB() 
{

	BROWSEINFO folder;
	folder.lpszTitle="ѡ�����ݿ�Ŀ¼";
	ZeroMemory(&folder,sizeof(BROWSEINFO));
	LPMALLOC pMalloc;
	LPITEMIDLIST getFolder=SHBrowseForFolder(&folder);
	if(getFolder!=NULL)
	{
		char dbPath[MAX_PATH];
		SHGetPathFromIDList(getFolder,dbPath);
		
		if(SUCCEEDED(SHGetMalloc(&pMalloc)))
		{
			pMalloc->Free(getFolder);
			pMalloc->Release();
		}
	    //OpenDB(dbPath);
		/*char *name=strrchr(dbPath,'\\');
		int k=name-dbPath;

		int i=strlen(dbPath);
		char *p=(char*)malloc((k+1)*sizeof(char));
		strncpy(p,dbPath,k);
		p[k]='\0';
		int j=i-k;
		char *dbname=(char *)malloc((j+1)*sizeof(char));
		name++;
		strcpy(dbname,name);

	
		SetCurrentDirectory(p);
		 OpenDB(dbname);*/

		SetCurrentDirectory(dbPath);
		if (access("SYSTABLES",0)!=0||access("SYSCOLUMNS",0)!=0){
			AfxMessageBox("Oops�����˷����ݿ��ļ�");
			return;
		}

		 //SetCurrentDirectory(dbPath);
		CHustBaseDoc *pDoc;
		pDoc=CHustBaseDoc::GetDoc();
		CHustBaseApp::pathvalue=true;
		pDoc->m_pTreeView->PopulateTree();
	}
	//���������ݿⰴť���˴�Ӧ��ʾ�û��������ݿ�����λ�ã�������OpenDB�����ı䵱ǰ���ݿ�·�������ڽ������Ŀؼ�����ʾ���ݿ��еı�������Ϣ��
}

void CHustBaseApp::OnDropDb() 
{

	BROWSEINFO folder;
	folder.lpszTitle="��ѡ����Ҫɾ�������ݿ�Ŀ¼";
	ZeroMemory(&folder,sizeof(BROWSEINFO));
	LPMALLOC pMalloc;
	LPITEMIDLIST getFolder=SHBrowseForFolder(&folder);
	if(getFolder!=NULL)
	{
		TCHAR *dbPath=new TCHAR[MAX_PATH];
		SHGetPathFromIDList(getFolder,dbPath);
		if(SUCCEEDED(SHGetMalloc(&pMalloc)))
		{
			pMalloc->Free(getFolder);
			pMalloc->Release();
		}
		DropDB(dbPath);
		free(dbPath);
	}
	//����ɾ�����ݿⰴť���˴�Ӧ��ʾ�û��������ݿ�����λ�ã�������DropDB����ɾ�����ݿ�����ݡ�
}