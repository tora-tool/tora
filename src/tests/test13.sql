WITH filterquery AS
(
SELECT task_id, rnumumber
FROM 
(
SELECT DISTINCT
ROW_NUMBER() OVER (ORDER BY jobt.LASTSTATUSDATE DESC) AS rnumumber,
jobt.id AS task_id
FROM ng_task_job jobt 
LEFT JOIN ng_job job ON job.id = jobt.job_id
LEFT JOIN ng_task_job_bl jobtbl ON jobtbl.task_job_id = jobt.id
-- LEFT JOIN ng_status_job jobstar ON jobstar.id = jobtbl.current_status_tar_id
-- LEFT JOIN ng_s_status s ON s.id = jobstar.s_status_id
-- INNER JOIN ng_rel_task_tra_job reltj ON reltj.task_job_id = jobt.id AND reltj.to_time IS NULL
-- INNER JOIN ng_rel_task_pro_tra relpt ON relpt.task_tra_id = reltj.task_tra_id AND relpt.to_time IS NULL
-- INNER JOIN ng_task_pro prot ON prot.id = relpt.task_pro_id
-- INNER JOIN tb_registration reg ON reg.id = prot.com_id AND reg.to_time IS NULL
-- LEFT JOIN ng_s_subcontractor scntr ON scntr.id = reg.subcontractor_id
WHERE BITAND(jobt.state, 128) = 0 AND (job.privateflag IS NULL OR job.privateflag = '0' )
AND (reg.vehicle_id IS NOT NULL
AND job.pickupdate BETWEEN :FromTime AND :ToTime
AND /*Checkpoint*/s.checkpoint IN ('GROUPAGE_SERVICE_PICKUP','CARGO_CARRIER_LOAD') AND scntr.subcontractorid = :subcontractor)
) limitpart
WHERE rnumumber > 0 AND rnumumber <= :resultlimit
ORDER BY rnumumber
)
-- getCachedOrdersByFilterQueries
SELECT 
cache.cached_result AS cached_result,
filterquery.task_id AS task_id,
--RANK() OVER(PARTITION BY cache.job_refid ORDER BY cache.job_date_time DESC, cache.tra_date_time DESC) AS ranking
1 AS ranking,
t.eta,
t.eta_min,
t.eta_max,
t.distance AS eta_distance,
protbl.state AS pro_state,
proaitar.ref_id_label AS pro_refid_label,
proaitar.ticket_id AS pro_ticketid,
proaitar.xml_obj AS pro_aitar_xml_obj,
pro.tour_begin AS pro_tourbegin,
pro.tour_end AS pro_tourend,
pro.loading_begin AS pro_loadbegin,
pro.processing_begin AS pro_processingbegin,
pro.processing_end AS pro_processingend,
pro.loading_end AS pro_loadend,
prot.taskid AS pro_taskid,
pro.ref_id AS pro_refid,
protbl.merged_status_obj pro_merged_status_obj,
job.order_note_summary AS order_note_summary
FROM filterquery
LEFT JOIN ng_search_cache cache ON cache.task_job_id = filterquery.task_id
-- LEFT JOIN ng_rel_task_tra_job rtj ON rtj.task_job_id = filterquery.task_id AND rtj.to_time IS NULL
-- LEFT JOIN ng_task_tra tt ON tt.id = rtj.task_tra_id
-- LEFT JOIN ng_tra t ON t.id = tt.tra_id
-- LEFT JOIN ng_rel_task_pro_tra rpt ON rpt.task_tra_id = rtj.task_tra_id AND rpt.to_time IS NULL
-- LEFT JOIN ng_task_pro_bl protbl ON protbl.task_pro_id = rpt.task_pro_id
-- LEFT JOIN ng_task_pro prot ON prot.id = rpt.task_pro_id
-- LEFT JOIN ng_pro pro ON pro.id = prot.pro_id
-- LEFT JOIN ng_pro_addinfo_tar proaitar ON proaitar.status_pro_id = protbl.current_status_tar_id
-- LEFT JOIN ng_task_job jobt ON jobt.id = filterquery.task_id
-- LEFT JOIN ng_job job ON job.id = jobt.job_id
;
