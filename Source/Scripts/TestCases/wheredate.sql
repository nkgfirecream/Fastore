select 
	l_shipdate
from
    lineitem
where
	l_shipdate <= date( '1998-12-01','-90 day')
limit 10;