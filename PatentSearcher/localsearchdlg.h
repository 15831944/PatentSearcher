// localsearchdlg.h : interface of the CLocalSearchDlg class
//
/////////////////////////////////////////////////////////////////////////////
/**
* @file
* Copyright (C) suxin.com
* @brief ����ר�������Ի���
* @author  sunlaibing88@gmail.com
* @date  2011-11-8
*/

#pragma once

class CLocalSearchDlg : public CDialogImpl<CLocalSearchDlg>,
		public CWinDataExchange<CLocalSearchDlg>
{
public:
	enum { IDD = IDD_DLG_LOCALSEARCH };

	BEGIN_MSG_MAP(CLocalSearchDlg)
		COMMAND_HANDLER(ID_BTN_SEARCH, BN_CLICKED, OnBnClickedBtnSearch)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	BEGIN_DDX_MAP(CLocalSearchDlg)
		DDX_TEXT(IDC_EDIT_APP_NUM, m_app_num)
		DDX_TEXT(IDC_EDIT_APP_NAME, m_app_name)
		DDX_TEXT(IDC_EDIT_APP_ABSTRACT, m_app_abstract)
		DDX_TEXT(IDC_EDIT_APPLY_DATE, m_apply_date)
		DDX_TEXT(IDC_EDIT_PUBLIC_DATE, m_public_date)
		DDX_TEXT(IDC_EDIT_PUBLIC_NUM, m_public_num)
		DDX_TEXT(IDC_EDIT_CLASSIFY_NUM, m_classify_num)
		DDX_TEXT(IDC_EDIT_MAJOR_CLASSIFY_NUM, m_major_classify_num)
		DDX_TEXT(IDC_EDIT_APP_PERSON, m_app_person)
		DDX_TEXT(IDC_EDIT_INVENT_PERSON, m_invent_person)
		DDX_TEXT(IDC_EDIT_ADDRESS, m_address)
		DDX_TEXT(IDC_EDIT_INTERNATIONAL_PUBLIC, m_international_public)
		DDX_TEXT(IDC_EDIT_CERTIFICATION_DATE, m_certification_date)
		DDX_TEXT(IDC_EDIT_AGENT, m_agent)
		DDX_TEXT(IDC_EDIT_ATTORNEY, m_attorney)
		DDX_TEXT(IDC_EDIT_PRIORITY, m_priority)
	END_DDX_MAP();
	
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedBtnSearch(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:
	CString m_app_num;             //���루ר������
	CString m_app_name;            //��  ��
	CString m_app_abstract;        //ժҪ
	CString m_apply_date;          //������
	CString m_public_date;         //���������棩��
	CString m_public_num;          //���������棩��
	CString m_classify_num;        //�����
	CString m_major_classify_num;  //�������
	CString m_app_person;          //����(ר��Ȩ)��
	CString m_invent_person;        //��������ƣ���
	CString m_address;              //��ַ
	CString m_international_public; //���ʹ���
	CString m_certification_date;   //��֤��
	CString m_agent;                //ר���������
	CString m_attorney;             //������
	CString m_priority;             //����Ȩ
};
