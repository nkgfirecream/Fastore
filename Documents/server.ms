.ad l 
.TL
Notes on Server Design 
.DA $Id$
.PP
The architecture has a physical and logical vocabulary.  
.TS
cb | cb
_ _
l | l.
Logical	Physical
Topology	Hive
Host	Service
Pod	Worker
Column	Repository
.TE
.EQ
delim $$
.EN
.PP
A \fBHive\fR comprises one or more \fIservers\fR, each with a \fBService\fR governing
one or more \fBWorkers\fR, where $Workers <= Processors - 1$.
.PP
Each worker maintains the in-memory repository for one or more
columns.  It is single-threaded; it fetches requests from the socket on
which it listens \[em] known as its \fIfront door\fR \[em] and answers
via the same socket.  Requests that cannot be satisfied without a
blocking call to a system service are \*Qparked\*U; other requests are
serviced until notification arrives (via the front door) that the
parked connection is ready to be resumed.  
.PS
SERVER: box invis height 2.5 width 3.5
x = SERVER.nw.x - 0.128
y = SERVER.nw.y - 0.128
#line from SERVER.nw to SERVER.ne "\[lq]Server\[rq]" above
line from SERVER.nw to SERVER.ne "Server" above
line from SERVER.ne to SERVER.se
line from SERVER.sw to SERVER.se
boxht = 0.25
boxwid = 1
line down 1/8 from SERVER.nw; move down boxht
line down 1/8; move down boxht
line down 1/8; move down boxht 
line down 1/8; move down boxht 
line down to SERVER.sw
define thread {
THREAD:	box with .nw at ($1,$2) "$3"
		line left 1/8 from last box .w
SOCKET:	circle radius 1/32 with .e at last line .end
}
thread(x,y,Service); 
y = y - (boxht + 1/8)
thread(x,y,Worker); 

line right 1/8 up 3/16 dashed 1/32 from THREAD.e 
box height boxht/2  with .w at last line .end "\s-2Repository ...\s0"
line right 1/8             dashed 1/32 from THREAD.e 
box height boxht/2  with .w at last line .end "\s-2Repository ...\s0"
line right 1/8 down 3/16 dashed 1/32 from THREAD.e 
box height boxht/2  with .w at last line .end "\s-2Repository ...\s0"

y = y - (boxht + 1/8)
thread(x,y,Worker); 
y = y - (boxht + 1/8)
thread(x,y,Worker); 
y = y - (boxht + 1/8)

line left dashed from 2nd last circle.w
CLIENT: box height 1 with .e at last line .end
line invis right from CLIENT.nw to CLIENT.ne "Client" above
move right 1

LOGGER: box dashed height 3 * boxht with .nw at THREAD.ne
MEM: box dashed height 2 * boxht with .nw at LOGGER.ne  \
	"memory-" "\0mapped" "WAL image"
line dashed from THREAD.se  to last box .w
box invis with .sw at THREAD.se  "Logger"
FLUSH: box invis with .nw at THREAD.se  "Flusher" 
line dashed from last box .sw boxwid
CKPT: box invis with .nw at last box .sw   "Checkpointer"
line <-> right 1.25 with .start at MEM.e

define drum {
	ellipse height 1/8 width 1/2
	line down
	line down from last ellipse .w
BD:	arc right rad 1/2 to 2nd last line .end
	line invis from 2nd last line .c to last line .c $1 $2 $3
}

drum("\dWAL")

spline -> from CKPT.e right 1.25 then down then right 1.0

drum("\dColumn" "\dStore(s)")

SHADOW: ellipse invis height 1/8 width 1/2 with .w at last ellipse.n
arc cw dashed 1/32 right rad 1/2 from SHADOW.w to SHADOW.e 
arc cw dashed 1/32 left rad 1/2 to 2nd last ellipse .e
line dashed 1/32 down from SHADOW.e
arc cw dashed 1/32 left rad 1/2 to BD.end

arrowht=.075
line dashed from FLUSH.w left boxwid; arc -> cw dashed left to SOCKET.s
line <-dashed from CKPT.w   left boxwid; arc -> cw dashed left to SOCKET.s

arc <- cw up from LOGGER.n up 1/2
spline -> right 1.5 then up 1.5 then left 3.5 \
then down to 4th last circle .n
arc up rad 1/16 dashed 1/32 from 1/2 of the way between \
	LOGGER.nw and LOGGER.n
