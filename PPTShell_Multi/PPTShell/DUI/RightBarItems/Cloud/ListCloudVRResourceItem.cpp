#include "stdafx.h"
#include "ListCloudVRResourceItem.h"
#include "NDCloud/NDCloudFile.h"
#include "NDCloud/NDCloudAPI.h"
#include "DUI/GroupExplorer.h"

CListCloudVRResourceItem::CListCloudVRResourceItem()
{

}

CListCloudVRResourceItem::~CListCloudVRResourceItem()
{

}

void CListCloudVRResourceItem::DoInit()
{
	__super::DoInit();

	m_nType = CloudFileVRResource;
	SetTotalCountUrl(NDCloudComposeUrlVRResourceCount());
	GetTotalCountInterface();
}

bool CListCloudVRResourceItem::OnChapterChanged( void* pObj )
{
	TEventNotify* pEventNotify = (TEventNotify*)pObj;
	CStream* pStream = (CStream*)pEventNotify->wParam;
	pStream->ResetCursor();
	tstring strGuid = pStream->ReadString();

	SetCurCountUrl(NDCloudComposeUrlVRResourceInfo(_T(""), _T(""), _T(""), 0, 1));
	SetJsonUrl(NDCloudComposeUrlVRResourceInfo(_T(""), _T(""), _T(""), 0, 500));

	if(m_pStream)
	{
		delete m_pStream;
		m_pStream = NULL; 
	}

	if(IsSelected())
	{
		if(m_dwDownloadId != -1)
		{
			NDCloudDownloadCancel(m_dwDownloadId);
		}

		CGroupExplorerUI * pGroupExplorer = CGroupExplorerUI::GetInstance();
		pGroupExplorer->ShowWindow(true);

		pGroupExplorer->StartMask();
		m_dwDownloadId = NDCloudDownload(GetJsonUrl(), MakeHttpDelegate(this, &CListCloudItem::OnDownloadDecodeList));
	}
// 	else
// 	{
// 		if(m_pStream == NULL)
// 			m_dwDownloadId = NDCloudDownload(GetJsonUrl(), MakeHttpDelegate(this, &ClistCloudItem::OnDownloadDecodeList));
// 	}

	if(m_dwCurCountDownId != -1)
	{
		NDCloudDownloadCancel(m_dwCurCountDownId);
	}

	__super::OnChapterChanged(pObj);

	return true;
}

bool CListCloudVRResourceItem::OnDownloadDecodeList( void* pObj )
{
	CGroupExplorerUI * pGroupExplorer = CGroupExplorerUI::GetInstance();
	THttpNotify* pHttpNotify = (THttpNotify*)pObj;

	if (pHttpNotify->dwErrorCode > 0)
	{
		pGroupExplorer->StopMask();
		pGroupExplorer->ShowNetlessUI(true);
		return true;
	}
	else
	{
		pGroupExplorer->ShowNetlessUI(false);
	}

	if(m_pStream)
		delete m_pStream;
	m_pStream = new CStream(1024);

	if (!NDCloudDecodeVRResourceList(pHttpNotify->pData, pHttpNotify->nDataSize, m_pStream))
	{
		pGroupExplorer->ShowNetlessUI(true);
		pGroupExplorer->StopMask();
		return false;
	}

	if(IsSelected())
	{
		pGroupExplorer->ShowResource(m_nType, m_pStream);
		pGroupExplorer->StopMask();
	}

	return true;
}
