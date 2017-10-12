select shipment.ship_id,
                ord.ordnum,
                ord.client_id,
                ord.cponum,
                sum_by_pckwrk.arecod,
                ord.wh_id,
                nvl(sum_by_pckwrk.stoloc, :q0) stoloc,
                shipment.carcod,
                shipment.srvlvl,
                shipment.stop_id,
                min(ord.carflg) carflg,
                stop.stop_nam,
                stop.stop_seq,
                stop.car_move_id,
                rt_adrmst.adrnam rt_adrnam,
                st_adrmst.adrnam st_adrnam,
                shipment.early_dlvdte,
                shipment.late_dlvdte,
                shipment.early_shpdte,
                shipment.late_shpdte,
                sum(sum_by_pckwrk.pckqty) pckqty,
                sum(sum_by_pckwrk.remqty) remqty,
                sum(sum_by_pckwrk.appqty) appqty,
                sum(sum_by_pckwrk.arrqty) arrqty
           from adrmst rt_adrmst
           join shipment
             on (shipment.rt_adr_id = rt_adrmst.adr_id)
           left outer
           join stop
             on (shipment.stop_id = stop.stop_id)
           join (select decode(pckwrk.pcksts, :q1, pckwrk.pckqty, :i2) pckqty,
                        decode(pckwrk.pcksts, :q3, pckwrk.pckqty - pckwrk.appqty, pckwrk.pckqty) remqty,
                        pckwrk.appqty,
                        sum(nvl(invdtl.untqty, :i4)) arrqty,
                        pckwrk.wrkref,
                        pckwrk.wh_id,
                        pckwrk.ship_id,
                        pckwrk.ship_line_id,
                        pckmov.arecod,
                        pckmov.stoloc
                   from pckwrk
                   join pckmov
                     on (pckwrk.cmbcod = pckmov.cmbcod and not exists(select :q5
                                                                        from pckmov pm3
                                                                       where pm3.cmbcod = pckmov.cmbcod
                                                                         and pm3.seqnum > pckmov.seqnum
                                                                         and exists(select :q6
                                                                                      from aremst
                                                                                     where aremst.wh_id = :q7
                                                                                       and aremst.stgflg = :i8
                                                                                       and not exists(select :q9
                                                                                                        from poldat_view
                                                                                                       where polcod = :q10
                                                                                                         and polvar = :q11
                                                                                                         and polval = :q12
                                                                                                         and wh_id = aremst.wh_id
                                                                                                         and rtrim(rtstr1) = aremst.arecod)
                                                                                       and pm3.arecod = aremst.arecod)) and exists(select :q13
                                                                                                                                     from aremst
                                                                                                                                    where aremst.wh_id = :q14
                                                                                                                                      and aremst.stgflg = :i15
                                                                                                                                      and not exists(select :q16
                                                                                                                                                       from poldat_view
                                                                                                                                                      where polcod = :q17
                                                                                                                                                        and polvar = :q18
                                                                                                                                                        and polval = :q19
                                                                                                                                                        and wh_id = aremst.wh_id
                                                                                                                                                        and rtrim(rtstr1) = aremst.arecod)
                                                                                                                                      and pckmov.arecod = aremst.arecod) and 1 = 1)
                   left outer
                   join invlod
                     on (invlod.stoloc = pckmov.stoloc and invlod.wh_id = pckmov.wh_id)
                   left outer
                   join invsub
                     on (invsub.lodnum = invlod.lodnum)
                   left outer
                   join invdtl
                     on (invdtl.subnum = invsub.subnum and invdtl.wrkref = pckwrk.wrkref)
                  where pckwrk.pcksts in (:q20, :q21, :q22)
                    and pckwrk.wrktyp = :q23
                    and pckwrk.prtnum != :q24
                 /* Exclude pick work entries with prtnum 'KITPART' */
                  group by pckwrk.wrkref,
                        pckwrk.wh_id,
                        pckwrk.pcksts,
                        pckwrk.pckqty,
                        pckwrk.appqty,
                        pckwrk.ship_id,
                        pckwrk.ship_line_id,
                        pckmov.arecod,
                        pckmov.stoloc) sum_by_pckwrk
             on (sum_by_pckwrk.ship_id = shipment.ship_id)
           join shipment_line
             on (shipment.ship_id = shipment_line.ship_id)
            and (shipment_line.ship_line_id = sum_by_pckwrk.ship_line_id)
           join ord
             on (ord.ordnum = shipment_line.ordnum)
            and (ord.client_id = shipment_line.client_id)
            and (ord.wh_id = shipment_line.wh_id)
           left outer
           join adrmst st_adrmst
             on (ord.st_adr_id = st_adrmst.adr_id)
           left outer
           join car_move
             on (stop.car_move_id = car_move.car_move_id)
          where shipment.super_ship_flg = :i25
            and shipment.stgdte is null
            and 1 = 1
            and 1 = 1
            and 1 = 1
            and 1 = 1
            and 1 = 1
            and 1 = 1
            and 1 = 1
            and 1 = 1
            and 1 = 1
            and 1 = 1
            and 1 = 1
            and ord.wh_id = :q26
            and 1 = 1
            and 1 = 1
            and 1 = 1
            and 1 = 1
            and 1 = 1
            and 1 = 1
            and 1 = 1
            and 1 = 1
            and 1 = 1
            and 1 = 1
            and 1 = 1
          group by shipment.ship_id,
                ord.ordnum,
                ord.client_id,
                ord.cponum,
                sum_by_pckwrk.arecod,
                sum_by_pckwrk.stoloc,
                shipment.carcod,
                shipment.srvlvl,
                shipment.stop_id,
                stop.stop_nam,
                stop.stop_seq,
                stop.car_move_id,
                rt_adrmst.adrnam,
                st_adrmst.adrnam,
                shipment.early_dlvdte,
                shipment.late_dlvdte,
                shipment.early_shpdte,
                shipment.late_shpdte,
                ord.wh_id
