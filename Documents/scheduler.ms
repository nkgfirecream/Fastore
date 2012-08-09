.DA $Id$
.NP
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
.PS
SERVER: box invis height 2 width 3.5
x = SERVER.nw.x - 0.128
y = SERVER.nw.y - 0.128
line from SERVER.nw to SERVER.ne "\[lq]Server\[rq]" above
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
		circle radius 1/32 with .e at last line .end
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

box dashed height 1/2 with .nw at THREAD.ne
line dashed from last box .w to last box .e "Logger" "\dFlusher"

box dashed height 1/2 width 3/4 with .nw at last box .ne  "memory-" "\0mapped" "WAL image"
line <-> right 1.5 
ellipse height 1/8 width 1/2
line down
line down from last ellipse .w
arc right rad 1/2 to 2nd last line .end
line invis from 2nd last line .c to last line .c "\dWAL" 

.PE
.na
.SH
Notes
.LP
Each worker is responsible for N column stores, each cached in a Repository.  
.LP
Each worker uses a Wal object to capture transactions to a log.  The Wal object updates the WAL as a memory-mapped file.  On construction the Wal object creates a separate thread of control, known as the Flusher, and a pipe to communicate with it.  The Flusher blocks against a read to the pipe.  
.LP
When the client sends a Change set to the worker, the work calls Wal::Write, which reacts as follows:
.nr TE 0 1
.IP \n+(TE.
Update the memory-mapped WAL image
.IP \n+(TE.
Return OK to the worker
.LP
When the client sends a Flush request to the worker, the worker calls Wal::Write(), parks the connection, and returns.  Wal::Write acts as follows:
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

