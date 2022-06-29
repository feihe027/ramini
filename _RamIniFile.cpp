#include "_RamIniFile.h"
#include "FileOp\SGFile\SGFile.h"
#include "FileOp\FileOp.h"
#include <memory>
#include <Winbase.h>


CRITICAL_SECTION _CRamIniFile::m_cs;
LONG volatile _CRamIniFile::m_csRefCount = 0;
_CRamIniFile::_CRamIniFile(void)
{	
	if (0 == m_csRefCount)
	{
		::InitializeCriticalSection(&m_cs);
	}

	InterlockedIncrement(&m_csRefCount);
}

_CRamIniFile::~_CRamIniFile(void)
{	
	Save();
	InterlockedDecrement(&m_csRefCount);
	if (0 == m_csRefCount)
	{
		::DeleteCriticalSection(&m_cs);
	}
}

BOOL _CRamIniFile::Init(LPCTSTR _lpcFilePath, LPCTSTR _lpDatPath/* = NULL*/)
{
	assert(NULL != _lpcFilePath);
	
	m_strPath = _lpcFilePath;
	m_strDatPath = _lpDatPath;

	CString strPath;
	BOOL bDecrypt = FALSE;
	if (PublicW32::FileExists(m_strPath.GetString()))
	{
		strPath = m_strPath;
	}
	else if (PublicW32::FileExists(m_strDatPath.GetString()))
	{
		bDecrypt = TRUE;
		strPath = m_strDatPath;
	}
	else
	{
		return FALSE;
	}

	char* pBuff = NULL;
	std::unique_ptr<char[]> smartPtr;
	DWORD dwSize = 0;
	CSGFile file;
	try
	{
		CMyLock lock;
		if (!file.Open(strPath.GetString(), CSGFile::modeRead))
		{
			return FALSE;
		}
		dwSize = static_cast<DWORD>(file.GetLength());
		pBuff = new char[dwSize+1](); // +1是为了增加终止符
		smartPtr.reset(pBuff);
		file.Read(pBuff, dwSize);
		file.Close();
	}
	catch (...)
	{
		return FALSE;
	}

	std::string strContent = pBuff;
	if (bDecrypt)
	{
		PublicW32::Decrypt(pBuff, dwSize, strContent);
	}

	if (!Parse(strContent.c_str(), strContent.length()))
	{
		return FALSE;
	}

	return TRUE;
}

BOOL _CRamIniFile::Parse(const char* _pBuff, DWORD _dwSize)
{
	m_vSection.clear();
	DWORD dwStart = 0;
	vector<CString> vstrLine;
	BOOL bHeaderComment = TRUE;
	for (DWORD dwIdx = 0; dwIdx < _dwSize; dwIdx++)
	{
		if (0xA == _pBuff[dwIdx])
		{
			char szLine[MAX_CHAR_SIZE] = {0};
			if (dwIdx <= dwStart+sizeof(szLine))
			{
				memcpy_s(szLine, sizeof(szLine), _pBuff+dwStart, dwIdx-dwStart);
			}

			CString strText = szLine;
			strText.Trim();

			if ((0xD == _pBuff[dwIdx+1]))
			{
				dwIdx++;
			}

			dwStart = dwIdx + 1;

			if (bHeaderComment && (0 == strText.Find('[')))
			{
				bHeaderComment = FALSE;
			}

			if (bHeaderComment)
			{
				m_strHeaderComment += strText;
				m_strHeaderComment += _T("\r\n");
			}
			else
			{
				vstrLine.push_back(strText);
			}
		}
		else if (dwIdx+1 == _dwSize)
		{
			char szLine[MAX_CHAR_SIZE] = {0};
			if (dwIdx <= dwStart+sizeof(szLine))
			{
				memcpy_s(szLine, sizeof(szLine), _pBuff+dwStart, _dwSize-dwStart);
			}

			CString strText = szLine;
			strText.Trim();
			vstrLine.push_back(strText);
		}
	}

	SECTION_INFO sectionInfo = {};
	for (auto it = vstrLine.begin(); it != vstrLine.end(); ++it)
	{
		CString strText = GetSection(it->GetString());
		if (!strText.IsEmpty())
		{
			if (sectionInfo.strSection.GetLength() > 0)
			{
				m_vSection.push_back(sectionInfo);
				sectionInfo.Clear();
			}
			sectionInfo.strSection = strText;
		}
		else
		{
			KEY_VALUE keyVal = {};
			if ((it->IsEmpty()) || (';' == it->GetAt(0)) || ('#' == it->GetAt(0)))
			{
				keyVal.bComment = TRUE;
				keyVal.strKey = *it;
			}
			else
			{
				CString strLine = TrimEqualsSign(*it);
				GetKeyValue(strLine, keyVal.strKey, keyVal.strVal);
			}
			sectionInfo.vKeyVal.push_back(keyVal);
		}
	}

	if (sectionInfo.strSection.GetLength() > 0)
	{
		m_vSection.push_back(sectionInfo);
	}

	return TRUE;
}

