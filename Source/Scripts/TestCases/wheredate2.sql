select 
	l_shipdate
from
    lineitem
where
	l_shipdate >= date('1993-12-01') and l_shipdate <= date('1994-01-01')
limit 100;