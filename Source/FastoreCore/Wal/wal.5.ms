.ad l
.DA $Id$
.TL 
WAL Data Structures
.SH
File Header
.PP
Each WAL file is organized as succession of revision records.  It
starts with a header record to distinguish it from other files.  
.begin dformat 
 style bitwid 0.125
style linethrutext 0
WAL Header Record
style charwid 0.05
	0--8 \*QFASTORE\\0\*U
style charwid reset
	8--4 OS Page Size 
	12--4 WAL size in pages
	16--4 Salt	       
	20--16 MD4 Fingerprint
.end

.PP
Recovery consists of reading the header, then reading records sequentially until the EOF marker. 
.PP
The header record and sets of data records contain an MD4 checksum, giving the reader great protection against file corruption. 
.PP
The first 8 bytes contain the \fBmagic\fR string \fIFASTORE\fR with a trailing NULL.  These serve to identify the file as a Fastore log file, and to denote the version of the file.  For consistency with future versions the first four bytes should be considered constant (to identify the file type).  The last four bytes are a version number; the final bit is zero for little-endian and one for big-endian.    
.PP
The \fBOS Page Size\fR serves tells the reader where to look for the other copy of the header record.  It is also a factor in computing the \fIphysical\fR file size, allowing \fBWAL size in pages\fR to describe a file up to 4 \*Qgigapages\*U, or 1 TB using a page size of 1 KB. 
.PP
.B Salt
is a random integer generated when the file is initialized. 
.PP
The 
.B MD4\ fingerprint
is computed over the whole header.  The random number helps to prevent any mixing of a header with the wrong file.  
.SH
Record Header
.EQ
delim $$
.EN
.PP
Records are variable length without regard to OS page boundaries.  A record may span two or more pages, or several records may appear on one page.  
.begin dformat 
 style bitwid 0.09
style linethrutext 0
WAL Page Header
	0--8 LSN
	8--8 Transaction ID
	16--8 time_t
	24-27-4 Length
	--10-dashed ... data ...	       
	N--1 End of Record
	N+1-N+16-16 MD4 Fingerprint
.end
.PP
Each WAL record begins with a 
.B Transaction\ ID ,
followed by \fCtime_t\fR as returned by \fCtime(3)\fR, and a data 
.B Length
in bytes. After the data is an 
.B EOR
marker, with one of these values:
.RS
.IP \(bu
0x00, end of record, no MD4 follows
.IP \(bu
0x01, MD4 follows next
.IP \(bu
0xFF, End of File
.RE
To ensure data integrity, an \fBMD4 Fingerprint\fR is added
.RS
.IP \(bu 
to multipage transactions, 
.IP \(bu 
once per page, for batches of small transactions, and
.IP \(bu 
after the last record. 
.RE
.PP 
This is done as follows:
.RS
.IP \(bu
Write record, write MD4
.IP \(bu
Note offset within page at end of MD4
.IP \(bu
Compute size of next record
.IP \(bu
If $offset + "next record size" <= "OS page size"$ then 
set write pointer to overwrite last MD4
.IP \(bu
Repeat until EOF
.RE
.PP
This algorithm ensures an MD4 is computed over most of a page or, for large records, over the entire record (and perhaps some small ones immediately preceding it).  It avoids recording an MD4 for every tiny record, and avoids interpersing MD4s on page boundaries within records.\**
.FS
An alternative design would be to have one MD4 per page, and interrupt a record with an MD4 when writing across a page boundary.  Such a design requires page-reading logic to splice together individually fingerprinted pages, whereas the chosen design permits the reader to simply read sequentially and compute an MD4 whenever the EOR indicates.  The number of MD4 computations would be the same, because the MD4 would have to be updated whenever the page is updated.  
.FE 
.PP
The MD4 Fingerprint is computed over the bytes since the preceding MD4, even if that MD4 was in another WAL file (in which case it includes the new file's File Header Record). 
.PP
