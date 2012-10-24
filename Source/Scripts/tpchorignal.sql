-- Sccsid:     @(#)dss.ddl	2.1.8.1
CREATE TABLE NATION  ( N_NATIONKEY  INTEGER NOT NULL,
                            N_NAME       CHAR(25) NOT NULL,
                            N_REGIONKEY  INTEGER NOT NULL,
                            N_COMMENT    VARCHAR(152));

CREATE TABLE REGION  ( R_REGIONKEY  INTEGER NOT NULL,
                            R_NAME       CHAR(25) NOT NULL,
                            R_COMMENT    VARCHAR(152));

CREATE TABLE PART  ( P_PARTKEY     INTEGER NOT NULL,
                          P_NAME        VARCHAR(55) NOT NULL,
                          P_MFGR        CHAR(25) NOT NULL,
                          P_BRAND       CHAR(10) NOT NULL,
                          P_TYPE        VARCHAR(25) NOT NULL,
                          P_SIZE        INTEGER NOT NULL,
                          P_CONTAINER   CHAR(10) NOT NULL,
                          P_RETAILPRICE DECIMAL(15,2) NOT NULL,
                          P_COMMENT     VARCHAR(23) NOT NULL );

CREATE TABLE SUPPLIER ( S_SUPPKEY     INTEGER NOT NULL,
                             S_NAME        CHAR(25) NOT NULL,
                             S_ADDRESS     VARCHAR(40) NOT NULL,
                             S_NATIONKEY   INTEGER NOT NULL,
                             S_PHONE       CHAR(15) NOT NULL,
                             S_ACCTBAL     DECIMAL(15,2) NOT NULL,
                             S_COMMENT     VARCHAR(101) NOT NULL);

CREATE TABLE PARTSUPP ( PS_PARTKEY     INTEGER NOT NULL,
                             PS_SUPPKEY     INTEGER NOT NULL,
                             PS_AVAILQTY    INTEGER NOT NULL,
                             PS_SUPPLYCOST  DECIMAL(15,2)  NOT NULL,
                             PS_COMMENT     VARCHAR(199) NOT NULL );

CREATE TABLE CUSTOMER ( C_CUSTKEY     INTEGER NOT NULL,
                             C_NAME        VARCHAR(25) NOT NULL,
                             C_ADDRESS     VARCHAR(40) NOT NULL,
                             C_NATIONKEY   INTEGER NOT NULL,
                             C_PHONE       CHAR(15) NOT NULL,
                             C_ACCTBAL     DECIMAL(15,2)   NOT NULL,
                             C_MKTSEGMENT  CHAR(10) NOT NULL,
                             C_COMMENT     VARCHAR(117) NOT NULL);

CREATE TABLE ORDERS  ( O_ORDERKEY       INTEGER NOT NULL,
                           O_CUSTKEY        INTEGER NOT NULL,
                           O_ORDERSTATUS    CHAR(1) NOT NULL,
                           O_TOTALPRICE     DECIMAL(15,2) NOT NULL,
                           O_ORDERDATE      DATE NOT NULL,
                           O_ORDERPRIORITY  CHAR(15) NOT NULL,  
                           O_CLERK          CHAR(15) NOT NULL, 
                           O_SHIPPRIORITY   INTEGER NOT NULL,
                           O_COMMENT        VARCHAR(79) NOT NULL);

CREATE TABLE LINEITEM ( L_ORDERKEY    INTEGER NOT NULL,
                             L_PARTKEY     INTEGER NOT NULL,
                             L_SUPPKEY     INTEGER NOT NULL,
                             L_LINENUMBER  INTEGER NOT NULL,
                             L_QUANTITY    DECIMAL(15,2) NOT NULL,
                             L_EXTENDEDPRICE  DECIMAL(15,2) NOT NULL,
                             L_DISCOUNT    DECIMAL(15,2) NOT NULL,
                             L_TAX         DECIMAL(15,2) NOT NULL,
                             L_RETURNFLAG  CHAR(1) NOT NULL,
                             L_LINESTATUS  CHAR(1) NOT NULL,
                             L_SHIPDATE    DATE NOT NULL,
                             L_COMMITDATE  DATE NOT NULL,
                             L_RECEIPTDATE DATE NOT NULL,
                             L_SHIPINSTRUCT CHAR(25) NOT NULL,
                             L_SHIPMODE     CHAR(10) NOT NULL,
                             L_COMMENT      VARCHAR(44) NOT NULL);
							 
							 
							 
							 
CONNECT TO TPCD;

--ALTER TABLE TPCD.REGION DROP PRIMARY KEY;
--ALTER TABLE TPCD.NATION DROP PRIMARY KEY;
--ALTER TABLE TPCD.PART DROP PRIMARY KEY;
--ALTER TABLE TPCD.SUPPLIER DROP PRIMARY KEY;
--ALTER TABLE TPCD.PARTSUPP DROP PRIMARY KEY;
--ALTER TABLE TPCD.ORDERS DROP PRIMARY KEY;
--ALTER TABLE TPCD.LINEITEM DROP PRIMARY KEY;
--ALTER TABLE TPCD.CUSTOMER DROP PRIMARY KEY;


-- For table REGION
ALTER TABLE TPCD.REGION
ADD PRIMARY KEY (R_REGIONKEY);

-- For table NATION
ALTER TABLE TPCD.NATION
ADD PRIMARY KEY (N_NATIONKEY);

ALTER TABLE TPCD.NATION
ADD FOREIGN KEY NATION_FK1 (N_REGIONKEY) references TPCD.REGION;

COMMIT WORK;

-- For table PART
ALTER TABLE TPCD.PART
ADD PRIMARY KEY (P_PARTKEY);

COMMIT WORK;

-- For table SUPPLIER
ALTER TABLE TPCD.SUPPLIER
ADD PRIMARY KEY (S_SUPPKEY);

ALTER TABLE TPCD.SUPPLIER
ADD FOREIGN KEY SUPPLIER_FK1 (S_NATIONKEY) references TPCD.NATION;

COMMIT WORK;

-- For table PARTSUPP
ALTER TABLE TPCD.PARTSUPP
ADD PRIMARY KEY (PS_PARTKEY,PS_SUPPKEY);

COMMIT WORK;

-- For table CUSTOMER
ALTER TABLE TPCD.CUSTOMER
ADD PRIMARY KEY (C_CUSTKEY);

ALTER TABLE TPCD.CUSTOMER
ADD FOREIGN KEY CUSTOMER_FK1 (C_NATIONKEY) references TPCD.NATION;

COMMIT WORK;

-- For table LINEITEM
ALTER TABLE TPCD.LINEITEM
ADD PRIMARY KEY (L_ORDERKEY,L_LINENUMBER);

COMMIT WORK;

-- For table ORDERS
ALTER TABLE TPCD.ORDERS
ADD PRIMARY KEY (O_ORDERKEY);

COMMIT WORK;

-- For table PARTSUPP
ALTER TABLE TPCD.PARTSUPP
ADD FOREIGN KEY PARTSUPP_FK1 (PS_SUPPKEY) references TPCD.SUPPLIER;

COMMIT WORK;

ALTER TABLE TPCD.PARTSUPP
ADD FOREIGN KEY PARTSUPP_FK2 (PS_PARTKEY) references TPCD.PART;

COMMIT WORK;

-- For table ORDERS
ALTER TABLE TPCD.ORDERS
ADD FOREIGN KEY ORDERS_FK1 (O_CUSTKEY) references TPCD.CUSTOMER;

COMMIT WORK;

-- For table LINEITEM
ALTER TABLE TPCD.LINEITEM
ADD FOREIGN KEY LINEITEM_FK1 (L_ORDERKEY)  references TPCD.ORDERS;

COMMIT WORK;

ALTER TABLE TPCD.LINEITEM
ADD FOREIGN KEY LINEITEM_FK2 (L_PARTKEY,L_SUPPKEY) references 
        TPCD.PARTSUPP;

COMMIT WORK;


