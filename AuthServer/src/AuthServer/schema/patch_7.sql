--
-- AJackson Oct. 8, 2008
--

SET QUOTED_IDENTIFIER OFF 
GO
SET ANSI_NULLS ON 
GO

-- adding in a new column to record kind of CD being used for a product
ALTER TABLE [dbo].[auth_activity_log]
	ADD [cd_kind] [int] NOT NULL DEFAULT 0
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

-- Adding the operating_system param into the INSERT

CREATE PROCEDURE [dbo].[sp_LogAuthActivity] 
	@account_name char(14),
	@uid int,
	@serverid int,
	@client_ip varchar(20),
	@auth_login_time datetime,
	@queue_login_time datetime,
	@game_login_time datetime,
	@game_logout_time datetime,
	@game_logout_type char,
    @cd_kind tinyint
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
	game_logout_type,
    cd_kind
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
	@game_logout_type,
    @cd_kind
)
GO

SET QUOTED_IDENTIFIER OFF 
GO
SET ANSI_NULLS ON 
GO
