select
	*
from
	lineitem,
	part
where
	(
		p_partkey = l_partkey
		and p_brand = 'Brand#12'
	)
	or
	(
		p_partkey = l_partkey
		and p_brand = 'Brand#23'
	)
	or
	(
		p_partkey = l_partkey
		and p_brand = 'Brand#34'
	)
limit 100;