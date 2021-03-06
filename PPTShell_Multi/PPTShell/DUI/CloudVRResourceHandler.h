#pragma once


class CCloudResourceHandler;
class CCloudVRResourceHandler:
	public CCloudResourceHandler
{

public:
	CCloudVRResourceHandler();
	virtual ~CCloudVRResourceHandler();

protected:
	//override
	virtual void	DoButtonClick(TNotifyUI* pNotify);
	virtual void	DoSetThumbnail( TNotifyUI* pNotify );
	virtual void	DoDropDown( TNotifyUI* pNotify );

protected:
	virtual void	ReadFrom( CStream* pStream );
	virtual void	WriteTo( CStream* pStream );

	bool			InsertVR(LPCTSTR lpVr);
	bool			OnGetPlaceHolderCompleted(void* pObj);
	bool			OnHandleThumbnail(void* pObj);
	bool			OnHandleInsert(void* pObj);
	bool			OnHandlePreivew(void* pObj);
	bool			OnHandleSession(void* pObj);
	bool			OnHandlePermission(void* pObj);

public:
	void			SetHasPreviewUrl(bool bHas);


private:
	tstring			m_strResource;
	tstring			m_strPreviewUrl;
	bool			m_bHasPreviewUrl;
	tstring			m_strThumbnailPath;
	tstring			m_strDesc;
	tstring			m_strApkPackageName;
	tstring			m_strApkActivityName;
	tstring			m_strApkVersion;
	CStream*		m_pIntroduceStream;
	DWORD			m_dwVRSize;

private:
	tstring m_strHandlerId;
public:
	virtual void	 InitHandlerId(){
		m_strHandlerId= typeid(this).name();
		m_strHandlerId += GetGuid();
	}
	virtual void	 SetHandlerId(LPCTSTR lptcsId){
		m_strHandlerId = lptcsId;
	}
	virtual tstring	 GetHandlerId(){
		return m_strHandlerId;
	}
};
