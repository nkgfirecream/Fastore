select
	sum(l_quantity) as sum_qty
from
	lineitem
group by
	l_returnflag
limit 100;