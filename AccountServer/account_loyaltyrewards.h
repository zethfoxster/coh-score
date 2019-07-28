#ifndef LOYALTY_REWARDS_H
#define LOYALTY_REWARDS_H

typedef struct Account Account;
typedef struct AccountServerShard AccountServerShard;
typedef struct Packet Packet;

bool accountLoyaltyValidateLoyaltyChange(Account *pAccount, int index, int set, int earnedDelta);
void accountLoyalityDecreaseLoyaltyPoints(Account *pAccount, int loyaltyEarned, bool csr_did_it);
void accountLoyalityIncreaseLoyaltyPoints(Account *pAccount, int loyaltyEarned, bool csr_did_it);
void accountLoyaltyChangeNode(Account *pAccount, int index, bool set, bool csr_did_it);

void handleLoyaltyChangeRequest(AccountServerShard *shard, Packet *packet_in);
void handleAuthUpdateRequest(AccountServerShard *shard, Packet *packet_in);

#endif
