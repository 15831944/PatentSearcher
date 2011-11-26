// MainFrm.cpp : implmentation of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "aboutdlg.h"
#include "netsearchdlg.h"
#include "localsearchdlg.h"
#include "MainFrm.h"
#include "tiff2pdf.h"
#include "md5.h"

#include <string>
#include <fstream>
#include <list>
using namespace std;

#include <stdio.h>

#include <wininet.h>
#define MAXBLOCKSIZE 1024*16
#pragma comment( lib, "wininet.lib" )

CString global_Search_str;
CString global_CheckBox_str;

typedef std::pair<CString, TIFF*> tiffFile;

bool alphabetOrder(const CString left, const CString right)
{
	if (left.CompareNoCase(right) <= 0)
		return true;
	return false;
}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	return CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg);
}

BOOL CMainFrame::OnIdle()
{
	UIUpdateToolBar();

	UIUpdateStatusBar();
	return FALSE;
}

LRESULT CMainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CreateSimpleToolBar();

//	CreateSimpleStatusBar();
	// Create the multipane status bar and pass its handle to CUpdateUI.
	m_hWndStatusBar = m_wndStatusBar.Create ( *this );
	UIAddStatusBar ( m_hWndStatusBar );
	// Create the status bar panes.
	int anPanes[] = { ID_DEFAULT_PANE, IDPANE_SOURCE, IDPANE_SEARCH_TIME, IDPANE_DOWNLOADLIST, IDPANE_COMPANY };

	m_wndStatusBar.SetPanes ( anPanes, 5, false );
	// Set the initial text for the status pane.
	UISetText( 1, _T("����Դ��SIPO��������") );
	UISetText( 2, _T("����ʱ�䣺--") );
	//���Ҫ�����ݿ��ж�ȡ
	UISetText( 3, _T("���ض��У�--��") );
	UISetText( 4, _T(" �����Ƽ�") );


	UIAddToolBar(m_hWndToolBar);

	UISetCheck(ID_VIEW_TOOLBAR, 1);
	UISetCheck(ID_VIEW_STATUS_BAR, 1);

	// Add your code
	if(LRunSql::InitConnectPtr()==false)		//��ʼ��COM�������������ݿ�����
		return false;

	m_localSearchStr = "";
	const DWORD dwSplitStyle = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		dwSplitExStyle = WS_EX_CLIENTEDGE;

	GetClientRect( &rcDefault );
	m_hWndClient = m_wndVertSplitter.Create( m_hWnd, rcDefault, NULL, dwSplitStyle, dwSplitExStyle);
	m_wndVertSplitter.m_bFullDrag = true;

	/************************************************************************/
	/* ���νṹ                                                             */
	/************************************************************************/
	m_wndVertSplitter.GetClientRect(&rcDefault);
	m_MenuTree.Create( m_wndVertSplitter, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | TVS_HASLINES | TVS_LINESATROOT | TVS_SHOWSELALWAYS | TVS_HASBUTTONS | WS_EX_CLIENTEDGE,0,(HMENU)IDC_LEFT_TREEVIEW);
	InitLeftTreeView(&m_MenuTree);


	m_wndHorzSplitter.Create( m_wndVertSplitter, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN );
	m_wndHorzSplitter.m_bFullDrag = true;
	m_wndVertSplitter.SetSplitterPanes( m_MenuTree, m_wndHorzSplitter );

	m_TopVertSplitter.Create( m_wndHorzSplitter, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN );
	m_wndHorzSplitter.m_bFullDrag = true;
	m_BottomVertSplitter.Create( m_wndHorzSplitter, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN );
	m_wndHorzSplitter.SetSplitterPanes( m_TopVertSplitter, m_BottomVertSplitter );
	m_wndHorzSplitter.SetSinglePaneMode( SPLIT_PANE_BOTTOM );

	/************************************************************************/
	/* ���������                                                           */
	/************************************************************************/
	//Create the bottom pane(browser)
	const DWORD dwIEStyle = WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_HSCROLL|WS_HSCROLL,
		dwIEExStyle = WS_EX_CLIENTEDGE;

	// ��õ�ǰ·��	
	TCHAR szFilePath[MAX_PATH + 1]; 
	GetModuleFileName(NULL, szFilePath, MAX_PATH);       
	(_tcsrchr(szFilePath, _T('\\')))[1] = 0; 

	currentPath = szFilePath;
	CString mainWebPath = currentPath + "home.htm";

	m_MainWebBrowser.Create( m_BottomVertSplitter, rcDefault, mainWebPath, dwIEStyle, dwIEExStyle );

	//	m_dlg->SetParent(m_BottomVertSplitter);
	//	m_dlg->m_hWnd = NULL;
	//	m_dlg->Create(m_BottomVertSplitter, rcDefault);
	//	m_dlg->ModifyStyle(0, WS_SIZEBOX|WS_CHILD);
	//	m_dlg->Set
	//	m_BottomVertSplitter.SetSplitterPanes( m_MainWebBrowser, m_dlg->m_hWnd);


	m_InfoWebBrowser = GetDlgItem( IDC_IE );
	m_InfoWebBrowser.Create( m_BottomVertSplitter, rcDefault, L"http://www.baidu.com", dwIEStyle, dwIEExStyle );
	m_BottomVertSplitter.SetSplitterPanes( m_MainWebBrowser, m_InfoWebBrowser );
	m_BottomVertSplitter.SetSinglePaneMode( SPLIT_PANE_LEFT );

	m_InfoListView.Create( m_TopVertSplitter, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | LVS_REPORT, LVS_EX_FULLROWSELECT,(HMENU)IDC_INFO_LISTVIEW);
	InitListView( &m_InfoListView , INFOLISTVIEW);
	m_TopHorzSplitter.Create( m_TopVertSplitter, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN );
	m_TopVertSplitter.SetSplitterPanes( m_InfoListView, m_TopHorzSplitter );

	m_DownloadListView.Create( m_TopHorzSplitter, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | LVS_REPORT, LVS_EX_FULLROWSELECT,(HMENU)IDC_DOWNLOAD_LISTVIEW);
	InitListView( &m_DownloadListView, DOWNLOADLISTVIEW );
	m_SearchListView.Create( m_TopHorzSplitter, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | LVS_REPORT, LVS_EX_FULLROWSELECT,(HMENU)IDC_SEARCH_LISTVIEW);
	InitListView( &m_SearchListView, SEARCHLISTVIEW );
	m_TopHorzSplitter.SetSplitterPanes( m_DownloadListView, m_SearchListView );


	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	UpdateLayout();

	m_wndVertSplitter.m_cxyMin = 150;
	m_wndVertSplitter.SetSplitterPos(200);
	m_wndHorzSplitter.SetSplitterPos(200);
	m_TopVertSplitter.SetSplitterPos(200);
	m_TopHorzSplitter.SetSplitterPos(100);

	current_display_groupid = 0;
	current_search_page =0;
	search_total_page = 0;
	last_dispay_page = 0;
	stop_download_flag = 0; // ��ʼ��

	sql = L"select * from sipo_data where downpdf=1 and finishdown=0";
	if (m_runsql.CheckSQLResult(sql))		//��������
	{
		//m_DownloadListView.DeleteAllItems();
		while (!m_runsql.m_recordset->adoEOF)
		{
			//* */int download_list_count = m_DownloadListView.GetItemCount();
			m_DownloadListView.InsertItem(0,(LPCTSTR)"");
			m_DownloadListView.SetItemText(0, 0, L"��ֹͣ");
			//CString ssss = (LPCTSTR)(_bstr_t)m_runsql.m_recordset->GetCollect("publication_number");
			m_DownloadListView.SetItemText(0, 1, (LPCTSTR)(_bstr_t)m_runsql.m_recordset->GetCollect("application_number"));
			m_DownloadListView.SetItemText(0, 2, (LPCTSTR)(_bstr_t)m_runsql.m_recordset->GetCollect("title"));
			m_DownloadListView.SetItemText(0, 3, L"0" /*(LPCTSTR)(_bstr_t)m_runsql.m_recordset->GetCollect("pagemax")*/);
			m_DownloadListView.SetItemText(0, 4, (LPCTSTR)(_bstr_t)m_runsql.m_recordset->GetCollect("web_url"));
			m_DownloadListView.SetItemText(0, 5, (LPCTSTR)(_bstr_t)m_runsql.m_recordset->GetCollect("pagemax"));
			m_DownloadListView.SetItemText(0, 6, (LPCTSTR)(_bstr_t)m_runsql.m_recordset->GetCollect("application_date"));
			m_DownloadListView.SetItemText(0, 7, (LPCTSTR)(_bstr_t)m_runsql.m_recordset->GetCollect("inventor"));
			m_DownloadListView.SetItemText(0, 8, (LPCTSTR)(_bstr_t)m_runsql.m_recordset->GetCollect("applicant"));

			m_runsql.m_recordset->MoveNext();
		}
	}
	//ת��pdf
	// 	CString tiffPath = "d:\\My\ Documents\\Visual\ Studio\ 2008\\ͼ���\\PatentSearcher\\PatentSearcher\\TempFolder\\CN01124194.2";
	// 	//CString tiffPath(filedir.c_str());
	// 	CString pdfPath = "d:\\My\ Documents\\Visual\ Studio\ 2008\\ͼ���\\PatentSearcher\\PatentSearcher\\TempFolder\\CN01124194.2\\hello.pdf";
	// 
	// 	DoMerge(tiffPath, pdfPath);

	return 0;
}

LRESULT CMainFrame::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	LRunSql::Close();		//�Ͽ����ݿ�����

	// unregister message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);

	bHandled = FALSE;
	return 1;
}

LRESULT CMainFrame::OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	PostMessage(WM_CLOSE);
	return 0;
}

LRESULT CMainFrame::OnFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: add code to initialize document

	return 0;
}

LRESULT CMainFrame::OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	BOOL bVisible = !::IsWindowVisible(m_hWndToolBar);
	::ShowWindow(m_hWndToolBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
	UISetCheck(ID_VIEW_TOOLBAR, bVisible);
	UpdateLayout();
	return 0;
}

LRESULT CMainFrame::OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	BOOL bVisible = !::IsWindowVisible(m_hWndStatusBar);
	::ShowWindow(m_hWndStatusBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
	UISetCheck(ID_VIEW_STATUS_BAR, bVisible);
	UpdateLayout();
	return 0;
}

LRESULT CMainFrame::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CAboutDlg dlg;
	dlg.DoModal();
	return 0;
}

LRESULT CMainFrame::OnNetSearch(WORD , WORD , HWND , BOOL& )
{
	CNetSearchDlg dlg;
	dlg.parent_hwnd = GetActiveWindow();
	if (dlg.DoModal() == IDOK)
	{
		CString search_str = global_Search_str;
		//���dlg���ַ�����ʹ���߳̽������أ� ���浽���ݿ��У��ٽ���Ϣ���뵽�༭����


		//�����㶨λ�����ڵ� ����������� 
		HTREEITEM	treeItem;
		treeItem = m_MenuTree.GetRootItem();
		HTREEITEM hChildItem = m_MenuTree.GetChildItem(treeItem);
		HTREEITEM hNextItem = m_MenuTree.GetNextSiblingItem(hChildItem);
		m_MenuTree.Select(hNextItem, TVGN_CARET);

		HANDLE  hThread;  
		hThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)StartNetSearchThread, this, 0, NULL);
		if (hThread == NULL)
		{
			throw runtime_error("CreateThread error!");
		}


	}

	return 0;
}

LRESULT CMainFrame::OnLocalSearch(WORD , WORD , HWND , BOOL& )
{
	CLocalSearchDlg dlg;
	if (dlg.DoModal() == IDOK)
	{
		m_localSearchStr = global_Search_str ;
		//�����㶨λ�����ڵ� �����ݲ�ѯ����� 
		HTREEITEM	treeItem;
		treeItem = m_MenuTree.GetRootItem();
		m_MenuTree.Select(treeItem, TVGN_CARET);
		HTREEITEM hChildItem = m_MenuTree.GetChildItem(treeItem);
		HTREEITEM hNextItem = m_MenuTree.GetNextSiblingItem(hChildItem);
		hNextItem = m_MenuTree.GetNextSiblingItem(hNextItem);
		HTREEITEM hChildItem2 = m_MenuTree.GetChildItem(hNextItem);
		hNextItem = m_MenuTree.GetNextSiblingItem(hChildItem2);
		hNextItem = m_MenuTree.GetNextSiblingItem(hNextItem);
		m_MenuTree.Select(hNextItem, TVGN_CARET);
		//m_MenuTree.SelectItem(hNextItem);
	}

	return 0;
}


