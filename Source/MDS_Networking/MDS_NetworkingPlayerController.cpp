#include "MDS_NetworkingPlayerController.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "EngineGlobals.h"
#include "Runtime/Engine/Classes/Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

const FName SESSION_NAME = "SessionName";
TSharedPtr<class FOnlineSessionSearch> pSearchSettings;
#define DISPLAY_LOG(fmt, ...) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Purple, FString::Printf(TEXT(fmt), __VA_ARGS__ )); 
#define MAP_DIRECTORY "/Game/FirstPersonCPP/Maps/FirstPersonExampleMap"
#define SEACH_KEYWORD "Custom"

AMDS_NetworkingPlayerController::AMDS_NetworkingPlayerController()
{
	IOnlineSubsystem* pSubSystem = Online::GetSubsystem(GetWorld());
	UE_LOG(LogTemp, Warning, TEXT("[MDS_NetworkingPlayerController] found Subsystem %s"), *pSubSystem->GetSubsystemName().ToString());
}

void AMDS_NetworkingPlayerController::OnLoginCompleteDelegate(int32 _i32LocalUserNum, bool _bWasSuccessful, const FUniqueNetId& _UserID, const FString& _Error)
{
	IOnlineIdentityPtr pIdentity = Online::GetIdentityInterface();
	if (!pIdentity.IsValid())
	{
		//Get Local Player
		ULocalPlayer* pLocalPlayer = Cast<ULocalPlayer>(Player);
		if (pLocalPlayer == NULL) return;
		
		//Get Unique ID
		FUniqueNetIdRepl UniqueID = PlayerState->GetUniqueId();
		UniqueID.SetUniqueNetId(FUniqueNetIdWrapper(_UserID).GetUniqueNetId());

		//Set User Net ID to PlayerState
		PlayerState->SetUniqueId(UniqueID);

		//Check Login Status
		int iControllerID = pLocalPlayer->GetControllerId();
		ELoginStatus::Type LoginStatus = pIdentity->GetLoginStatus(iControllerID);
		DISPLAY_LOG("Login Status: %s", ELoginStatus::ToString(LoginStatus));
	}
	else
	{
		UE_LOG_ONLINE(Warning, TEXT("LOGIN ERROR :%S"), FCommandLine::Get());
		DISPLAY_LOG("Login Status: IDENTITY INVALID");
	}
}

void AMDS_NetworkingPlayerController::OnCreateSessionCompleteDelegate(FName _InSessionName, bool _bWasSuccessful)
{
	if (!_bWasSuccessful) return;

	//Move levels after successful session creation (with "listen"option)
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
		//Found Session Successfully
		if (pSearchSettings->SearchResults.Num() == 0)
		{
			DISPLAY_LOG("NO SESSION IDs FOUND!!")
		}
		else
		{
			//Call the session join process if the search is successful
			const TCHAR* pSessionID = *pSearchSettings->SearchResults[0].GetSessionIdStr();
			//DISPLAY_LOG("FOUND SESSION ID : %s", *pSessionID);
			JoinSession(pSearchSettings->SearchResults[0]);
		}
	}
	else
	{
		DISPLAY_LOG("FAILED TO FIND SESSION!!");
	}
}

void AMDS_NetworkingPlayerController::JoinSession(FOnlineSessionSearchResult SearchResult)
{
	IOnlineSubsystem* pSubSystem = Online::GetSubsystem(GetWorld());
	if (!pSubSystem) return;
	
	IOnlineSessionPtr pSession = pSubSystem->GetSessionInterface();
	if (!pSession.IsValid()) return;
	
	if (SearchResult.IsValid())
	{
		pSession->AddOnJoinSessionCompleteDelegate_Handle
		(
			FOnJoinSessionCompleteDelegate::CreateUObject
			(
				this,
				&AMDS_NetworkingPlayerController::OnJoinSessionCompleteDelegate
			)
		);

		//Specify a user to join the session
		TSharedPtr<const FUniqueNetId> pNetID = GetLocalPlayer()->GetPreferredUniqueNetId().GetUniqueNetId(); 
		
		//Call JoinSession on the Sessionlnterface
		pSession->JoinSession(*pNetID, SESSION_NAME, SearchResult);
		
		DISPLAY_LOG("JOINING SESSION!!");
	}
	else
	{
		DISPLAY_LOG("INVALID SESSION");
	}
}

void AMDS_NetworkingPlayerController::OnJoinSessionCompleteDelegate(FName _SessionName, EOnJoinSessionCompleteResult::Type _Result)
{
	//----------------------------------------------------------
	const IOnlineSubsystem* pSubSystem = Online::GetSubsystem(GetWorld());
	if (!pSubSystem) return;
	
	//----------------------------------------------------------
	IOnlineSessionPtr pSession = pSubSystem->GetSessionInterface();
	if (!pSession.IsValid()) return;
	
	//----------------------------------------------------------
	if (_Result != EOnJoinSessionCompleteResult::Success) return;
	
	//Client Travel to the Server
	FString strConnectInfo;
	if (!pSession->GetResolvedConnectString(SESSION_NAME, strConnectInfo)) return;
	
	//Transition with ClientTravel if you succeed in joining the session
	UE_LOG_ONLINE_SESSION(Log, TEXT("Joined Session: Travelling to %s"), *strConnectInfo);
	AMDS_NetworkingPlayerController::ClientTravel(strConnectInfo, TRAVEL_Absolute);
}

