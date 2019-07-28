CREATE TABLE config ( 
    id          INTEGER        PRIMARY KEY
                               NOT NULL
                               UNIQUE,
    active      BOOLEAN        NOT NULL,
    remote_ip   VARCHAR( 16 )  NOT NULL,
    remote_port INTEGER        NOT NULL,
    server_ip   VARCHAR( 16 )  NOT NULL,
    server_port INTEGER        NOT NULL 
);

INSERT INTO [config] VALUES (1, 0, '65.181.125.136', 80, '172.30.208.102', 31416);
INSERT INTO [config] VALUES (2, 1, '127.0.0.1', 80, '192.168.100.200', 31416);

CREATE TABLE coupons ( 
    name        VARCHAR( 16 )  PRIMARY KEY
                               NOT NULL
                               UNIQUE,
    rewardgroup VARCHAR( 16 )  NOT NULL,
    uses        INTEGER        NOT NULL,
    reward      VARCHAR( 16 )  NOT NULL,
    random      INTEGER        NOT NULL,
    daily       BOOLEAN        NOT NULL,
    expiration  DATE           NOT NULL 
);

INSERT INTO [coupons] VALUES ('EARLYBIRDRESPEC', 'EARLYBIRD*', 100, 'RESPEC', 0, 0, '2014-12-05');
INSERT INTO [coupons] VALUES ('EARLYBIRDMERITS', 'EARLYBIRD*', 100, 'MERITS25', 0, 0, '2014-12-05');
INSERT INTO [coupons] VALUES ('EARLYBIRDPACK', 'EARLYBIRD*', 100, 'SUPERPACK', 3, 0, '2014-12-05');
INSERT INTO [coupons] VALUES ('DAILYTAILOR', 'DAILYTAILOR', 10000, 'TAILOR', 0, 1, '2014-12-05');

CREATE TABLE rewards ( 
    reward  TEXT    NOT NULL,
    idx     INTEGER NOT NULL,
    sku     TEXT    NOT NULL,
    amount  INTEGER NOT NULL,
    message TEXT    NOT NULL,
    PRIMARY KEY ( reward, idx ) 
);

INSERT INTO [rewards] VALUES ('SUPERPACK', 0, 'COSPHEVI', 1, '<img src="http://i.imgur.com/0N9LpRL.png"><br><br>You received a Super Pack: Heroes and Villains!');
INSERT INTO [rewards] VALUES ('SUPERPACK', 1, 'COSPROVI', 1, '<img src="http://i.imgur.com/9yYRPS2.png"><br><br>You received a Super Pack: Rogues and Vigilantes!');
INSERT INTO [rewards] VALUES ('SUPERPACK', 2, 'COWPWIWA', 1, '<img src="http://i.imgur.com/pqtfFyK.png"><br><br>You received a Super Pack: Lords of Winter!');
INSERT INTO [rewards] VALUES ('RESPEC', 0, 'SVRESPEC', 1, '<img src="http://i.imgur.com/UIMxuEF.png"><br><br>You received a Character Respec!');
INSERT INTO [rewards] VALUES ('TAILOR', 0, 'SVCOSTUM', 1, '<img src="http://i.imgur.com/QSNJmzv.png"><br><br>You received a Free Tailor Session!');
INSERT INTO [rewards] VALUES ('MERITS25', 0, 'COBOSPRM', 1, '<img src="http://i.imgur.com/vy0syNY.png"><br><br>You received 25 Reward Merits!');

CREATE TABLE ledger ( 
    authid      INTEGER        NOT NULL,
    rewardgroup VARCHAR( 16 )  NOT NULL,
    applied     DATE           NOT NULL,
    message     TEXT           NOT NULL,
    PRIMARY KEY ( authid, rewardgroup ) 
);

CREATE TABLE errors ( 
    idx      INTEGER        PRIMARY KEY AUTOINCREMENT
                            NOT NULL
                            UNIQUE,
    authid   INTEGER,
    coupon   VARCHAR( 16 ),
    message  TEXT,
    datetime DATETIME 
);

