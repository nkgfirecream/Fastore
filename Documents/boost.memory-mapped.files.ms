.\" $Id$
.DA
.TL
Analysis of Boost Memory-Mapped Files
.AB
Boost's Memory-Mapped Files implementation is a compent, thin veneer over the underlying OS functions.  It provides the intersection of functionality between Windows and POSIX, with no access to the underlying file descriptors.  Because it prevents full exploitation and control of map that the OS provides, it isn't well suited to use for the write-ahead log.  
.AE
.na
.SH
Features
.PP
The \fCmapped_file\f[] class offers these methods:
.IP
.CW constructor
.IP
.CW open
.IP
.CW is_open
.IP
.CW close
.IP
.CW size
.IP
.CW data
.IP
.CW const_data
.IP
.CW alignment
.R
.PP
Only functionality available on both operating systems is supported.  Note the lack of a \fChandle\f[] method or similar.  (This is consistent with \fCiostreams\f[] but in this case prevents necessary access.)  
.PP
One benefit of using Boost is consolidated error handling.  I assume it's well done, but it seems to me that this critical function needs careful and specific treatment.  
.SH
Questionable choices
.PP
In both Windows and Linux, the map function \(em 
.CW mmap
or
.CW CreateFileMappingA
&
.CW MapViewOfFileEx
\(em are passed hard-coded flags based on the value of the \fCreadonly\f[] member. If the mapping function fails for any reason, the functions are calls again without the flags.  That this done is not documented; why it is done doesn't merit a comment in the source code.  In consequence, the attributes of the mapping are nondeterministic and, moreover, cannot be ascertained from the object.  
.SH 
Unsupported Features
.IP \fBmsync\f[] 5m
POSIX \fCmsync\f[] permits committing an arbitrary range of pages to disk. Windows has a similar function, \fCFlushViewOfFile\f[].  This feature and absolutely necessary to support user-requested \fIflush\f[] functionality. 
.IP \fBmadvise\f[]
POSIX \fCmadvise\f[] lets the application advise the kernel of the intended use of the mapped file.  Chief among these for our purpose is \fCMADV_SEQUENTIAL\f[], because the file will be accessed for the most part sequentially.  Windows offers no similar function, but \fCmadvise\f[] might well turn out to be important for performance on Linux.  
.IP \fBshared\ memory\f[]
Both POSIX and Windows provide for a memory-mapped file to be shared between processes.  This could prove to be useful in limited ways.  For instance, the \fCwald\f[] daemon that creates new WAL files could map the file into memory before advising the \fCWal\f[] object that the file is ready, saving the Wal object the time to map the file.  (It would only gain access to already-mapped pages.) It also seems possible that the Worker would write to the WAL file and that only synchronization would fall to the \fCWal\f[] object.  
.SH
Alternative design
.PP
My recommendation is to develop a POSIX-like C library of missing functions for Windows.  I would make it open source to allow others to offer advice and fixes.  For mapped files, there are only a few functions needed and implementation simple.  The flags are differently named, but are semantically equivalent (and, in any case, any differences are also to be found in Boost's implementation).  
.PP
By creating a C library, the same WAL implementation would compile in Linux and Windows.  Problems would be directly addressable, and the reference implementation available for comparison.  
.PP
Having created a compatible C library, should a C++ object such as Boost's be created as a wrapper?  If so, Boost's own implementation could be used, less the Windows code and with the above missing functionality added.  Because RAII cannot be applied to this object in this application, I wouldn't bother to create the wrapper unless we decided to implement I/O operators \fCoperator<<\f[] and \fCoperator>>\f[] for I/O with the Thrift classes, in which case the mapped-memory object could act as source and sink.  If the Thift classes will serialize to/from a \fCchar*\f[] pointer, it's hard to see any reason to write a wrapper class. 
.PP
Below is a list of the needed functions.
.TS
l l .
POSIX	Windows
_
open	T{
CreateFile\(dg
\!.FS \(dg
\!The POSIX function exists in Windows, but the \fCFILE*\f[] handle cannot be
\!used with memory-mapped files.  
\!.FE
T}
lseek	LZSeek\(dg
close	FileClose\(dg
mmap	CreateFileMapping, MapViewOfFileEx
madvise	(not possible)
msync	FlushViewOfFile
munmap	T{
UnmapViewOfFile, CloseHandle\(**
\!.FS \(**
\!CloseHandle must be called twice, once for the file handle, 
\!and once for the handle to the mapping object. 
\!.FE
T}
.TE
.PP
Because Windows already has some of these functions, wrappers \(em perhaps called mopen, mlseek, mclose \(em would be needed.  
.PP
The mapped-memory functionality in Windows is, like a lot of Windows' functionality, similar to POSIX and different enough to be a nuisance.  Implementing a POSIX version is perhaps one day's work.  It would provide a more powerful and flexible interface to the operating system, and remove the uncertainty and limitations inherent in the Boost implementation. 
