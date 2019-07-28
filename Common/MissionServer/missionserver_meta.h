// ============================================================
// META generated file. do not edit!
// Generated: Mon Apr 19 23:43:22 2010
// ============================================================

#pragma once
//#include "ncstd.h"
#include "UtilsNew/meta.h"

void missionserver_meta_register(void);

void missionserver_unpublishArc(int arcid, int auth_id, int accesslevel);
void missionserver_updateLevelRange(int arcid, int low, int high);
void missionserver_invalidateArc(int arcid, int unableToPlay, char *error);
void missionserver_restoreArc(int arcid);
void missionserver_changeHonors(int arcid, int ihonors);
void missionserver_banArc(int arcid);
void missionserver_unbannableArc(int arcid);
void missionserver_grantTickets(int tickets, int db_id, int auth_id);
void missionserver_grantOverflowTickets(int tickets, int db_id, int auth_id);
void missionserver_setPaidPublishSlots(int paid_slots, int db_id, int auth_id);
void missionserver_recordArcCompleted(int arcid, int authid);
void missionserver_recordArcStarted(int arcid, int authid);
void missionserver_printUserInfo(int auth_id, char *auth_region_str);
void missionserver_setBestArcStars(int stars, int db_id, int auth_id);
void missionserver_reportArc(int arcid, int auth_id, int type, const char *comment, const char *globalName);
void missionserver_comment(int arcid, int auth_id, const char *comment);
void missionserver_setKeywordsForArc(int accesslevel, int arcid, int auth_id, int keywords);
void missionserver_voteForArc(int accesslevel, int arcid, int auth_id, int ivote);
void missionserver_setGuestBio(int arcid, char *author, char *bio, char *image);

// Use these to force a local call while handling a remote call
// e.g. If you need to call a function marked with REMOTE from a remote call,
//      but you don't want its output to be sent to the original caller
#define missionserver_unpublishArc_local(arcid, auth_id, accesslevel)	meta_local(missionserver_unpublishArc(arcid, auth_id, accesslevel))
#define missionserver_updateLevelRange_local(arcid, low, high)	meta_local(missionserver_updateLevelRange(arcid, low, high))
#define missionserver_invalidateArc_local(arcid, unableToPlay, error)	meta_local(missionserver_invalidateArc(arcid, unableToPlay, error))
#define missionserver_restoreArc_local(arcid)	meta_local(missionserver_restoreArc(arcid))
#define missionserver_changeHonors_local(arcid, ihonors)	meta_local(missionserver_changeHonors(arcid, ihonors))
#define missionserver_banArc_local(arcid)	meta_local(missionserver_banArc(arcid))
#define missionserver_unbannableArc_local(arcid)	meta_local(missionserver_unbannableArc(arcid))
#define missionserver_grantTickets_local(tickets, db_id, auth_id)	meta_local(missionserver_grantTickets(tickets, db_id, auth_id))
#define missionserver_grantOverflowTickets_local(tickets, db_id, auth_id)	meta_local(missionserver_grantOverflowTickets(tickets, db_id, auth_id))
#define missionserver_setPaidPublishSlots_local(paid_slots, db_id, auth_id)	meta_local(missionserver_setPaidPublishSlots(paid_slots, db_id, auth_id))
#define missionserver_recordArcCompleted_local(arcid, authid)	meta_local(missionserver_recordArcCompleted(arcid, authid))
#define missionserver_recordArcStarted_local(arcid, authid)	meta_local(missionserver_recordArcStarted(arcid, authid))
#define missionserver_printUserInfo_local(auth_id, auth_region_str)	meta_local(missionserver_printUserInfo(auth_id, auth_region_str))
#define missionserver_setBestArcStars_local(stars, db_id, auth_id)	meta_local(missionserver_setBestArcStars(stars, db_id, auth_id))
#define missionserver_reportArc_local(arcid, auth_id, type, comment, globalName)	meta_local(missionserver_reportArc(arcid, auth_id, type, comment, globalName))
#define missionserver_comment_local(arcid, auth_id, comment)	meta_local(missionserver_comment(arcid, auth_id, comment))
#define missionserver_setKeywordsForArc_local(accesslevel, arcid, auth_id, keywords)	meta_local(missionserver_setKeywordsForArc(accesslevel, arcid, auth_id, keywords))
#define missionserver_voteForArc_local(accesslevel, arcid, auth_id, ivote)	meta_local(missionserver_voteForArc(accesslevel, arcid, auth_id, ivote))
#define missionserver_setGuestBio_local(arcid, author, bio, image)	meta_local(missionserver_setGuestBio(arcid, author, bio, image))

