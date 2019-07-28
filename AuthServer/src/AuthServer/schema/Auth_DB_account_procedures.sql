--******************************************************---
-- Global Auth Server stored procedures for accounts
--
--******************************************************---

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