CString _CRamIniFile::GetSection(LPCTSTR _lpcText)
{
	CString strSection;
	if ('[' == _lpcText[0])
	{
		CString strText = _lpcText;
		if (strText.Find(']') > 0)
		{
			strSection = strText.Mid(1, strText.GetLength()-2);
		}
	}

	return strSection;
}

// 去掉等号两边的空白字符
CString _CRamIniFile::TrimEqualsSign(LPCTSTR _lpcText)
{
	CString strText = _lpcText;
	int nPos = strText.Find('=');

	// trim left
	for (int nIdx = nPos-1; nIdx > 0; nIdx--)
	{
		if (' ' == strText.GetAt(nIdx))
		{
			strText.Delete(nIdx, 1);
		}
		else
		{
			break;
		}
	}

	// trim right
	nPos = strText.Find('=');
	for (int nIdx = nPos+1; nIdx < strText.GetLength();)
	{
		if (' ' == strText.GetAt(nIdx))
		{
			strText.Delete(nIdx, 1);
		}
		else
		{
			break;
		}
	}

	return strText;
}

void _CRamIniFile::GetKeyValue(LPCTSTR _lpcText, CString& _strKey, CString& _strVal)
{
	CString strText = _lpcText;
	int nPos = strText.Find('=');

	_strKey = strText.Left(nPos);
	_strVal = strText.Right(strText.GetLength() - nPos - 1);
}

CString _CRamIniFile::ReadProfileString(LPCTSTR _lpAppName, LPCTSTR _lpKeyName)
{
	CString strAppName = _lpAppName;
	CString strKeyName = _lpKeyName;
	for (auto it = m_vSection.begin(); it != m_vSection.end(); ++it)
	{
		if (0 == strAppName.CompareNoCase(it->strSection))
		{
			for (auto subIt = it->vKeyVal.begin(); subIt != it->vKeyVal.end(); ++subIt)
			{
				if (0 == strKeyName.CompareNoCase(subIt->strKey))
				{
					return subIt->strVal;
				}
			}
		}
	}

	return CString();
}

void _CRamIniFile::WriteProfileString(LPCTSTR _lpAppName, LPCTSTR _lpKeyName, LPCTSTR _lpVal)
{
	if (NULL == _lpAppName)
	{
		return;
	}

	CString strAppName = _lpAppName;
	CString strKeyName = _lpKeyName;
	CString strVal = _lpVal;
	BOOL bSectionExist = FALSE;
	BOOL bKeyExist = FALSE;
	for (auto it = m_vSection.begin(); it != m_vSection.end(); ++it)
	{
		if (0 == strAppName.CompareNoCase(it->strSection))
		{
			if (NULL == _lpKeyName)
			{
				m_vSection.erase(it);
				return;
			}

			bSectionExist = TRUE;
			for (auto subIt = it->vKeyVal.begin(); subIt != it->vKeyVal.end(); ++subIt)
			{
				if (0 == strKeyName.CompareNoCase(subIt->strKey))
				{
					if (NULL == _lpVal)
					{
						it->vKeyVal.erase(subIt);
						return;
					}

					bKeyExist = TRUE;
					subIt->strVal = strVal;
					break;
				}
			}

			if (!bKeyExist)
			{
				KEY_VALUE keyVal = {};
				keyVal.strKey = strKeyName;
				keyVal.strVal = strVal;
				it->vKeyVal.push_back(keyVal);
				Adjust(it->vKeyVal);
			}
			break;
		}
	}

	if (!bSectionExist)
	{
		SECTION_INFO sectionInfo = {};
		sectionInfo.strSection = strAppName;
		KEY_VALUE keyVal = {0};
		keyVal.strKey = strKeyName;
		keyVal.strVal = strVal;
		sectionInfo.vKeyVal.push_back(keyVal);
		m_vSection.push_back(sectionInfo);
	}
}

