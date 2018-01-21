select k.*, (select * from dual d0) as q0d
from (select * from dual d1) d1q
join (select * from dual d2) d2q on (d1q.dummy = d2q.dummy)
