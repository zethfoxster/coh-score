--******************************************************---
-- drop all existing procs if they exist
--
--******************************************************---
IF EXISTS (SELECT * FROM dbo.sysobjects WHERE name = 'sp_ClearAllTestAccounts' AND xtype = 'P')
BEGIN
    DROP PROCEDURE [dbo].[sp_ClearAllTestAccounts]
END
GO

IF EXISTS (SELECT * FROM dbo.sysobjects WHERE name = 'sp_ClearSingleTestAccount' AND xtype = 'P')
BEGIN
    DROP PROCEDURE [dbo].[sp_ClearSingleTestAccount]
END
GO

IF EXISTS (SELECT * FROM dbo.sysobjects WHERE name = 'sp_CreateMultipleTestAccounts' AND xtype = 'P')
BEGIN
    DROP PROCEDURE [dbo].[sp_CreateMultipleTestAccounts]
END
GO

IF EXISTS (SELECT * FROM dbo.sysobjects WHERE name = 'sp_CreateSingleTestAccount' AND xtype = 'P')
BEGIN
    DROP PROCEDURE [dbo].[sp_CreateSingleTestAccount]
END
GO

IF EXISTS (SELECT * FROM dbo.sysobjects WHERE name = 'sp_DisableGM' AND xtype = 'P')
BEGIN
    DROP PROCEDURE [dbo].[sp_DisableGM]
END
GO

IF EXISTS (SELECT * FROM dbo.sysobjects WHERE name = 'sp_EnableGm' AND xtype = 'P')
BEGIN
    DROP PROCEDURE [dbo].[sp_EnableGm]
END
GO

IF EXISTS (SELECT * FROM dbo.sysobjects WHERE name = 'sp_GetBlockMessages' AND xtype = 'P')
BEGIN
    DROP PROCEDURE [dbo].[sp_GetBlockMessages]
END
GO

IF EXISTS (SELECT * FROM dbo.sysobjects WHERE name = 'sp_GetServers' AND xtype = 'P')
BEGIN
    DROP PROCEDURE [dbo].[sp_GetServers]
END
GO

IF EXISTS (SELECT * FROM dbo.sysobjects WHERE name = 'sp_GetUserData' AND xtype = 'P')
BEGIN
    DROP PROCEDURE [dbo].[sp_GetUserData]
END
GO

IF EXISTS (SELECT * FROM dbo.sysobjects WHERE name = 'sp_GetUserGameInfo' AND xtype = 'P')
BEGIN
    DROP PROCEDURE [dbo].[sp_GetUserGameInfo]
END
GO

IF EXISTS (SELECT * FROM dbo.sysobjects WHERE name = 'sp_LogAuthActivity' AND xtype = 'P')
BEGIN
    DROP PROCEDURE [dbo].[sp_LogAuthActivity]
END
GO

IF EXISTS (SELECT * FROM dbo.sysobjects WHERE name = 'sp_LogUserNumbers' AND xtype = 'P')
BEGIN
    DROP PROCEDURE [dbo].[sp_LogUserNumbers]
END
GO

IF EXISTS (SELECT * FROM dbo.sysobjects WHERE name = 'sp_UserLogin' AND xtype = 'P')
BEGIN
    DROP PROCEDURE [dbo].[sp_UserLogin]
END
GO

IF EXISTS (SELECT * FROM dbo.sysobjects WHERE name = 'sp_UserLogout' AND xtype = 'P')
BEGIN
    DROP PROCEDURE [dbo].[sp_UserLogout]
END
GO

IF EXISTS (SELECT * FROM dbo.sysobjects WHERE name = 'sp_WriteUserData' AND xtype = 'P')
BEGIN
    DROP PROCEDURE [dbo].[sp_WriteUserData]
END
GO


--******************************************************---
-- Create procedure sp_ClearAllTestAccounts
--
--******************************************************---
CREATE PROCEDURE [dbo].[sp_ClearAllTestAccounts]
AS

BEGIN TRANSACTION

DELETE FROM user_data WHERE uid IN
	(SELECT uid
  	 FROM user_account 
	WHERE user_account.account like 'dummy%')

DELETE FROM ssn WHERE ssn IN
	(SELECT uid
  	 FROM user_account 
	WHERE user_account.account like 'dummy%')

DELETE FROM user_account WHERE account like 'dummy%'
DELETE FROM user_auth WHERE account like 'dummy%'
DELETE FROM user_info WHERE account like 'dummy%'

COMMIT TRANSACTION
GO


