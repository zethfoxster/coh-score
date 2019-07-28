#ifndef _CONTAINERBROADCAST_H
#define _CONTAINERBROADCAST_H

#include "entVarUpdate.h"

#define	DBMSG_CHAT_MSG							"ChatMsg:"
#define	DBMSG_HERO_CHAT_MSG						"CoHChatMsg:"
#define	DBMSG_VILLAIN_CHAT_MSG						"CoVChatMsg:"
#define	DBMSG_PLAYER_MSG(e)						(e?(ENT_IS_VILLAIN(e)?DBMSG_VILLAIN_CHAT_MSG:DBMSG_HERO_CHAT_MSG):DBMSG_CHAT_MSG)

#define	DBMSG_ALLIANCECHAT_MSG					"AllianceChatMsg:"
#define	DBMSG_EMAIL_MSG							"Email:"
#define DBMSG_STORYARC_TASK_REFRESH				"StoryarcTaskRefresh:"
#define DBMSG_STORYARC_FULL_REFRESH				"StoryarcFullRefresh:"
#define DBMSG_MISSION_COMPLETE					"MissionComplete:"
#define DBMSG_MISSION_FAILED					"MissionFailed:"
#define	DBMSG_MISSION_STOPPED_RUNNING 			"MissionStoppedRunning:"
#define DBMSG_MISSION_OBJECTIVE					"MissionObjective:"
#define DBMSG_TEAM_TASK_SELECTED				"TeamTaskSelected:"
#define DBMSG_TASKFORCE_RESUMETEAM				"TaskForceResumeTeam:"
#define DBMSG_FLASHBACK_GETCREDIT				"TaskForceGetCredit:"
#define DBMSG_TFCHALLENGE_FAILED				"TFChallengeFailed:"
#define DBMSG_TFCHALLENGE_SHOWSUMMARY			"TFChallengeShowSummary:"
#define DBMSG_MAPLIST_REFRESH					"MapListRefresh:"
#define DBMSG_SUPERGROUP_REFRESH				"SupergroupRefresh:"
#define DBMSG_PAYMENT_REMAIN_TIME				"PaymentRemainTime:"
#define DBMSG_ARCHITECT_COMPLETE				"ArchitectComplete:"
#define DBMSG_ARCHITECT_COMPLETE_MISSION		"ArchitectCompleteMission:"
#define DBMSG_TASKFORCE_COMPLETE				"TaskForceComplete:"
// Generated by mkproto
void dealWithBroadcast(int container,int *members,int count,char *msg,int msg_type,int senderID,int container_type);
void sendEntsMsg(int container_type,int container_num, int msg_type, int senderID, char *msgs,...);
// End mkproto
#endif