// Use these to treat the call as a remote call (for output etc.)
// These calls are made with an accesslevel of 11 (internal)
#define missionserver_unpublishArc_remote(link, forward_id, arcid, auth_id, accesslevel)	meta_remote(link, forward_id, missionserver_unpublishArc(arcid, auth_id, accesslevel))
#define missionserver_updateLevelRange_remote(link, forward_id, arcid, low, high)	meta_remote(link, forward_id, missionserver_updateLevelRange(arcid, low, high))
#define missionserver_invalidateArc_remote(link, forward_id, arcid, unableToPlay, error)	meta_remote(link, forward_id, missionserver_invalidateArc(arcid, unableToPlay, error))
#define missionserver_restoreArc_remote(link, forward_id, arcid)	meta_remote(link, forward_id, missionserver_restoreArc(arcid))
#define missionserver_changeHonors_remote(link, forward_id, arcid, ihonors)	meta_remote(link, forward_id, missionserver_changeHonors(arcid, ihonors))
#define missionserver_banArc_remote(link, forward_id, arcid)	meta_remote(link, forward_id, missionserver_banArc(arcid))
#define missionserver_unbannableArc_remote(link, forward_id, arcid)	meta_remote(link, forward_id, missionserver_unbannableArc(arcid))
#define missionserver_grantTickets_remote(link, forward_id, tickets, db_id, auth_id)	meta_remote(link, forward_id, missionserver_grantTickets(tickets, db_id, auth_id))
#define missionserver_grantOverflowTickets_remote(link, forward_id, tickets, db_id, auth_id)	meta_remote(link, forward_id, missionserver_grantOverflowTickets(tickets, db_id, auth_id))
#define missionserver_setPaidPublishSlots_remote(link, forward_id, paid_slots, db_id, auth_id)	meta_remote(link, forward_id, missionserver_setPaidPublishSlots(paid_slots, db_id, auth_id))
#define missionserver_recordArcCompleted_remote(link, forward_id, arcid, authid)	meta_remote(link, forward_id, missionserver_recordArcCompleted(arcid, authid))
#define missionserver_recordArcStarted_remote(link, forward_id, arcid, authid)	meta_remote(link, forward_id, missionserver_recordArcStarted(arcid, authid))
#define missionserver_printUserInfo_remote(link, forward_id, auth_id, auth_region_str)	meta_remote(link, forward_id, missionserver_printUserInfo(auth_id, auth_region_str))
#define missionserver_setBestArcStars_remote(link, forward_id, stars, db_id, auth_id)	meta_remote(link, forward_id, missionserver_setBestArcStars(stars, db_id, auth_id))
#define missionserver_reportArc_remote(link, forward_id, arcid, auth_id, type, comment, globalName)	meta_remote(link, forward_id, missionserver_reportArc(arcid, auth_id, type, comment, globalName))
#define missionserver_comment_remote(link, forward_id, arcid, auth_id, comment)	meta_remote(link, forward_id, missionserver_comment(arcid, auth_id, comment))
#define missionserver_setKeywordsForArc_remote(link, forward_id, accesslevel, arcid, auth_id, keywords)	meta_remote(link, forward_id, missionserver_setKeywordsForArc(accesslevel, arcid, auth_id, keywords))
#define missionserver_voteForArc_remote(link, forward_id, accesslevel, arcid, auth_id, ivote)	meta_remote(link, forward_id, missionserver_voteForArc(accesslevel, arcid, auth_id, ivote))
#define missionserver_setGuestBio_remote(link, forward_id, arcid, author, bio, image)	meta_remote(link, forward_id, missionserver_setGuestBio(arcid, author, bio, image))
