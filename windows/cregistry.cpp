// Registry Access class (updated) by David Overton (david@insomniavisions.com).
// These will compile either ANSI or UNICODE.

#include "cregistry.h"

// Returns true if the OS is Windows NT or Windows 2000
bool CRegistry::IsWinNTor2K()
{
    OSVERSIONINFO OsVer;
    GetVersionEx(&OsVer);
    return (OsVer.dwPlatformId == VER_PLATFORM_WIN32_NT) != 0;
}

// Creates a key specified by pszSubKey - you can't create
// keys directly under HKEY_LOCAL_MACHINE in Windows NT or 2000
// just for an extra bit of info.
bool CRegistry::CreateKey(HKEY hKeyRoot, LPCTSTR pszSubKey)
{
    HKEY hKey;
    DWORD dwFunc;
    LONG lRet;

    lRet = RegCreateKeyEx(
               hKeyRoot,
               pszSubKey,
               0,
               (LPTSTR)NULL,
               REG_OPTION_NON_VOLATILE,
               KEY_WRITE,
               (LPSECURITY_ATTRIBUTES)NULL,
               &hKey,
               &dwFunc
           );

    if (lRet == ERROR_SUCCESS)
    {

        RegCloseKey(hKey);
        hKey = (HKEY)NULL;

        return true;
    }

    SetLastError((DWORD)lRet);
    return false;
}

bool CRegistry::DeleteKey(HKEY hKeyRoot, LPCTSTR pszSubKey)
{
    DWORD dwRet = ERROR_SUCCESS;

    if (IsWinNTor2K())
    {
        // WinNT/2K will not allow you to delete keys which have
        // subkeys/values inside them. MS's platform SDK tells you
        // to use the SHDeleteKey function in shlwapi.dll. This dll
        // is not available on NT platforms without IE 4.0 or later.
        // Because of this I first attempt to delete the key in the
        // hope that it is empty. If that is not possible I load shlwapi
        // and call the function in that. This prevents the app bombing
        // out if the dll can't be found.
        if (RegDeleteKey(hKeyRoot, pszSubKey) != ERROR_SUCCESS)
        {

            HINSTANCE hLibInst = LoadLibrary(_T("shlwapi.dll"));

            if (!hLibInst)
            {
                throw ERROR_NO_SHLWAPI_DLL;
            }

#if defined(UNICODE) || defined(_UNICODE)
            SHDELKEYPROC DeleteKeyRecursive = (SHDELKEYPROC)GetProcAddress(hLibInst, "SHDeleteKeyW");
#else

            SHDELKEYPROC DeleteKeyRecursive = (SHDELKEYPROC)GetProcAddress(hLibInst, "SHDeleteKeyA");
#endif

            if (!DeleteKeyRecursive)
            {
                FreeLibrary(hLibInst);
                throw ERROR_NO_SHDELETEKEY;
            }

            dwRet = DeleteKeyRecursive(hKeyRoot, pszSubKey);

            FreeLibrary(hLibInst);
        }
    }
    else
    {
        // Windows 9x will allow RegDeleteKey to delete keys
        // even if they have subkeys/values.
        dwRet = RegDeleteKey(hKeyRoot, pszSubKey);
    }

    if (dwRet == ERROR_SUCCESS)
        return true;

    SetLastError(dwRet);
    return false;
}

// Deletes a value from a given subkey and root
bool CRegistry::DeleteValue(HKEY hKeyRoot, LPCTSTR pszSubKey, LPCTSTR pszValue)
{
    HKEY hKey;
    LONG lRes;

    if ((lRes = RegOpenKeyEx(hKeyRoot, pszSubKey, 0, KEY_SET_VALUE, &hKey)) != ERROR_SUCCESS)
    {
        SetLastError((DWORD)lRes);
        return false;
    }

    lRes = RegDeleteValue(hKey, pszValue);

    RegCloseKey(hKey);

    if (lRes == ERROR_SUCCESS)
        return true;

    SetLastError(lRes);
    return false;
}

// Fetch a binary value. If the size specified by rdwSize is too small, rdwSize will
// be set to the correct size.
bool CRegistry::GetBinaryValue(HKEY hKeyRoot, LPCTSTR pszSubKey, LPCTSTR pszValue, PVOID pBuffer, DWORD& rdwSize)
{
    HKEY hKey;
    DWORD dwType = REG_BINARY;
    DWORD dwSize = rdwSize;
    LONG lRes = 0;

    if ((lRes = RegOpenKeyEx(hKeyRoot, pszSubKey, 0, KEY_READ, &hKey)) != ERROR_SUCCESS)
    {
        SetLastError((DWORD)lRes);
        return false;
    }

    lRes = RegQueryValueEx(hKey, pszValue, 0, &dwType, (LPBYTE)pBuffer, &dwSize);

    rdwSize = dwSize;
    RegCloseKey(hKey);

    if (lRes != ERROR_SUCCESS)
    {
        SetLastError(lRes);
        return false;
    }

    if (dwType != REG_BINARY)
    {
        throw ERROR_WRONG_TYPE;
    }

    return true;
}

