--
-- an update to the Auth database where to split the PlayNC bits and the game bits
-- ajackson Oct. 1, 2008
--

-- make new column for publishing (aka PlayNC) account data
ALTER TABLE [dbo].[user_data] 
    ADD [user_game_data] [binary] (16) NULL 
GO

-- autofill the new column with the old data
UPDATE [dbo].[user_data]
SET    [dbo].[user_data].[user_game_data] = [dbo].[user_data].[user_data]
GO

-- drop the old stored create test account sproc
IF EXISTS (SELECT * FROM dbo.sysobjects WHERE name = 'sp_CreateSingleTestAccount' AND xtype = 'P')
BEGIN
    DROP PROCEDURE [dbo].[sp_CreateSingleTestAccount]
END
GO

-- create the new sproc

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

