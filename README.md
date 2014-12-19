Fastore
=======

NewSQL database management system based on a columnar scale-out topology.  By embracing modern hardware design, Fastore provides ultra-high transaction and query throughput.  

Features include:
  * Scale out - add more "cheap" nodes for redundancy and faster performance
  * Primary in-memory, but durable
  * For both analytics and transaction processing
  * Optimistic and pessimistic concurrency
  * Supports Windows and Linux
  * Accessible via C, C++, .NET, and Java APIs

Technical Details:
* Since Fastore uses columnar storage, no indexes are needed, the join cost is even across columns and tables, and there is a high potential for analytic queries.
* Fastore is able to store data in-memory as it utilizes memory-optimized structures; this enables durable logging and check-pointing and support for joins using random access.
* Transactions can satisfy the ACID properties, with isolation via revisions and true durability with torn-page handling.  Fastore uses a consistency model to facilitate backup and recovery.
* Fastore supports optimistic concurrency - the application server handles the bulk of the transaction processing, conflicts result in retry, and snapshot isolation and early conflict detection can prevent data inconsistencies or failure. Pessimistic concurrency is also used in explicit locking for high contention points.
* Fastore's ability to replicate its network topology across several servers allows for multi-site distribution, and thus prevents the need to scale-up. This also means there will be no single point of contention or failure, so clients can coordinate the transactions.
* No latch overhead and maximized cache locality make Fastore optimal for NUMA memory architecture and disk throughput.

Implementation details:
* Written primarily in C++ 11
* Uses SQLite's query engine, which was well suited given that the query processor resides on the "client" (typically an application server, but client relative to the database system).

Status:
* Reading is primarily in place
* Transactions are partly implemented

See [Contributing](https://github.com/n8allan/Fastore/wiki/Contributing) for information on how and where to help out.
More information here: http://www.digithought.com/products-services/fastore