// Fetch a little endian DWORD from the registry (see platform SDK "Registry Value Types")
bool CRegistry::GetDWORDValue(HKEY hKeyRoot, LPCTSTR pszSubKey, LPCTSTR pszValue, DWORD &rdwBuff)
{
    HKEY hKey;
    DWORD dwType = REG_DWORD;
    DWORD dwSize = sizeof(DWORD);
    DWORD dwValue = 0;
    LONG lRes;

    rdwBuff = 0;

    if ((lRes = RegOpenKeyEx(hKeyRoot, pszSubKey, 0, KEY_READ, &hKey)) != ERROR_SUCCESS)
    {
        SetLastError(lRes);
        return false;
    }

    lRes = RegQueryValueEx(hKey, pszValue, 0, &dwType, (LPBYTE) & dwValue, &dwSize);

    RegCloseKey(hKey);

    if (dwType != REG_DWORD)
        throw ERROR_WRONG_TYPE;

    if (lRes != ERROR_SUCCESS)
    {
        SetLastError(lRes);
        return false;
    }

    rdwBuff = dwValue;

    return true;
}

// Retrieve a string value. If the given buffer for the string is too small (specified
// by rdwSize), rdwSize is increased to the correct value. If the buffer is bigger than
// the retrieved string, rdwSize is set to the length of the string (in bytes) including
// the terminating null.
bool CRegistry::GetStringValue(HKEY hKeyRoot, LPCTSTR pszSubKey, LPCTSTR pszValue, LPTSTR pszBuffer, DWORD& rdwSize)
{
    HKEY hKey;
    DWORD dwType = REG_SZ;
    LONG lRes;
    DWORD dwBufferSize = rdwSize;

    if (!pszBuffer)
        throw ERROR_INVALID_BUFFER;

    if ((lRes = RegOpenKeyEx(hKeyRoot, pszSubKey, 0, KEY_READ, &hKey)) != ERROR_SUCCESS)
    {
        SetLastError(lRes);
        return false;
    }

    lRes = RegQueryValueEx(hKey, pszValue, NULL, &dwType, (unsigned char*)pszBuffer, &dwBufferSize);

    RegCloseKey(hKey);
    rdwSize = dwBufferSize;

    if (dwType != REG_SZ)
        throw ERROR_WRONG_TYPE;


    if (lRes != ERROR_SUCCESS)
    {
        SetLastError(lRes);
        return false;
    }

    return true;
}

// Writes a binary value to the registry
bool CRegistry::SetBinaryValue(HKEY hKeyRoot, LPCTSTR pszSubKey, LPCTSTR pszValue, PVOID pData, DWORD dwSize)
{
    HKEY hKey;
    LONG lRes = 0;

    if ((lRes = RegOpenKeyEx(hKeyRoot, pszSubKey, 0, KEY_WRITE, &hKey)) != ERROR_SUCCESS)
    {
        SetLastError(lRes);
        return false;
    }

    lRes = RegSetValueEx(hKey, pszValue, 0, REG_BINARY, reinterpret_cast<BYTE*>(pData), dwSize);

    RegCloseKey(hKey);

    if (lRes != ERROR_SUCCESS)
    {
        SetLastError(lRes);
        return false;
    }

    return true;
}

// Writes a DWORD value to the registry
bool CRegistry::SetDWORDValue(HKEY hKeyRoot, LPCTSTR pszSubKey, LPCTSTR pszValue, DWORD dwValue)
{
    HKEY hKey;
    LONG lRes;

    if ((lRes = RegOpenKeyEx(hKeyRoot, pszSubKey, 0, KEY_WRITE, &hKey)) != ERROR_SUCCESS)
    {
        SetLastError(lRes);
        return false;
    }

    lRes = RegSetValueEx(hKey, pszValue, 0, REG_DWORD, reinterpret_cast<BYTE*>(&dwValue), sizeof(DWORD));

    RegCloseKey(hKey);

    if (lRes != ERROR_SUCCESS)
    {
        SetLastError(lRes);
        return false;
    }

    return true;
}

// Writes a string to the registry.
bool CRegistry::SetStringValue(HKEY hKeyRoot, LPCTSTR pszSubKey, LPCTSTR pszValue, LPCTSTR pszString)
{
    HKEY hKey;
    LONG lRes;
    DWORD dwSize = lstrlen(pszString) * sizeof(TCHAR);

    if ((lRes = RegOpenKeyEx(hKeyRoot, pszSubKey, 0, KEY_WRITE, &hKey)) != ERROR_SUCCESS)
    {
        SetLastError(lRes);
        return false;
    }

    lRes = RegSetValueEx(hKey, pszValue, 0, REG_SZ,
                         (BYTE*)(pszString), dwSize);

    RegCloseKey(hKey);

    if (lRes != ERROR_SUCCESS)
    {
        SetLastError(lRes);
        return false;
    }

    return true;
}
