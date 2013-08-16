#define EXPORT __declspec(dllexport)

/* These routine names were determined experimentally
 * by continually trying to run the executable until it stopped
 * complaining about missing entry points. Intent is solely to
 * provide a stub DLL to silence complaints about a missing oracle
 * install, not to provide any actual functionality.
 */

EXPORT void OCIBindByName(void) { }
EXPORT void OCISvcCtxToLda(void) { }
EXPORT void oopen(void) { }
EXPORT void oparse(void) {}
EXPORT void oclose(void) {}
EXPORT void OCILdaToSvcCtx(void) { }
EXPORT void OCILobTrim(void) { }
EXPORT void OCILobIsOpen(void) { }
EXPORT void OCILobClose(void) { }
EXPORT void OCILobLocatorIsInit(void) { }
EXPORT void OCILobRead(void) { }
EXPORT void OCILobGetLength(void) { }
EXPORT void OCITransRollback(void) { }
EXPORT void OCIParamGet(void) { }
EXPORT void OCIStmtFetch(void) { }
EXPORT void OCIStmtPrepare(void) { }
EXPORT void OCIStmtExecute(void) { }
EXPORT void OCIAttrGet(void) { }
EXPORT void OCILobWrite(void) { }
EXPORT void OCIDescriptorAlloc(void) { }
EXPORT void OCIDateTimeConstruct(void) { }
EXPORT void OCIDescriptorFree(void) { }
EXPORT void OCITransCommit(void) { }
EXPORT void OCIErrorGet(void) { }
EXPORT void OCISessionEnd(void) { }
EXPORT void OCIServerDetach(void) { }
EXPORT void OCIHandleFree(void) { }
EXPORT void OCIPasswordChange(void) { }
EXPORT void OCISessionBegin(void) { }
EXPORT void OCIEnvCreate(void) { }
EXPORT void OCIHandleAlloc(void) { }
EXPORT void OCIServerAttach(void) { }
EXPORT void OCIAttrSet(void) { }
EXPORT void OCIBreak(void) { }
EXPORT void OCIDefineByPos(void) { }

