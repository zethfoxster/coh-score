// SteamAuthTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <assert.h>
#include <stdio.h>
#include <windows.h>
#include "..\..\3rdparty\steam\steam_api.h"

class SteamCallbackHandlerClass;

static int  s_debugLevel = 0;
static bool s_haveStats = false;		// Waiting for user stats/achievements
static bool s_storingStats = false;		// Waiting for a store operation to finish
static bool s_needStoreStats = false;	// Need to store stats (local stats/achievements have changed)
static EResult AuthSessionTicketResponse;
static int waitingForAuthSessionTicketResponse = 0;
static int waitingForValidateAuthTicketResponse = 0;
static SteamCallbackHandlerClass *s_callbackHandler = NULL;	// Class to handle callbacks


class SteamCallbackHandlerClass
{
public:
	// Constructor
	SteamCallbackHandlerClass();

	// Callbacks
	STEAM_CALLBACK( SteamCallbackHandlerClass, OnUserStatsReceived, UserStatsReceived_t, m_CallbackUserStatsReceived );
	STEAM_CALLBACK( SteamCallbackHandlerClass, OnUserStatsStored, UserStatsStored_t, m_CallbackUserStatsStored );
	STEAM_CALLBACK( SteamCallbackHandlerClass, OnValidateAuthTicketResponse, ValidateAuthTicketResponse_t, m_CallbackGSAuthTicketResponse );
	STEAM_CALLBACK( SteamCallbackHandlerClass, OnGetAuthSessionTicketResponse, GetAuthSessionTicketResponse_t, m_CallbackGetAuthSessionTicketResponse );
};

SteamCallbackHandlerClass::SteamCallbackHandlerClass() :
	m_CallbackUserStatsReceived( this, &SteamCallbackHandlerClass::OnUserStatsReceived ),
	m_CallbackUserStatsStored( this, &SteamCallbackHandlerClass::OnUserStatsStored ),
	m_CallbackGSAuthTicketResponse( this, &SteamCallbackHandlerClass::OnValidateAuthTicketResponse ),
	m_CallbackGetAuthSessionTicketResponse( this, &SteamCallbackHandlerClass::OnGetAuthSessionTicketResponse )
{
}

void SteamCallbackHandlerClass::OnUserStatsReceived(UserStatsReceived_t *pCallback)
{
	printf("SteamCallbackHandlerClass::OnUserStatsReceived()\n");
	// TODO: re-retrieve stats if the request failed?
	if (pCallback->m_nGameID == SteamUtils()->GetAppID() &&
		pCallback->m_eResult == k_EResultOK)
	{
		printf("  s_haveStats = true\n");
		s_haveStats = true;
	}
}

void SteamCallbackHandlerClass::OnUserStatsStored(UserStatsStored_t *pCallback)
{
	printf("SteamCallbackHandlerClass::OnUserStatsStored()\n");
	if (pCallback->m_nGameID == SteamUtils()->GetAppID())
	{
		if ( pCallback->m_eResult != k_EResultOK)
		{
			printf("  s_needStoreStats = false\n");
			// Try storing the stats again
			s_needStoreStats = false;
		}

		printf("  s_storingStats = false\n");
		s_storingStats = false;
	}
}

void SteamCallbackHandlerClass::OnValidateAuthTicketResponse(ValidateAuthTicketResponse_t *pResponse)
{
	printf("SteamCallbackHandlerClass::OnValidateAuthTicketResponse()\n");
	//printf("SteamCallbackHandlerClass::OnValidateAuthTicketResponse() for steamID %s\n", pResponse->m_SteamID.Render());
	if ( pResponse->m_eAuthSessionResponse == k_EAuthSessionResponseOK )
	{
		printf( "  Auth completed for client\n" );
	}
	else
	{
		printf( "  Auth failed for client: %d\n", pResponse->m_eAuthSessionResponse );
	}
	waitingForValidateAuthTicketResponse--;
}	

void SteamCallbackHandlerClass::OnGetAuthSessionTicketResponse(GetAuthSessionTicketResponse_t *pResponse)
{
	printf("SteamCallbackHandlerClass::OnGetAuthSessionTicketResponse() for AuthTicket %d result %d\n", pResponse->m_hAuthTicket, pResponse->m_eResult);
	AuthSessionTicketResponse = pResponse->m_eResult;
	waitingForAuthSessionTicketResponse--;
}