void CMainFrame::InitLeftTreeView(CTreeViewCtrlEx *m_pTreeView)
{
	int nIconIndexNormal = -1, nIconIndexSelected = -1;
	HICON hIcon = NULL;

	// NOTE: Don't Load using the LR_LOADTRANSPARENT bit, because the icon
	//  already properly deals with transparency (setting it causes problems).
	//  We will load this as LR_SHARED so that we don't have to do a DeleteIcon.
	m_LeftTreeImageList.Create(16, 16, ILC_COLOR32 | ILC_MASK, 10, 30);

	hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(),
		MAKEINTRESOURCE(IDI_ICON1),IMAGE_ICON, 16, 16, LR_SHARED);
	nIconIndexNormal = m_LeftTreeImageList.AddIcon(hIcon);
	nIconIndexSelected = nIconIndexNormal;

	int nIconFolderIndexNormal = -1, nIconFolderIndexSelected = -1;
	hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(),
		MAKEINTRESOURCE(IDI_ICON2),IMAGE_ICON, 16, 16, LR_SHARED);
	nIconFolderIndexNormal = m_LeftTreeImageList.AddIcon(hIcon);

	hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(),
		MAKEINTRESOURCE(IDI_ICON3),IMAGE_ICON, 16, 16, LR_SHARED);
	nIconFolderIndexSelected = m_LeftTreeImageList.AddIcon(hIcon);

	hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(),
		MAKEINTRESOURCE(IDI_ICON4),IMAGE_ICON, 16, 16, LR_SHARED);
	nIconFolderIndexSelected = m_LeftTreeImageList.AddIcon(hIcon);

	hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(),
		MAKEINTRESOURCE(IDI_ICON5),IMAGE_ICON, 16, 16, LR_SHARED);
	nIconFolderIndexSelected = m_LeftTreeImageList.AddIcon(hIcon);

	hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(),
		MAKEINTRESOURCE(IDI_ICON6),IMAGE_ICON, 16, 16, LR_SHARED);
	nIconFolderIndexSelected = m_LeftTreeImageList.AddIcon(hIcon);

	// Hook up the image list to the tree view
	m_pTreeView->SetImageList(m_LeftTreeImageList, TVSIL_NORMAL);

	CTreeItem tiRoot = m_pTreeView->InsertItem(_T("ר����Ϣƽ̨"), 0, 0, TVI_ROOT, NULL);

	CTreeItem tiFolder1 = m_pTreeView->InsertItem(_T("��������"), 1, 1, tiRoot, NULL);
	CTreeItem tiFolder2 = m_pTreeView->InsertItem(_T("�������"), 2, 2, tiRoot, NULL);
	CTreeItem tiFolder3 = m_pTreeView->InsertItem(_T("ȫ����������"), 3, 3, tiRoot, NULL);
	//	m_pTreeView->InsertItem(_T("��������"), nIconIndexNormal, nIconIndexSelected, tiFolder3, NULL);
	//	m_pTreeView->InsertItem(_T("��������"), nIconIndexNormal, nIconIndexSelected, tiFolder3, NULL);
	m_pTreeView->InsertItem(_T("���ݲ�ѯ���"), 4, 4, tiFolder3, NULL);

	CTreeItem tiFolder4 = m_pTreeView->InsertItem(_T("�Զ������"), 5, 5, tiRoot, NULL);

	sql = L"select parent_class, class_name from custom_class";
	if (m_runsql.CheckSQLResult(sql))
	{
		CTreeItem tiFoldertemp;
		m_runsql.m_recordset->MoveFirst();
		//		while (!m_runsql.m_recordset->adoBOF)
		{	
			CString name = (char*)(_bstr_t)m_runsql.m_recordset->GetCollect("class_name");
			m_pTreeView->InsertItem( (LPCTSTR)name, 5, 5, tiFolder4, NULL);

			m_runsql.m_recordset->MoveNext();
		}
	}

	tiRoot.Expand();
	tiFolder1.Expand();
	tiFolder3.Expand();
	tiFolder4.Expand();
}


void CMainFrame::InitListView(CListViewCtrl *m_pListView,int nType)
{
	if(m_StatusListImageList.m_hImageList == NULL)
	{
		m_StatusListImageList.Create(16, 16, ILC_COLOR32, 10, 30);
	}
	HBITMAP m_hBitmap = LoadBitmap(NULL,MAKEINTRESOURCE(IDB_STATUS_BMP));
	m_StatusListImageList.Add(m_hBitmap);

	m_pListView->SetImageList(m_StatusListImageList.m_hImageList,LVSIL_SMALL);

	if(nType == INFOLISTVIEW)
	{
		m_pListView->InsertColumn(0, L"���",LVCFMT_LEFT,50,0);	
		m_pListView->InsertColumn(1, L"ר����",LVCFMT_LEFT,100,0);	
		m_pListView->InsertColumn(2, L"ר������",LVCFMT_LEFT,240,0);
		m_pListView->InsertColumn(3, L"��Դ",LVCFMT_LEFT,50,0);	
		m_pListView->InsertColumn(4, L"ҳ��",LVCFMT_LEFT,50,0);
		m_pListView->InsertColumn(5, L"ȫ��",LVCFMT_LEFT,50,0);
		m_pListView->InsertColumn(6, L"����״̬",LVCFMT_LEFT,80,2);
		m_pListView->InsertColumn(7, L"����ʱ��",LVCFMT_LEFT,100,2);
	}
	else if(nType == DOWNLOADLISTVIEW)
	{
		m_pListView->InsertColumn(0, L"״̬",LVCFMT_LEFT,100,0);	
		m_pListView->InsertColumn(1, L"ר����",LVCFMT_LEFT,100,0);
		m_pListView->InsertColumn(2, L"ר������",LVCFMT_LEFT,200,0);	
		m_pListView->InsertColumn(3, L"����",LVCFMT_LEFT,100,0);
		m_pListView->InsertColumn(4, L"��Դ",LVCFMT_LEFT,50,0);
		m_pListView->InsertColumn(5, L"ҳ��",LVCFMT_LEFT,50,0);
		m_pListView->InsertColumn(6, L"������",LVCFMT_LEFT,100,0);
		m_pListView->InsertColumn(7, L"������",LVCFMT_LEFT,100,0);
		m_pListView->InsertColumn(8, L"������",LVCFMT_LEFT,100,0);
	}
	else if (nType == SEARCHLISTVIEW)
	{
		m_pListView->InsertColumn(0, L"���",LVCFMT_LEFT,50,0);	
		m_pListView->InsertColumn(1, L"ר����",LVCFMT_LEFT,100,0);
		m_pListView->InsertColumn(2, L"ר������",LVCFMT_LEFT,240,0);	
		m_pListView->InsertColumn(3, L"��Դ",LVCFMT_LEFT,50,0);
	}
	DWORD dwStyle = m_pListView->GetExtendedListViewStyle();
	dwStyle |= LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES;
	m_pListView->SetExtendedListViewStyle(dwStyle);
}

LRESULT CMainFrame::OnSelChanged(int idCtrl, LPNMHDR pNMHDR, BOOL& bHandled)
{

	if(idCtrl == IDC_LEFT_TREEVIEW)
	{	
		CTreeItem m_SelectItem;
		CString m_ItemText;
		NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
		TVITEM item = pNMTreeView->itemNew;

		m_SelectItem = m_MenuTree.GetSelectedItem();
		m_SelectItem.GetText(m_ItemText);
		if ( m_ItemText == _T("ר����Ϣƽ̨"))
		{
			m_wndHorzSplitter.SetSinglePaneMode( SPLIT_PANE_BOTTOM );
			m_BottomVertSplitter.SetSinglePaneMode( SPLIT_PANE_LEFT );
		}
		else if ( m_ItemText == _T("��������") )
		{
			m_wndHorzSplitter.SetSinglePaneMode( SPLIT_PANE_TOP );
			m_TopVertSplitter.SetSinglePaneMode( SPLIT_PANE_RIGHT );
			m_TopHorzSplitter.SetSinglePaneMode( SPLIT_PANE_TOP );
			
			/*
			sql = L"select * from sipo_data where downpdf=1 and finishdown=0";
			
			if (m_runsql.CheckSQLResult(sql))		//��������
			{
				//m_DownloadListView.DeleteAllItems();
				while (!m_runsql.m_recordset->adoEOF)
				{
					//int download_list_count = m_DownloadListView.GetItemCount();
					m_DownloadListView.InsertItem(0,(LPCTSTR)"");
					m_DownloadListView.SetItemText(0, 0, L"δ����");
					//CString ssss = (LPCTSTR)(_bstr_t)m_runsql.m_recordset->GetCollect("publication_number");
					m_DownloadListView.SetItemText(0, 1, (LPCTSTR)(_bstr_t)m_runsql.m_recordset->GetCollect("application_number"));
					m_DownloadListView.SetItemText(0, 2, (LPCTSTR)(_bstr_t)m_runsql.m_recordset->GetCollect("title"));
					m_DownloadListView.SetItemText(0, 3, L"0" );//(LPCTSTR)(_bstr_t)m_runsql.m_recordset->GetCollect("pagemax")
					m_DownloadListView.SetItemText(0, 4, (LPCTSTR)(_bstr_t)m_runsql.m_recordset->GetCollect("web_url"));
					m_DownloadListView.SetItemText(0, 5, (LPCTSTR)(_bstr_t)m_runsql.m_recordset->GetCollect("pagemax"));
					m_DownloadListView.SetItemText(0, 6, (LPCTSTR)(_bstr_t)m_runsql.m_recordset->GetCollect("application_date"));
					m_DownloadListView.SetItemText(0, 7, (LPCTSTR)(_bstr_t)m_runsql.m_recordset->GetCollect("inventor"));
					m_DownloadListView.SetItemText(0, 8, (LPCTSTR)(_bstr_t)m_runsql.m_recordset->GetCollect("applicant"));

					m_runsql.m_recordset->MoveNext();
				}
			}*/
		} else if (m_ItemText == _T("�������"))
		{
			m_wndHorzSplitter.SetSinglePaneMode( SPLIT_PANE_TOP );
			m_TopVertSplitter.SetSinglePaneMode( SPLIT_PANE_RIGHT );
			m_TopHorzSplitter.SetSinglePaneMode( SPLIT_PANE_BOTTOM );
		} else if (m_ItemText == _T("��������")  || m_ItemText == _T("��������")
			|| m_ItemText == _T("�Զ������") )
		{
			m_InfoListView.DeleteAllItems();
			//m_wndHorzSplitter.SetSinglePaneMode( SPLIT_PANE_NONE );
			m_wndHorzSplitter.SetSinglePaneMode( SPLIT_PANE_TOP );
			m_TopVertSplitter.SetSinglePaneMode( SPLIT_PANE_LEFT );
			m_BottomVertSplitter.SetSinglePaneMode( SPLIT_PANE_RIGHT );
		}
		else if (m_ItemText == _T("���ݲ�ѯ���"))
		{
			m_InfoListView.DeleteAllItems();
			//m_wndHorzSplitter.SetSinglePaneMode( SPLIT_PANE_NONE );
			m_wndHorzSplitter.SetSinglePaneMode( SPLIT_PANE_TOP );
			m_TopVertSplitter.SetSinglePaneMode( SPLIT_PANE_LEFT );
			m_BottomVertSplitter.SetSinglePaneMode( SPLIT_PANE_RIGHT );
			sql = m_localSearchStr;
			//sql = "select * from sipo_data where application_number like '%2010%'";
			if (m_runsql.CheckSQLResult(sql))		//��������
			{
				m_InfoListView.DeleteAllItems();
				while (!m_runsql.m_recordset->adoEOF)
				{
					m_InfoListView.InsertItem(0,(LPCTSTR)"");
					m_InfoListView.SetItemText(0, 0, (LPCTSTR)(_bstr_t)m_runsql.m_recordset->GetCollect("id"));
					m_InfoListView.SetItemText(0, 1, (LPCTSTR)(_bstr_t)m_runsql.m_recordset->GetCollect("application_number"));
					m_InfoListView.SetItemText(0, 2, (LPCTSTR)(_bstr_t)m_runsql.m_recordset->GetCollect("title"));
					m_InfoListView.SetItemText(0, 3, (LPCTSTR)(_bstr_t)m_runsql.m_recordset->GetCollect("web_url"));
					m_InfoListView.SetItemText(0, 4, (LPCTSTR)(_bstr_t)m_runsql.m_recordset->GetCollect("pagemax"));
					m_InfoListView.SetItemText(0, 5, (LPCTSTR)(_bstr_t)m_runsql.m_recordset->GetCollect("abstracts"));
					m_InfoListView.SetItemText(0, 6, (LPCTSTR)L"");
					//					m_InfoListView.SetItemText(0, 6, (LPCTSTR)(_bstr_t)m_runsql.m_recordset->GetCollect("applicant"));
					m_InfoListView.SetItemText(0, 7, (LPCTSTR)(_bstr_t)m_runsql.m_recordset->GetCollect("update_time"));

					m_runsql.m_recordset->MoveNext();
				}
			}
			//m_localSearchStr = "";
		}
		else if (m_ItemText == _T("ȫ����������"))
		{
			//m_wndHorzSplitter.SetSinglePaneMode( SPLIT_PANE_NONE );
			m_wndHorzSplitter.SetSinglePaneMode( SPLIT_PANE_TOP );
			m_TopVertSplitter.SetSinglePaneMode( SPLIT_PANE_LEFT );
			m_BottomVertSplitter.SetSinglePaneMode( SPLIT_PANE_RIGHT );
			sql = L"select * from sipo_data inner join legal_state on sipo_data.application_number=legal_state.patent_data_app_number where downpdf=1 and finishdown=1";
			if (m_runsql.CheckSQLResult(sql))		//��������
			{
				m_InfoListView.DeleteAllItems();
				while (!m_runsql.m_recordset->adoEOF)
				{
					m_InfoListView.InsertItem(0,(LPCTSTR)"");
					m_InfoListView.SetItemText(0, 0, (LPCTSTR)(_bstr_t)m_runsql.m_recordset->GetCollect("sipo_data.id"));
					m_InfoListView.SetItemText(0, 1, (LPCTSTR)(_bstr_t)m_runsql.m_recordset->GetCollect("application_number"));
					m_InfoListView.SetItemText(0, 2, (LPCTSTR)(_bstr_t)m_runsql.m_recordset->GetCollect("title"));
					m_InfoListView.SetItemText(0, 3, (LPCTSTR)(_bstr_t)m_runsql.m_recordset->GetCollect("web_url"));	//����
					m_InfoListView.SetItemText(0, 4, (LPCTSTR)(_bstr_t)m_runsql.m_recordset->GetCollect("pagemax"));	//��Դ
					m_InfoListView.SetItemText(0, 5, (LPCTSTR)(_bstr_t)m_runsql.m_recordset->GetCollect("abstracts"));
					CString legal_state;
					legal_state.Format(L"%s,%s", (CString)(char*)(_bstr_t)m_runsql.m_recordset->GetCollect("legal_state"),
						(CString)(char*)(_bstr_t)m_runsql.m_recordset->GetCollect("pub_time"));
					m_InfoListView.SetItemText(0, 6, (LPCTSTR)legal_state);
					//					m_InfoListView.SetItemText(0, 6, (LPCTSTR)(_bstr_t)m_runsql.m_recordset->GetCollect("applicant"));
					m_InfoListView.SetItemText(0, 7, (LPCTSTR)(_bstr_t)m_runsql.m_recordset->GetCollect("update_time"));

					m_runsql.m_recordset->MoveNext();
				}
			}
		}
	}

	return 0;
}



