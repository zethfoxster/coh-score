/*
	Updates USER_ACCOUNT_HISTORY to work with new queue_level column.

*/

ALTER TABLE [dbo].[user_account_history] ADD [queue_level] [int] NULL
go