int charToByte(char character)
{
	if (character >= '0' && character <= '9')
		return character - '0';

	if (character >= 'a' && character <= 'f')
		return character - 'a';

	if (character >= 'A' && character <= 'F')
		return character - 'A';

	//assert(0);
	return 0;
}

int GenerateAuthTicket(unsigned char ticket[1024])
{
	unsigned int ticketLength;
	waitingForAuthSessionTicketResponse++;

	int ticketID = SteamUser()->GetAuthSessionTicket(ticket, 1024, &ticketLength);
	printf("SteamUser()->GetAuthSessionTicket() returned ticketID %d\n", ticketID);

	while(waitingForAuthSessionTicketResponse)
	{
		Sleep(100);
		SteamAPI_RunCallbacks();
	}

	if (AuthSessionTicketResponse == k_EResultOK)
	{
		printf("Ticket is : 0x");

		for (unsigned int i = 0; i < ticketLength; i++)
		{
			printf("%02x", ticket[i]);
		}
		printf("\n");
		return ticketLength;
	}

	return 0;
}

int ReadAuthTicket(unsigned char ticket[1024], char *ticketString)
{
	int ticketStrLen = (int)strlen(ticketString);

	if (ticketStrLen > (1024 * 2))
	{
		printf("Argument is too long\n");
		return 0;
	}

	if (ticketStrLen & 1)
	{
		printf("Argument does not have an even number of characters\n");
		return 0;
	}

	int j = 0;
	int k = 0;

	// Skip over leading "0x" if present
	if (ticketString[0] == '0' && ticketString[1] == 'x')
	{
		j = 2;
	}

	for (; j < ticketStrLen; j += 2, k++)
	{
		ticket[k] = (charToByte(ticketString[j]) << 4) |  charToByte(ticketString[j+1]);

	}

	return k;
}

//int _tmain(int argc, _TCHAR* argv[])
int main(int argc, char* argv[])
{
	int i;
	bool rc = SteamAPI_Init();
	int delay = 0;

	if (rc)
	{
		s_callbackHandler = new SteamCallbackHandlerClass();

		// TODO: get steam ID from arguments instead of using local steam id?
		CSteamID steamID = SteamUser()->GetSteamID();

		printf("SteamID is 0x%016I64x\n", steamID.ConvertToUint64());

		for (i = 1; i < argc; i++)
		{
			unsigned char ticket[1024];
			int ticketLen = 0;

			if (strcmp(argv[i], "auto") == 0)
			{
				ticketLen = GenerateAuthTicket(ticket);
			}
			else
			{
				ticketLen = ReadAuthTicket(ticket, argv[i]);
			}

			if (ticketLen < 1)
				continue;

			printf("SteamUser()->BeginAuthSession() with ticket length %d\n", ticketLen);
			EBeginAuthSessionResult result = SteamUser()->BeginAuthSession(ticket, ticketLen, steamID);

			if (result == k_EBeginAuthSessionResultOK)
			{
				printf("SteamUser()->BeginAuthSession() returned k_EBeginAuthSessionResultOK\n");
				waitingForValidateAuthTicketResponse++;
			}
			else
			{
				printf("SteamUser()->BeginAuthSession() returned ");
				switch(result)
				{
					case k_EBeginAuthSessionResultInvalidTicket: printf("k_EBeginAuthSessionResultInvalidTicket\n"); break;
					case k_EBeginAuthSessionResultDuplicateRequest: printf("k_EBeginAuthSessionResultDuplicateRequest\n"); break;
					case k_EBeginAuthSessionResultInvalidVersion: printf("k_EBeginAuthSessionResultInvalidVersion\n"); break;
					case k_EBeginAuthSessionResultGameMismatch: printf("k_EBeginAuthSessionResultGameMismatch\n"); break;
					case k_EBeginAuthSessionResultExpiredTicket: printf("k_EBeginAuthSessionResultExpiredTicket\n"); break;
					default: printf("%d\n", result);
				}
			}
		}

		if (waitingForValidateAuthTicketResponse && delay)
		{
			printf("** Sleeping for %d seconds **\n", delay);
			Sleep(delay*1000);
		}

		while (waitingForValidateAuthTicketResponse > 0)
		{
			Sleep(100);
			SteamAPI_RunCallbacks();
		}

		SteamUser()->EndAuthSession(steamID);

		delete s_callbackHandler;
	}


	SteamAPI_Shutdown();
}

