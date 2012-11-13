select
	*
from
	lineitem,
	part
where
	p_partkey = l_partkey
	and p_brand = 'Brand#12'
limit 100;