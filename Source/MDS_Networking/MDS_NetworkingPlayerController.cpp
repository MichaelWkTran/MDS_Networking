#include "MDS_NetworkingPlayerController.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "EngineGlobals.h"
#include "Runtime/Engine/Classes/Engine/Engine.h" 
#include "Kismet/GameplayStatics.h" 

const FName SESSION_NAME = "SessionName";
TSharedPtr<class FOnlineSessionSearch> searchSettings;
#define DISPLAY_LOG(fmt, ...) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Purple, FString::Printf(TEXT(fmt), __VA_ARGS__ )); 
#define MAP_DIRECTORY "/Game/FirstPersonCPP/Maps/FirstPersonExampleMap"

AMDS_NetworkingPlayerController::AMDS_NetworkingPlayerController()
{
	IOnlineSubsystem* pSubSystem = Online::GetSubsystem(GetWorld());
	UE_LOG(LogTemp, Warning, TEXT("[MDS_NetworkingPlayerController] found Subsystem %s"), *pSubSystem->GetSubsystemName().ToString());
}

void AMDS_NetworkingPlayerController::OnLoginCompleteDelegate(int32 _i32LocalUserNum, bool _bWasSuccessful, const FUniqueNetId& _UserID, const FString& _Error)
{
	IOnlineIdentityPtr identity = Online::GetIdentityInterface();
	
	if (identity.IsValid())
	{
		ULocalPlayer* localPlayer = Cast<ULocalPlayer>(Player);
		if (localPlayer != NULL)
		{
			FUniqueNetIdRepl uniqueId = PlayerState->GetUniqueId();
			uniqueId.SetUniqueNetId(FUniqueNetIdWrapper(_UserID).GetUniqueNetId());

			//Set User Net ID to PlayerState
			PlayerState->SetUniqueId(uniqueId);

			//Check login status
			int controllerId = localPlayer->GetControllerId();
			ELoginStatus::Type status = identity->GetLoginStatus(controllerId);
			DISPLAY_LOG("Login Status: %s", ELoginStatus::ToString(status));
		}
		else
		{
			UE_LOG_ONLINE(Warning, TEXT("LOGIN ERROR :%S"), FCommandLine::Get());
			DISPLAY_LOG("Login Status: IDENTITY INVALID");
		}
	}
}

void AMDS_NetworkingPlayerController::OnCreateSessionCompleteDelegate(FName _InSessionName, bool _bWasSuccessful)
{
	if (!_bWasSuccessful) return;

	//Move levels after successful session creation (with "listen" option)
	UGameplayStatics::OpenLevel
	(
		this,
		FName(TEXT(MAP_DIRECTORY)),
		true,
		"listen"
	);
}

void AMDS_NetworkingPlayerController::OnFindSessionsCompleteDelegate(bool _bWasSuccessful)
{
	if (_bWasSuccessful)
	{
		// Found Session Successfully
		if (searchSettings->SearchResults.Num() == 0)
		{
			DISPLAY_LOG("NO SESSION IDs FOUND !!")
		}
		else
		{
			// Call the session join process if the search is successful
			const TCHAR* SessionId = *searchSettings->SearchResults[0].GetSessionIdStr();
			DISPLAY_LOG("FOUND SESSION ID : %s", *SessionId);
			JoinSession(searchSettings->SearchResults[0]);
		}
	}
	else
	{
		DISPLAY_LOG("FAILED TO FIND SESSION !!");
	}
}

void AMDS_NetworkingPlayerController::JoinSession(FOnlineSessionSearchResult SearchResult)
{
	IOnlineSubsystem* subSystem = Online::GetSubsystem(GetWorld());
	if (subSystem)
	{
		IOnlineSessionPtr session = subSystem->GetSessionInterface();
		if (session.IsValid())
		{
			if (SearchResult.IsValid())
			{
				session->AddOnJoinSessionCompleteDelegate_Handle
				(
					FOnJoinSessionCompleteDelegate::CreateUObject
					(
						this,
						&AMDS_NetworkingPlayerController::OnJoinSessionCompleteDelegate
					)
				);

				// Specify a user to join the session
				TSharedPtr<const FUniqueNetId> uniqueNetIdPtr = GetLocalPlayer()->GetPreferredUniqueNetId().GetUniqueNetId(); 
				
				// Call JoinSession on the Sessionlnterface
				session->JoinSession(*uniqueNetIdPtr, SESSION_NAME, SearchResult);
				
				DISPLAY_LOG(" JOINING SESSION !!");
			}
			else
			{
				DISPLAY_LOG(" INVALID SESSION ");
			}
		}
	}
}

