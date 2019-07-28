/*
	Creates tables used for the Auth regioning system

*/

ALTER TABLE server add server_group_id int NOT NULL default 0
GO
CREATE TABLE dbo.user_server_group (
	uid int NOT NULL,
	server_group_id int NOT NULL,
	PRIMARY KEY CLUSTERED (uid, server_group_id)
)
GO

CREATE TABLE dbo.server_group (
    server_group_id int NOT NULL PRIMARY KEY CLUSTERED,
    server_group_name varchar (200) NOT NULL
)
GO

CREATE PROCEDURE [dbo].[get_server_groups]
@uid int
AS

select server_group_id from user_server_group
where uid = @uid

GO