#s0me r&nd0m text contaning mess  ¤, ¦, ¨, ´, ¸, ¼, ½, §, and ¾.
4.2 Graphic Characters


   Column/Row  Symbol      Name

   2/0         SP          Space (Normally Non-Printing)
   2/1         !           Exclamation Point
   2/2         "  "        Quotation Marks (Diaeresis [2])
   2/3         #           Number Sign [3,4]
   2/4         $           Dollar Sign
   2/5         %           Percent
   2/6         &           Ampersand
   2/7         '  '        Apostrophe (Closing Single Quotation Mark
                           Acute Accent [2])
   2/8         (           Opening Parenthesis
   2/9         )           Closing Parenthesis
   2/10        *           Asterisk
   2/11        +           Plus
   2/12        ,           Comma (Cedilla [2])
   2/13        -           Hyphen (Minus)
   2/14        .           Period (Decimal Point)
   2/15        /           Slant
   3/10        :           Colon
   3/11        ;           Semicolon
   3/12        <           Less Than
   3/13        =           Equals
   3/14        >           Greater Than
   3/15        ?           Question Mark
   4/0         @           Commercial At [3]
   5/11        [           Opening Bracket [3]
   5/12        \           Reverse Slant [3]
   5/13        ]           Closing Bracket [3]
   5/14        ^           Circumflex [2,3]
   5/15        _           Underline
   6/0         `           Grave Accent [2,3] (Opening Single ` Quotation
                                   Mark)
   7/11        {           Opening Brace [3]
   7/12        |           Vertical Line [3]
   7/13        }           Closing Brace [3]
   7/14        ~           Overline [3] (Tilde [2]; General Accent [2])


 select
  'hello'
  , 'oracle.dbs'
  , 'jackie''s raincoat'
  , '09-mar-98'
  , ''
  , ''''
  , q'!name like '%dbms_%%'!'
  , q'<'so,' she said, 'it's finished.'>'
  , q'{select * from employees where last_name = 'smith'}'
  , q'"name like '['"'
  , n'nchar literal'
  from dual
;  

select 25
, +6.34
, 0.5
, 25e-03
, -1 -- Here are some valid floating-point number literals:
, 25f
, +6.34F
, 0.5d
, -1D
, 1.
, .5
, (sysdate -1d)   -- here we substract "one" in decimal format
, sysdate -1m   -- here we substract "one" and "m" is column's alias
, sysdate -1dm
, 1.-+.5
, 1.+.5
, 1.+.5D
, 1.+.5DM
, 1.D -- XXX
, 1.M
, .5M
, .5DM
from dual
;

abc

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
  SERVICEDESK.ITSM_INC_CUSTOM_FIELDS T3,
  SERVICEDESK.ITSM_WORKGROUPS T4,
  SERVICEDESK.ITSM_CODES_LOCALE T11,
  SERVICEDESKREPO.REP_CODES_TEXT T12,
  SERVICEDESK.ITSM_CODES_LOCALE T13,
  SERVICEDESKREPO.REP_CODES_TEXT T99,
  SERVICEDESK.ITSM_CODES_LOCALE T16,
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
        (SERVICEDESK.ITSM_INCIDENTS T1 LEFT OUTER JOIN SERVICEDESK.ITSM_PERSONS T5 on T1.INC_ASSIGN_PERSON_TO=T5.PER_OID)
	 LEFT OUTER JOIN SERVICEDESK.ITSM_CODES T14 on T1.INC_CLO_OID=T14.COD_OID)
	 LEFT OUTER JOIN SERVICEDESK.ITSM_CODES_LOCALE T6 on T14.COD_OID=T6.CDL_COD_OID)
	 LEFT OUTER JOIN SERVICEDESK.ITSM_CODES_LOCALE T7 on T14.COD_COD_OID=T7.CDL_COD_OID)
	 LEFT OUTER JOIN SERVICEDESK.ITSM_CODES T15 on T1.INC_CLA_OID=T15.COD_OID)
	 LEFT OUTER JOIN SERVICEDESK.ITSM_CODES_LOCALE T8 on T15.COD_OID=T8.CDL_COD_OID)
	 LEFT OUTER JOIN SERVICEDESK.ITSM_CODES_LOCALE T9 on T15.COD_COD_OID=T9.CDL_COD_OID)
	 LEFT OUTER JOIN SERVICEDESK.ITSM_CONFIGURATION_ITEMS T10 on T1.INC_CIT_OID=T10.CIT_OID)
	 LEFT OUTER JOIN (select d.hin_subject as firstreswg, d.hin_inc_oid as firstresoid, to_char(d.reg_created, 'YYYY-MM-DD HH24:MI:SS') as firstrescreated
				from itsm_historylines_incident d,
				itsm_incidents e
				where d.hin_inc_oid= e.inc_oid
				and ( d.hin_subject like 'To Workgroup from%' and d.hin_subject not like 'To Workgroup from%to "ITSC % Service Desk"%' )
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
 ;

merge into bonuses d 
   using (select employee_id.* from employees) s 
   on (employee_id = a) 
   when matched then update set d.bonus = bonus 
     delete where (salary > 8000)
   when not matched then insert (d.employee_id, d.bonus) 
     values (s.employee_id, s.salary)
     where (s.salary <= 8000)

merge into bonuses d 
   using (select employee_id.* from employees) s 
   on (employee_id = a) 
   when not matched then insert (d.employee_id, d.bonus) 
     values (s.employee_id, s.salary)
     where (s.salary <= 8000)
   when matched then update set d.bonus = bonus 
     delete where (salary > 8000)

     