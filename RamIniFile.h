#pragma once
#include <atlstr.h>
#include <vector>
using std::vector;


class _CRamIniFile;
namespace PublicW32
{
	class CRamIniFile
	{
	public:
		CRamIniFile(void);
		~CRamIniFile(void);

	public:
		BOOL Init(LPCTSTR _lpFilePath, LPCTSTR _lpDatPath = NULL);
		CAtlString ReadProfileString(LPCTSTR _lpAppName, LPCTSTR _lpKeyName, LPCTSTR _lpDefault = NULL);
		void WriteProfileString(LPCTSTR _lpAppName, LPCTSTR _lpKeyName, LPCTSTR _lpVal);
		int ReadProfileInt(LPCTSTR _lpAppName, LPCTSTR _lpKeyName, int _nDefault = 0);
		void WriteProfileInt(LPCTSTR _lpAppName, LPCTSTR _lpKeyName, int _nVal);
		BOOL ReadProfileBool(LPCTSTR _lpAppName, LPCTSTR _lpKeyName, BOOL _bDefault = FALSE);
		void WriteProfileBool(LPCTSTR _lpAppName, LPCTSTR _lpKeyName, BOOL _bVal);
		LONGLONG ReadProfileHex(LPCTSTR _lpAppName, LPCTSTR _lpKeyName, LONGLONG _llDefault = 0);
		LONGLONG ReadProfileInt64(LPCTSTR _lpAppName, LPCTSTR _lpKeyName, LONGLONG _llDefault = 0);
		void WriteProfileInt64(LPCTSTR _lpAppName, LPCTSTR _lpKeyName, LONGLONG _llVal);
		double ReadProfileDouble(LPCTSTR _lpAppName, LPCTSTR _lpKeyName, double _dfDefault = 0.0f);
		void WriteProfileDouble(LPCTSTR _lpAppName, LPCTSTR _lpKeyName, double _dfVal);
		void GetAllSections(vector<CAtlString>& _vstrSection);
		void GetAllKeys(LPCTSTR _lpAppName, vector<CAtlString>& _vstrKey);
		BOOL Save();
		BOOL SaveAs(LPCTSTR _lpFilePath);

	private:
		_CRamIniFile* m_pIniFile;
	};

}
