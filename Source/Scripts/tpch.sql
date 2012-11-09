create table NATION (
N_NATIONKEY int primary key,
N_NAME varchar not null,
N_REGIONKEY int not null,
N_COMMENT varchar
);

create table REGION (
R_REGIONKEY int primary key,
R_NAME varchar not null,
R_COMMENT varchar
);

create table PART (
P_PARTKEY int primary key,
P_NAME varchar not null,
P_MFGR varchar not null,
P_BRAND varchar not null,
P_TYPE varchar not null,
P_SIZE int not null,
P_CONTAINER varchar not null,
P_RETAILPRICE float not null,
P_COMMENT varchar not null
);

create table SUPPLIER (
S_SUPPKEY int primary key,
S_NAME varchar not null,
S_ADDRESS varchar not null,
S_NATIONKEY int not null,
S_PHONE varchar not null,
S_ACCTBAL float not null,
S_COMMENT varchar not null
);

create table PARTSUPP (
PS_PARTKEY int not null,
PS_SUPPKEY int not null,
PS_AVAILQTY int not null,
PS_SUPPLYCOST float not null,
PS_COMMENT varchar not null
);

create table CUSTOMER (
C_CUSTKEY int primary key,
C_NAME varchar not null,
C_ADDRESS varchar not null,
C_NATIONKEY int not null,
C_PHONE varchar not null,
C_ACCTBAL float not null,
C_MKTSEGMENT varchar not null,
C_COMMENT varchar not null
);

create table ORDERS (
O_ORDERKEY int primary key,
O_CUSTKEY int not null,
O_ORDERSTATUS varchar not null,
O_TOTALPRICE float not null,
O_ORDERDATE date not null,
O_ORDERPRIORITY varchar not null,
O_CLERK varchar not null,
O_SHIPPRIORITY int not null,
O_COMMENT varchar not null
);

create table LINEITEM (
L_ORDERKEY int not null,
L_PARTKEY int not null,
L_SUPPKEY int not null,
L_LINENUMBER int not null,
L_QUANTITY float not null,
L_EXTENDEDPRICE float not null,
L_DISCOUNT float not null,
L_TAX float not null,
L_RETURNFLAG varchar not null,
L_LINESTATUS varchar not null,
L_SHIPDATE date not null,
L_COMMITDATE date not null,
L_RECEIPTDATE date not null,
L_SHIPINSTRUCT varchar not null,
L_SHIPMODE varchar not null,
L_COMMENT varchar not null
);