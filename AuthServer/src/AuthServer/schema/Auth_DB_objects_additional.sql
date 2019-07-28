---------------------------------------------------------------
-- 
--	auth_tables.sql
--
-- It does not modify the existing
-- tables used by the older L2 Auth Server from Korea.
--
---------------------------------------------------------------


--***********************************************************************--
--
-- TABLE: 	auth_activity_log
--
-- PURPOSE:	holds log records for each auth server login and logout
-- SPs:		sp_LogUserNumbers() writes to this table
--
--***********************************************************************--

CREATE TABLE [dbo].[auth_activity_log] (
	[account_name] [char] (14) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL ,
	[uid] [int] NOT NULL ,
	[serverid] [int] NOT NULL ,
	[client_ip] [varchar] (20) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL ,
	[auth_login_time] [datetime] NULL ,
	[game_login_time] [datetime] NULL ,
	[game_logout_time] [datetime] NULL ,
	[game_logout_type] [char] (1) COLLATE SQL_Latin1_General_CP1_CI_AS NULL ,
	[queue_login_time] [datetime] NULL,
    [cd_kind] [int] NOT NULL DEFAULT 0
) ON [PRIMARY]
GO

CREATE TABLE [dbo].[block_msg] (
	[uid] [int] NOT NULL ,
	[account] [varchar] (14) NOT NULL ,
	[msg] [varchar] (50) NOT NULL ,
	[reason] [int] NOT NULL 
) ON [PRIMARY]
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
