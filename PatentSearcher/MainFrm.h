// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
using namespace std;

#include "MySplitterWindowT.h"
//#include "ClipSpyListCtrl.h"
#include "MyPaneContainer.h"
#include "TreeCtrl.h"
#include "controls.h"
#include "myfunction.h"




const int INFOLISTVIEW = 1;
const int DOWNLOADLISTVIEW = 2;
const int SEARCHLISTVIEW = 4;

class CMainFrame : public CFrameWindowImpl<CMainFrame>, public CUpdateUI<CMainFrame>,
		public CMessageFilter, public CIdleHandler
{
public:
	DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();

	BEGIN_UPDATE_UI_MAP(CMainFrame)
		UPDATE_ELEMENT(ID_VIEW_TOOLBAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_STATUS_BAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(1, UPDUI_STATUSBAR)  // clock status
		UPDATE_ELEMENT(2, UPDUI_STATUSBAR)  // CAPS indicator
		UPDATE_ELEMENT(3, UPDUI_STATUSBAR)  // clock status
		UPDATE_ELEMENT(4, UPDUI_STATUSBAR)  // CAPS indicator
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainFrame)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		NOTIFY_CODE_HANDLER(TVN_SELCHANGED, OnSelChanged)
		NOTIFY_CODE_HANDLER(NM_RCLICK, OnNMRClickList)
		COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)
		COMMAND_ID_HANDLER(ID_FILE_NEW, OnFileNew)
		COMMAND_ID_HANDLER(ID_VIEW_TOOLBAR, OnViewToolBar)
		COMMAND_ID_HANDLER(ID_VIEW_STATUS_BAR, OnViewStatusBar)
		COMMAND_ID_HANDLER(ID_VIEW_TREE, OnViewTree)
		COMMAND_ID_HANDLER(ID_VIEW_DOWNLOAD, OnViewDownLoad)
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		COMMAND_ID_HANDLER(ID_NET_SEARCH, OnNetSearch)
		COMMAND_ID_HANDLER(ID_LOCAL_SEARCH, OnLocalSearch)

		/************************************************************************/
		/* bellow added by aiwen                                               */
		/************************************************************************/

        COMMAND_ID_HANDLER(ID_SEARCH_LOADSELECT, OnSearchLoadselect)                //�����ļ�

		COMMAND_ID_HANDLER(ID_DOWNLOAD_START, OnStartDownloadSelect)      // ��ʼ�����ļ�
		COMMAND_ID_HANDLER(ID_DOWNLOAD_STOP, OnStopDownload)              // ֹͣ�����ļ�

		COMMAND_ID_HANDLER(ID_INFO_OPEN, OnOpenPdf)                                 //��pdf�ļ�

        COMMAND_ID_HANDLER(ID_SEARCH_PREV, OnSearchPrev)							//��һҳ
        COMMAND_ID_HANDLER(ID_SEARCH_NEXT, OnSearchNext)							//��һҳ

		CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
		CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
		REFLECT_NOTIFICATIONS()				/*handle TVN_SELCHANGED etc.*/
//		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()
/************************************************************************/
/* �ڸ������ж�����REFLECT_NOTIFICATIONS(),
��������ӿؼ�����Ϣӳ�䵽������ DEFAULT_REFLECTION_HANDLER();��������           
*/
/************************************************************************/

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewTree(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewDownLoad(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnNetSearch(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnLocalSearch(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnOpenPdf(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnSelChanged(int idCtrl, LPNMHDR pNMHDR, BOOL& bHandled);
	LRESULT OnNMRClickList(int idCtrl, LPNMHDR pNMHDR, BOOL& bHandled);



	/************************************************************************/
	/* bellow added by aiwen                                                */
	/************************************************************************/
	LRESULT OnSearchLoadselect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);


	LRESULT OnSearchPrev(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnSearchNext(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

    LRESULT OnStartDownloadSelect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnStopDownload(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	void InitLeftTreeView(CTreeViewCtrlEx *m_pTreeView);
	void InitListView(CListViewCtrl *m_pListView,int nType);
//	bool GetListValue();

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/


protected:
	//�ȴ�ֱ�ָ��ˮƽ�ָ�
	CSplitterWindow m_wndVertSplitter;
	CHorSplitterWindow m_wndHorzSplitter;
	CSplitterWindow m_TopVertSplitter;
	CSplitterWindow m_BottomVertSplitter;
	CHorSplitterWindow m_TopHorzSplitter;

//	CMySplitterWindowT<true> m_wndVertSplitter;
//	CMySplitterWindowT<false> m_wndHorzSplitter;

	CMyPaneContainer m_wndTreeContainer;
	CMyPaneContainer m_wndIEContainer;
	

	CTreeCtrl m_Tree;

	CImageList m_LeftTreeImageList;
	CImageList m_StatusListImageList;
	
	CTreeViewCtrlEx m_MenuTree;			//������β˵�
	CAxWindow m_MainWebBrowser;					//��ʼ���ܽ���
	CAxWindow m_InfoWebBrowser;					//ר����ϸ��Ϣ����

	CListViewCtrl m_InfoListView;
	CListViewCtrl m_DownloadListView;
	CListViewCtrl m_SearchListView;

	CMultiPaneStatusBarCtrl m_wndStatusBar;

	LRunSql m_runsql;					//�������ݿ�ӿ�
	CString sql;						//���ݿ�ִ�����

	CString m_localSearchStr;

	CString currentPath ;

	string m_page_content;              // ҳ������

	int search_total_page;              // ��ѯ�������ҳ��

    int current_search_page;            // ��ǰ��ѯ�����ҳ��

    int current_display_groupid;            // ��ǰ��ʾ��groupid

	int last_dispay_page;                 // ��ǰ��ʾҳ��

	bool stop_download_flag ; // 1 Ϊ����ֹͣ��0Ϊ������

	bool download(CString url,const char *save_as);//����ҳ��

	string  UrlGB2312(char * str);     // ��ַ����ת��

	bool GetPatents(string pageCont, CString url);  // ��ȡר���б�

	int GetPageNum(string pageCont) ;  // ��ȡ�б�ҳ��

	string ReadHtmlPage(string filename);  // ��ȡ���ص�a.htmlҳ��

	bool DoMerge(CString szPath, CString pdfPath);			//Tiffת��Ϊpdf

	static UINT StartNetSearchThread(LPVOID); //ThreadProc1()
	static UINT StartDownloadThread(LPVOID);  //ThreadProc2()

    static UINT StartDownloadUnfinishedThread(LPVOID); //ThreadProc5()

	static UINT SearchNextThread(LPVOID);  //ThreadProc3()
	static UINT SearchPrevThread(LPVOID);  //ThreadProc4()

	bool ThreadProc1();  // ��ȡtmp.html
	bool ThreadProc2();  // ��ʼ����TIF�ļ�
	bool ThreadProc3();  // next page
	bool ThreadProc4();  // prev page
	bool ThreadProc5();  // ����δ��ɵ�

public:
 
	void Parse_Invent_Design_PatentDetails(string pgcontent ,string save_as_path,CString application_number,int item_index);//����ר��ϸ��

	void Parse_Appearance_PatentDetails(string pgcontent,string save_as_path,CString application_number,int item_index);//����ר��ϸ��

	// ����ר��˵��
	void Patent_Specification_Download(string patent_num,string specification_url, string pgtotal,string save_as_path,int item_index);


	CString BCD_to_AnsiString(char *P_BCD,int length);

	string  Get_application_number(string url);

	bool Is_Stop_Current_Download(CString cstr_patent_number); // �ж��Ƿ�Ҫֹͣ��ǰ������
};
