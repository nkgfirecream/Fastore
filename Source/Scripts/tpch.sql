create table CUSTOMER (
C_CUSTKEY int primary key,
C_NAME varchar,
C_ADDRESS varchar,
C_NATIONKEY int,
C_PHONE varchar,
C_ACCTBAL float,
C_MKTSEGMENT varchar,
C_COMMENT varchar,
);

create table HISTORY (
H_P_KEY int,
H_S_KEY int,
H_O_KEY int,
H_L_KEY int,
H_DELTA int,
H_DATE_T datetime
);

create table LINEITEM (
L_ORDERKEY int,
L_PARTKEY int,
L_SUPPKEY int,
L_LINENUMBER int,
L_QUANTITY float,
L_EXTENDEDPRICE float,
L_DISCOUNT float,
L_TAX float,
L_RETURNFLAG varchar,
L_LINESTATUS varchar,
L_SHIPDATE date,
L_COMMITDATE date,
L_RECEIPTDATE date,
L_SHIPINSTRUCT varchar,
L_SHIPMODE varchar,
L_COMMENT varchar,
);

create table NATION (
N_NATIONKEY int primary key,
N_NAME varchar,
N_REGIONKEY int,
N_COMMENT varchar,
);

create table ORDERS (
O_ORDERKEY int primary key,
O_CUSTKEY int,
O_ORDERSTATUS varchar,
O_TOTALPRICE float,
O_ORDERDATE date,
O_ORDERPRIORITY varchar,
O_CLERK varchar,
O_SHIPPRIORITY int,
O_COMMENT varchar,
);

create table PART (
P_PARTKEY int primary key,
P_NAME varchar,
P_MFGR varchar,
P_BRAND varchar,
P_TYPE varchar,
P_SIZE int,
P_CONTAINER varchar,
P_RETAILPRICE float,
P_COMMENT varchar,
);

create table PARTSUPP (
PS_PARTKEY int,
PS_SUPPKEY int,
PS_AVAILQTY int,
PS_SUPPLYCOST float,
PS_COMMENT varchar,
);

create table REGION (
R_REGIONKEY int primary key,
R_NAME varchar,
R_COMMENT varchar,
);

create table SUPPLIER (
S_SUPPKEY int primary key,
S_NAME varchar,
S_ADDRESS varchar,
S_NATIONKEY int,
S_PHONE varchar,
S_ACCTBAL float,
S_COMMENT varchar,
);