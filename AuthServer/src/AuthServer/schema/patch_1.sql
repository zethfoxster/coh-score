IF EXISTS (SELECT * FROM dbo.sysobjects WHERE name = 'sp_LogAuthActivity' AND xtype = 'P')
BEGIN
    DROP PROCEDURE [dbo].[sp_LogAuthActivity]
END
GO

ALTER TABLE [dbo].[auth_activity_log] add [queue_login_time] [datetime] NULL
GO

ALTER TABLE [dbo].[user_account] ADD [queue_level] [int] NOT NULL DEFAULT 1
GO

--******************************************************---
-- Create procedure sp_LogAuthActivity
--
--******************************************************---
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