line left 1.25 dashed 1/32
arc -> dashed 1/32 right rad 1/16 to last circle .n
.PE
.PP
The
.B
Logger
.R
is an object instantiated by the worker.  It spawns two sub-processes,
each connected by a pipe: the \fBFlusher\fR and the
\fBCheckpointer\fR. 
.PP
To process a \fCwrite()\fR function call (invoked by the worker), the
logger updates the WAL memory image and
.I
does not call \fRthe
.B
Flusher
.R
Rather, it calls \fCmsync(2)\fR with the flag setting \fCMS_ASYNC\fR.
On Linux, this schedules the pages for synchronization with disk, cf. 
.I
The Linux Programming Interface\fR:
.QP
If we take no further action after an \fCMS_ASYNC\fR operation, then the
modified pages in the memory region will eventually be flushed as part
of the automatic buffer flushing performed by the \fIpdflush\fR kernel
thread....
.PP
The \fBFlusher\fR exists solely to handle an explict \fCFlush()\fR
call, provoked by a Flush request from the client. The Flusher calls
\fCmsync(2)\fR with the flag setting \fCMS_SYNC\fR.  When the call
returns, it posts a Flush-Done message to the Worker's front door.\**
.FS  
No attempt is made to flush \fIparts\fR of the WAL; a flush for any
column managed by a worker is, in effect, a flush for all of them.  
.FE
.PP
The
.B
Checkpointer
.R
listens on its pipe for explicit Checkpoint requests from the
Logger, provoked by one of:
.RS
.IP \(bu
explicit Checkpoint requests from the client
.IP \(bu
\fIcheckpoint pressure\fR detected by the Logger  
.IP \(bu
a user-defined timeout, subject to \fBServer\fR idleness
.RE
.PP
The Checkpointer never writes to Logger memory.  Rather, it
communicates with the Logger via a pipe.  Explicit and
pressure-induced checkpoint requests are written to the Checkpointer
pipe; timeouts occur if no explicit requests arrive.  
.PP
Checkpoint pressure is a function unapplied non-inserts \[em] number,
size, and age \[em] plus Hive activity. How exactly these
combine to provoke a \*Qcheckpoint situation\*U is unclear at
this time, but we know that algorithm will belong to the Logger,
because it has the most information about the WAL status and can use
the Worker's connection to the Service to obtain Hive status. 
.PP
On completion (regardless of why) the Checkpointer will have changed
the name of the new column store (and the status of the old one), as
well as the last checkpointed revision for the column.  To avoid
entangling the Worker uselessly, that information is posted to the
Logger-Checkpointer pipe, where in due time the Logger will read it
and update its books, notifying the Worker if appropriate.  This
suggests that the prelude in every Logger function body include a loop
to process completion status messages on the Checkpointer pipe.\**
.FS
N.B. Fancier arrangements could be used: the Checkpointer could update the
Logger's memory, or the Logger could use a signal handler to read the
completion status messages.  These both introduce volatility to the
Logger's memory, and injure the worker's single-threaded cache.  By
checking the status messages at the start of each function call, we
keep the Logger single-threaded and avoid a set of race conditions.  
.FE
The Logger notifies the Worker via the front door of completed
explicit checkpoint requests. 
.PP
To move data from the Worker's repository to the on-disk column store,
the Checkpointer acts much as a client: it issues a range request
(with a for-checkpoint indicator) for all rows in the repository.  The
Worker responds by first freezing the repository and then, per normal,
sending sets of rows out its front door.  
.SH
Notes
.LP
Each worker is responsible for N column stores, each cached in a Repository.  
.LP
Each worker uses a Wal object to capture transactions to a log.  The
Wal object updates the WAL as a memory-mapped file.  On construction
the Wal object creates a separate thread of control, known as the
Flusher, and a pipe to communicate with it.  The Flusher blocks
against a read to the pipe.
.LP
When the client sends a Change set to the worker, the work calls
Wal::Write, which reacts as follows:
.nr TE 0 1
.IP \n+(TE.
Update the memory-mapped WAL image
.IP \n+(TE.
Return OK to the worker
.LP
When the client sends a Flush request to the worker, the worker calls
Wal::Write(), parks the connection, and returns.  Wal::Write acts as
follows:
.nr TE 0 1
.IP \n+(TE.
Write a \fIsync\fR request to the Flusher pipe
.IP \n+(TE.
Return OK to the worker
.IP \n+(TE.
Flusher reads sync request
.IP \n+(TE.
Flusher calls OS sync function e.g. \fBmsync(2)\fR and blocks
.IP \n+(TE.
Flusher sends Flush_OK to the worker's socket
.LP
On receipt of Flush_OK, the worker unparks the connection and returns OK to the Client. 
.LP
The Flusher is required to sync the memory-mapped image periodically, perhaps every 500 milliseconds.  (Tunable.  Another option might be every N pages or 30 seconds, whichever came first.)  The is effected by a timeout on reading the pipe using e.g. \fBpoll(2)\fR. On timeout, the Flusher reacts as to a Flush request, except that no notification is sent to the worker.  

