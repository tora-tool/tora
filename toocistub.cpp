#include <oci.h>

#ifdef WIN32
#define TO_EXPORT __declspec(dllexport)
#else
#define TO_EXPORT
#endif
extern "C" {

  TO_EXPORT sword OCIEnvInit(OCIEnv **envp,ub4 mode,
			     size_t xtramem_sz,dvoid **usrmempp)
  {
    return OCI_ERROR;
  }

  TO_EXPORT sword OCIHandleAlloc(CONST dvoid *parenth,dvoid **hndlpp,CONST ub4 type,
				 CONST size_t xtramem_sz,dvoid **usrmempp)
  {
    return OCI_ERROR;
  }

  TO_EXPORT sword OCIServerAttach(OCIServer *srvhp,OCIError *errhp,
				  CONST OraText *dblink,sb4 dblink_len,ub4 mode)
  {
    return OCI_ERROR;
  }

  TO_EXPORT sword OCIAttrGet(CONST dvoid *trgthndlp,ub4 trghndltyp,
			     dvoid *attributep,ub4 *sizep,ub4 attrtype,
			     OCIError *errhp)
  {
    return OCI_ERROR;
  }

  TO_EXPORT sword OCIAttrSet(dvoid *trgthndlp,ub4 trghndltyp,dvoid *attributep,
			     ub4 size,ub4 attrtype,OCIError *errhp)
  {
    return OCI_ERROR;
  }

  TO_EXPORT sword OCISessionBegin(OCISvcCtx *svchp,OCIError *errhp,OCISession *usrhp,
				  ub4 credt,ub4 mode)
  {
    return OCI_ERROR;
  }

  TO_EXPORT sword OCISessionEnd(OCISvcCtx *svchp,OCIError *errhp,OCISession *usrhp,
				ub4 mode)
  {
    return OCI_ERROR;
  }

  TO_EXPORT sword OCIBreak(dvoid *hndlp,OCIError *errhp)
  {
    return OCI_ERROR;
  }

  TO_EXPORT sword OCIBindByName(OCIStmt *stmtp,OCIBind **bindp,OCIError *errhp,
				CONST OraText *placeholder,sb4 placeh_len,
				dvoid *valuep,sb4 value_sz,ub2 dty,
				dvoid *indp,ub2 *alenp,ub2 *rcodep,
				ub4 maxarr_len,ub4 *curelep,ub4 mode)
  {
    return OCI_ERROR;
  }

  TO_EXPORT sword OCIInitialize(ub4 mode,dvoid *ctxp,
				dvoid *(*malocfp)(dvoid *ctxp,size_t size),
				dvoid *(*ralocfp)(dvoid *ctxp,dvoid *memptr,size_t newsize),
				void (*mfreefp)(dvoid *ctxp,dvoid *memptr))
  {
    return OCI_ERROR;
  }

  TO_EXPORT sword OCIHandleFree(dvoid *hndlp,CONST ub4 type)
  {
    return OCI_ERROR;
  }

  TO_EXPORT sword OCILobGetLength(OCISvcCtx *svchp,OCIError *errhp,
				  OCILobLocator *locp,
				  ub4 *lenp)
  {
    return OCI_ERROR;
  }

  TO_EXPORT sword OCILobLocatorIsInit(OCIEnv *envhp,OCIError *errhp,
				      CONST OCILobLocator *locp,
				      boolean *is_initialized)
  {
    return OCI_ERROR;
  }

  TO_EXPORT sword OCILobRead(OCISvcCtx *svchp,OCIError *errhp,OCILobLocator *locp,
			     ub4 *amtp,ub4 offset,dvoid *bufp,ub4 bufl,
			     dvoid *ctxp,sb4 (*cbfp)(dvoid *ctxp,
						     CONST dvoid *bufp,
						     ub4 len,
						     ub1 piece),
			     ub2 csid,ub1 csfrm)
  {
    return OCI_ERROR;
  }

  TO_EXPORT sword OCIStmtPrepare(OCIStmt *stmtp,OCIError *errhp,CONST OraText *stmt,
				 ub4 stmt_len,ub4 language,ub4 mode)
  {
    return OCI_ERROR;
  }

  TO_EXPORT sword OCIStmtExecute(OCISvcCtx *svchp,OCIStmt *stmtp,OCIError *errhp,
				 ub4 iters,ub4 rowoff,CONST OCISnapshot *snap_in,
				 OCISnapshot *snap_out,ub4 mode)
  {
    return OCI_ERROR;
  }

  TO_EXPORT sword OCITransCommit (OCISvcCtx *svchp,OCIError *errhp,ub4 flags)
  {
    return OCI_ERROR;
  }


  TO_EXPORT sword OCITransRollback (OCISvcCtx *svchp,OCIError *errhp,ub4 flags)
  {
    return OCI_ERROR;
  }

  TO_EXPORT sword OCIDescriptorAlloc(CONST dvoid *parenth,dvoid **descpp,
				     CONST ub4 type,CONST size_t xtramem_sz,
				     dvoid **usrmempp)
  {
    return OCI_ERROR;
  }

  TO_EXPORT sword OCIDescriptorFree(dvoid *descp,CONST ub4 type)
  {
    return OCI_ERROR;
  }

  TO_EXPORT sword OCIParamGet (CONST dvoid *hndlp,ub4 htype,OCIError *errhp,
			       dvoid **parmdpp,ub4 pos)
  {
    return OCI_ERROR;
  }

  TO_EXPORT sword OCIServerDetach (OCIServer *srvhp,OCIError *errhp,ub4 mode)
  {
    return OCI_ERROR;
  }

  TO_EXPORT sword OCILobWrite (OCISvcCtx *svchp,OCIError *errhp,OCILobLocator *locp,
			       ub4 *amtp,ub4 offset,dvoid *bufp,ub4 buflen,
			       ub1 piece,dvoid *ctxp,
			       sb4 (*cbfp)(dvoid *ctxp,
					   dvoid *bufp,
					   ub4 *len,
					   ub1 *piece),
			       ub2 csid,ub1 csfrm)
  {
    return OCI_ERROR;
  }

  TO_EXPORT sword OCIStmtFetch (OCIStmt *stmtp,OCIError *errhp,ub4 nrows,
				ub2 orientation,ub4 mode)
  {
    return OCI_ERROR;
  }

  TO_EXPORT sword OCIDefineByPos (OCIStmt *stmtp,OCIDefine **defnp,OCIError *errhp,
				  ub4 position,dvoid *valuep,sb4 value_sz,ub2 dty,
				  dvoid *indp,ub2 *rlenp,ub2 *rcodep,ub4 mode)
  {
    return OCI_ERROR;
  }

  TO_EXPORT sword OCIErrorGet(dvoid *hndlp, ub4 recordno, OraText *sqlstate,
			      sb4 *errcodep, OraText *bufp, ub4 bufsiz, ub4 type)
  {
    return OCI_ERROR;
  }

}
