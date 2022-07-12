#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MDS_NetworkingPlayerController.generated.h"

UCLASS()
class MDS_NETWORKING_API AMDS_NetworkingPlayerController : public APlayerController
{
	GENERATED_BODY()
	
private:
	AMDS_NetworkingPlayerController();

	void OnLoginCompleteDelegate(int32 _i32LocalUserNum, bool _bWasSuccessful, const FUniqueNetId& _UserID, const FString& _Error);
	void OnCreateSessionCompleteDelegate(FName _InSessionName, bool _bWasSuccessful);
	void OnFindSessionsCompleteDelegate(bool _bWasSuccessful);
	void JoinSession(FOnlineSessionSearchResult SearchResult);
	void OnJoinSessionCompleteDelegate(FName _SessionName, EOnJoinSessionCompleteResult::Type _Result);

public:
	UFUNCTION(BlueprintCallable, Category = "OnlineSession")
		void Login();

	UFUNCTION(BlueprintCallable, Category = "OnlineSession")
		bool HostSession();

	UFUNCTION(BlueprintCallable, Category = "OnlineSession")
		void FindSession();

	UFUNCTION(BlueprintCallable, Category = "OnlineSession")
		void QuitSession();
};
