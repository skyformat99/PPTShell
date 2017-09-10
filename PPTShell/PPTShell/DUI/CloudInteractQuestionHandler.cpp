#include "StdAfx.h"
#include "DUI/IItemHandler.h"
#include "DUI/INotifyHandler.h"
#include "NDCloud/NDCloudAPI.h"
#include "Http/HttpDelegate.h"
#include "Http/HttpDownload.h"
#include "DUI/IDownloadListener.h"
#include "Util/Stream.h"
#include "DUI/IStreamReader.h"
#include "DUI/IVisitor.h"
#include "DUI/ItemHandler.h"
#include "DUI/CloudResourceHandler.h"
#include "DUI/IThumbnailListener.h"

#include "DUI/CloudInteractQuestionHandler.h"

#include "DUI/BaseParamer.h"
#include "DUI/InsertParamer.h"


#include "GUI/MainFrm.h"
#include "PPTControl/PPTController.h"
#include "PPTControl/PPTControllerManager.h"
#include "Util/Util.h"
#include <Windef.h>
#include "DUI/PreviewDialogUI.h"
#include "Statistics/Statistics.h"

#include "DUI/ITransfer.h"
#include "Http/HttpDelegate.h"
#include "Http/HttpDownload.h"
#include "DUI/IDownloadListener.h"
#include "DUI/ResourceDownloader.h"
#include "DUI/AssetDownloader.h"
#include "DUI/QuestionDownloader.h"
#include "DUI/InteractQuestionDownloader.h"

#include "DUI/IButtonTag.h"
#include "DUI/ItemExplorer.h"
#include "DUI/CoursePlayUI.h"
#include "NDCloud/NDCloudQuestion.h"

#include "DUI/ItemMenu.h"
#include "QuestionPreviewDialogUI.h"
#include "NDCloud/NDCloudContentService.h"
#include "GroupExplorer.h"

ImplementHandlerId(CCloudInteractQuestionHandler);
CCloudInteractQuestionHandler::CCloudInteractQuestionHandler()
{
	isDeleting = false;
}


CCloudInteractQuestionHandler::~CCloudInteractQuestionHandler()
{

}

void CCloudInteractQuestionHandler::ReadFrom( CStream* pStream )
{
	m_strTitle			= pStream->ReadString();
	m_strGuid			= pStream->ReadString();
	m_strDesc			= pStream->ReadString();
	m_strQuestionName	= pStream->ReadString();
	m_strPreviewUrl		= pStream->ReadString();
	m_strXmlUrl			= pStream->ReadString();
	m_strJsonInfo		= pStream->ReadString();
	InitHandlerId();
}

void CCloudInteractQuestionHandler::WriteTo( CStream* pStream )
{
	pStream->WriteString(m_strTitle);
	pStream->WriteString(m_strGuid);
	pStream->WriteString(m_strDesc);
	pStream->WriteString(m_strQuestionName);
	pStream->WriteString(m_strPreviewUrl);
	pStream->WriteString(m_strXmlUrl);
	pStream->WriteString(m_strJsonInfo);

}

void CCloudInteractQuestionHandler::DoRClick( TNotifyUI* pNotify )
{
	CItemHandler::DoRClick(pNotify);

	if (IsDbank()&&!isDeleting)
	{
		CItemMenuUI* pMenu = new CItemMenuUI();
		pMenu->SetHolder(pNotify->pSender);
		pMenu->CreateMenu();
		pMenu->AddMenuItem(_T("ɾ��"), eMenu_Delete);
		pMenu->SetLeftWeight(0.4f);
		pMenu->ShowMenu();

	}
}

