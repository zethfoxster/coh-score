if exists (select * from dbo.sysobjects where id = object_id(N'[dbo].[user_account_history]') and OBJECTPROPERTY(id, N'IsUserTable') = 1)
drop table [dbo].[user_account_history]
GO

CREATE TABLE [dbo].[user_account_history] (
[user_account_history_id] [int] IDENTITY (1, 1),
[hist_timestamp]datetime NOT NULL,
[uid]	int	NOT NULL,
[account]	varchar(14)	NOT NULL,
[pay_stat]int	NOT NULL,
[login_flag]	int	NOT NULL,
[warn_flag]	int	NOT NULL,
[block_flag]    int	NOT NULL,
[block_flag2]	int	NOT NULL,
[last_login]	datetime	NULL,
[last_logout]	datetime	NULL,
[subscription_flag]	int	NULL,
[last_world]	tinyint		NULL,
[last_game]	int		NULL,
[last_ip]	varchar(15)		NULL,
[block_end_date]	datetime	NULL,
[queue_level]	int	NULL
) ON [PRIMARY]
GO
--sp_help user_account_history


ALTER TABLE [dbo].[user_account_history] ADD 
	CONSTRAINT [DF_user_account_hist_timestamp] DEFAULT (getdate()) FOR [hist_timestamp]
GO


ALTER TABLE [dbo].[user_account_history] WITH NOCHECK ADD 
	CONSTRAINT [PK_user_account_history] PRIMARY KEY  CLUSTERED 
	(
		[user_account_history_id]
	)  ON [PRIMARY] 
GO



drop trigger user_account_trg1
go

create trigger user_account_trg1
on user_account
WITH ENCRYPTION
for update
as
  begin 
          if update(pay_stat)
            begin 
		insert user_account_history 
		select getdate(),* from inserted
            end 
  end
go


drop trigger user_account_trg2
go

create trigger user_account_trg2
on user_account
WITH ENCRYPTION
for insert
as
  begin 
            begin 
		insert user_account_history 
		select getdate(),* from inserted
            end 
  end
go



