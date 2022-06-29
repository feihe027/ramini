#include "RamIniFile.h"
#include "FileOp\SGFile\SGFile.h"
#include "_RamIniFile.h"


using namespace PublicW32;
CRamIniFile::CRamIniFile(void)
	: m_pIniFile(NULL)
{
	m_pIniFile = new _CRamIniFile();
}

CRamIniFile::~CRamIniFile(void)
{
	delete m_pIniFile;
	m_pIniFile = NULL;
}

BOOL CRamIniFile::Init(LPCTSTR _lpFilePath, LPCTSTR _lpDatPath/* = NULL*/)
{
	if (!m_pIniFile->Init(_lpFilePath, _lpDatPath))
	{
		return FALSE;
	}

	return TRUE;
}

CString CRamIniFile::ReadProfileString(LPCTSTR _lpAppName, LPCTSTR _lpKeyName, LPCTSTR _lpDefault/* = NULL*/)
{
	CString strVal = m_pIniFile->ReadProfileString(_lpAppName, _lpKeyName);
	if (strVal.IsEmpty() && (NULL != _lpDefault)) 
	{
		return _lpDefault;
	}

	return strVal;
}

void CRamIniFile::WriteProfileString(LPCTSTR _lpAppName, LPCTSTR _lpKeyName, LPCTSTR _lpVal)
{
	m_pIniFile->WriteProfileString(_lpAppName, _lpKeyName, _lpVal);
}

int CRamIniFile::ReadProfileInt(LPCTSTR _lpAppName, LPCTSTR _lpKeyName, int _nDefault/* = 0*/)
{
	CString strVal = m_pIniFile->ReadProfileString(_lpAppName, _lpKeyName);
	if (strVal.IsEmpty())
	{
		return _nDefault;
	}

#ifdef  UNICODE  
	USES_CONVERSION;
	char* pBuff = W2A(strVal.GetString());
#else
	const char* pBuff = strVal.GetString();
#endif
	int nVal = 0;
	sscanf_s(pBuff, "%d", &nVal);

	return nVal;
}

void CRamIniFile::WriteProfileInt(LPCTSTR _lpAppName, LPCTSTR _lpKeyName, int _nVal)
{
	CString strVal;
	strVal.Format(_T("%d"), _nVal);
	m_pIniFile->WriteProfileString(_lpAppName, _lpKeyName, strVal);
}

BOOL CRamIniFile::ReadProfileBool(LPCTSTR _lpAppName, LPCTSTR _lpKeyName, BOOL _bDefault/* = FALSE*/)
{
	CString strVal = m_pIniFile->ReadProfileString(_lpAppName, _lpKeyName);
	if (strVal.IsEmpty())
	{
		return _bDefault;
	}

	if (0 == strVal.CompareNoCase(_T("true")))
	{
		return TRUE;
	}

	return FALSE;
}

void CRamIniFile::WriteProfileBool(LPCTSTR _lpAppName, LPCTSTR _lpKeyName, BOOL _bVal)
{
	CString strVal = _bVal ? _T("TRUE") : _T("FALSE");
	m_pIniFile->WriteProfileString(_lpAppName, _lpKeyName, strVal);
}

LONGLONG CRamIniFile::ReadProfileHex(LPCTSTR _lpAppName, LPCTSTR _lpKeyName, LONGLONG _llDefault/* = 0*/)
{
	CString strVal = m_pIniFile->ReadProfileString(_lpAppName, _lpKeyName);
	if (strVal.IsEmpty())
	{
		return _llDefault;
	}

#ifdef  UNICODE  
	USES_CONVERSION;
	char* pBuff = W2A(strVal.GetString());
#else
	const char* pBuff = strVal.GetString();
#endif
	LONGLONG llVal = 0;
	sscanf_s(pBuff, "%llx", &llVal);

	return llVal;
}

LONGLONG CRamIniFile::ReadProfileInt64(LPCTSTR _lpAppName, LPCTSTR _lpKeyName, LONGLONG _llDefault/* = 0*/)
{
	CString strVal = m_pIniFile->ReadProfileString(_lpAppName, _lpKeyName);
	if (strVal.IsEmpty())
	{
		return _llDefault;
	}

#ifdef  UNICODE  
	USES_CONVERSION;
	char* pBuff = W2A(strVal.GetString());
#else
	const char* pBuff = strVal.GetString();
#endif

	LONGLONG llVal = 0;
	sscanf_s(pBuff, "%lld", &llVal);

	return llVal;
}

void CRamIniFile::WriteProfileInt64(LPCTSTR _lpAppName, LPCTSTR _lpKeyName, LONGLONG _llVal)
{
	CString strVal;
	strVal.Format(_T("%lld"), _llVal);
	m_pIniFile->WriteProfileString(_lpAppName, _lpKeyName, strVal);
}

double CRamIniFile::ReadProfileDouble(LPCTSTR _lpAppName, LPCTSTR _lpKeyName, double _dfDefault/* = 0.0f*/)
{
	CString strVal = m_pIniFile->ReadProfileString(_lpAppName, _lpKeyName);
	if (strVal.IsEmpty())
	{
		return _dfDefault;
	}

#ifdef  UNICODE  
	USES_CONVERSION;
	char* pBuff = W2A(strVal.GetString());
#else
	const char* pBuff = strVal.GetString();
#endif

	double dfVal = 0;
	sscanf_s(pBuff, "%lf", &dfVal);

	return dfVal;
}

void CRamIniFile::WriteProfileDouble(LPCTSTR _lpAppName, LPCTSTR _lpKeyName, double _dfVal)
{
	CString strVal;
	strVal.Format(_T("%lf"), _dfVal);
	m_pIniFile->WriteProfileString(_lpAppName, _lpKeyName, strVal);
}

void CRamIniFile::GetAllSections(vector<CString>& _vstrSection)
{
	return m_pIniFile->GetAllSections(_vstrSection);
}

void CRamIniFile::GetAllKeys(LPCTSTR _lpAppName, vector<CString>& _vstrKey)
{
	return m_pIniFile->GetAllKeys(_lpAppName, _vstrKey);
}

BOOL CRamIniFile::Save()
{
	return m_pIniFile->Save();
}

BOOL CRamIniFile::SaveAs(LPCTSTR _lpFilePath)
{
	return m_pIniFile->Save(_lpFilePath);
}