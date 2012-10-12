create table CUSTOMER (
	C_CUSTKEY	integer,
	C_NAME		varchar(25),
	C_ADDRESS	varchar(40),
	C_NATIONKEY	integer,
	C_PHONE		character(15),
	C_ACCTBAL	numeric (20,2),
	C_MKTSEGMENT	character(10),
	C_COMMENT	varchar(117),
	primary key (C_CUSTKEY)
)
;

create table HISTORY (
	H_P_KEY integer,
	H_S_KEY integer,
	H_O_KEY integer,
	H_L_KEY integer,
	H_DELTA integer,
	H_DATE_T datetime
)
;

create table LINEITEM (
	L_ORDERKEY	integer,
	L_PARTKEY	integer,
	L_SUPPKEY	integer,
	L_LINENUMBER	integer,
	L_QUANTITY	numeric (20,2),
	L_EXTENDEDPRICE	numeric (20,2),
	L_DISCOUNT	numeric (3,2),
	L_TAX		numeric (3,2),
	L_RETURNFLAG	character(1),
	L_LINESTATUS	character(1),
	L_SHIPDATE	date,
	L_COMMITDATE	date,
	L_RECEIPTDATE	date,
	L_SHIPINSTRUCT	character(25),
	L_SHIPMODE	character(10),
	L_COMMENT	varchar(44),
	primary key (L_ORDERKEY, L_LINENUMBER)
)
;

create table NATION (
	N_NATIONKEY	integer,
	N_NAME		character(25),
	N_REGIONKEY	integer,
	N_COMMENT	varchar(152),
	primary key (N_NATIONKEY)
)
;

create table ORDERS (
	O_ORDERKEY	integer,
	O_CUSTKEY	integer,
	O_ORDERSTATUS	character(1),
	O_TOTALPRICE	numeric (20,2),
	O_ORDERDATE	date,
	O_ORDERPRIORITY	character(15),
	O_CLERK		character(15),
	O_SHIPPRIORITY	integer,
	O_COMMENT	varchar(79),
	primary key(O_ORDERKEY)
)
;

create table PART (
	P_PARTKEY	integer,
	P_NAME		varchar(55),
	P_MFGR		character(25),
	P_BRAND		character(10),
	P_TYPE		varchar(25),
	P_SIZE		integer,
	P_CONTAINER	character(10),
	P_RETAILPRICE	numeric (20,2),
	P_COMMENT	varchar(23),
	primary key (P_PARTKEY)
)
;

create table PARTSUPP (
	PS_PARTKEY	integer,
	PS_SUPPKEY	integer,
	PS_AVAILQTY	integer,
	PS_SUPPLYCOST	numeric (20,2),
	PS_COMMENT	varchar(199),
	primary key (PS_PARTKEY, PS_SUPPKEY)
)
;

create table REGION (
	R_REGIONKEY	integer,
	R_NAME		character(25),
	R_COMMENT	varchar(152),
	primary key (R_REGIONKEY)
)
;

create table SUPPLIER (
	S_SUPPKEY	integer,
	S_NAME		character(25),
	S_ADDRESS	varchar(40),
	S_NATIONKEY	integer,
	S_PHONE		character(15),
	S_ACCTBAL	numeric (20,2),
	S_COMMENT	varchar(101),
	primary key (S_SUPPKEY)
)
;