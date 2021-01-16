// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperSpriteActor.h"
#include "Tile.generated.h"

/**
 * 
 */
UCLASS()
class MM_CONNECT3_API ATile : public APaperSpriteActor
{
	GENERATED_BODY()


public:
	// Sets default values for this actor's properties
	ATile();

	int32 CurrentIndex;
	int32 TileTypeID;
	int32 TargetIndex;
	bool bIsSelected;	

	void SelectTile();
	void DeselectTile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};