// 将空行排到最后
void _CRamIniFile::Adjust(vector<KEY_VALUE>& _vKeyVal)
{
	vector<KEY_VALUE> vKeyVal;
	vector<KEY_VALUE> vNull;
	for (auto it = _vKeyVal.begin(); it != _vKeyVal.end(); ++it)
	{
		if (it->strKey.IsEmpty())
		{
			vNull.push_back(*it);
		}
		else
		{
			vKeyVal.push_back(*it);
		}
	}

	_vKeyVal.clear();
	_vKeyVal = vKeyVal;
	_vKeyVal.insert(_vKeyVal.end(), vNull.begin(), vNull.end());
}

void _CRamIniFile::GetAllSections(vector<CString>& _vstrSection)
{
	for (auto it = m_vSection.begin(); it != m_vSection.end(); ++it)
	{
		_vstrSection.push_back(it->strSection);
	}
}

void  _CRamIniFile::GetAllKeys(LPCTSTR _lpAppName, vector<CString>& _vstrKey)
{
	if (NULL == _lpAppName)
	{
		return;
	}

	CString strAppName = _lpAppName;
	for (auto it = m_vSection.begin(); it != m_vSection.end(); ++it)
	{
		if (0 == strAppName.CompareNoCase(it->strSection))
		{
			for (auto subIt = it->vKeyVal.begin(); subIt != it->vKeyVal.end(); ++subIt)
			{
				if (!subIt->bComment)
				{
					_vstrKey.push_back(subIt->strKey);
				}
			}
		}
	}
}

BOOL _CRamIniFile::Save(LPCTSTR _lpFilePath/* = NULL*/)
{
	CString strContent = m_strHeaderComment;
	for (auto it = m_vSection.begin(); it != m_vSection.end(); ++it)
	{
		strContent += _T("[");
		strContent += it->strSection;
		strContent += _T("]");
		strContent += _T("\r\n");

		for (auto subIt = it->vKeyVal.begin(); subIt != it->vKeyVal.end(); ++subIt)
		{
			if (subIt->bComment)
			{
				strContent += subIt->strKey;
			}
			else
			{
				strContent += subIt->strKey;
				strContent += _T("=");
				strContent += subIt->strVal;
			}
			strContent += _T("\r\n");
		}
	}

#ifdef  UNICODE  
	USES_CONVERSION;
	char* pBuff = W2A(strContent.GetString());
#else
	const char* pBuff = strContent.GetString();
#endif

	// 如果有指定路径,则只保存指定路径
	if (NULL != _lpFilePath)
	{
		CString strPath = _lpFilePath;
		BOOL bEncrpt = (0 == strPath.Right(3).CompareNoCase(_T(".ini")));
		if (!SaveFile(_lpFilePath, pBuff, strlen(pBuff), bEncrpt))
		{
			return FALSE;
		}

		return TRUE;
	}

	if (PublicW32::FileExists(m_strPath.GetString()))
	{
		if (!SaveFile(m_strPath, pBuff, strlen(pBuff)))
		{
			return FALSE;
		}

		if (!SaveFile(m_strDatPath, pBuff, strlen(pBuff), TRUE))
		{
			return FALSE;
		}
	}
	else if (PublicW32::FileExists(m_strDatPath.GetString()))
	{
		if (!SaveFile(m_strDatPath, pBuff, strlen(pBuff), TRUE))
		{
			return FALSE;
		}
	}

	return TRUE;
}

BOOL _CRamIniFile::SaveFile(LPCTSTR _lpFilePath, const char* _pBuff, DWORD _dwSize, BOOL _bEncrypt/* = FALSE*/)
{
	U32 uDstSize = _dwSize;
	S8* pDesBuff = (S8*)_pBuff;
	std::unique_ptr<S8[]> smartPtr;
	if (_bEncrypt)
	{
		uDstSize = (_dwSize + 12) * 1001 / 1000 + 1024;
		pDesBuff = new S8[uDstSize]();
		smartPtr.reset(pDesBuff);
		PublicW32::Encrypt((char*)_pBuff, _dwSize, pDesBuff, &uDstSize);
	}

	CSGFile file;
	try
	{
		CMyLock lock;
		if (!file.Open(_lpFilePath, CSGFile::modeCreate|CSGFile::modeReadWrite))
		{
			return FALSE;
		}
		file.Write(pDesBuff, uDstSize);
		file.Close();
	}
	catch (...)
	{
		return FALSE;
	}

	return TRUE;
}