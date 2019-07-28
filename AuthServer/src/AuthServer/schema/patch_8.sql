--
-- DGoodenough Mar. 24, 2010
--

SET QUOTED_IDENTIFIER OFF 
GO
SET ANSI_NULLS ON 
GO

-- adding in the two new columns for extended auth bits
-- extending from 16 bytes to 128, thus the reason this is 112
ALTER TABLE [dbo].[user_data]
	ADD [user_data_new] [binary] (112) NOT NULL DEFAULT 0
GO

ALTER TABLE [dbo].[user_data]
	ADD [user_game_data_new] [binary] (112) NOT NULL DEFAULT 0
GO

SET QUOTED_IDENTIFIER OFF 
GO
SET ANSI_NULLS ON 
GO