--******************************************************---
-- Create procedure sp_ClearSingleTestAccount
--
--******************************************************---
CREATE PROCEDURE [dbo].[sp_ClearSingleTestAccount]
    @account varchar(14)
AS

DECLARE @account_num int

SELECT @account_num = uid FROM user_account WHERE account = @account

IF @@ROWCOUNT <> 1
BEGIN
    raiserror ('Error: This account does not exist in the database!',16,-1)
    return
END

BEGIN TRANSACTION

DELETE FROM user_account WHERE account = @account
DELETE FROM user_auth WHERE account = @account
DELETE FROM user_info WHERE account = @account
DELETE FROM ssn WHERE ssn = @account_num
DELETE FROM user_data WHERE uid = @account_num

COMMIT TRANSACTION
GO



--******************************************************---
-- Create procedure sp_CreateSingleTestAccount
--
--******************************************************---

CREATE PROCEDURE [dbo].[sp_CreateSingleTestAccount]
@account varchar(14)
AS

-- password : 11111111

DECLARE @account_num int

SELECT account FROM user_info WHERE account = @account
IF @@ROWCOUNT <> 0
BEGIN
raiserror ('Error: These accounts already exist in the database!',16,-1)
   	return
END


BEGIN TRAN
    INSERT INTO user_account (account, pay_stat, login_flag) VALUES (@account, 1001, 0)
    select @account_num = (select uid from user_account where account = @account)
    IF @@ERROR <> 0 GOTO DO_ROLLBACK

    Insert INTO ssn ( ssn, name, email, newsletter, job, phone, mobile, reg_date, zip, addr_main, addr_etc, account_num, status_flag )
    values (@account_num, '?????', 'test@ncsoft.net',0,0,'','',getdate(),'','','',0,0)
    IF @@ERROR <> 0 GOTO DO_ROLLBACK

    Insert user_auth ( account, password, quiz1, quiz2, answer1, answer2 )
    values ( @account, 0xB5BAE690B15763B2871E3838F56F4949, '?? ?? ???? ??? ?????', '?? ?? ???? ??? ?????', 0x337B31E4B1F9CB291C85A3A36EF4D2D220CC3A188A2012F0C55C7A7AB72D0B0B, 0x337B31E4B1F9CB291C85A3A36EF4D2D220CC3A188A2012F0C55C7A7AB72D0B0B)
    IF @@ERROR <> 0 GOTO DO_ROLLBACK

    Insert user_info ( account, create_date, ssn, status_flag, kind )
    values ( @account, getdate(),@account_num, 0, 99  )
    IF @@ERROR <> 0 GOTO DO_ROLLBACK

    INSERT INTO user_data (uid, user_data, user_game_data) VALUES (@account_num, 0x0, 0x0)
    IF @@ERROR <> 0 GOTO DO_ROLLBACK

    update user_account set pay_stat=1001, login_flag=0 where account = @account
    IF @@ERROR <> 0 GOTO DO_ROLLBACK

commit TRAN
RETURN 1

DO_ROLLBACK:
ROLLBACK TRAN
raiserror ('Error: The accounts insert died in the database!',16,-1)
RETURN 0
GO


--******************************************************---
-- Create procedure sp_CreateMultipleTestAccounts
--
--******************************************************---
CREATE PROCEDURE [dbo].[sp_CreateMultipleTestAccounts]
(
    @beginIndex    INT,
    @endIndex    INT
)

AS


set nocount on
BEGIN

--create a bunch of account names in a temp table, then pull from it to make them
declare @accounts varchar(13)
DECLARE @nowIndex INT
SELECT @nowIndex = @beginIndex
DECLARE @nowAccount VARCHAR(11)

WHILE @nowIndex <= @endIndex
BEGIN
IF @nowIndex < 10
    BEGIN

    SELECT @nowAccount = 'dummy0000' + CONVERT(VARCHAR, @nowIndex)
    END
    ELSE IF @nowIndex < 100
    BEGIN
        SELECT @nowAccount = 'dummy000' + CONVERT(VARCHAR, @nowIndex)
    END
    ELSE IF @nowIndex < 1000
    BEGIN
        SELECT @nowAccount = 'dummy00' + CONVERT(VARCHAR, @nowIndex)
    END
    ELSE IF @nowIndex < 10000
    BEGIN
        SELECT @nowAccount = 'dummy0' + CONVERT(VARCHAR, @nowIndex)
    END
    ELSE IF @nowIndex < 100000
    BEGIN
        SELECT @nowAccount = 'dummy' + CONVERT(VARCHAR, @nowIndex)
    END
    ELSE
    BEGIN
        BREAK
    END

    BEGIN
    BEGIN TRAN
   exec sp_CreateSingleTestAccount @nowaccount
    IF @@ERROR <> 0 GOTO ERROR_TRAP1
        begin
    COMMIT TRAN
    END
