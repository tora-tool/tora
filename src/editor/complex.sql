select
  T1.INC_ID "ID",
  T2.RCT_NAME "Status",
  T1.INC_DESCRIPTION "Description",
  T4.WOG_SEARCHCODE "Workgroup",
  T5.PER_SEARCHCODE "Person",
  T1.INC_SOLUTION "Solution",
  T6.CDL_NAME "Closure Code",
  T7.CDL_NAME "Closure Code Parent",
  T9.CDL_NAME "Reason Caused Code Parent",
  T10.CIT_SEARCHCODE "CI",
  T11.CDL_NAME "Severity",
  T12.RCT_NAME "Category",
  T13.CDL_NAME "Business Impact",
  T16.CDL_NAME "Priority",
  TO_CHAR(T1."REG_CREATED", 'DD-MM-YY HH24:MI:SS') "Registered",
  TO_CHAR(T1."INC_DEADLINE", 'DD-MM-YY HH24:MI:SS') "Deadline",
  TO_CHAR(T1."INC_ACTUALFINISH", 'DD-MM-YY HH24:MI:SS') "Finish",
  T3.ICF_INCSHORTTEXT3 "Message Group",
  T3.ICF_INCSHORTTEXT4 "Application",
  T3.ICF_INCSHORTTEXT2 "Msg ID",
  T99.RCT_NAME "Folder",
  c.firstwg "FirstWG",
  f.firstreswg "FirstRes"
from
  SERVICEDESKREPO.REP_CODES_TEXT T2,
  SERVICEDESK.INC_CUSTOM_FIELDS T3,
  SERVICEDESK.WORKGROUPS T4,
  SERVICEDESK.CODES_LOCALE T11,
  SERVICEDESKREPO.REP_CODES_TEXT T12,
  SERVICEDESK.CODES_LOCALE T13,
  SERVICEDESKREPO.REP_CODES_TEXT T99,
  SERVICEDESK.CODES_LOCALE T16,
 (select
           a.hin_subject as firstwg,
           a.hin_inc_oid as firstwgoid,
           to_char(a.reg_created, 'YYYY-MM-DD HH24:MI:SS') as firstwgcreated
        from
           itsm_historylines_incident a,
           itsm_incidents b
        where a.hin_inc_oid= b.inc_oid
        and a.hin_subject like 'To Workgroup set to%'
        -- order by firstwgcreated desc
 ) c,
 (
  (
   (
    (
     (
      (
       (
        (SERVICEDESK.INCIDENTS T1 LEFT OUTER JOIN SERVICEDESK.PERSONS T5 on T1.INC_ASSIGN_PERSON_TO=T5.PER_OID)
	 LEFT OUTER JOIN SERVICEDESK.CODES T14 on T1.INC_CLO_OID=T14.COD_OID)
	 LEFT OUTER JOIN SERVICEDESK.CODES_LOCALE T6 on T14.COD_OID=T6.CDL_COD_OID)
	 LEFT OUTER JOIN SERVICEDESK.CODES_LOCALE T7 on T14.COD_COD_OID=T7.CDL_COD_OID)
	 LEFT OUTER JOIN SERVICEDESK.CODES T15 on T1.INC_CLA_OID=T15.COD_OID)
	 LEFT OUTER JOIN SERVICEDESK.CODES_LOCALE T8 on T15.COD_OID=T8.CDL_COD_OID)
	 LEFT OUTER JOIN SERVICEDESK.CODES_LOCALE T9 on T15.COD_COD_OID=T9.CDL_COD_OID)
	 LEFT OUTER JOIN SERVICEDESK.CONFIGURATION_ITEMS T10 on T1.INC_CIT_OID=T10.CIT_OID)
	 LEFT OUTER JOIN (select d.hin_subject as firstreswg, d.hin_inc_oid as firstresoid, to_char(d.reg_created, 'YYYY-MM-DD HH24:MI:SS') as firstrescreated
				from itsm_historylines_incident d,
				itsm_incidents e
				where d.hin_inc_oid= e.inc_oid
				and ( d.hin_subject like 'To Workgroup from%' and d.hin_subject not like 'To Workgroup to%' )
        order by
        firstrescreated desc
      )
     f on f.firstresoid=T1.INC_OID
where
  T1.INC_STA_OID=T2.RCT_RCD_OID and
  T1.INC_OID=T3.ICF_INC_OID and
  T1.INC_ASSIGN_WORKGROUP=T4.WOG_OID and
  T1.INC_SEV_OID=T11.CDL_COD_OID and
  T1.INC_CAT_OID=T12.RCT_RCD_OID and
  T13.CDL_COD_OID=T1.INC_IMP_OID and
  T1.INC_POO_OID=T99.RCT_RCD_OID and
  T1.INC_PRI_OID=T16.CDL_COD_OID
  and TO_DATE(T1."REG_CREATED", 'YYYY-MM-DD HH24:MI:SS') >= SYSDATE -1
  and TO_DATE(T1."REG_CREATED", 'YYYY-MM-DD HH24:MI:SS') <= SYSDATE
  and c.firstwgoid=T1.INC_OID

     
