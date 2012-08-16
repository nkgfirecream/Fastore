


#data: file.gen
data:
	./file.gen > $@~
	@mv $@~ $@

import: data.db
data.db: data-import.sqliterc
	@rm -f $@
	time sqlite3 -init $(.ALLSRC) $@ < /dev/null

copy: data
	time cp data d
	@rm d

select: data.db
	time sqlite3 $(.ALLSRC) 'select count(*) from data'

read-sqlite: data.db
	time sqlite3 $(.ALLSRC) 'select * from data' | wc -l

wc: data
	time wc -l $(.ALLSRC)

test: data import cp select read-sqlite wc