void AMDS_NetworkingPlayerController::OnJoinSessionCompleteDelegate(FName _SessionName, EOnJoinSessionCompleteResult::Type _Result)
{
	IOnlineSubsystem* const subSystem = Online::GetSubsystem(GetWorld());
	if (subSystem)
	{
		IOnlineSessionPtr session = subSystem->GetSessionInterface();
		if (session.IsValid())
		{
			if (_Result == EOnJoinSessionCompleteResult::Success)
			{
				// Client Travel to the Server
				FString connectInfo;

				if (session->GetResolvedConnectString(SESSION_NAME, connectInfo))
				{
					// Transition with ClientTravel if you succeed in joining the session
					UE_LOG_ONLINE_SESSION(Log, TEXT("Joined Session: Travelling to %s"), *connectInfo);
					AMDS_NetworkingPlayerController::ClientTravel(connectInfo, TRAVEL_Absolute);
				}
			}
		} 
	}
}

void AMDS_NetworkingPlayerController::Login()
{
	auto OnLoginCompleteDelegate = [&](int32 _i32LocalUserNum, bool _bWasSuccessful, const FUniqueNetId& _UserID, const FString& _Error)
	{
		IOnlineIdentityPtr identity = Online::GetIdentityInterface();

		if (identity.IsValid())
		{
			ULocalPlayer* localPlayer = Cast<ULocalPlayer>(Player);
			if (localPlayer != NULL)
			{
				FUniqueNetIdRepl uniqueId = PlayerState->GetUniqueId();
				uniqueId.SetUniqueNetId(FUniqueNetIdWrapper(_UserID).GetUniqueNetId());

				//Set User Net ID to PlayerState
				PlayerState->SetUniqueId(uniqueId);

				//Check login status
				int controllerId = localPlayer->GetControllerId();
				ELoginStatus::Type status = identity->GetLoginStatus(controllerId);
				DISPLAY_LOG("Login Status: %s", ELoginStatus::ToString(status));
			}
			else
			{
				UE_LOG_ONLINE(Warning, TEXT("LOGIN ERROR :%S"), FCommandLine::Get());
				DISPLAY_LOG("Login Status: IDENTITY INVALID");
			}
		}
	};

	IOnlineSubsystem* pSubSystem = Online::GetSubsystem(GetWorld());
	if (pSubSystem)
	{
		//Identity Interface - Identity Online Service used for authentication of user identities
		IOnlineIdentityPtr Identity = pSubSystem->GetIdentityInterface();
		if (Identity.IsValid())
		{
			//Get Local Player
			ULocalPlayer* pLocalPlayer = Cast<ULocalPlayer>(Player);
			if (pLocalPlayer != NULL)
			{
				int iControllerId = pLocalPlayer->GetControllerId();
				if (Identity->GetLoginStatus(iControllerId) != ELoginStatus::LoggedIn)
				{
					// On Login delegate
					Identity->AddOnLoginCompleteDelegate_Handle
					(
						iControllerId,
						FOnLoginCompleteDelegate::CreateUObject
						(
							this,
							&AMDS_NetworkingPlayerController::OnLoginCompleteDelegate
						)
					);

					//Controller ID required for auto login
					Identity->AutoLogin(iControllerId); 
				}
			}
		}
	}
	
	PlayerState;
}

