/*
	Creates tables used for Special Events. Special Events require that user_account
	data be captured prior to enabling all accounts for Special Event access and then
	again when the Special Event ends.  The data from both snapshots are compared for
	various reporting requirements.  (this was previously done with BCP dumps)

*/
GO
/****** Object:  Table [dbo].[user_account]    Script Date: 09/28/2006 13:33:15 ******/
/*
	drop table [event_user_account_start]
	drop table [event_user_account_end]
	drop procedure [usp_event_useraccount]
	usp_event_useraccount @action='start'
	usp_event_useraccount @action='end'
	usp_event_useraccount @action='init'
	select count(*) from user_account
	select count(*) from event_user_account_start
	select count(*) from event_user_account_end

	-- only give exec on this proc for reporting user(s), they already have readonly on tables
	grant execute on [dbo].[usp_event_useraccount] to [nclareporter]
*/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[event_user_account_start](
	[uid] [int] NOT NULL,
	[account] [varchar](14) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL,
	[pay_stat] [int] NOT NULL CONSTRAINT [DF_user_account__pay_stat_eb]  DEFAULT (0),
	[login_flag] [int] NOT NULL CONSTRAINT [DF_user_account__login_flag_eb]  DEFAULT (0),
	[warn_flag] [int] NOT NULL CONSTRAINT [DF_user_account__warn_flag_eb]  DEFAULT (0),
	[block_flag] [int] NOT NULL CONSTRAINT [DF_user_account__block_flag_eb]  DEFAULT (0),
	[block_flag2] [int] NOT NULL CONSTRAINT [DF_user_account__block_flag2_eb]  DEFAULT (0),
	[last_login] [datetime] NULL,
	[last_logout] [datetime] NULL,
	[subscription_flag] [int] NOT NULL CONSTRAINT [DF_user_account_subscription_flag2]  DEFAULT (0),
	[last_world] [tinyint] NULL,
	[last_game] [int] NULL,
	[last_ip] [varchar](15) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[block_end_date] [datetime] NULL,
	[queue_level] [int] NOT NULL
) ON [PRIMARY]
GO
CREATE TABLE [dbo].[event_user_account_end](
	[uid] [int]  NOT NULL,
	[account] [varchar](14) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL,
	[pay_stat] [int] NOT NULL CONSTRAINT [DF_user_account__pay_stat_ee]  DEFAULT (0),
	[login_flag] [int] NOT NULL CONSTRAINT [DF_user_account__login_flag_ee]  DEFAULT (0),
	[warn_flag] [int] NOT NULL CONSTRAINT [DF_user_account__warn_flag_ee]  DEFAULT (0),
	[block_flag] [int] NOT NULL CONSTRAINT [DF_user_account__block_flag_ee]  DEFAULT (0),
	[block_flag2] [int] NOT NULL CONSTRAINT [DF_user_account__block_flag2_ee]  DEFAULT (0),
	[last_login] [datetime] NULL,
	[last_logout] [datetime] NULL,
	[subscription_flag] [int] NOT NULL CONSTRAINT [DF_user_account_subscription_flag_ee]  DEFAULT (0),
	[last_world] [tinyint] NULL,
	[last_game] [int] NULL,
	[last_ip] [varchar](15) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[block_end_date] [datetime] NULL,
	[queue_level] [int] NOT NULL
) ON [PRIMARY]
GO
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


SET ANSI_PADDING OFF