END
SELECT @nowIndex = @nowIndex + 1
    END

return
ERROR_TRAP1:
    begin
rollback tran
print 'Unable to create test accounts'
end
end
GO

--******************************************************---
-- Create procedure sp_DisableGM
--
--******************************************************---
CREATE PROCEDURE [dbo].[sp_DisableGM]
@account varchar(14)
AS

UPDATE user_account SET login_flag = 0 WHERE account = @account

GO


--******************************************************---
-- Create procedure sp_EnableGM
--
--******************************************************---
CREATE PROCEDURE [dbo].[sp_EnableGM]
@account varchar(14)
AS

UPDATE user_account SET login_flag = 16 WHERE account = @account

GO


--******************************************************---
-- Create procedure sp_GetBlockMessages
--
--******************************************************---
CREATE PROCEDURE [dbo].[sp_GetBlockMessages] 
	@uid int
AS

SELECT reason, msg
FROM block_msg (nolock)
WHERE uid = @uid
GO


--******************************************************---
-- Create procedure sp_GetServers
--
--******************************************************---
CREATE PROCEDURE [dbo].[sp_GetServers] 
	@gameId int
AS

SELECT id, ageLimit, pk_flag, ip, inner_ip
FROM   server (nolock)
GO

--******************************************************---
-- Create procedure sp_GetUserData
--
--******************************************************---
CREATE PROCEDURE [dbo].[sp_GetUserData] 
	@userId int
AS

BEGIN TRAN

SELECT user_data FROM user_data WHERE uid = @userId

if @@ROWCOUNT <> 1
begin
	ROLLBACK TRAN
	SELECT 0
	RETURN
end

COMMIT TRAN
SELECT 1
RETURN
GO


--******************************************************---
-- Create procedure sp_GetUserGameInfo
--
--******************************************************---
CREATE PROCEDURE [dbo].[sp_GetUserGameInfo] 
	@account VARCHAR(14),
	@gameId int
AS

declare @password binary(16)
declare @uid int
declare @login_flag int
declare @warn_flag int
declare @block_flag int
declare @block_flag2 int
declare @pay_stat int
declare @time_left int
declare @last_world tinyint
SELECT
	@password = b.password,
	@uid = a.uid, 
	@login_flag = a.login_flag, 
	@warn_flag = a.warn_flag, 
	@block_flag = a.block_flag,
	@block_flag2 = a.block_flag2,
	@pay_stat = a.pay_stat,
	@last_world = a.last_world
FROM
	user_account a (nolock) 
	INNER JOIN user_auth b (nolock) ON a.account = b.account
WHERE
	a.account = @account

IF @@ROWCOUNT = 0
BEGIN
	SELECT 0 where (0 =1)
	RETURN
END

SELECT @time_left = total_time
FROM user_time (nolock)
WHERE uid = @uid

IF @@ROWCOUNT = 0
BEGIN
	SELECT @time_left = 0
END

SELECT @account, @password, @uid, @login_flag, @warn_flag, @block_flag, @block_flag2, @gameId, @pay_stat, @time_left, @last_world
RETURN
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
	@game_logout_type char,
    @cd_kind int
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

--******************************************************---
-- Create procedure sp_LogUserNumbers
--
--******************************************************---
CREATE PROCEDURE [dbo].[sp_LogUserNumbers] 
	@record_time datetime,
	@server_id int,
	@world_user int,
	@limit_user int,
	@auth_user int,
        @wait_user int
AS
INSERT INTO user_count
(
	record_time,
	server_id,
	world_user,
	limit_user,
	auth_user,
	wait_user
)
VALUES
(
	@record_time,
	@server_id,
	@world_user,
	@limit_user,
	@auth_user,
	@wait_user
)
GO

--******************************************************---
-- Create procedure sp_UserLogin
--
--******************************************************---
CREATE PROCEDURE [dbo].[sp_UserLogin] 
	@userId int,
	@lastServerId tinyint,
	@loginIP varchar(15)
AS

BEGIN TRAN
update user_account set last_login = getdate() , last_world = @lastServerId, last_ip = @loginIP
where uid = @userId

if @@ROWCOUNT <> 1
begin
	ROLLBACK TRAN
	SELECT 0
	RETURN
