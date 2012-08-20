.NP
.\" $Id$
.ad l
.SH 
WAL Data Structures
.PP
Each WAL file is organized as succession of revision records in a ring.  The status of the ring is maintained in two header records, located on pages 0 and 1, updated alternately in classic \*Qping-pong\*U fashion after each write to the file.  
.PP
Recovery consists of reading both pages, determining which is more recent, seeking to the first record, and reading records sequentially until the EOF marker. 
.PP
Both header records and all data records contain an MD4 checksum, giving the reader great protection against file corruption. 
.LP
.br
.po 0.125i
.B \dWAL\ Header\ Record
.begin dformat 
 style bitwid 0.125
style linethrutext 0
noname
style charwid 0.05
	0--8 \*QFASTORE\\0\*U
style charwid reset
	8--4 OS Page Size 
	12--4 WAL size in pages
	16--4 Checkpoint seq#
	20--8 First record offset
	28--8 Trailer offset
	36--4 Salt-1	       
	40--4 Salt-2	       
	44--16 MD4 Fingerprint
.end
.br
.po 1i
.PP
The first 8 bytes contain the \fBmagic\fR string \fIFASTORE\fR with a trailing NULL.  These serve to identify the file as a Fastore log file, and to denote the version of the file.  For consistency with future versions the first four bytes should be considered constant (to identify the file type) and the last four bytes a version number.  
.PP
The \fBOS Page Size\fR serves tells the reader where to look for the other copy of the header record.  It is also a factor in computing the physical file size, allowing \fBWA: size in pages\fR to describe a file up to 4 \*Qgigapages\*U, or 1 TB using a page size of 1 KB. 
.PP
The \fBCheckpoint sequence number\fR begins with zero R and is incremented at every checkpoint.  It wraps back to zero after 0xFFFF.  The \fBFirst record offset\fR will likely be an \fCoff_t\fR; it can be used with \fBlseek(2)\fR or, when the file is mapped into memory, with pointer arithmetic.  
.PP
The \fBTrailer offset\fR gives the logical EOF for the file.  Note that the \fBtrailer offset\fR will be less than the \fBFirst record offset\fR if the file has wrapped around. 
.PP
.B Salt-1
is a random integer incremented with each checkpoint, whereas 
.B Salt-2
is a random integer regenerated with each checkpoint. 
.PP
The 
.B MD4\ fingerprint
is computed over the whole header each time it is written.  The random numbers help to prevent any mixing of a header with the wrong file.  
.EQ
delim $$
.EN
.begin dformat 
 style bitwid 0.125
style linethrutext 0
\fBWAL Page Header\fR
	0--8 Transaction ID
	8-11-4 Length
	--8-dashed ... data ...	       
	N-N+15-16 MD4 Fingerprint
.end
.PP
(The WAL record header is provisional until the mechanism for checkpointing and obsoleting records is clarified.)
.PP
Each WAL record begins with a 
.B Transaction\ ID
and a \fBLength\fR in bytes.  The \fBMD4 Fingerprint\fR is computed over the bytes in the interval [0-N). 