void CCloudInteractQuestionHandler::DoButtonClick(TNotifyUI*	pNotify)
{
	__super::DoButtonClick(pNotify);
	
	if (pNotify->wParam == eClickFor_Insert)
	{
		//insert
		DoDropDown(pNotify);
	}
	else if (pNotify->wParam == eClickFor_Preview)
	{
		if (GetDownloader())
		{
			//already had downloader
			return;
		}

		CInvokeParamer* pParamer	= new CInvokeParamer();
		pParamer->SetTrigger(GetTrigger());
		pParamer->SetCompletedDelegate(MakeDelegate(this, &CCloudInteractQuestionHandler::OnHandlePreivew));

		//don't delete
		CQuestionDownloader* pQuestionDownloader = new CInteractQuestionDownloader();
		pQuestionDownloader->SetQuestionGuid(GetGuid());
		pQuestionDownloader->SetQuestionTitle(GetTitle());
		pQuestionDownloader->SetParamer(pParamer);
		pQuestionDownloader->AddListener(&GetDownloadListeners());
		pQuestionDownloader->AddListener(this);//self must be added  at rearmost

		SetDownloader(pQuestionDownloader);
		pQuestionDownloader->Transfer();
	}
}

void CCloudInteractQuestionHandler::DoSetThumbnail( TNotifyUI* pNotify )
{
	if (m_strPreviewUrl.empty())
	{
		return;
	}
	//delegate
	IThumbnailListener* pListener = dynamic_cast<IThumbnailListener*>(pNotify->pSender);
	if (pListener)
	{
		pListener->OnGetThumbnailBefore();
	}

	CInvokeParamer* pParamer = new CInvokeParamer();
	pParamer->SetCompletedDelegate(MakeDelegate(this, &CCloudInteractQuestionHandler::OnHandleThumbnail));

	//don't delete
	CAssetDownloader* pAssetDownloader = new CAssetDownloader();
	SetThumbnailDownloader(pAssetDownloader);

	pAssetDownloader->SetThumbnailSize(240);
	pAssetDownloader->SetAssetGuid(GetGuid());
	pAssetDownloader->SetAssetTitle(GetTitle());
	pAssetDownloader->SetAssetType(CloudFileImage);
	pAssetDownloader->SetAssetUrl(m_strPreviewUrl.c_str());
	pAssetDownloader->SetParamer(pParamer);
	pAssetDownloader->AddListener(this);//self must be added  at rearmost
	pAssetDownloader->Transfer();
	
}

bool CCloudInteractQuestionHandler::OnHandleThumbnail( void* pObj )
{
	CInvokeParamer* pParamer = (CInvokeParamer*)pObj;
	//notify
	NotifyThumbnailCompleted(pParamer->GetHttpNotify());

	//clean user data
	if (pParamer->GetHttpNotify())
	{
		pParamer->GetHttpNotify()->pUserData = NULL;
	}
	delete pParamer;
	
	return true;
}

void CCloudInteractQuestionHandler::DoDropDown( TNotifyUI* pNotify )
{
	if (!pNotify)
	{
		return;
	}

	GetPlaceHolderId(pNotify->sType.GetData(),
		pNotify->ptMouse.x,
		pNotify->ptMouse.y,
		(int)LOWORD(pNotify->lParam),
		(int)HIWORD(pNotify->lParam),
		0);
}

