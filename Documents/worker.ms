.\" Worker state machine diagram, (working draft)
.\" $Id$
.PS
circlerad = 1/2
arcrad = 1/2

CLIENT: circle "client"
arrow right 1.0 "apply" "prepare" "\0" "commit" "flush"

READ: circle "worker" "read"

arrow        right   from READ.e
PREPARED: circle "prepared"
arrow right  "TID" above
DONE: circle "done"

arrow up 2   right 1 from READ.n
circle "applied"
arrow from last circle to DONE chop "\0revision list" ljust
arrow dashed right from last circle .e
circle dashed "parked!"
arc -> cw dashed from PREPARED.n to last circle .sw

arrow down 1 right 5/8 from READ.se
circle "committed"
arrow from last circle to DONE chop

arrow down 0.5 from last circle .s
circle "flushed"
arrow from READ to last circle chop
arrow from last circle to DONE chop

arrow right from last circle.e
circle "flush done" "(told worker)"
arrow from last circle to DONE chop


arrow down 4 right 1 from READ.s
circle "flush" "confirmed" "(told client)"
spline -> right 2 then up 2  then to DONE.se

.PE
.bp
.SH 
Worker State Machine
.IP \(sc
Client sends a message, one of apply, prepare, commit, or flush.  
.IP \(sc
Prepare 
.RS
.IP \(bu
(client generates TID)
.IP \(bu
(client \*Qaligns reads\*U including dependent revisions, sends Prepare)
.IP \(bu
check revisions, error if input too old
.IP \(bu
request deltas from logger, park. How to unpark? 
.IP \(bu
verify deltas for conflict
.IP \(bu
log transaction
.IP \(bu 
proceed as with Apply
.RE
.IP \(sc
Apply: return revision list or, if transaction exists, park.  
How to unpark? 
.IP \(sc
Commit
.RS
.IP \(bu
log transaction, queue lazy flush
.IP \(bu
update repository
.IP \(bu
close transaction flag
.RE
.IP \(sc
Flush
.RS
.IP \(bu
post flush request to logger, return
.IP \(bu
logger posts flush confirmation when WAL is sync'ed
.RE
.IP \(sc
Flush Done (confirmation), respond to client
.sp
.SH Thoughts, maybe
.LP
How to unpark Apply? 
.LP
Revisor is asynchronous, which means deltas have to be written to the Worker socket.  Suggest making this synchronous for now because WAL is in memory.  Might be faster anyway.  
.LP
N.B. Above diagram departs slightly from Nate's description on 13 August wrt worker-logger interaction. 