LRESULT CMainFrame::OnNMRClickList(int idCtrl, LPNMHDR pNMHDR, BOOL& bHandled)
{
	CMenu obMenu ;
	HMENU m_hMenu = NULL;
	CString m_ItemText;



	CPoint obCursorPoint = (0, 0);
	GetCursorPos(&obCursorPoint);
	if (idCtrl == IDC_LEFT_TREEVIEW)
	{
		CTreeItem m_RSelectItem;			//�һ�������Ŀ
		NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
		TVITEM item = pNMTreeView->itemNew;
		m_RSelectItem = m_MenuTree.GetDropHilightItem();
		m_MenuTree.SelectItem(m_RSelectItem);
		m_RSelectItem.GetText(m_ItemText);
		if ( m_ItemText == _T("ר����Ϣƽ̨") || m_ItemText == _T("��������") || m_ItemText == _T("�������") || \
			m_ItemText == _T("ȫ����������") || m_ItemText == _T("��������") || m_ItemText == _T("��������") || m_ItemText == _T("���ݲ�ѯ���"))
		{
		}
		else 
		{
			obMenu.LoadMenu(IDR_TREE_POP); 
			m_hMenu = obMenu.GetSubMenu(0);
			if (m_ItemText == _T("�Զ������"))
			{
				EnableMenuItem(m_hMenu,ID_TREE_DEL, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
				EnableMenuItem(m_hMenu,ID_TREE_EDIT, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED); 
			}
			else
			{
				EnableMenuItem(m_hMenu,ID_TREE_CREATE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
				EnableMenuItem(m_hMenu,ID_TREE_DEL, MF_BYCOMMAND | MF_ENABLED);
				EnableMenuItem(m_hMenu,ID_TREE_EDIT, MF_BYCOMMAND | MF_ENABLED); 
			}
		}
	}
	if(idCtrl == IDC_DOWNLOAD_LISTVIEW)
	{
		obMenu.LoadMenu(IDR_DOWNLOAD_POP); 
		m_hMenu = obMenu.GetSubMenu(0);
		// Get the cursor position
		if (0 >= m_DownloadListView.GetSelectedCount())
		{
			EnableMenuItem(m_hMenu,ID_DOWNLOAD_START, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			EnableMenuItem(m_hMenu,ID_DOWNLOAD_STOP, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			EnableMenuItem(m_hMenu, ID_DOWNLOAD_DEL, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
		}
		else if (1 == m_DownloadListView.GetSelectedCount())
		{
			CString status_name;
			//int nItemSelected=m_InfoListView.GetSelectedCount();//��ѡ������
			int nItemCount=m_DownloadListView.GetItemCount();//��������
			//if(nItemSelected<1) return 0;
			for(int i=nItemCount-1;i>=0;i--)
			{
				int iState=m_DownloadListView.GetItemState(i,LVIS_SELECTED);
				if(iState!=0)
				{
					m_DownloadListView.GetItemText(i, 0, status_name);
					if (status_name == _T("��ֹͣ"))
					{
						EnableMenuItem(m_hMenu, ID_DOWNLOAD_START, MF_BYCOMMAND | MF_ENABLED);
						EnableMenuItem(m_hMenu, ID_DOWNLOAD_STOP, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
					}
					else if (status_name == _T("��������"))
					{
						EnableMenuItem(m_hMenu,ID_DOWNLOAD_START, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
						EnableMenuItem(m_hMenu, ID_DOWNLOAD_STOP, MF_BYCOMMAND | MF_ENABLED);
					}
					else
					{
						EnableMenuItem(m_hMenu,ID_DOWNLOAD_START, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
						EnableMenuItem(m_hMenu, ID_DOWNLOAD_STOP, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
					}
				}
			}
			EnableMenuItem(m_hMenu, ID_DOWNLOAD_DEL, MF_BYCOMMAND | MF_ENABLED);
		}
		else
		{	
			EnableMenuItem(m_hMenu,ID_DOWNLOAD_START, MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(m_hMenu,ID_DOWNLOAD_STOP, MF_BYCOMMAND | MF_ENABLED); 
			EnableMenuItem(m_hMenu, ID_DOWNLOAD_DEL, MF_BYCOMMAND | MF_ENABLED); 
		}
	}
	else if(idCtrl == IDC_INFO_LISTVIEW)
	{
		obMenu.LoadMenu(IDR_INFO_POP); 
		m_hMenu = obMenu.GetSubMenu(0);
		// Get the cursor position
		CPoint obCursorPoint = (0, 0);
		GetCursorPos(&obCursorPoint);

		if (0 >= m_InfoListView.GetSelectedCount())
		{
			EnableMenuItem(m_hMenu,ID_INFO_OPEN, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			EnableMenuItem(m_hMenu,ID_INFO_RELOAD, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED); 
		}
		else
		{
			EnableMenuItem(m_hMenu,ID_INFO_OPEN, MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(m_hMenu,ID_INFO_RELOAD, MF_BYCOMMAND | MF_ENABLED); 
		}
	}
	else if ( idCtrl == IDC_SEARCH_LISTVIEW )
	{
		obMenu.LoadMenu(IDR_SEARCH_POP); 
		m_hMenu = obMenu.GetSubMenu(0);
		// Get the cursor position
		CPoint obCursorPoint = (0, 0);
		GetCursorPos(&obCursorPoint);

		if (0 >= m_SearchListView.GetSelectedCount())
		{
			EnableMenuItem(m_hMenu,ID_SEARCH_LOADSELECT, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			EnableMenuItem(m_hMenu,ID_SEARCH_RELOAD, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED); 
		}
		else
		{
			EnableMenuItem(m_hMenu,ID_SEARCH_LOADSELECT, MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(m_hMenu,ID_SEARCH_RELOAD, MF_BYCOMMAND | MF_ENABLED); 
		}
	}
	// Track the popup menu
	if ( m_hMenu != NULL )
		TrackPopupMenu(m_hMenu,TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, obCursorPoint.x,obCursorPoint.y,0, this->m_hWnd,&rcDefault);

	return 0;
}


/************************************************************************/
/* bellow added by aiwen                                               */
/************************************************************************/

bool CMainFrame::download(CString url,const char *save_as)/*��Urlָ��ĵ�ַ���ļ����ص�save_asָ��ı����ļ�*/
{
	bool result = 0;

	char Temp[MAXBLOCKSIZE];
	ULONG Number = 1;

	FILE *stream;
	HINTERNET hSession = InternetOpen(L"RookIE/1.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (hSession != NULL)
	{
		HINTERNET handle2 = InternetOpenUrl(hSession, url, NULL, 0, INTERNET_FLAG_DONT_CACHE, 0);
		if (handle2 != NULL)
		{


			if( (stream = fopen( save_as, "w+b" )) != NULL)
			{
				while (Number > 0)
				{
					InternetReadFile(handle2, Temp, MAXBLOCKSIZE - 1, &Number);

					fwrite(Temp, sizeof (char), Number , stream);

					//pageContent.append(Temp);

				}
				fclose( stream );
				result = 1;
			}

			InternetCloseHandle(handle2);
			handle2 = NULL;
		}
		InternetCloseHandle(hSession);
		hSession = NULL;
	}
	return result;
}

string CMainFrame::UrlGB2312(char * str)
{
	string dd;
	size_t len = strlen(str);
	for (size_t i=0;i<len;i++)
	{
		if(isalnum((BYTE)str[i]))
		{
			char tempbuff[2];
			sprintf(tempbuff,"%c",str[i]);
			dd.append(tempbuff);
		}
		else if (isspace((BYTE)str[i]))
		{
			dd.append("+");
		}
		else
		{
			char tempbuff[4];
			sprintf(tempbuff,"%%%X%X",((BYTE*)str)[i] >>4,((BYTE*)str)[i] %16);
			dd.append(tempbuff);
		}
	}

	return dd;
}



bool CMainFrame::GetPatents(string pageCont, CString url) 
{

	/************************************************************************/
	/* ��ȡtmp.html�ļ�������                                               */
	/************************************************************************/
	m_SearchListView.DeleteAllItems();

	string str1 = "\"zi_04\">";
	string str2 = "</span>";

	int i =1,j=1;
	string sub_pgcont = pageCont;
	string number; 
	string patent_url; // ר�����ӵ�ַ
	string patent_num; // ר����
	string patent_CN_num; // CNר����
	string patent_name; // ר������

	SYSTEMTIME st;
	CString str_system_time;
	GetLocalTime(&st);
	str_system_time.Format(L"%4d-%2d-%2d %2d:%2d:%2d",st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);

	MD5_CTX *before_url;
	before_url = new MD5_CTX();

	unsigned char buf[200] = {0},final_string[16] = {0};

	char  ch_url[1024];
	memset(ch_url,0,1024);
	USES_CONVERSION;
	strcpy(ch_url,W2A(url.LockBuffer()));
	url.UnlockBuffer();

	memcpy(buf,ch_url,strlen(ch_url));
	before_url->MD5Update(buf,strlen(ch_url));
	before_url->MD5Final(final_string);

	char md5_restult_char[100]={0};
	memcpy(md5_restult_char,final_string,16);
	CString md5_result =  BCD_to_AnsiString(md5_restult_char,16);

	CString sqlsentence;
	sqlsentence.Format(L"insert into search_history_group(search_key,search_param,pageindex,pagemax,[create_time]) \
						values('%s','%s','%d','%d','%s')",global_Search_str, md5_result,current_search_page,search_total_page,str_system_time);

	if (!m_runsql.RunSQL(sqlsentence))
	{
		MessageBox(L"error insert",L"INSERT",0);
	}

	sqlsentence.Format(L"select MAX(id) as groupid from search_history_group");

	if (!m_runsql.CheckSQLResult(sqlsentence)) // ��ȡ�ղ����ID
	{
		MessageBox(L"Select error",L"SELECT",0);
	}

	CString groupid = (LPCTSTR)(_bstr_t)m_runsql.m_recordset->GetCollect("groupid");

	char ch_display[20];
	memset(ch_display,0,20);

	strcpy(ch_display,W2A(groupid.LockBuffer()));
	groupid.UnlockBuffer();

	current_display_groupid = atoi(ch_display);// ��ȡ��ǰ����ʾ���

	while(1)
	{
		i = sub_pgcont.find(str1);
		if (-1 ==i )
		{
			break;
		}
		sub_pgcont = sub_pgcont.substr(i+str1.length(),sub_pgcont.length()- i-str1.length());

		j = sub_pgcont.find(str2);

		number  = sub_pgcont.substr(0,j);  // ��Ž�������
		/***********************************************************************************/

		sub_pgcont = sub_pgcont.substr(j,sub_pgcont.length()-j);

		str1 = "class=\"a01\">";
		str2 = "</a>";

		i = sub_pgcont.find(str1);
		sub_pgcont = sub_pgcont.substr(i+str1.length(),sub_pgcont.length()- i-str1.length());

		j = sub_pgcont.find(str2);

		patent_num  = sub_pgcont.substr(0,j);  // ר���Ž�������

		/***********************************************************************************/

		sub_pgcont = sub_pgcont.substr(j,sub_pgcont.length()-j);

		str1 = "href=\"";
		str2 = "\"";

		i = sub_pgcont.find(str1);
		sub_pgcont = sub_pgcont.substr(i+str1.length(),sub_pgcont.length()- i-str1.length());

		j = sub_pgcont.find(str2);

		patent_url  ="http://211.157.104.87:8080/sipo/zljs/" + sub_pgcont.substr(0,j);  // ר�����ӵ�ַ ��������


		patent_CN_num = Get_application_number(patent_url); //  ��ȡCN ר����
		/***********************************************************************************/

		sub_pgcont = sub_pgcont.substr(j,sub_pgcont.length()-j);

		str1 = "class=\"a01\">";
		str2 = "</a>";
		i = sub_pgcont.find(str1);
		sub_pgcont = sub_pgcont.substr(i+str1.length(),sub_pgcont.length()- i-str1.length());

		j = sub_pgcont.find(str2);

		patent_name  = sub_pgcont.substr(0,j);  // ר�����ƽ�������

		/***********************************************************************************/

		sub_pgcont = sub_pgcont.substr(j,sub_pgcont.length()-j);
		str1 = "\"zi_04\">";
		str2 = "</span>";

		// ��ʾ��ѯ����
		int insert_pos = m_SearchListView.GetItemCount();
		m_SearchListView.InsertItem(insert_pos,L"");
		m_SearchListView.SetItemText(insert_pos, 0, (CString)number.c_str());
		m_SearchListView.SetItemText(insert_pos, 1, (CString)patent_CN_num.c_str());
		m_SearchListView.SetItemText(insert_pos, 2,  (CString)patent_name.c_str());
		m_SearchListView.SetItemText(insert_pos, 3, (CString)patent_url.c_str());

		sqlsentence.Format(L"insert into search_history_item(group_id,res_id,application_number,title,link_url) \
							values('%s','%s','%s','%s','%s')",
							groupid,
							(CString)number.c_str(),
							(CString)patent_CN_num.c_str(),
							(CString)patent_name.c_str(),
							(CString)patent_url.c_str());

		if (!m_runsql.RunSQL(sqlsentence))
		{
			MessageBox(L"Insert search_history_group",L"INSERT",0);
		}

	}


	return 1;
}

int CMainFrame::GetPageNum(string pageCont) // �������������ҳ��
{
	int i = pageCont.find("ҳ�Σ�");

	int j = pageCont.find("&nbsp;&nbsp;��");



	string substr= pageCont.substr(i,j-i);

	i= substr.find("/");

	substr = substr.substr(i+1,substr.length()-1-i);

	int pagenum = atoi(substr.c_str());

	return pagenum;
}

string CMainFrame::ReadHtmlPage(string filename)
{
	string pgcontent;
	ifstream infile;

	infile.open(filename.c_str());

	string inhtml;
	while(!infile.eof())
	{
		infile>>inhtml;
		pgcontent.append(inhtml);

	}

	//cout<<pageContent<<endl;
	infile.close();
	return pgcontent;
}
UINT CMainFrame::StartNetSearchThread(LPVOID lp) // ��ʼ��ѯ
{
	CMainFrame * cnetdlg = (CMainFrame*)lp;

	cnetdlg->ThreadProc1();

	return TRUE;
}
UINT CMainFrame::StartDownloadThread(LPVOID lp) // ��ʼ����
{
	CMainFrame * cnetdlg = (CMainFrame*)lp;

	cnetdlg->ThreadProc2();

	return TRUE;
}
UINT CMainFrame::SearchNextThread(LPVOID lp) // ��һҳ
{
	CMainFrame * cnetdlg = (CMainFrame*)lp;

	cnetdlg->ThreadProc3();

	return TRUE;
}
UINT CMainFrame::SearchPrevThread(LPVOID lp) // ��һҳ
{
	CMainFrame * cnetdlg = (CMainFrame*)lp;

	cnetdlg->ThreadProc4();

	return TRUE;
}
bool CMainFrame::ThreadProc1()// ��ʼ��ѯ�߳�
{

	string searchinfo;

	//MessageBox(str,L"xxxx",0);
	char  ch [4096];
	memset(ch,0,4096);
	USES_CONVERSION;
	strcpy(ch,W2A(global_Search_str.LockBuffer()));
	global_Search_str.UnlockBuffer();

	searchinfo = UrlGB2312(ch);

	CString urltail;

	urltail.Format(L"&flag3=1&pg=%d&sign=0",1);

	CString url = L"http://211.157.104.87:8080/sipo/zljs/hyjs-jieguo.jsp?recshu=20&searchword="+(CString)searchinfo.c_str()+L"&flag3=1&pg=1&sign=0"+global_CheckBox_str;/*L"&flag3=1&pg=1&sign=0"urltail*/

	//MessageBox(url);
	while(!download( url ,"tmp.html"))/*����ʾ�������ذٶȵ���ҳ��c:\index.html�ļ�*/
	{
		Sleep(150);
	}
	m_page_content = ReadHtmlPage("tmp.html");

	 
	if ( -1 != m_page_content.find("û�м��������ר��"))
	{
        MessageBox(L"û�м��������ר��",L"û�м��������ר��",0);
		 
		return 0;
	}

	search_total_page = GetPageNum(m_page_content); //��ò�ѯ����ҳ��
	current_search_page = 1;
	last_dispay_page = 1;
	GetPatents(m_page_content,url);


	//m_page_content=_T("");
	/*for(int ipg = 2; ipg<= pgnumber; ipg++)
	{
	urltail.Format(L"&flag3=1&pg=%d&sign=0",ipg);
	url = L"http://211.157.104.87:8080/sipo/zljs/hyjs-jieguo.jsp?recshu=20&searchword="+(CString)searchinfo.c_str()+urltail; 
	download( url ,"tmp.html"); 
	ReadHtmlPage();
	GetPatents(m_page_content);
	}
	*/	

	return 0;
}
bool CMainFrame::ThreadProc2()
{
	CString cstr_Number;
	CString cstr_Name;
	CString cstr_URL;

	int i,iState;
	int nItemSelected=m_SearchListView.GetSelectedCount();//��ѡ������
	int nItemCount=m_SearchListView.GetItemCount();//��������
	if(nItemSelected<1) return 0;
	for(i=nItemCount-1;i>=0;i--)
	{
		iState=m_SearchListView.GetItemState(i,LVIS_SELECTED);
		if(iState!=0)
		{
			m_SearchListView.GetItemText(i, 1, cstr_Number);
			m_SearchListView.GetItemText(i, 2, cstr_Name);
			m_SearchListView.GetItemText(i, 3, cstr_URL);


			CString tiffFolderPath;			//���ص�TIFF�ļ��� ·��
			tiffFolderPath = currentPath + "TempFolder\\" + cstr_Number;

			CreateDirectory(tiffFolderPath, NULL);

			CString htmlPath;			//˵��HTML���ĵ�·��
			htmlPath = tiffFolderPath + "\\" + cstr_Number + ".html";


			char tiffPath_ch[200];     //˵��TIF���ļ��� ·��
			memset(tiffPath_ch,0,200);
			USES_CONVERSION;
			strcpy(tiffPath_ch,W2A(tiffFolderPath.LockBuffer()));
			tiffFolderPath.UnlockBuffer();

			char  htmlPath_ch[200];
			memset(htmlPath_ch,0,200);

			strcpy(htmlPath_ch,W2A(htmlPath.LockBuffer()));
			htmlPath.UnlockBuffer();

			while (!download(cstr_URL,htmlPath_ch))
			{
				Sleep(150);
			}

			if (-1 != cstr_URL.Find(L"leixin=wgzl"))//��������ר��
			{
				Parse_Appearance_PatentDetails(ReadHtmlPage((string)htmlPath_ch), (string)tiffPath_ch, cstr_Number);

			}
			else // ����ר����ʵ������ר��
			{
				Parse_Invent_Design_PatentDetails(ReadHtmlPage((string)htmlPath_ch), (string)tiffPath_ch, cstr_Number);

				CString pdfFolderPath = currentPath + "DownLoads\\" + cstr_Number ;
				CreateDirectory(pdfFolderPath, NULL);
				CString pdfPath = pdfFolderPath + "\\" + cstr_Name + "_ȫ��.pdf";
				DoMerge(tiffFolderPath, pdfPath);
			}



		}
	}

	return 0;
}

LRESULT CMainFrame::OnSearchLoadselect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// ѡ�п�ʼ����
	//�����㶨λ�����ڵ� ���������ء�

	stop_download_flag = 0;

	HTREEITEM	treeItem;
	treeItem = m_MenuTree.GetRootItem();
	HTREEITEM hChildItem = m_MenuTree.GetChildItem(treeItem);
	m_MenuTree.Select(hChildItem, TVGN_CARET);

	HANDLE  hThread;  
	hThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)StartDownloadThread, this, 0, NULL);
	if (hThread == NULL)
	{
		throw runtime_error("CreateThread error!");
	}


	//	m_MenuTree.Select()
	return 0;
}

void CMainFrame::Parse_Invent_Design_PatentDetails(string pgcontent ,string save_as_path,CString application_number)
{
	string patent_app_num;             //���루ר������
	string patent_app_name;            //��  ��
	string patent_app_abstract;        //ժҪ
	string patent_apply_date;          //������
	string patent_public_date;         //���������棩��
	string patent_public_num;          //���������棩��
	string patent_classify_num;        //�����
	string patent_major_classify_num;  //�������
	string patent_app_person;          //����(ר��Ȩ)��
	string patent_invent_person;        //��������ƣ���
	string patent_address;              //��ַ
	string patent_international_public; //���ʹ���
	string patent_certification_date;   //��֤��
	string patent_agent;                //ר���������
	string patent_attorney;             //������
	string patent_priority;             //����Ȩ

	string patent_fen_orgin_num;                  //�ְ�ԭ�����
	string patent_international_apply;  //��������
	string patent_into_coutry;          //�����������

	string patent_specification_url;  // ר��˵��

	string patent_specification_total_page;  // ר��˵����ҳ��


	/****************************************************************************************/
	int i;
	int j;
	string str1 = "name=\"tifpath\"";
	string str2 = "\"/>";

	i = pgcontent.find(str1);

	string sub_pgcontent;

	sub_pgcontent = pgcontent.substr(i,pgcontent.length()-i);

	str1 = "value=\"";
	i = sub_pgcontent.find(str1);

	sub_pgcontent = sub_pgcontent.substr(i+str1.length(),sub_pgcontent.length()-i-str1.length());

	j = sub_pgcontent.find(str2);

	patent_specification_url.append("http://211.157.104.86/");

	patent_specification_url =patent_specification_url + sub_pgcontent.substr(0,j); //ר��˵����ַ ����������



	cout<<"ר��˵����ַ :"<<patent_specification_url<<endl;


	/****************************************************************************************/

	sub_pgcontent = sub_pgcontent.substr(j,sub_pgcontent.length()-j);

	str1 = "name=\"totalpage\"";
	str2 = "\"";

	i = pgcontent.find(str1);

	sub_pgcontent = pgcontent.substr(i,pgcontent.length()-i);

	str1 = "value=\"";
	i = sub_pgcontent.find(str1);

	sub_pgcontent = sub_pgcontent.substr(i+str1.length(),sub_pgcontent.length()-i-str1.length());

	j = sub_pgcontent.find(str2);

	patent_specification_total_page = sub_pgcontent.substr(0,j); //ר��˵��ҳ�� ����������

	cout<<"ר��˵��ҳ�� :"<<patent_specification_total_page<<endl;


	/****************************************************************************************/
	sub_pgcontent = sub_pgcontent.substr(j,sub_pgcontent.length()-j);

	str1 = "class=\"zi_10\">";

	str2 = "</span>";

	i = sub_pgcontent.find(str1);

	sub_pgcontent = sub_pgcontent.substr(i+str1.length(),sub_pgcontent.length()-i-str1.length());



	j = sub_pgcontent.find(str2);

	patent_app_num = sub_pgcontent.substr(0,j); //���루ר�����Ž���������

	cout<<"���루ר������ :"<<patent_app_num<<endl;

	/****************************************************************************************/
	sub_pgcontent = sub_pgcontent.substr(j,sub_pgcontent.length()-j);

	str1 = "class=\"kuang2\">&nbsp;";

	str2 = "</td>";

	i = sub_pgcontent.find(str1);
	sub_pgcontent = sub_pgcontent.substr(i + str1.length(),sub_pgcontent.length()-i-str1.length());

	j = sub_pgcontent.find(str2);

	patent_apply_date = sub_pgcontent.substr(0,j); //�������� ����������

	cout<<"�������� :"<<patent_apply_date<<endl;

	/****************************************************************************************/
	sub_pgcontent = sub_pgcontent.substr(j,sub_pgcontent.length()-j);

	str1 = "class=\"kuang2\">&nbsp;";

	str2 = "</td>";

	i = sub_pgcontent.find(str1);

	sub_pgcontent =  sub_pgcontent.substr(i+str1.length(),sub_pgcontent.length()-i-str1.length());

	j = sub_pgcontent.find(str2);

	patent_app_name = sub_pgcontent.substr(0,j); //ר�� �� �� ����������

	cout<<"ר�� �� �� :"<<patent_app_name<<endl;

	/****************************************************************************************/

	sub_pgcontent = sub_pgcontent.substr(j,sub_pgcontent.length()-j);


	str1 = "class=\"kuang2\">&nbsp;";

	str2 = "</td>";

	i = sub_pgcontent.find(str1);

	sub_pgcontent =  sub_pgcontent.substr(i+str1.length(),sub_pgcontent.length()-i-str1.length());

	j = sub_pgcontent.find(str2);

	patent_public_num = sub_pgcontent.substr(0,j); //�� �� (����) �� ����������

	cout<<"�� �� (����) ��:"<<patent_public_num<<endl;

	/****************************************************************************************/	

	sub_pgcontent = sub_pgcontent.substr(j,sub_pgcontent.length()-j);


	str1 = "class=\"kuang2\">&nbsp;";

	str2 = "</td>";

	i = sub_pgcontent.find(str1);

	sub_pgcontent =  sub_pgcontent.substr(i+str1.length(),sub_pgcontent.length()-i-str1.length());

	j = sub_pgcontent.find(str2);

	patent_public_date = sub_pgcontent.substr(0,j); //����(����)�� ����������

	cout<<"����(����)��:"<<patent_public_date<<endl;

	/****************************************************************************************/

	sub_pgcontent = sub_pgcontent.substr(j,sub_pgcontent.length()-j);


	str1 = "class=\"kuang2\">&nbsp;";

	str2 = "</td>";

	i = sub_pgcontent.find(str1);

	sub_pgcontent =  sub_pgcontent.substr(i+str1.length(),sub_pgcontent.length()-i-str1.length());

	j = sub_pgcontent.find(str2);

	patent_major_classify_num = sub_pgcontent.substr(0,j); //�� �� �� �� ����������

	cout<<"�� �� �� ��:"<<patent_major_classify_num<<endl;

	/****************************************************************************************/

	sub_pgcontent = sub_pgcontent.substr(j,sub_pgcontent.length()-j);

	str1 = "class=\"kuang2\">&nbsp;";

	str2 = "</td>";

	i = sub_pgcontent.find(str1);

	sub_pgcontent =  sub_pgcontent.substr(i+str1.length(),sub_pgcontent.length()-i-str1.length());

	j = sub_pgcontent.find(str2);

	patent_fen_orgin_num = sub_pgcontent.substr(0,j); //�ְ�ԭ����� ����������

	cout<<"�ְ�ԭ�����:"<<patent_fen_orgin_num<<endl;

	/****************************************************************************************/

	sub_pgcontent = sub_pgcontent.substr(j,sub_pgcontent.length()-j);

	str1 = "class=\"kuang2\">&nbsp;";

	str2 = "</td>";

	i = sub_pgcontent.find(str1);

	sub_pgcontent =  sub_pgcontent.substr(i+str1.length(),sub_pgcontent.length()-i-str1.length());

	j = sub_pgcontent.find(str2);

	patent_classify_num = sub_pgcontent.substr(0,j); //�� �� �� ����������

	cout<<"�� �� ��:"<<patent_classify_num<<endl;

	/****************************************************************************************/

	sub_pgcontent = sub_pgcontent.substr(j,sub_pgcontent.length()-j);

	str1 = "class=\"kuang2\">&nbsp;";

	str2 = "</td>";

	i = sub_pgcontent.find(str1);

	sub_pgcontent =  sub_pgcontent.substr(i+str1.length(),sub_pgcontent.length()-i-str1.length());

	j = sub_pgcontent.find(str2);

	patent_certification_date = sub_pgcontent.substr(0,j); //�� ֤ �� ����������

	cout<<"�� ֤ ��:"<<patent_certification_date<<endl;

	/****************************************************************************************/

	sub_pgcontent = sub_pgcontent.substr(j,sub_pgcontent.length()-j);

	str1 = "class=\"kuang2\">&nbsp;";

	str2 = "</td>";

	i = sub_pgcontent.find(str1);

	sub_pgcontent =  sub_pgcontent.substr(i+str1.length(),sub_pgcontent.length()-i-str1.length());

	j = sub_pgcontent.find(str2);

	patent_priority = sub_pgcontent.substr(0,j); //�� �� Ȩ ����������

	cout<<"�� �� Ȩ:"<<patent_priority<<endl;

	/****************************************************************************************/

	sub_pgcontent = sub_pgcontent.substr(j,sub_pgcontent.length()-j);

	str1 = "class=\"kuang2\">&nbsp;";

	str2 = "</td>";

	i = sub_pgcontent.find(str1);

	sub_pgcontent =  sub_pgcontent.substr(i+str1.length(),sub_pgcontent.length()-i-str1.length());

	j = sub_pgcontent.find(str2);

	patent_app_person = sub_pgcontent.substr(0,j); //����(ר��Ȩ)�� ����������

	cout<<"����(ר��Ȩ)��:"<<patent_app_person<<endl;

	/****************************************************************************************/

	sub_pgcontent = sub_pgcontent.substr(j,sub_pgcontent.length()-j);

	str1 = "class=\"kuang2\">&nbsp;";

	str2 = "</td>";

	i = sub_pgcontent.find(str1);

	sub_pgcontent =  sub_pgcontent.substr(i+str1.length(),sub_pgcontent.length()-i-str1.length());

	j = sub_pgcontent.find(str2);

	patent_address = sub_pgcontent.substr(0,j); //�� ַ ����������

	cout<<"�� ַ:"<<patent_address<<endl;

	/****************************************************************************************/

	sub_pgcontent = sub_pgcontent.substr(j,sub_pgcontent.length()-j);

	str1 = "class=\"kuang2\">&nbsp;";

	str2 = "</td>";

	i = sub_pgcontent.find(str1);

	sub_pgcontent =  sub_pgcontent.substr(i+str1.length(),sub_pgcontent.length()-i-str1.length());

	j = sub_pgcontent.find(str2);

	patent_invent_person = sub_pgcontent.substr(0,j); //�� �� (���)�� ����������

	cout<<"�� �� (���)��:"<<patent_invent_person<<endl;

	/****************************************************************************************/

	sub_pgcontent = sub_pgcontent.substr(j,sub_pgcontent.length()-j);

	str1 = "class=\"kuang2\">&nbsp;";

	str2 = "</td>";

	i = sub_pgcontent.find(str1);

	sub_pgcontent =  sub_pgcontent.substr(i+str1.length(),sub_pgcontent.length()-i-str1.length());

	j = sub_pgcontent.find(str2);

	patent_international_apply = sub_pgcontent.substr(0,j); //�� �� �� �� ����������

	cout<<"�� �� �� ��:"<<patent_international_apply<<endl;

	/****************************************************************************************/

	sub_pgcontent = sub_pgcontent.substr(j,sub_pgcontent.length()-j);

	str1 = "class=\"kuang2\">&nbsp;";

	str2 = "</td>";

	i = sub_pgcontent.find(str1);

	sub_pgcontent =  sub_pgcontent.substr(i+str1.length(),sub_pgcontent.length()-i-str1.length());

	j = sub_pgcontent.find(str2);

	patent_international_public = sub_pgcontent.substr(0,j); //�� �� �� �� ����������

	cout<<"�� �� �� ��:"<<patent_international_public<<endl;

	/****************************************************************************************/

	sub_pgcontent = sub_pgcontent.substr(j,sub_pgcontent.length()-j);

	str1 = "class=\"kuang2\">&nbsp;";

	str2 = "</td>";

	i = sub_pgcontent.find(str1);

	sub_pgcontent =  sub_pgcontent.substr(i+str1.length(),sub_pgcontent.length()-i-str1.length());

	j = sub_pgcontent.find(str2);

	patent_into_coutry = sub_pgcontent.substr(0,j); //����������� ����������

	cout<<"�����������:"<<patent_into_coutry<<endl;

	/****************************************************************************************/

	sub_pgcontent = sub_pgcontent.substr(j,sub_pgcontent.length()-j);

	str1 = "class=\"kuang3\">&nbsp;";

	str2 = "</td>";

	i = sub_pgcontent.find(str1);

	sub_pgcontent =  sub_pgcontent.substr(i+str1.length(),sub_pgcontent.length()-i-str1.length());

	j = sub_pgcontent.find(str2);

	patent_agent = sub_pgcontent.substr(0,j); //ר�� ���� ���� ����������

	cout<<"ר�� ���� ����:"<<patent_agent<<endl;

	/****************************************************************************************/

	sub_pgcontent = sub_pgcontent.substr(j,sub_pgcontent.length()-j);

	str1 = "class=\"kuang3\">&nbsp;";

	str2 = "</td>";

	i = sub_pgcontent.find(str1);

	sub_pgcontent =  sub_pgcontent.substr(i+str1.length(),sub_pgcontent.length()-i-str1.length());

	j = sub_pgcontent.find(str2);

	patent_attorney = sub_pgcontent.substr(0,j); //�� �� �� ����������

	cout<<"�� �� ��:"<<patent_attorney<<endl;

	/****************************************************************************************/


	sub_pgcontent = sub_pgcontent.substr(j,sub_pgcontent.length()-j);

	str1 = "class=\"zi_zw\">&nbsp;";

	str2 = "</td>";

	i = sub_pgcontent.find(str1);

	sub_pgcontent =  sub_pgcontent.substr(i+str1.length(),sub_pgcontent.length()-i-str1.length());

	j = sub_pgcontent.find(str2);

	patent_app_abstract = sub_pgcontent.substr(0,j); //ժҪ ����������

	cout<<"ժҪ:"<<patent_app_abstract<<endl;


	/************************************************************************/
	/* ���������ݿ�洢                                                     */
	/************************************************************************/

	string save_as_dir ;
	int isave = save_as_path.find_last_of("\\");
	save_as_dir = save_as_path.substr(isave+1,save_as_path.length()-isave-1); // ��ȡ�洢Ŀ¼

	SYSTEMTIME st;
	CString str_system_time;
	GetLocalTime(&st);
	str_system_time.Format(L"%4d-%2d-%2d %2d:%2d:%2d",st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);


	CString sqlsentence;
	sqlsentence = L"insert into sipo_data( \
				   application_number,\
				   application_date,\
				   title,\
				   publication_number,\
				   publication_date,\
				   main_classification,\
				   divisional_application_original_number,\
				   classification,\
				   grant_date,\
				   priority,\
				   applicant,\
				   address,\
				   inventor,\
				   international_application,\
				   international_publication,\
				   enter_into_national_phase_date,\
				   agent,\
				   attorney,\
				   abstracts,\
				   update_time,\
				   pagemax ,\
				   web_url, \
				   save_dir) \
				   values('"
				   +application_number+L"','"
				   +(CString)patent_apply_date.c_str()+L"','"
				   +(CString)patent_app_name.c_str()+L"','"
				   +(CString)patent_public_num.c_str()+L"','"
				   +(CString)patent_public_date.c_str()+L"','"
				   +(CString)patent_major_classify_num.c_str()+L"','"
				   +(CString)patent_fen_orgin_num.c_str()+L"','"
				   +(CString)patent_classify_num.c_str()+L"','"
				   +(CString)patent_certification_date.c_str()+L"','"
				   +(CString)patent_priority.c_str()+L"','"
				   +(CString)patent_app_person.c_str()+L"','"
				   +(CString)patent_address.c_str()+L"','"
				   +(CString)patent_invent_person.c_str()+L"','"
				   +(CString)patent_international_apply.c_str()+L"','"
				   +(CString)patent_international_public.c_str()+L"','"
				   +(CString)patent_into_coutry.c_str()+L"','"
				   +(CString)patent_agent.c_str()+L"','"
				   +(CString)patent_attorney.c_str()+L"','"
				   +(CString)patent_app_abstract.c_str()+L"','"
				   +str_system_time +L"','"
				   +(CString)patent_specification_total_page.c_str()+L"','"
				   +(CString)patent_specification_url.c_str()+L"','"
				   +(CString)save_as_dir.c_str()+L"')";

	if (!m_runsql.RunSQL(sqlsentence))
	{
		MessageBox(L"error insert",L"INSERT",0);
	}


	/************************************************************************/
	/* ��������ʾ����������                                                 */
	/************************************************************************/

	/*
	USES_CONVERSION;  

	CString cstr_patent_num = application_number;          //ר����
	LPWSTR pwStr_Num=cstr_patent_num.GetBuffer(cstr_patent_num.GetLength());  
	cstr_patent_num.ReleaseBuffer();

	CString cstr_patent_description = (CString)patent_app_name.c_str();  // ר������
	LPWSTR pwStr_Description= cstr_patent_description.GetBuffer(cstr_patent_description.GetLength());  
	cstr_patent_description.ReleaseBuffer();

	CString cstr_patent_url = (CString) patent_specification_url.c_str(); // ������Դ
	LPWSTR pwStr_URL=cstr_patent_url.GetBuffer(cstr_patent_url.GetLength());
	cstr_patent_url.ReleaseBuffer();

	CString cstr_patent_pgnum = (CString) patent_specification_total_page.c_str(); // ����ҳ��
	LPWSTR pwStr_pgnum=cstr_patent_pgnum.GetBuffer(cstr_patent_pgnum.GetLength());
	cstr_patent_pgnum.ReleaseBuffer();

	CString cstr_patent_app_date = (CString) patent_apply_date.c_str();          // ������
	LPWSTR pwStr_app_date=cstr_patent_app_date.GetBuffer(cstr_patent_app_date.GetLength());
	cstr_patent_app_date.ReleaseBuffer();

	CString cstr_patent_invent_person = (CString) patent_invent_person.c_str(); // ������
	LPWSTR pwStr_invent_person=cstr_patent_invent_person.GetBuffer(cstr_patent_invent_person.GetLength());
	cstr_patent_invent_person.ReleaseBuffer();

	CString cstr_patent_app_person = (CString) patent_app_person.c_str(); // ������
	LPWSTR pwStr_app_person=cstr_patent_app_person.GetBuffer(cstr_patent_app_person.GetLength());
	cstr_patent_app_person.ReleaseBuffer();

	LVITEM lvitem;

	int download_list_count = m_DownloadListView.GetItemCount();
	lvitem.mask        = LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;   
	lvitem.iItem       = download_list_count ;   
	lvitem.iSubItem=0;   
	lvitem.pszText   = L"��������";     //LPWSTR ����״̬
	lvitem.lParam     = 0;   
	int iPos=m_DownloadListView.InsertItem(&lvitem);//���ر��������������   

	lvitem.mask         =LVIF_TEXT;   
	lvitem.iItem       =iPos;   
	lvitem.iSubItem=1;   
	lvitem.pszText   = pwStr_Num;                 //ר����
	m_DownloadListView.SetItem(&lvitem);   

	lvitem.iSubItem=2;   
	lvitem.pszText   =pwStr_Description;          // ר������
	m_DownloadListView.SetItem(&lvitem);  

	lvitem.iSubItem=3;   
	lvitem.pszText   =L"������0ҳ";                  // ���ؽ���
	m_DownloadListView.SetItem(&lvitem); 

	lvitem.iSubItem=4;   
	lvitem.pszText   =pwStr_URL;                     // ������Դ
	m_DownloadListView.SetItem(&lvitem);

	lvitem.iSubItem=5;   
	lvitem.pszText   =pwStr_pgnum;                    // ����ҳ��
	m_DownloadListView.SetItem(&lvitem); 

	lvitem.iSubItem=6;   
	lvitem.pszText   =pwStr_app_date;                  // ������
	m_DownloadListView.SetItem(&lvitem); 

	lvitem.iSubItem=7;   
	lvitem.pszText   =pwStr_invent_person;            // ������
	m_DownloadListView.SetItem(&lvitem); 

	lvitem.iSubItem=8;   
	lvitem.pszText   =pwStr_app_person;                  // ������
	m_DownloadListView.SetItem(&lvitem); 
*/
    int download_list_count = m_DownloadListView.GetItemCount();
	m_DownloadListView.InsertItem(download_list_count,(LPCTSTR)"");
	m_DownloadListView.SetItemText(download_list_count,0,L"��������");
	m_DownloadListView.SetItemText(download_list_count,1,application_number);//ר����
	m_DownloadListView.SetItemText(download_list_count,2,(CString)patent_app_name.c_str());// ר������
	m_DownloadListView.SetItemText(download_list_count,3,L"������0ҳ"); // ���ؽ���
	m_DownloadListView.SetItemText(download_list_count,4,(CString) patent_specification_url.c_str());// ������Դ
	m_DownloadListView.SetItemText(download_list_count,5,(CString) patent_specification_total_page.c_str());// ����ҳ��
	m_DownloadListView.SetItemText(download_list_count,6,(CString) patent_apply_date.c_str()); // ������
	m_DownloadListView.SetItemText(download_list_count,7,(CString) patent_invent_person.c_str()); // ������
    m_DownloadListView.SetItemText(download_list_count,8,(CString) patent_app_person.c_str()); // ������

	char CN_temp_app_num[20];
	memset(CN_temp_app_num,0,20);
	USES_CONVERSION;
	strcpy(CN_temp_app_num,W2A(application_number.LockBuffer()));
	application_number.UnlockBuffer();

	Patent_Specification_Download((string)CN_temp_app_num,patent_specification_url,patent_specification_total_page,save_as_path,download_list_count);
	/****************************************************************************************/
}

void CMainFrame::Patent_Specification_Download(string patent_num,string specification_url, string pgtotal,string save_as_path,int item_index)
{
	//lvitem->iItem       = i ;
	 
	
	
// 
// 	lvitem->iSubItem=1;   
// 	lvitem->pszText   = L"xxxxxxxxxxxxxxxx";     //LPWSTR ����״̬
// 	lvitem->lParam     = 0; 
// 	m_DownloadListView.SetItem(&(*lvitem));

	int pg_total = atoi(pgtotal.c_str());

	string str_url = "000001.tif";
    string file_ext = "%6.6d.tif";
	int k = specification_url.find(str_url);

	if (-1 == k)
	{
		k= specification_url.find("000001.jpg");
		file_ext = "%6.6d.jpg";
	}
	specification_url = specification_url.substr(0,k);


	char tiffPath_ch[200];
	memset(tiffPath_ch,0,200);
	USES_CONVERSION;
	strcpy(tiffPath_ch,W2A(currentPath.LockBuffer()));
	currentPath.UnlockBuffer();
	string filedir;

	filedir = (string)tiffPath_ch;

	filedir =filedir+ "TempFolder\\"+patent_num;  

	int downloaded_pages = 0; // �����ж��Ƿ��������

	for (int i=  1; i<=pg_total; i++)
	{

		char ch[40];
		memset(ch,0,40);

		string url_format;
		string save_as;

		url_format.append(file_ext);

		sprintf(ch,url_format.c_str(),i);

		save_as = save_as_path+"\\"+(string)ch;

		cout<<"\n"<<specification_url+ (string)ch<<"\n"<<endl;

		str_url=specification_url +(string)ch;

		while(!download(str_url.c_str(),save_as.c_str()))
		{
			if(Is_Stop_Current_Download((CString) patent_num.c_str())) //�������ֹͣ������������
				break;
			Sleep(1000);
		}

		if(Is_Stop_Current_Download((CString) patent_num.c_str())) //�������ֹͣ������������
			break;

		USES_CONVERSION;
		CString cstr_patent_pgnum; // ����ҳ��
		cstr_patent_pgnum.Format(L"������%dҳ",i);
/*
		LPWSTR pwStr_pgnum=cstr_patent_pgnum.GetBuffer(cstr_patent_pgnum.GetLength());
		cstr_patent_pgnum.ReleaseBuffer();

		lvitem->iSubItem=3;   
		lvitem->pszText   =pwStr_pgnum;                    // ����ҳ��
		m_DownloadListView.SetItem(&(*lvitem)); 
*/
		m_DownloadListView.SetItemText(item_index,3,cstr_patent_pgnum);

		downloaded_pages = i; // ������ҳ�����ƣ����������Ƿ�ȥ�ϲ�

	}

	if (Is_Stop_Current_Download((CString) patent_num.c_str()))
	{
// 		lvitem->iSubItem=0;   
// 		lvitem->pszText   =L"��ֹͣ";                  
// 		m_DownloadListView.SetItem(&(*lvitem)); 

		m_DownloadListView.SetItemText(item_index,0,L"��ֹͣ");  // ����״̬

		//д���ݿ⣬����finishdownload�ֶ�
		sql.Empty();
		sql.Format(L"update sipo_data set finishdown=0 where application_number='%s'", (CString)patent_num.c_str());
		if (!m_runsql.RunSQL(sql))
		{
			AtlMessageBox(NULL, L"Update finishdown error");
		}
	}
	else
	{
// 		lvitem->iSubItem=0;   
// 		lvitem->pszText   =L"�����";                    // ����״̬
// 		m_DownloadListView.SetItem(&(*lvitem)); 
		m_DownloadListView.SetItemText(item_index,0,L"�����");  // ����״̬

		//д���ݿ⣬����finishdownload�ֶ�
		sql.Empty();
		sql.Format(L"update sipo_data set finishdown=1 where application_number='%s'", (CString)patent_num.c_str());
		if (!m_runsql.RunSQL(sql))
		{
			AtlMessageBox(NULL, L"Update finishdown error");
		}
	}

}

bool CMainFrame::DoMerge(CString szPath, CString pdfPath)
{
	//	CString mergedFileName = szPath +_T("0.tif");
	CString wildCard = szPath + _T("\\*");
	std::list<CString> fileNameArray;


	CFindFile finder;
	if (finder.FindFile(wildCard))
	{
		BOOL bWorking = TRUE;
		while (bWorking)
		{
			bWorking = finder.FindNextFile();

			if (finder.IsDots() || finder.IsDirectory())
				continue;

			CString fileName = finder.GetFilePath();
			if (fileName.Right(3) == "tif")
				fileNameArray.push_back(fileName);
		}
	}

	fileNameArray.sort(alphabetOrder);

	LoadDll();
	CString str ;
	char  filename[1024];
	USES_CONVERSION;

	if (OpenPDF("tmp_image")== 0){

		CString strImagePath;
		std::list<CString>::iterator iter;

		for (iter = fileNameArray.begin(); iter != fileNameArray.end(); iter++)
		{
			CString tempName = *iter;

			memset(filename,0,1024);
			strcpy(filename,W2A(tempName.LockBuffer()));
			tempName.UnlockBuffer();

			AddTIFF(filename);
		}
		memset(filename, 0, 1024);
		strcpy(filename, W2A(pdfPath.LockBuffer()));
		pdfPath.UnlockBuffer();

		ClosePDF("tmp_image", filename);
	}

	if (hDll!=NULL)	FreeLibrary(hDll);

	return 0;
}

LRESULT CMainFrame::OnOpenPdf(WORD , WORD , HWND , BOOL& )
{
	CString patentNumber;
	int nItemSelected=m_InfoListView.GetSelectedCount();//��ѡ������
	int nItemCount=m_InfoListView.GetItemCount();//��������
	if(nItemSelected<1) return 0;
	for(int i=nItemCount-1;i>=0;i--)
	{
		int iState=m_InfoListView.GetItemState(i,LVIS_SELECTED);
		if(iState!=0)
		{
			m_InfoListView.GetItemText(i, 1, patentNumber);
			CString pdfPath = currentPath + "DownLoads\\" + patentNumber;

			CString wildCard = pdfPath + _T("\\*");

			CFindFile finder;
			if (finder.FindFile(wildCard))
			{
				BOOL bWorking = TRUE;
				while (bWorking)
				{
					bWorking = finder.FindNextFile();

					if (finder.IsDots() || finder.IsDirectory())
						continue;

					CString fileName = finder.GetFilePath();
					if (fileName.Right(3) == "pdf")
						ShellExecute(NULL, L"open", fileName, NULL, NULL, SW_HIDE);
					;
				}
			}

		}// end of if(iState!=0)
	}//end of for

	return 0;
}

LRESULT CMainFrame::OnSearchPrev(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{

	HANDLE  hThread;  
	hThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SearchPrevThread, this, 0, NULL);
	if (hThread == NULL)
	{
		throw runtime_error("CreateThread error!");
	}
	return 0;
}

LRESULT CMainFrame::OnSearchNext(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
{
	HANDLE  hThread;  
	hThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SearchNextThread, this, 0, NULL);
	if (hThread == NULL)
	{
		throw runtime_error("CreateThread error!");
	}

	return 0;
}

bool CMainFrame::ThreadProc3()   // ��һҳ
{

	if (0 <= last_dispay_page  && last_dispay_page <current_search_page && current_search_page>0)
	{
		CString sqlsentence;

		sqlsentence.Format(L"select res_id,application_number,title,link_url \
							from search_history_item where group_id=%d",++current_display_groupid);

		++last_dispay_page;

		if (m_runsql.CheckSQLResult(sqlsentence))		//��������
		{

			m_SearchListView.DeleteAllItems();
			while (!m_runsql.m_recordset->adoEOF)
			{	
				int insert_pos = m_SearchListView.GetItemCount();
				m_SearchListView.InsertItem(insert_pos,L"");
				m_SearchListView.SetItemText(insert_pos, 0,(LPCTSTR)(_bstr_t)m_runsql.m_recordset->GetCollect("res_id"));
				m_SearchListView.SetItemText(insert_pos, 1, (LPCTSTR)(_bstr_t)m_runsql.m_recordset->GetCollect("application_number"));
				m_SearchListView.SetItemText(insert_pos, 2,  (LPCTSTR)(_bstr_t)m_runsql.m_recordset->GetCollect("title"));
				m_SearchListView.SetItemText(insert_pos, 3, (LPCTSTR)(_bstr_t)m_runsql.m_recordset->GetCollect("link_url"));

				m_runsql.m_recordset->MoveNext();
			}
		}
	}
	else if ( current_search_page > 0 &&  search_total_page >0 )
	{

		string searchinfo;
		char  ch [4096];
		memset(ch,0,4096);
		USES_CONVERSION;
		strcpy(ch,W2A(global_Search_str.LockBuffer()));
		global_Search_str.UnlockBuffer();

		searchinfo = UrlGB2312(ch);

		CString urltail;
		CString url;


		if (current_search_page > search_total_page)
		{
			MessageBox(L"û����һҳ��",L"xxxx",0);

		}
		else
		{
			urltail.Format(L"&flag3=1&pg=%d&sign=0",++current_search_page);

			url = L"http://211.157.104.87:8080/sipo/zljs/hyjs-jieguo.jsp?recshu=20&searchword="+(CString)searchinfo.c_str()+urltail; 

			download( url ,"tmp.html"); 

			string page_content = ReadHtmlPage("tmp.html");

			GetPatents(page_content,url);
			++last_dispay_page;

		}
	}
	else
	{
		MessageBox(L"û����һҳ����ʾ",L"��ʾ",0);
	}


	return 0;
}

bool CMainFrame::ThreadProc4() // ��һҳ
{

	if (last_dispay_page <= 1)
	{
		MessageBox(L"û����һҳ��",L"��һҳ",0);
	}
	else
	{
		CString sqlsentence;

		sqlsentence.Format(L"select res_id,application_number,title,link_url \
							from search_history_item where group_id=%d",--current_display_groupid);
		--last_dispay_page;

		if (m_runsql.CheckSQLResult(sqlsentence))		//��������
		{

			m_SearchListView.DeleteAllItems();
			while (!m_runsql.m_recordset->adoEOF)
			{	
				int insert_pos = m_SearchListView.GetItemCount();
				m_SearchListView.InsertItem(insert_pos,L"");
				m_SearchListView.SetItemText(insert_pos, 0,(LPCTSTR)(_bstr_t)m_runsql.m_recordset->GetCollect("res_id"));
				m_SearchListView.SetItemText(insert_pos, 1, (LPCTSTR)(_bstr_t)m_runsql.m_recordset->GetCollect("application_number"));
				m_SearchListView.SetItemText(insert_pos, 2,  (LPCTSTR)(_bstr_t)m_runsql.m_recordset->GetCollect("title"));
				m_SearchListView.SetItemText(insert_pos, 3, (LPCTSTR)(_bstr_t)m_runsql.m_recordset->GetCollect("link_url"));

				m_runsql.m_recordset->MoveNext();
			}
		}
	}
	return 0;
}

CString CMainFrame::BCD_to_AnsiString(char *P_BCD,int length) 
{ 
	//TODO:   Add   your   source   code   here 
	CString returnstring =L" ";
	char   high_char,low_char,temp;
	for(int   i=0;i <length;i++) 
	{ 
		temp   =   *P_BCD++; 
		high_char   =   (temp&0xf0)>>4;
		low_char   =   temp&0x0f; 
		if   ((high_char>=0x00)&&(high_char<=0x09)) 
			high_char+=0x30; 

		if   ((high_char>=0x0A)&&(high_char<=0x0F)) 

			high_char+=0x37; 
		if   ((low_char>=0x00)&&(low_char<=0x09))
			low_char+=0x30; 
		if   ((low_char>=0x0A)&&(low_char<=0x0F)) 
			low_char+=0x37; 

		returnstring+=high_char; 
		returnstring+=low_char;  
	} 
	return   returnstring; 
} 

string CMainFrame::Get_application_number(string url)
{
	int i;
	int j;
	i = url.find("=");
	j = url.find("&leixin");

	string patent_application_number = url.substr(i+1,j-i-1);

	return patent_application_number;
}

LRESULT CMainFrame::OnViewTree(WORD , WORD , HWND , BOOL& )
{
	static bool b_hide = false;

	if (!b_hide)		//���Ϊ�Ѿ���ʾ����Ϊ����
	{
		m_wndVertSplitter.SetSinglePaneMode( SPLIT_PANE_RIGHT );
		b_hide = true;
	}
	else
	{
		m_wndVertSplitter.SetSinglePaneMode( SPLIT_PANE_NONE );
		b_hide = false;
	}

	return 0;
}

LRESULT CMainFrame::OnViewDownLoad(WORD , WORD , HWND , BOOL& )
{
	//�����㶨λ�����ڵ� ����������� 
	HTREEITEM	treeItem;
	treeItem = m_MenuTree.GetRootItem();
	HTREEITEM hChildItem = m_MenuTree.GetChildItem(treeItem);
	m_MenuTree.Select(hChildItem, TVGN_CARET);

	return 0;
}

LRESULT CMainFrame::OnStartDownloadSelect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	stop_download_flag = 0;
	HANDLE  hThread;  
	hThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)StartDownloadUnfinishedThread, this, 0, NULL);
	if (hThread == NULL)
	{
		throw runtime_error("CreateThread error!");
	}
	return 0;
}
LRESULT CMainFrame::OnStopDownload(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	stop_download_flag = 1;
	return 0;
}

UINT CMainFrame::StartDownloadUnfinishedThread(LPVOID lp) // ��ʼ����δ������ɵ�
{
	CMainFrame * cnetdlg = (CMainFrame*)lp;

	cnetdlg->ThreadProc5();

	return TRUE;
}
bool CMainFrame::ThreadProc5() // ��ʼ����δ������ɵ�
{
	CString cstr_Number;
	CString cstr_Name;
	CString cstr_URL;
	CString cstr_Total_Page;
 

	int i,iState;
	int nItemSelected=m_DownloadListView.GetSelectedCount();//��ѡ������
	int nItemCount=m_DownloadListView.GetItemCount();//��������
	if(nItemSelected<1) return 0;
	for(i=nItemCount-1;i>=0;i--)
	{
		iState=m_DownloadListView.GetItemState(i,LVIS_SELECTED);
		if(iState!=0)
		{
			m_DownloadListView.GetItemText(i, 1, cstr_Number);
			
			m_DownloadListView.GetItemText(i, 2, cstr_Name);

			m_DownloadListView.GetItemText(i, 4, cstr_URL);

			m_DownloadListView.GetItemText(i, 5, cstr_Total_Page);


			CString tiffFolderPath;			//���ص�TIFF�ļ��� ·��
			tiffFolderPath = currentPath + "TempFolder\\" + cstr_Number;
/*
			CreateDirectory(tiffFolderPath, NULL);

			CString htmlPath;			//˵��HTML���ĵ�·��
			htmlPath = tiffFolderPath + "\\" + cstr_Number + ".html";


			char tiffPath_ch[200];
			memset(tiffPath_ch,0,200);
			USES_CONVERSION;
			strcpy(tiffPath_ch,W2A(tiffFolderPath.LockBuffer()));
			tiffFolderPath.UnlockBuffer();

			char  htmlPath_ch[200];
			memset(htmlPath_ch,0,200);

			strcpy(htmlPath_ch,W2A(htmlPath.LockBuffer()));
			htmlPath.UnlockBuffer();


			while (!download(cstr_URL,htmlPath_ch))
			{
				Sleep(150);
			}
*/
	//		ParsePatentDetails(ReadHtmlPage((string)htmlPath_ch), (string)tiffPath_ch, cstr_Number);


	/*	
			int iPos=m_DownloadListView.InsertItem(&lvitem);//���ر��������������   

			lvitem.mask         =LVIF_TEXT;   
			lvitem.iItem       =iPos;   
			lvitem.iSubItem=1;   
			lvitem.pszText   = pwStr_Num;                 //ר����
			m_DownloadListView.SetItem(&lvitem);   

			lvitem.iSubItem=2;   
			lvitem.pszText   =pwStr_Description;          // ר������
			m_DownloadListView.SetItem(&lvitem);  

			lvitem.iSubItem=3;   
			lvitem.pszText   =L"������0ҳ";                  // ���ؽ���
			m_DownloadListView.SetItem(&lvitem); 

			lvitem.iSubItem=4;   
			lvitem.pszText   =pwStr_URL;                     // ������Դ
			m_DownloadListView.SetItem(&lvitem);

			lvitem.iSubItem=5;   
			lvitem.pszText   =pwStr_pgnum;                    // ����ҳ��
			m_DownloadListView.SetItem(&lvitem); 

			lvitem.iSubItem=6;   
			lvitem.pszText   =pwStr_app_date;                  // ������
			m_DownloadListView.SetItem(&lvitem); 

			lvitem.iSubItem=7;   
			lvitem.pszText   =pwStr_invent_person;            // ������
			m_DownloadListView.SetItem(&lvitem); 

			lvitem.iSubItem=8;   
			lvitem.pszText   =pwStr_app_person;                  // ������
			m_DownloadListView.SetItem(&lvitem); 
*/
			char CN_temp_app_num[20];
			memset(CN_temp_app_num,0,20);

			USES_CONVERSION;
			strcpy(CN_temp_app_num,W2A(cstr_Number.LockBuffer()));
			cstr_Number.UnlockBuffer();

			char patent_specification_url[200];
			memset(patent_specification_url,0,200);
			strcpy(patent_specification_url,W2A(cstr_URL.LockBuffer()));
			cstr_URL.UnlockBuffer();

			char patent_specification_total_page[200];
			memset(patent_specification_total_page,0,200);
			strcpy(patent_specification_total_page,W2A(cstr_Total_Page.LockBuffer()));
			cstr_Total_Page.UnlockBuffer();

			char tiffPath_ch[200];
			memset(tiffPath_ch,0,200);
			strcpy(tiffPath_ch,W2A(tiffFolderPath.LockBuffer()));
			tiffFolderPath.UnlockBuffer();
/*
			LVITEM lvitem;
			lvitem.mask        = LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;   
			lvitem.iItem       = i ;   
			lvitem.iSubItem=0;   
			lvitem.pszText   = L"��������";     //LPWSTR ����״̬
			lvitem.lParam     = 0; 
			m_DownloadListView.SetItem(&lvitem);
*/
			m_DownloadListView.SetItemText(i,0,L"��������");

			//m_DownloadListView.GetItem(&lvitem);

			Patent_Specification_Download((string)CN_temp_app_num,(string)patent_specification_url,(string)patent_specification_total_page,(string)tiffPath_ch,i);


			CString pdfFolderPath = currentPath + "DownLoads\\" + cstr_Number ;
			CreateDirectory(pdfFolderPath, NULL);
			CString pdfPath = pdfFolderPath + "\\" + cstr_Name + "_ȫ��.pdf";
			DoMerge(tiffFolderPath, pdfPath);

		}
	}
	return 0;
}

bool CMainFrame::Is_Stop_Current_Download(CString cstr_patent_number)
{
	CString cstr_Number;
	CString cstr_URL;
	CString cstr_Name;

	int i,iState;
	int nItemSelected=m_DownloadListView.GetSelectedCount();//��ѡ������
	int nItemCount=m_DownloadListView.GetItemCount();//��������
	if(nItemSelected<1) return 0;
	for(i=nItemCount-1;i>=0;i--)
	{
		iState=m_DownloadListView.GetItemState(i,LVIS_SELECTED);
		if(iState!=0)
		{
			m_DownloadListView.GetItemText(i, 1, cstr_Number);
			m_DownloadListView.GetItemText(i, 2, cstr_Name);
			if ((cstr_Number == cstr_patent_number) && stop_download_flag)// �Ƿ�͵�ǰѡ��ֹͣ��һ��
			{
				return 1;
			}
		}
	}
	return 0;
}

void CMainFrame::Parse_Appearance_PatentDetails(string pgcontent,string save_as_path,CString application_number)
{
	 string patent_app_num;             //���루ר������
	 string patent_app_name;            //��  ��
	 string patent_app_abstract;        //ժҪ
	 string patent_apply_date;          //������
	 string patent_public_date;         //���������棩��
	 string patent_public_num;          //���������棩��
	 string patent_classify_num;        //�����
	 string patent_major_classify_num;  //�������
	 string patent_app_person;          //����(ר��Ȩ)��
	 string patent_invent_person;        //��������ƣ���
	 string patent_address;              //��ַ
	 string patent_international_public; //���ʹ���
	 string patent_certification_date;   //��֤��
	 string patent_agent;                //ר���������
	 string patent_attorney;             //������
     string patent_priority;             //����Ȩ

	 string patent_fen_orgin_num;                  //�ְ�ԭ�����
	 string patent_international_apply; //��������
	 string patent_into_coutry; //��������
	 
	 string patent_specification_url;  // ר��˵��

	 string patent_specification_total_page;  // ר��˵����ҳ��


	 /****************************************************************************************/
	 int i;
	 int j;
	 string str1 = "������š�&nbsp;";
	 string str2 = "</";
	 
	 i = pgcontent.find(str1);

	 pgcontent = pgcontent.substr(i+str1.length(),pgcontent.length()-i-str1.length());
     
	 j = pgcontent.find(str2);

	 patent_app_num = pgcontent.substr(0,j); //����Ž���������

	  
	 cout<<"������š�"<<patent_app_num<<endl;
	 /****************************************************************************************/

	 str1 = "�������ա�&nbsp;";

	 i = pgcontent.find(str1);
	 
	 pgcontent = pgcontent.substr(i+str1.length(),pgcontent.length()-i-str1.length());
     
	 j = pgcontent.find(str2);
	 
	 patent_apply_date = pgcontent.substr(0,j);  //������ ����������

      
	 cout<<"�������ա�"<<patent_apply_date<<endl;
	 /****************************************************************************************/
	 
	 str1 = "�����ơ�&nbsp;";
	 
	 i = pgcontent.find(str1);
	 
	 pgcontent = pgcontent.substr(i+str1.length(),pgcontent.length()-i-str1.length());
     
	 j = pgcontent.find(str2);
	 
	 patent_app_name = pgcontent.substr(0,j);   //���� ����������
	  
	 cout<<"�����ơ�"<<patent_app_name<<endl;  

	 /****************************************************************************************/
	 
	 str1 = "�����������棩�š�&nbsp;";
	 
	 i = pgcontent.find(str1);
	 
	 pgcontent = pgcontent.substr(i+str1.length(),pgcontent.length()-i-str1.length());
     
	 j = pgcontent.find(str2);
	 
	 patent_public_num = pgcontent.substr(0,j);  //���������棩�� ����������
	 
	 cout<<"�����������棩�š�"<<patent_public_num<<endl;
	 /****************************************************************************************/
	 
	 str1 = "�����������棩�ա�&nbsp;";
	 
	 i = pgcontent.find(str1);
	 
	 pgcontent = pgcontent.substr(i+str1.length(),pgcontent.length()-i-str1.length());
     
	 j = pgcontent.find(str2);
	 
	 patent_public_date = pgcontent.substr(0,j);  //���������棩�� ����������
	 
	 cout<<"�����������棩�ա�"<<patent_public_date<<endl;
	 /****************************************************************************************/
	 
	 str1 = "��������š�&nbsp;";
	 
	 i = pgcontent.find(str1);
	 
	 pgcontent = pgcontent.substr(i+str1.length(),pgcontent.length()-i-str1.length());
     
	 j = pgcontent.find(str2);
	 
	 patent_major_classify_num = pgcontent.substr(0,j); //������� ����������
	 
	 cout<<"��������š�"<<patent_major_classify_num<<endl;
	 /****************************************************************************************/
	 
	 str1 = "���ְ�ԭ����š�&nbsp;";
	 
	 i = pgcontent.find(str1);
	 
	 pgcontent = pgcontent.substr(i+str1.length(),pgcontent.length()-i-str1.length());
     
	 j = pgcontent.find(str2);
	 
	 patent_fen_orgin_num = pgcontent.substr(0,j); //�ְ�ԭ����� ����������
	 
	 cout<<"���ְ�ԭ����š�"<<patent_fen_orgin_num<<endl;
	 /****************************************************************************************/
	 
	 str1 = "������š�&nbsp;";
	 
	 i = pgcontent.find(str1);
	 
	 pgcontent = pgcontent.substr(i+str1.length(),pgcontent.length()-i-str1.length());
     
	 j = pgcontent.find(str2);
	 
	 patent_classify_num = pgcontent.substr(0,j);  //����� ����������
	 
	 cout<<"������š�"<<patent_classify_num<<endl;
	 /****************************************************************************************/
	 
	 str1 = "����֤�ա�&nbsp;";
	 
	 i = pgcontent.find(str1);
	 
	 pgcontent = pgcontent.substr(i+str1.length(),pgcontent.length()-i-str1.length());
     
	 j = pgcontent.find(str2);
	 
	 patent_certification_date = pgcontent.substr(0,j); //��֤�� ����������
	 
	 cout<<"����֤�ա�"<<patent_certification_date<<endl;
	 /****************************************************************************************/
	 
	 str1 = "������Ȩ��&nbsp;";
	 
	 i = pgcontent.find(str1);
	 
	 pgcontent = pgcontent.substr(i+str1.length(),pgcontent.length()-i-str1.length());
     
	 j = pgcontent.find(str2);
	 
	 patent_priority = pgcontent.substr(0,j); //����Ȩ ����������
	 
	 cout<<"������Ȩ��"<<patent_priority<<endl;
	 /****************************************************************************************/
	 
	 str1 = "�����루ר��Ȩ���ˡ�&nbsp;";
	 
	 i = pgcontent.find(str1);
	 
	 pgcontent = pgcontent.substr(i+str1.length(),pgcontent.length()-i-str1.length());
     
	 j = pgcontent.find(str2);
	 
	 patent_app_person = pgcontent.substr(0,j); //���루ר��Ȩ���� ����������
	 
	 cout<<"�����루ר��Ȩ���ˡ�"<<patent_app_person<<endl;
	 /****************************************************************************************/
	 
	 str1 = "����ַ��&nbsp;";
	 
	 i = pgcontent.find(str1);
	 
	 pgcontent = pgcontent.substr(i+str1.length(),pgcontent.length()-i-str1.length());
     
	 j = pgcontent.find(str2);
	 
	 patent_address = pgcontent.substr(0,j);  //��ַ ����������
	 
	 cout<<"����ַ��"<<patent_address<<endl;
	 /****************************************************************************************/
	 
	 str1 = "���������롿&nbsp;";
	 
	 i = pgcontent.find(str1);
	 
	 pgcontent = pgcontent.substr(i+str1.length(),pgcontent.length()-i-str1.length());
     
	 j = pgcontent.find(str2);
	 
	 patent_international_apply = pgcontent.substr(0,j);   //�������� ����������
	 
	 cout<<"���������롿"<<patent_international_apply<<endl;
	 /****************************************************************************************/
	 
	 str1 = "�����ʹ�����&nbsp;";
	 
	 i = pgcontent.find(str1);
	 
	 pgcontent = pgcontent.substr(i+str1.length(),pgcontent.length()-i-str1.length());
     
	 j = pgcontent.find(str2);
	 
	 patent_international_public = pgcontent.substr(0,j);   //���ʹ��� ����������
	 
	 cout<<"�����ʹ�����"<<patent_international_public<<endl;
	 /****************************************************************************************/
	 
	 str1 = "������������ڡ�&nbsp;";
	 
	 i = pgcontent.find(str1);
	 
	 pgcontent = pgcontent.substr(i+str1.length(),pgcontent.length()-i-str1.length());
     
	 j = pgcontent.find(str2);
	 
	 patent_into_coutry = pgcontent.substr(0,j);  //����������� ����������
	 
	 cout<<"������������ڡ�"<<patent_into_coutry<<endl;
	 /****************************************************************************************/
	 
	 str1 = "��ר�����������&nbsp;";
	 
	 i = pgcontent.find(str1);
	 
	 pgcontent = pgcontent.substr(i+str1.length(),pgcontent.length()-i-str1.length());
     
	 j = pgcontent.find(str2);
	 
	 patent_agent = pgcontent.substr(0,j); //ר��������� ����������
	 
	 cout<<"��ר�����������"<<patent_agent<<endl;
	 /****************************************************************************************/
	 
	 str1 = "�������ˡ�&nbsp;";
	 
	 i = pgcontent.find(str1);
	 
	 pgcontent = pgcontent.substr(i+str1.length(),pgcontent.length()-i-str1.length());
     
	 j = pgcontent.find(str2);
	 
	 patent_attorney = pgcontent.substr(0,j); //������ ����������
	 
	 cout<<"�������ˡ�"<<patent_attorney<<endl;
	 /****************************************************************************************/
	 
	 str1 = "��ժҪ��&nbsp;";
	 
	 i = pgcontent.find(str1);
	 
	 pgcontent = pgcontent.substr(i+str1.length(),pgcontent.length()-i-str1.length());
     
	 j = pgcontent.find(str2);
	 
	 patent_app_abstract = pgcontent.substr(0,j); //ժҪ ����������
	 
	 cout<<"��ժҪ��"<<patent_app_abstract<<endl;

	 /****************************************************************************************/
	 
	 str1 = "jpgsubmit('";

	 str2 = "'";
	 
	 i = pgcontent.find(str1);
	 
	 pgcontent = pgcontent.substr(i+str1.length(),pgcontent.length()-i-str1.length());
     
	 j = pgcontent.find(str2);
	 
	 patent_specification_url = pgcontent.substr(0,j)+".jpg"; //ͼƬ��ַ ����������
	 
	 cout<<"��ͼƬ��ַ��"<<patent_specification_url<<endl;

	 /****************************************************************************************/
	 
	 str1 = ">��";
	 
	 str2 = "��";
	 
	 i = pgcontent.find(str1);
	 
	 pgcontent = pgcontent.substr(i+str1.length(),pgcontent.length()-i-str1.length());
     
	 j = pgcontent.find(str2);
	 
	 patent_specification_total_page = pgcontent.substr(0,j); //ͼƬҳ�� ����������
	 
	 cout<<"��ͼƬҳ����"<<patent_specification_total_page<<endl;

	 int total_pges = atoi(patent_specification_total_page.c_str());

	 cout<<"����ҳ����"<<total_pges<<endl;


	 /************************************************************************/
	 /* ���������ݿ�洢                                                     */
	 /************************************************************************/

	 string save_as_dir ;
	 int isave = save_as_path.find_last_of("\\");
	 save_as_dir = save_as_path.substr(isave+1,save_as_path.length()-isave-1); // ��ȡ�洢Ŀ¼

	 SYSTEMTIME st;
	 CString str_system_time;
	 GetLocalTime(&st);
	 str_system_time.Format(L"%4d-%2d-%2d %2d:%2d:%2d",st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);


	 CString sqlsentence;
	 sqlsentence = L"insert into sipo_data( \
					application_number,\
					application_date,\
					title,\
					publication_number,\
					publication_date,\
					main_classification,\
					divisional_application_original_number,\
					classification,\
					grant_date,\
					priority,\
					applicant,\
					address,\
					inventor,\
					international_application,\
					international_publication,\
					enter_into_national_phase_date,\
					agent,\
					attorney,\
					abstracts,\
					update_time,\
					pagemax ,\
					web_url, \
					save_dir) \
					values('"
					+application_number+L"','"
					+(CString)patent_apply_date.c_str()+L"','"
					+(CString)patent_app_name.c_str()+L"','"
					+(CString)patent_public_num.c_str()+L"','"
					+(CString)patent_public_date.c_str()+L"','"
					+(CString)patent_major_classify_num.c_str()+L"','"
					+(CString)patent_fen_orgin_num.c_str()+L"','"
					+(CString)patent_classify_num.c_str()+L"','"
					+(CString)patent_certification_date.c_str()+L"','"
					+(CString)patent_priority.c_str()+L"','"
					+(CString)patent_app_person.c_str()+L"','"
					+(CString)patent_address.c_str()+L"','"
					+(CString)patent_invent_person.c_str()+L"','"
					+(CString)patent_international_apply.c_str()+L"','"
					+(CString)patent_international_public.c_str()+L"','"
					+(CString)patent_into_coutry.c_str()+L"','"
					+(CString)patent_agent.c_str()+L"','"
					+(CString)patent_attorney.c_str()+L"','"
					+(CString)patent_app_abstract.c_str()+L"','"
					+str_system_time +L"','"
					+(CString)patent_specification_total_page.c_str()+L"','"
					+(CString)patent_specification_url.c_str()+L"','"
					+(CString)save_as_dir.c_str()+L"')";

	 if (!m_runsql.RunSQL(sqlsentence))
	 {
		 MessageBox(L"error insert",L"INSERT",0);
	 }


	 int download_list_count = m_DownloadListView.GetItemCount();
	 m_DownloadListView.SetItemText(download_list_count,0,L"��������");
	 m_DownloadListView.SetItemText(download_list_count,1,application_number);//ר����
	 m_DownloadListView.SetItemText(download_list_count,2,(CString)patent_app_name.c_str());// ר������
	 m_DownloadListView.SetItemText(download_list_count,3,L"������0ҳ"); // ���ؽ���
	 m_DownloadListView.SetItemText(download_list_count,4,(CString) patent_specification_url.c_str());// ������Դ
	 m_DownloadListView.SetItemText(download_list_count,5,(CString) patent_specification_total_page.c_str());// ����ҳ��
	 m_DownloadListView.SetItemText(download_list_count,6,(CString) patent_apply_date.c_str()); // ������
	 m_DownloadListView.SetItemText(download_list_count,7,(CString) patent_invent_person.c_str()); // ������
	 m_DownloadListView.SetItemText(download_list_count,8,(CString) patent_app_person.c_str()); // ������

	 char CN_temp_app_num[20];
	 memset(CN_temp_app_num,0,20);
	 USES_CONVERSION;
	 strcpy(CN_temp_app_num,W2A(application_number.LockBuffer()));
	 application_number.UnlockBuffer();

	 Patent_Specification_Download((string)CN_temp_app_num,patent_specification_url,patent_specification_total_page,save_as_path,download_list_count);
	 /****************************************************************************************/
}