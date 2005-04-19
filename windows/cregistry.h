// Registry Access class (updated) by David Overton (david@insomniavisions.com)

// Proper errors are available in this version. The functions call
// SetLastError if it is a standard windows error. The user defined
// errors shown below are exceptions. Catch them as shown here:

// try {
//     pRegistry->SetDWORDValue(...); // whatever
// }
// catch(unsigned int error) {
//     // error is one of the error codes defined below
// }

#if !defined(__CRegistry_H_)
#define __CRegistry_H_

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlwapi.h>
#include <tchar.h>

const unsigned int ERROR_WRONG_TYPE = 0x01; // Wrong type of data
const unsigned int ERROR_BUFFER_SIZE = 0x02; // Buffer too small
const unsigned int ERROR_NO_SHLWAPI_DLL = 0x03; // Only under NT if shlwapi.dll doesn't exist
const unsigned int ERROR_INVALID_BUFFER = 0x04; // Occurs if buffer invalid (ie no buffer)
const unsigned int ERROR_NO_SHDELETEKEY = 0x05; // Occurs in NT if no SHDeleteKey fn in shlwapi.dll

typedef DWORD (__stdcall* SHDELKEYPROC)(HKEY, LPCTSTR);

class CRegistry
{
    bool IsWinNTor2K();
public:
    bool CreateKey(HKEY hKeyRoot, LPCTSTR pszSubKey);
    bool DeleteKey(HKEY hKeyRoot, LPCTSTR pszSubKey);
    bool DeleteValue(HKEY hKeyRoot, LPCTSTR pszSubKey, LPCTSTR pszValue);
    bool GetBinaryValue(HKEY hKeyRoot, LPCTSTR pszSubKey, LPCTSTR pszValue, PVOID pData, DWORD& rdwSize);
    bool GetDWORDValue(HKEY hKeyRoot, LPCTSTR pszSubKey, LPCTSTR pszValue, DWORD& rdwBuff);
    bool GetStringValue(HKEY hKeyRoot, LPCTSTR pszSubKey, LPCTSTR pszValue, LPTSTR pszBuffer, DWORD& rdwSize);
    bool SetBinaryValue(HKEY hKeyRoot, LPCTSTR pszSubKey, LPCTSTR pszValue, PVOID pData, DWORD dwSize);
    bool SetDWORDValue(HKEY hKeyRoot, LPCTSTR pszSubKey, LPCTSTR pszValue, DWORD dwValue);
    bool SetStringValue(HKEY hKeyRoot, LPCTSTR pszSubKey, LPCTSTR pszValue, LPCTSTR pszString);
};

#endif
