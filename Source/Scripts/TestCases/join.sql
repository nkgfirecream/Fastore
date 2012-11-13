select
	*
from
	lineitem,
	part
where
	p_partkey = l_partkey
limit 100;	