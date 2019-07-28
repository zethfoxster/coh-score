ALTER TABLE [dbo].[user_account]
	ADD [queue_level] [int] NOT NULL
	CONSTRAINT [DF_user_account_queue_level] DEFAULT (1)
GO

ALTER TABLE [user_auth]
ADD hash_type tinyint DEFAULT 0 NOT NULL,
    salt int DEFAULT 0 NOT NULL
GO
ALTER TABLE [user_auth]
ALTER COLUMN password binary(128) NOT NULL
GO
ALTER TABLE [user_auth]
ALTER COLUMN answer1 binary(128) NOT NULL
GO
ALTER TABLE [user_auth]
ALTER COLUMN answer2 binary(128) NOT NULL
GO


IF EXISTS (SELECT * FROM dbo.sysobjects WHERE name = 'ap_GPwd' AND xtype = 'P')
BEGIN
    DROP PROCEDURE [dbo].[ap_GPwd]
END
GO

IF EXISTS (SELECT * FROM dbo.sysobjects WHERE name = 'ap_GStat' AND xtype = 'P')
BEGIN
    DROP PROCEDURE [dbo].[ap_GStat]
END
GO

SET QUOTED_IDENTIFIER OFF 
GO
SET ANSI_NULLS OFF 
GO

CREATE PROCEDURE [dbo].[ap_GPwd]  @account varchar(14), @pwd binary(128) output, @hash_type tinyint output, @salt int output
AS
SELECT @pwd=password, @hash_type=hash_type, @salt=salt FROM user_auth with (nolock) WHERE account=@account
GO

CREATE PROCEDURE [dbo].[ap_GStat]
@account varchar(14), 
@uid int OUTPUT, 
@payStat int OUTPUT, 
@loginFlag int OUTPUT, 
@warnFlag int OUTPUT, 
@blockFlag int OUTPUT, 
@blockFlag2 int OUTPUT, 
@subFlag int OUTPUT, 
@lastworld tinyint OUTPUT,
@block_end_date datetime OUTPUT,
@queue_level int OUTPUT
 AS
SELECT @uid=uid, 
	@payStat=pay_stat,
	@loginFlag = login_flag, 
	@warnFlag = warn_flag, 
	@blockFlag = block_flag, 
	@blockFlag2 = block_flag2, 
	@subFlag = subscription_flag , 
	@lastworld=last_world, 
	@block_end_date=block_end_date,
	@queue_level=queue_level
	FROM user_account WITH (nolock)
WHERE account=@account
GO

SET QUOTED_IDENTIFIER OFF 
GO
SET ANSI_NULLS ON 
GO

ALTER TABLE [dbo].[auth_activity_log]
	ADD [queue_login_time] [datetime] NULL
GO


IF EXISTS (SELECT * FROM dbo.sysobjects WHERE name = 'sp_LogAuthActivity' AND xtype = 'P')
BEGIN
    DROP PROCEDURE [dbo].[sp_LogAuthActivity]
END
GO

SET QUOTED_IDENTIFIER OFF 
GO
SET ANSI_NULLS OFF 
GO

CREATE PROCEDURE [dbo].[sp_LogAuthActivity] 
	@account_name char(14),
	@uid int,
	@serverid int,
	@client_ip varchar(20),
	@auth_login_time datetime,
	@queue_login_time datetime,
	@game_login_time datetime,
	@game_logout_time datetime,
	@game_logout_type char
AS
INSERT INTO auth_activity_log
(
	account_name,
	uid,
	serverid,
	client_ip,
	auth_login_time,
	queue_login_time,
	game_login_time,
	game_logout_time,
	game_logout_type
)
VALUES
(
	@account_name,
	@uid,
	@serverid,
	@client_ip,
	@auth_login_time,
	@queue_login_time,
	@game_login_time,
	@game_logout_time,
	@game_logout_type
)
GO

SET QUOTED_IDENTIFIER OFF 
GO
SET ANSI_NULLS ON 
GO
