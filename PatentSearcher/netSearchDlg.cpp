// netsearchdlg.cpp : implementation of the CNetSearchDlg class
//
/////////////////////////////////////////////////////////////////////////////



#include "stdafx.h"


//#include <afxinet.h>
#include "resource.h"

#include "netsearchdlg.h"
#include <string>
#include <fstream>

using namespace std;

#include <stdio.h>


#include <wininet.h>
#define MAXBLOCKSIZE 1024*16
#pragma comment( lib, "wininet.lib" )

LRESULT CNetSearchDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{

	CenterWindow(GetParent());


	return TRUE;
}

LRESULT CNetSearchDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{

	EndDialog(wID);
	return 0;
}

LRESULT CNetSearchDlg::OnBnClickedBtnSearch(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{

	 global_Search_str = GetSearchStr();
 	
	 if ( BST_CHECKED == IsDlgButtonChecked(IDC_CHECK_INVENT_PATENT))
	 {
		 global_CheckBox_str.Append(L"&selectbase=11");
	 }
	 else if ( BST_CHECKED == IsDlgButtonChecked(IDC_CHECK_APP_PATENT))
	 {
		 global_CheckBox_str.Append(L"&selectbase=22");
	 }
	 else if ( BST_CHECKED == IsDlgButtonChecked(IDC_CHECK_DESIGN_PATENT))
	 {
		 global_CheckBox_str.Append(L"&selectbase=33");
	 }
	 else global_CheckBox_str=L"";
	 
	 

	 if (L"" != global_Search_str)
	 {
		/* SendMessage(parent_hwnd,WM_SEARCH_RESULT,0,0);*/
	     EndDialog(IDOK);
	 }
	 

	
	return 0;
}

CString CNetSearchDlg::GetSearchStr()
{
	if ( !DoDataExchange(true) )
		exit(0);

	CString num;     //���루ר������
	CString zldate;  //���������棩��
	CString op_num;  // ���������棩��
	CString man;     //���루ר��Ȩ����
	CString cate_num; //�����
	CString zhaiyao;   //ժҪ
	CString address;  //��ַ
	CString name;     //����
	CString design_man; //���������
	CString zcate_num; //�������
	CString sqdate;      //������
	CString bzdate;      //��֤��
	CString dl_orga;     //ר���������
	CString priority;    //����Ȩ
	CString ysq_num;     //�ְ�ԭ�����
	CString dl_man;      //������
	CString gjgb;     //���ʹ���
	CString zqx;  //��Ȩ��
	CString multi_str;   //�û�д�ļ������ʽ
	CString temp_str;  
	CString str;         //�������ʽ

	int str_leng;
	int  leng;
	int  mulleng;

	CString  counstr;


	num = m_app_num;             //���루ר������
	name = m_app_name;            //��  ��
	zhaiyao = m_app_abstract;        //ժҪ
	sqdate = m_apply_date;          //������
	zldate = m_public_date;         //���������棩��
	op_num = m_public_num;          //���������棩��
	cate_num = m_classify_num;        //�����
	zcate_num = m_major_classify_num;  //�������
	man = m_app_person;          //����(ר��Ȩ)��
	design_man = m_invent_person;        //��������ƣ���
	address = m_address;              //��ַ
	gjgb = m_international_public; //���ʹ���
	bzdate = m_certification_date;   //��֤��
	dl_orga = m_agent;                //ר���������
	dl_man = m_attorney;             //������
	priority = m_priority;             //����Ȩ


	leng=num.GetLength();
	mulleng=multi_str.GetLength();

	if (cate_num!="") { 
		cate_num.Replace(L"/",L"\\/");
		//cate_num=cate_num.replace("-","?");
		cate_num.Replace(L"-",L"\\-");        
		str=str + "�����=('%" + cate_num + "%') and ";         
	}

	if (num!="")
	{
		CString tmpstr;
		for (int  i=0;i < num.GetLength();i++)
		{
			tmpstr="";
			if ((Substring(num,i,i+1) >= '0') && (Substring(num,i,i+1) <= '9'))
			{	
				tmpstr=Substring(num,i,num.GetLength()-i+2);
				break;
			}	
		}

		if (tmpstr.GetLength()>10)
		{
			tmpstr=Substring(tmpstr,0,12);
		}
		else if (tmpstr.GetLength()>0)
		{
			tmpstr=Substring(tmpstr,0,8);
		}
		else
		{
			MessageBox(L"������������");
			exit(0);
		}
		//alert(tmpstr);
		tmpstr.Replace(L" ",L"");
		tmpstr.Replace(L"��",L"");
		str=str + "�����=('CN%" + tmpstr + "%" + "') and ";

		/*
		num=num.replace("zl","CN");
		num=num.replace("ZL","CN");
		num=num.replace("�ڣ�","CN");
		num=num.replace(" ","");
		num=num.replace("��","");
		counstr=num.substring(0,2);

		if (counstr.toUpperCase()=="CN")
		{	str=str + "�����=('" + num + "%" + "') and ";}
		else
		{	str=str + "�����=('CN%" + num + "%" + "') and ";}
		*/
	} 		

	if (zldate!="")
	{
		if (Datechk(zldate)==1)
		{
			MessageBox(L"��������ڸ�ʽ��");
			//document.Advanceform1.textfield4.focus();
			exit(0);
		}

		zldate.Replace(L"-",L".");
		zldate.Replace(L"/",L".");
		zldate.Replace(L"����",L".");
		zldate.Replace(L"\\",L".");
		str=str + "���������棩��=(" + zldate + ") and ";
	}

	if (op_num!="") 
	{	
		counstr=Substring(op_num,0,2);
		CString Ctr = counstr;
		Ctr.MakeUpper();
		if (Ctr=="CN")
		{	str=str + "���������棩��=('" + op_num + "%') and ";
		}
		else
		{
			str=str + "���������棩��=('CN%" + op_num + "%') and ";
		}
	}

	if (man!="") 
	{
		/*	while (man.indexOf("%")!=-1)
		{
		man=man.replace("%","");
		}
		*/
		while (man.FindOneOf(L"��")!=-1)
		{
			man.Replace(L"��",L"%");
		}

		str=str + "���루ר��Ȩ����=(" + man + ") and ";
	}

	if (design_man!="")
	{
		/*	while (design_man.indexOf("%")!=-1)
		{
		design_man=design_man.replace("%","");
		}
		*/
		while (design_man.FindOneOf(L"��")!=-1)
		{
			design_man.Replace(L"��",L"%");
		}
		str=str + "��������ƣ���=(" + design_man + ") and "; 
	}

	if (address!="")
	{
		//while (address.indexOf("%")!=-1)
		//{
		//	address=address.replace("%","");
		//}

		while (address.FindOneOf(L"��")!=-1)
		{
			address.Replace(L"��",L"%");
		}
		str=str + "��ַ=('" + address + "') and ";
	}

	if (name!="") 
	{ 
		//while (name.indexOf("%")!=-1)
		//{
		//	name=name.replace("%","");
		//}

		while (name.FindOneOf(L"��")!=-1)
		{
			name.Replace(L"��",L"%");
		}

		str=str + "����=(" + name + ") and "; 
	}

	if (zhaiyao!="") 
	{
		//while (zhaiyao.indexOf("%")!=-1)
		//{
		//	zhaiyao=zhaiyao.replace("%","");
		//}

		while (zhaiyao.FindOneOf(L"��")!=-1)
		{
			zhaiyao.Replace(L"��",L"%");
		}

		str=str + "ժҪ=(" + zhaiyao + ") and ";
	}

	if (zcate_num!="") 
	{
		zcate_num.Replace(L"/",L"\\/");
		//zcate_num=zcate_num.replace("-","?");
		zcate_num.Replace(L"-",L"\\-");
		str=str + "�������=('" + zcate_num + "%') and ";
	}       

	if (sqdate!="")
	{
		if (Datechk(sqdate)==1)
		{
			MessageBox(L"��������ڸ�ʽ��");
			//document.Advanceform1.textfield12.focus();
			exit(0);
		}
		sqdate.Replace(L"-",L".");
		sqdate.Replace(L"/",L".");
		sqdate.Replace(L"\\",L".");
		sqdate.Replace(L"����",L".");
		str=str + "������=(" + sqdate + ") and ";
	}

	if (bzdate!="")
	{
		if (Datechk(bzdate)==1)
		{
			MessageBox(L"��������ڸ�ʽ��");
			//document.Advanceform1.textfield12.focus();
			exit(0);
		}         
		bzdate.Replace(L"-",L".");
		bzdate.Replace(L"/",L".");
		bzdate.Replace(L"\\",L".");
		bzdate.Replace(L"����",L".");
		str=str + "��֤��=(" + bzdate + ") and ";
	}

	if (dl_orga!="") 
	{
		/*	while (dl_orga.indexOf("%")!=-1)
		{
		dl_orga=dl_orga.replace("%","");
		}
		*/
		while (dl_orga.FindOneOf(L"��")!=-1)
		{
			dl_orga.Replace(L"��",L"%");
		}

		str=str + "ר���������=('" + dl_orga + "') and ";
	}

	if (dl_man!="") {
		/*	while (dl_man.indexOf("%")!=-1)
		{
		dl_man=dl_man.replace("%","");
		}
		*/
		while (dl_man.FindOneOf(L"��")!=-1)
		{
			dl_man.Replace(L"��",L"%");
		}

		str=str + "������=('" + dl_man + "') and ";
	}
	if (gjgb!="") {
		//gjgb=gjgb.replace(".","\\*");
		//gjgb=gjgb.replace(".","\\.");

		str=str + "���ʹ���=('%" + gjgb + "%') and ";
		str.Replace(L"/",L"\\/");
		str.Replace(L"-",L"\\-");
		str.Replace(L",",L"\\,");
		str.Replace(L".",L"\\*");
		str.Replace(L".",L"\\.");
		str.Replace(L"\\*",L"\\.");
	}

	if (priority!="") { 
		/*	while (priority.indexOf("%")!=-1)
		{
		priority=priority.replace("%","");
		}
		*/
		while (priority.FindOneOf(L"��")!=-1)
		{
			priority.Replace(L"��",L"%");
		}

		str=str + "����Ȩ=('" + priority + "') and ";

		str.Replace(L"/",L"\\/");
		str.Replace(L"-",L"\\-");
		str.Replace(L",",L"\\,");
		str.Replace(L".",L"\\*");
		str.Replace(L".",L"\\.");
		str.Replace(L"\\*",L"\\.");

	}

	//if(flzt!="") { str=str + "����״̬=%" + flzt+ "% and ";} 

	if (zqx!="") 
	{
		str=str + "��Ȩ��=(" + zqx + ") and ";
	}
	if (ysq_num!="")
	{ 
		str=str + "�ְ�ԭ�����=('%" + ysq_num + "%') and ";
	}

	str_leng=str.GetLength();
	if (str_leng==0)
	{   
		str="";
	}
	else
	{    
		str=Substring(str,0,str_leng-4);
	}

	if(str=="")
	{
		MessageBox(L"������������Ϊ��");
		//exit(0);
	}

	

	return str;
}

int CNetSearchDlg::Datechk(CString str)
{  
	//ȫ��Ϊ��ĸ
	CString strSource ="abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	LPCTSTR   ch ;
	int  i;
	int  temp;

	for (i=0;i<=(str.GetLength()-1);i++)
	{

		ch = new wchar_t( str.GetAt(i)) ;
		temp = strSource.FindOneOf((LPCTSTR) ch);
		if (temp==-1) 
		{
			return 0;
		}
	}
	if (strSource.FindOneOf((LPCTSTR)ch)==-1)
	{
		return 0;
	}
	else
	{
		return 1;
	} 
}

CString CNetSearchDlg::Substring(CString oldstr,int i,int j)
{
	CString sbstring;
	if (i>=oldstr.GetLength() ||/* j>=oldstr.GetLength() ||*/ i>=j)
	{
		return "";
	}


	//oldstr.Delete(j,oldstr.GetLength()-(j-i));

	sbstring = oldstr.Mid(i,j-i);

	return sbstring;
}