void AMDS_NetworkingPlayerController::Login()
{
	//Get SubSystem
	IOnlineSubsystem* pSubSystem = Online::GetSubsystem(GetWorld());
	if (!pSubSystem) return;
	
	//Identity Interface - Identity Online Service used for authentication of user identities
	IOnlineIdentityPtr pIdentity = pSubSystem->GetIdentityInterface();
	if (!pIdentity.IsValid()) return;
	
	//Get Local Player
	ULocalPlayer* pLocalPlayer = Cast<ULocalPlayer>(Player);
	if (pLocalPlayer == NULL) return;
	
	//Get Controller ID and check whether it has already logged in to the subsystem
	int iControllerID = pLocalPlayer->GetControllerId();
	if (pIdentity->GetLoginStatus(iControllerID) == ELoginStatus::LoggedIn)
	{
		DISPLAY_LOG("Already Logged in");
		return;
	}

	//On Login delegate
	pIdentity->AddOnLoginCompleteDelegate_Handle
	(
		iControllerID,
		FOnLoginCompleteDelegate::CreateUObject
		(
			this,
			&AMDS_NetworkingPlayerController::OnLoginCompleteDelegate
		)
	);

	//Controller ID required for auto login
	pIdentity->AutoLogin(iControllerID);
}

bool AMDS_NetworkingPlayerController::HostSession()
{
	//Get Subsystem
	IOnlineSubsystem* pSubsystem = Online::GetSubsystem(GetWorld());
	if (!pSubsystem) return false;
	
	//Get Session Interface
	IOnlineSessionPtr pSession = pSubsystem->GetSessionInterface();
	if (!pSession.IsValid()) return false;
	
	//Create Session Settings
	TSharedPtr<class FOnlineSessionSettings> pSessionSettings = MakeShareable(new FOnlineSessionSettings());

	//Only public connections
	pSessionSettings->NumPublicConnections = 4; //Only Public Connection
	pSessionSettings->NumPrivateConnections = 0; //No Private Connections 
	pSessionSettings->bShouldAdvertise = true; //Host should broadcast
	pSessionSettings->bAllowJoinInProgress = true; //Allow Join after the session begins
	pSessionSettings->bAllowInvites = true; //Allow to receieve invites 

	//Would return information about the clients of a given session
	//You can use friendlists and check is the friend currently is in a game?
	pSessionSettings->bUsesPresence = true;
	pSessionSettings->bAllowJoinViaPresence = true; //Allow to Join
	pSessionSettings->bUseLobbiesIfAvailable = true;//Use EOS lobby service 

	//Set search keyword
	//Only Online service and no LAN/ Ping Service
	pSessionSettings->Set(SEARCH_KEYWORDS, FString(SEACH_KEYWORD), EOnlineDataAdvertisementType::ViaOnlineService);

	//Set the callback function to be called when the session creation is finished
	pSession->AddOnCreateSessionCompleteDelegate_Handle
	(
		FOnCreateSessionCompleteDelegate::CreateUObject
		(
			this,
			&AMDS_NetworkingPlayerController::OnCreateSessionCompleteDelegate
		)
	);

	TSharedPtr<const FUniqueNetId> pNetId = GetLocalPlayer()->GetPreferredUniqueNetId().GetUniqueNetId();
	bool bSuccessful = pSession->CreateSession(*pNetId, SESSION_NAME, *pSessionSettings);

	if (bSuccessful) { DISPLAY_LOG("CreateSession: Success"); return true; }
	else { DISPLAY_LOG("CreateSession: Failed"); return false; }
}

void AMDS_NetworkingPlayerController::FindSession()
{
	const IOnlineSubsystem* pSubSystem = Online::GetSubsystem(GetWorld());
	if (!pSubSystem) return;

	IOnlineSessionPtr pSession = pSubSystem->GetSessionInterface();
	if (!pSession.IsValid()) return;

	pSearchSettings = MakeShareable(new FOnlineSessionSearch());

	//Search for sessions that are Indicated to Presence
	pSearchSettings->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	//Use Lobby as priority
	pSearchSettings->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);

	//Search the lobby by its search keyword
	pSearchSettings->QuerySettings.Set(SEARCH_KEYWORDS, FString(SEACH_KEYWORD), EOnlineComparisonOp::Equals);

	//Create the Find Session Complete Delegate
	pSession->AddOnFindSessionsCompleteDelegate_Handle
	(
		FOnFindSessionsCompleteDelegate::CreateUObject
		(
			this,
			&AMDS_NetworkingPlayerController::OnFindSessionsCompleteDelegate
		)
	);

	//Search for sessions by specifying users and search conditions
	bool bIsSuccess = pSession->FindSessions
	(
		*GetLocalPlayer()->GetPreferredUniqueNetId().GetUniqueNetId(),
		pSearchSettings.ToSharedRef()
	);
}

void AMDS_NetworkingPlayerController::QuitSession()
{
	IOnlineSubsystem* pSubSystem = Online::GetSubsystem(GetWorld());
	if (!pSubSystem) return;
	
	IOnlineSessionPtr pSession = pSubSystem->GetSessionInterface();
	if (!pSession.IsValid()) return;
	
	//Destroy Session
	pSession->DestroySession(SESSION_NAME);

	//Load map without listen
	UGameplayStatics::OpenLevel
	(
		this,
		FName(TEXT(MAP_DIRECTORY)),
		true,
		""
	);
}