bool CCloudInteractQuestionHandler::OnGetPlaceHolderCompleted( void* pObj )
{
	__super::OnGetPlaceHolderCompleted(pObj);
	TEventNotify* pNotify	= (TEventNotify*)pObj;
	CStream* pStream		= (CStream*)pNotify->wParam;

	pStream->ResetCursor();

	BOOL	bResult			= pStream->ReadBOOL();
	DWORD	dwSlideId		= pStream->ReadDWORD();
	DWORD	PlaceHolderId	= pStream->ReadDWORD();
	int		nX				= pStream->ReadInt();
	int		nY				= pStream->ReadInt();
	if (!bResult)
	{
		//get placeholder id fail
		return true;
	}

	if (GetDownloader())
	{
		RemovePalceHolderByThread(dwSlideId, PlaceHolderId);
		//already had downloader
		return true;
	}

	CMainFrame*		pMainFrame	= (CMainFrame*)AfxGetMainWnd();
	CInsertParamer* pParamer	= new CInsertParamer();
	pParamer->SetOperationerId(pMainFrame->GetOperationerId());
	pParamer->SetSlideId(dwSlideId);
	pParamer->SetPlaceHolderId(PlaceHolderId);
	pParamer->SetInsertPos(nX, nY);
	pParamer->SetCompletedDelegate(MakeDelegate(this, &CCloudInteractQuestionHandler::OnHandleInsert));
	

	//don't delete
	CQuestionDownloader* pQuestionDownloader = new CInteractQuestionDownloader();
	pQuestionDownloader->SetQuestionGuid(GetGuid());
	pQuestionDownloader->SetQuestionTitle(GetTitle());
	pQuestionDownloader->SetParamer(pParamer);
	pQuestionDownloader->AddListener(&GetDownloadListeners());
	pQuestionDownloader->AddListener(this);//self must be added  at rearmost

	SetDownloader(pQuestionDownloader);
	pQuestionDownloader->Transfer();


	return true;
}

bool CCloudInteractQuestionHandler::OnHandleInsert( void* pObj )
{
	CInsertParamer* pParamer = (CInsertParamer*)pObj;
	//insert

	do 
	{
		if (!pParamer->GetHttpNotify()
			|| pParamer->GetHttpNotify()->dwErrorCode != 0)
		{
			RemovePalceHolderByThread(pParamer->GetSlideId(), pParamer->GetPlaceHolderId());
			CToast::Toast(_STR_FILE_DWONLOAD_FAILED);
			break;
		}

		CMainFrame*		pMainFrame	= (CMainFrame*)AfxGetMainWnd();
		if (pMainFrame->IsOperationerChanged(pParamer->GetOperationerId()))
		{
			break;
		}

		tstring strMainXmlPath = pParamer->GetHttpNotify()->strFilePath;
		strMainXmlPath += _T("\\main.xml");


		TiXmlDocument doc;
		bool res = doc.LoadFile(Str2Utf8(strMainXmlPath).c_str());
		if( !res )
			break;

		TiXmlElement* pRootElement = doc.FirstChildElement();
		TiXmlElement* pPagesElement = GetElementsByTagName(pRootElement, "pages");
		if( pPagesElement == NULL )
			break;

		TiXmlElement* pPageElement = pPagesElement->FirstChildElement();
		while( pPageElement != NULL )
		{
			pPageElement->SetAttribute("id", Str2Utf8(GetGuid()).c_str());
			pPageElement->SetAttribute("name", Str2Utf8(GetGuid()).c_str());
			pPageElement->SetAttribute("reportable", "true");
			pPageElement = pPageElement->NextSiblingElement();
		}

		doc.SaveFile(Str2Utf8(strMainXmlPath).c_str()); 

		InsertQuestionByThread(strMainXmlPath.c_str(),
			GetTitle(),
			GetGuid(),
			pParamer->GetSlideId(),
			pParamer->GetPlaceHolderId());


		Statistics::GetInstance()->Report(STAT_INSERT_QUESTION);

	} while (false);

	//clean user data
	if (pParamer->GetHttpNotify())
	{
		pParamer->GetHttpNotify()->pUserData = NULL;
	}
	delete pParamer;


	return true;
}


