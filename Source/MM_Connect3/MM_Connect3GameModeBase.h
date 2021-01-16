// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MM_Connect3GameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class MM_CONNECT3_API AMM_Connect3GameModeBase : public AGameModeBase
{
	GENERATED_BODY()
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 Score;

	UFUNCTION(BlueprintImplementableEvent, Category = "Game")
	void ScoreValueUpdate(int32 points);
};
