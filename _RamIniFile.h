#pragma once
#include <Windows.h>
#include <vector>
#include <atlstr.h>
#include <assert.h>
using std::vector;


class _CRamIniFile
{
	class  CMyLock
	{
	public:
		CMyLock()
		{
			EnterCriticalSection(&_CRamIniFile::m_cs);
		}
		~CMyLock()
		{
			::LeaveCriticalSection(&_CRamIniFile::m_cs);
		}
	};

	static const int MAX_CHAR_SIZE = 2048;

	typedef struct _KEY_VALUE
	{
		BOOL bComment;
		CString strKey;
		CString strVal;
		
	}KEY_VALUE, *PKEY_VALUE;

	typedef struct _SECTION_INFO
	{
		CString strSection;
		vector<KEY_VALUE> vKeyVal;
		void Clear()
		{
			strSection.Empty();
			vKeyVal.clear();
		}
	}SECTION_INFO, *PSECTION_INFO;

public:
	_CRamIniFile(void);
	~_CRamIniFile(void);

public:
	BOOL Init(LPCTSTR _lpcFilePath, LPCTSTR _lpDatPath = NULL);
	CString ReadProfileString(LPCTSTR _lpAppName, LPCTSTR _lpKeyName);
	void WriteProfileString(LPCTSTR _lpAppName, LPCTSTR _lpKeyName, LPCTSTR _lpVal);
	void GetAllSections(vector<CString>& _vstrSection);
	void GetAllKeys(LPCTSTR _lpAppName, vector<CString>& _vstrKey);
	BOOL Save(LPCTSTR _lpFilePath = NULL);

private:
	BOOL Parse(const char* _pBuff, DWORD _dwSize);
	CString GetSection(LPCTSTR _lpcText);
	CString TrimEqualsSign(LPCTSTR _lpcText);
	void GetKeyValue(LPCTSTR _lpcText, CString& _strKey, CString& _strVal);
	BOOL SaveFile(LPCTSTR _lpFilePath, const char* _pBuff, DWORD _dwSize, BOOL _bEncrypt = FALSE);
	void Adjust(vector<KEY_VALUE>& _vKeyVal);

private:
	CString m_strPath;
	CString m_strDatPath;
	CString m_strHeaderComment;
	vector<SECTION_INFO> m_vSection;
	static CRITICAL_SECTION m_cs;
	static LONG volatile m_csRefCount;
};

