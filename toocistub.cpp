#include <windows.h>

#define OCI_ERROR -1
typedef signed int sword;

#define TO_EXPORT __declspec(dllexport)

extern "C" {

  BOOL APIENTRY DllMain(HANDLE hModule, 
                        DWORD  ul_reason_for_call, 
                        LPVOID lpReserved)
  {
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
  }

  TO_EXPORT sword OCIEnvInit()
  {
    return OCI_ERROR;
  }

  TO_EXPORT sword OCIHandleAlloc()
  {
    return OCI_ERROR;
  }

  TO_EXPORT sword OCIServerAttach()
  {
    return OCI_ERROR;
  }

  TO_EXPORT sword OCIAttrGet()
  {
    return OCI_ERROR;
  }

  TO_EXPORT sword OCIAttrSet()
  {
    return OCI_ERROR;
  }

  TO_EXPORT sword OCISessionBegin()
  {
    return OCI_ERROR;
  }

  TO_EXPORT sword OCISessionEnd()
  {
    return OCI_ERROR;
  }

  TO_EXPORT sword OCIBreak()
  {
    return OCI_ERROR;
  }

  TO_EXPORT sword OCIBindByName()
  {
    return OCI_ERROR;
  }

  TO_EXPORT sword OCIInitialize()
  {
    return OCI_ERROR;
  }

  TO_EXPORT sword OCIHandleFree()
  {
    return OCI_ERROR;
  }

  TO_EXPORT sword OCILobGetLength()
  {
    return OCI_ERROR;
  }

  TO_EXPORT sword OCILobLocatorIsInit()
  {
    return OCI_ERROR;
  }

  TO_EXPORT sword OCILobRead()
  {
    return OCI_ERROR;
  }

  TO_EXPORT sword OCIStmtPrepare()
  {
    return OCI_ERROR;
  }

  TO_EXPORT sword OCIStmtExecute()
  {
    return OCI_ERROR;
  }

  TO_EXPORT sword OCITransCommit()
  {
    return OCI_ERROR;
  }


  TO_EXPORT sword OCITransRollback()
  {
    return OCI_ERROR;
  }

  TO_EXPORT sword OCIDescriptorAlloc()
  {
    return OCI_ERROR;
  }

  TO_EXPORT sword OCIDescriptorFree()
  {
    return OCI_ERROR;
  }

  TO_EXPORT sword OCIParamGet()
  {
    return OCI_ERROR;
  }

  TO_EXPORT sword OCIServerDetach()
  {
    return OCI_ERROR;
  }

  TO_EXPORT sword OCILobWrite()
  {
    return OCI_ERROR;
  }

  TO_EXPORT sword OCIStmtFetch()
  {
    return OCI_ERROR;
  }

  TO_EXPORT sword OCIDefineByPos()
  {
    return OCI_ERROR;
  }

  TO_EXPORT sword OCIErrorGet()
  {
    return OCI_ERROR;
  }

}