bool AMDS_NetworkingPlayerController::HostSession()
{
	IOnlineSubsystem* subSystem = Online::GetSubsystem(GetWorld());
	if (subSystem)
	{
		//Get Session Interface
		IOnlineSessionPtr session = subSystem->GetSessionInterface();
		if (session.IsValid())
		{
			//Create Session Settings
			TSharedPtr<class FOnlineSessionSettings> sessionSettings = MakeShareable(new FOnlineSessionSettings());

			//Only public connections
			sessionSettings->NumPublicConnections = 4; // Only Public Connection
			sessionSettings->NumPrivateConnections = 0; // No Private Connections 
			sessionSettings->bShouldAdvertise = true; // host should broadcast
			sessionSettings->bAllowJoinInProgress = true; // Allow Join after the session begins
			sessionSettings->bAllowInvites = true; // Allow to receieve invites 

			// Would return information about the clients of a given session
			// You can use friendlists and check is the friend currently is in a game?
			sessionSettings->bUsesPresence = true;
			sessionSettings->bAllowJoinViaPresence = true; // Allow to Join
			sessionSettings->bUseLobbiesIfAvailable = true;// Use EOS lobby service 

			// Set "Custom" as a search keyword
			// Only Online service and no LAN/ Ping Service
			sessionSettings->Set(SEARCH_KEYWORDS, FString("Custom"), EOnlineDataAdvertisementType::ViaOnlineService);

			// Set the callback function to be called when the session creation is finished
			session->AddOnCreateSessionCompleteDelegate_Handle
			(
				FOnCreateSessionCompleteDelegate::CreateUObject
				(
					this,
					&AMDS_NetworkingPlayerController::OnCreateSessionCompleteDelegate
				)
			);

			TSharedPtr<const FUniqueNetId> uniqueNetIdPtr = GetLocalPlayer()->GetPreferredUniqueNetId().GetUniqueNetId();
			bool res = session->CreateSession(*uniqueNetIdPtr, SESSION_NAME, *sessionSettings);
			
			if (res) { DISPLAY_LOG("CreateSession: Success"); } else { DISPLAY_LOG("CreateSession: Failed"); }
		}
	}

	return false;
}

void AMDS_NetworkingPlayerController::FindSession()
{
	IOnlineSubsystem* const subSystem = Online::GetSubsystem(GetWorld());
	
	if (subSystem)
	{
		IOnlineSessionPtr session = subSystem->GetSessionInterface();
		if (session.IsValid())
		{
			searchSettings = MakeShareable(new FOnlineSessionSearch());

			//Search for sessions that are Indicated to Presence
			searchSettings->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals); 
			
			// Use Lobby as priority
			searchSettings->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);

			// Search the lobby where "Custom" is set as the search keyword
			searchSettings->QuerySettings.Set(SEARCH_KEYWORDS, FString("Custom"), EOnlineComparisonOp::Equals);
		
			// Create the Find Session Complete Delegate
			session->AddOnFindSessionsCompleteDelegate_Handle
			(
				FOnFindSessionsCompleteDelegate::CreateUObject
				(
					this,
					&AMDS_NetworkingPlayerController::OnFindSessionsCompleteDelegate
				)
			);
			
			// Search for sessions by specifying users and search conditions
			TSharedRef<FOnlineSessionSearch> searchSettingsRef = searchSettings.ToSharedRef();
			TSharedPtr<const FUniqueNetId> uniqueNetIdPtr = GetLocalPlayer()->GetPreferredUniqueNetId().GetUniqueNetId(); 
			
			bool bIsSuccess = session->FindSessions(*uniqueNetIdPtr, searchSettingsRef);
		}
	}
}

void AMDS_NetworkingPlayerController::QuitSession()
{
	IOnlineSubsystem* subSystem = Online::GetSubsystem(GetWorld());
	if (subSystem)
	{
		IOnlineSessionPtr session = subSystem->GetSessionInterface();
		if (session.IsValid())
		{
			// Destroy Session
			session->DestroySession(SESSION_NAME);

			// Load map without listen
			UGameplayStatics::OpenLevel
			(
				this,
				FName(TEXT(MAP_DIRECTORY)),
				true,
				""
			);
		}
	}
}