bool CCloudInteractQuestionHandler::OnHandlePreivew( void* pObj )
{
	CInvokeParamer* pParamer = (CInvokeParamer*)pObj;
	//insert
	if (pParamer->GetHttpNotify()&&pParamer->GetHttpNotify()->dwErrorCode == 0)
	{
		if (GetTrigger()&&GetTrigger() == pParamer->GetTrigger()&&GetTrigger()->IsVisible())
		{
			tstring strQuestionPath = pParamer->GetHttpNotify()->strFilePath;
			strQuestionPath += _T("\\main.xml");
			// translate slash
			for(int i = 0; i < (int)strQuestionPath.length(); i++)
			{
				if( strQuestionPath.at(i) == _T('\\') )
					strQuestionPath.replace(i, 1, _T("/"));
			}
			// js player path
			tstring strLocalPath = GetLocalPath();
			tstring strPlayerPath = GetHtmlPlayerPath();
			TCHAR szParam[MAX_PATH*2]={0};
			_stprintf_s(szParam, _T("%s?main=/%s&sys=pptshell&hidePage=footer"), 
				UrlEncode(Str2Utf8(strPlayerPath)).c_str(), 
				UrlEncode(Str2Utf8(strQuestionPath)).c_str());

			Statistics::GetInstance()->Report(STAT_PREVIEW_QUESTION);

			/*CCoursePlayUI * pCoursePlayUI = CoursePlayUI::GetInstance();
			pCoursePlayUI->SetQuestionInfo(strQuestionPath, m_strTitle, m_strGuid);
			pCoursePlayUI->Init((WCHAR *)AnsiToUnicode(szParam).c_str(),COURSEPLAY_PREVIEW);*/
			//��ȡ����item��Ϣ
			CStream stream(1024);
			tstring strUrl = szParam;
			stream.WriteString(strUrl);
			CContainerUI* pParaent	= (CContainerUI*)GetTrigger()->GetParent();
			int nCount				= pParaent->GetCount();
			int nIndex				= pParaent->GetItemIndex(GetTrigger());

			stream.WriteDWORD(nIndex);
			stream.WriteDWORD(nCount);

			int nRealCount	= 0;
			int nRealIndex	= 0;
			for (int i = 0; i < nCount; ++i)
			{
				IHandlerVisitor* pHandlerVisitor = dynamic_cast<IHandlerVisitor*>(pParaent->GetItemAt(i));
				if (pHandlerVisitor)
				{
					CCloudInteractQuestionHandler* pHandler = dynamic_cast<CCloudInteractQuestionHandler*>(pHandlerVisitor->GetHandler());
					if (pHandler)
					{
						if (nIndex == i)
						{
							nRealIndex = nRealCount;
						}
						nRealCount++;
						pHandler->WriteTo(&stream);
					}
				}
			}	
			//update index and count
			stream.ResetCursor();		
			stream.ReadString();
			stream.WriteDWORD(nRealIndex);
			stream.WriteDWORD(nRealCount);
			stream.ResetCursor();
			//
			CQuestionPreviewDialogUI* pPreviewUI = QuestionPreviewDialogUI::GetInstance();
			pPreviewUI->SetQuestionType(QUESTION_INTERACTIVE_CLOUD);
			pPreviewUI->InitPosition();
			pPreviewUI->InitData(&stream);
			pPreviewUI->ShowWindow();
		}
	}
	else
	{
		CToast::Toast(_STR_FILE_DWONLOAD_FAILED);
	}
	//clean user data
	if (pParamer->GetHttpNotify())
	{
		pParamer->GetHttpNotify()->pUserData = NULL;
	}
	delete pParamer;
	return true;
}

tstring CCloudInteractQuestionHandler::GetQuestionName()
{
	return m_strQuestionName;
}

bool CCloudInteractQuestionHandler::OnDeleteComplete( void* pObj )
{
	THttpNotify*		pHttpNotify	= (THttpNotify*)pObj;
	CNDCloudContentService* pService = (CNDCloudContentService*)pHttpNotify->pUserData;
	if (pService->IsSuccess())
	{
		CGroupExplorerUI::GetInstance()->AddDBankItemCount(DBankCoursewareObjects);
		if(CGroupExplorerUI::GetInstance()->GetCurrentType()==DBankCoursewareObjects)
		{
			int count = CGroupExplorerUI::GetInstance()->GetTypeCount();
			CGroupExplorerUI::GetInstance()->SetTypeCount(--count);
			if(count<=0)
			{
				CGroupExplorerUI::GetInstance()->ShowReslessUI(true);
			}
		}
	}
	return __super::OnDeleteComplete(pObj);
}