end

COMMIT TRAN
SELECT 1
RETURN
GO


--******************************************************---
-- Create procedure sp_UserLogout
--
--******************************************************---
CREATE  PROCEDURE [dbo].[sp_UserLogout] 
	@userId int,
	@useTime int,
	@reason smallint
AS

BEGIN TRAN
update user_account set last_logout = getdate() 
where uid = @userId

if @@ROWCOUNT <> 1
begin
	ROLLBACK TRAN
	SELECT 0
	RETURN
end

COMMIT TRAN
SELECT 1
RETURN

GO


--******************************************************---
-- Create procedure sp_WriteUserData
--
--******************************************************---
CREATE PROCEDURE [dbo].[sp_WriteUserData] 
	@userId int,
	@userData binary(16)
AS

BEGIN TRAN

UPDATE user_data SET user_data=@userData WHERE uid = @userId

if @@ROWCOUNT <> 1
begin
	ROLLBACK TRAN
	SELECT 0
	RETURN
end

COMMIT TRAN
SELECT 1
RETURN
GO


--******************************************************---
-- Create procedure usp_event_useraccount
--
--******************************************************---
create procedure [dbo].[usp_event_useraccount] (
 @action	varchar(10)	= 'start'	-- 'start', 'end', 'init'
) as
begin
	/*
		This procedure interacts with special event tables
		event_user_account_begin and event_user_account_end which
		are used to store user_account states before and after special
		events for reporting purposes.
	*/
	declare @message varchar(128)
	declare @error	 bit			-- 0 okay, 1 error

	if (@action='start')
	begin
		insert into event_user_account_start
			select * from user_account (nolock)
		if @@error = 0 goto exit_normal
			else goto error_select
	end
	if (@action='end')
	begin
		insert into event_user_account_end
			select * from user_account (nolock)
		if @@error = 0 goto exit_normal
			else goto error_select
	end
	if (@action='init')
	begin
		delete from event_user_account_start
		delete from event_user_account_end 
		goto exit_normal
	end
	
	goto error_param
	
	error_select:
	begin
		set @message = 'use_event_useraccount: error ' + cast(@@error as varchar) + ' occured. Aborting.'
		set @error = 1
		goto exit_now
	end

	error_param:
	begin
		set @message = 'usp_event_useraccount: invalid @action=''' + isnull(@action,'NULL') + ''', valid values ''start'','' end'', ''init''. Aborting.'
		set @error = 1
		goto exit_now
	end

	exit_normal:
	begin
		set @message = 'usp_event_useraccount: @action(' + @action + ') completed successfully.'
		set @error = 0
		goto exit_now
	end

	exit_now:
	begin
		print @message
		return @error
	end
end -- usp_event_user_account
GO

--******************************************************---
-- allow public to access to all procedures
--
--******************************************************---
GRANT EXECUTE 
ON [dbo].[sp_ClearAllTestAccounts] 
TO PUBLIC
GO

GRANT EXECUTE 
ON [dbo].[sp_ClearSingleTestAccount] 
TO PUBLIC
GO

GRANT EXECUTE 
ON [dbo].[sp_CreateMultipleTestAccounts] 
TO PUBLIC
GO

GRANT EXECUTE 
ON [dbo].[sp_CreateSingleTestAccount] 
TO PUBLIC
GO

GRANT EXECUTE 
ON [dbo].[sp_DisableGM] 
TO PUBLIC
GO

GRANT EXECUTE 
ON [dbo].[sp_EnableGM] 
TO PUBLIC
GO

GRANT EXECUTE 
ON [dbo].[sp_GetBlockMessages] 
TO PUBLIC
GO

GRANT EXECUTE 
ON [dbo].[sp_GetServers] 
TO PUBLIC
GO

GRANT EXECUTE 
ON [dbo].[sp_GetUserData] 
TO PUBLIC
GO

GRANT EXECUTE 
ON [dbo].[sp_GetUserGameInfo] 
TO PUBLIC
GO

GRANT EXECUTE 
ON [dbo].[sp_LogAUthActivity] 
TO PUBLIC
GO

GRANT EXECUTE 
ON [dbo].[sp_LogUserNumbers] 
TO PUBLIC
GO

GRANT EXECUTE 
ON [dbo].[sp_UserLogin] 
TO PUBLIC
GO

GRANT EXECUTE 
ON [dbo].[sp_UserLogout] 
TO PUBLIC
GO

GRANT EXECUTE 
ON [dbo].[sp_WriteUserData] 
TO PUBLIC
GO

