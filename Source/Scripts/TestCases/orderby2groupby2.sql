select
	sum(l_quantity) as sum_qty
from
	lineitem
group by
	l_returnflag,
	l_linestatus
order by
	l_returnflag,
	l_linestatus
limit 100;