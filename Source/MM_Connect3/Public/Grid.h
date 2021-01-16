// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Grid.generated.h"

USTRUCT(BlueprintType)
struct FTileType
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Probability = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class ATile> TileClass;
};

UCLASS()
class MM_CONNECT3_API AGrid : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGrid();

	// Number of tiles that grid can have in horizontal direction
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Grid)
	int32 MaxColumns;
	// Number of tiles that grid can have in vertical direction
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Grid)
	int32 MaxRows;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FTileType> TileLibrary;

	UPROPERTY(EditAnywhere, Category = Tile)
	FVector2D TileSize;

	UPROPERTY(EditAnywhere, Category = Tile)
	int32 MinSelectionLength;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<ATile*> SpawnedTiles;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<ATile*> SelectedTiles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Tile)
	float TileFallSpeed;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bUpdateTiles;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector mouseLocation;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector	mouseDirection;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 ScoreMult;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class AMM_Connect3GameModeBase* GameMode;



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	bool AddToSelection(class ATile* tileToSelect);
	void PointerUp();

private:

	void Initialize();

	int32 GetRandomTileID();
	// Get index of tile based on set probability
	int32 GridAddressToIndex(int32 x, int32 y);
	FVector IndexToLocation(int32 index);
	FVector GridAddressToLocation(int32 x, int32 y);
	FVector2D IndexToGridAddress(int32 index);
	FVector2D LocationToGridAddress(FVector location);

	void CreateTile(TSubclassOf<class ATile> tileObject, FVector spawnLocation, int32 tileIndex, int32 tileTypeID, int32 targerIndex);
	void UpdateTilesPosition();

private:
	//private vairables to restrict update
	int32 minColumn, maxColumn, minRow, maxRow;

	APlayerController* playerController;
};
