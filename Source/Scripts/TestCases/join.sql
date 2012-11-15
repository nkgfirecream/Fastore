select
	*
from
	lineitem,
	part
where
	p_partkey = l_partkey
order by
	p_partkey;