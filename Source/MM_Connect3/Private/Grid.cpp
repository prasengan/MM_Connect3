// Fill out your copyright notice in the Description page of Project Settings.

#include "Grid.h"
#include "Tile.h"
#include "Runtime/Core/Public/Async/ParallelFor.h"
#include "Chaos/Defines.h"
#include "Kismet/GameplayStatics.h"
#include "../MM_Connect3GameModeBase.h"

// Sets default values
AGrid::AGrid()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	MaxRows = 6;
	MaxColumns = 7;
	MinSelectionLength = 3;
	TileFallSpeed = 3.f;
	ScoreMult = 1;
}

// Called when the game starts or when spawned
void AGrid::BeginPlay()
{
	Super::BeginPlay();
	playerController = UGameplayStatics::GetPlayerController(this, 0);
	GameMode = (AMM_Connect3GameModeBase*)UGameplayStatics::GetGameMode(this);
	Initialize();
}
// Called every frame
void AGrid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (playerController->IsInputKeyDown(EKeys::LeftMouseButton))
	{
		playerController->DeprojectMousePositionToWorld(mouseLocation, mouseDirection);
		FVector2D mouseAddress = LocationToGridAddress(mouseLocation);
		//UE_LOG(LogTemp, Log, TEXT("last is %s"), *mouseAddress.ToString());
		if (mouseAddress.X >= 0 && mouseAddress.Y >= 0)
		{
			int32 tileIndex = GridAddressToIndex(mouseAddress.X, mouseAddress.Y);
			if (SpawnedTiles[tileIndex] != nullptr && !SpawnedTiles[tileIndex]->bIsSelected)
			{
				SpawnedTiles[tileIndex]->bIsSelected = AddToSelection(SpawnedTiles[tileIndex]);
			}
		}
		else { PointerUp(); }
	}
	else { PointerUp(); }

	UpdateTilesPosition();

}

void AGrid::Initialize()
{
	SpawnedTiles.Empty(2*MaxRows * MaxColumns);
	SpawnedTiles.AddZeroed(SpawnedTiles.Max());
	for (int32 row = 0; row < MaxRows; ++row)
	{
		for (int32 column = 0; column < MaxColumns; ++column)
		{
			int32 tileTypeID = GetRandomTileID();
			FVector spawnLocation = GridAddressToLocation(row+MaxRows,column);
			CreateTile(TileLibrary[tileTypeID].TileClass, spawnLocation, GridAddressToIndex(row + MaxRows, column), tileTypeID, 0);
		}
	}

	minColumn = 0, maxColumn = MaxColumns;
	minRow = 0, maxRow = 2 * MaxRows;
	bUpdateTiles = true;
}

void AGrid::CreateTile(TSubclassOf<class ATile> tileObject, FVector spawnLocation, int32 tileIndex, int32 tileTypeID, int32 targetIndex)
{
	// check if assigned blueprint asset is not null
	if (tileObject)
	{
		checkSlow(TileLibrary.IsValidIndex(tileTypeID));
		// Check for a valid World:
		UWorld* const World = GetWorld();
		if (World)
		{
			// Set the spawn parameters.
			FActorSpawnParameters spawnParams;
			spawnParams.Owner = this;
			spawnParams.Instigator = GetInstigator();
			spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			FRotator SpawnRotation(0.0f, 0.0f, 0.0f);
			// Spawn the tile.
			ATile* const newTile = World->SpawnActor<ATile>(tileObject, spawnLocation, SpawnRotation, spawnParams);

			newTile->TileTypeID = tileTypeID;
			newTile->CurrentIndex = tileIndex;
			newTile->TargetIndex = targetIndex;
			SpawnedTiles[tileIndex] = newTile;
		}
	}
}

void AGrid::UpdateTilesPosition()
{
	if (bUpdateTiles)
	{
		int32 count = 0;

		for (int32 row = minRow; row < maxRow; row++)
		{
			for (int32 Column = minColumn; Column < maxColumn; Column++)
			{
				int32 currentIndex = GridAddressToIndex(row, Column);
				int32 targetIndex = GridAddressToIndex(row - 1, Column); //target for falling

				if (SpawnedTiles[currentIndex] != nullptr && SpawnedTiles[currentIndex]->TargetIndex != SpawnedTiles[currentIndex]->CurrentIndex) //imples tile is falling
				{
					FVector targetLocation = GridAddressToLocation(row - 1, Column); //just need to consider next block below as the target

					FVector newLocation = SpawnedTiles[currentIndex]->GetActorLocation();
					newLocation.Z -= TileFallSpeed;
					SpawnedTiles[currentIndex]->SetActorLocation(newLocation);

					if (newLocation.Z < targetLocation.Z) //Time to finish updating this tile
					{
						SpawnedTiles[currentIndex]->SetActorLocation(targetLocation);
						if (SpawnedTiles[targetIndex] == nullptr)
						{
							SpawnedTiles[currentIndex]->CurrentIndex = targetIndex;
							SpawnedTiles[targetIndex] = SpawnedTiles[currentIndex];
							SpawnedTiles[currentIndex] = nullptr;
						}
					}
					if ((row != maxRow - 1) && SpawnedTiles[GridAddressToIndex(row + 1, Column)] != nullptr)
					{
						SpawnedTiles[GridAddressToIndex(row + 1, Column)]->TargetIndex = currentIndex;
					}
					continue;
				}
				else if (SpawnedTiles[currentIndex] == nullptr)
				{
					if ((row != maxRow - 1) && SpawnedTiles[GridAddressToIndex(row + 1, Column)] != nullptr)
					{
						SpawnedTiles[GridAddressToIndex(row + 1, Column)]->TargetIndex = currentIndex;
					}
					continue;
				}
				count++;
			}
		}
		//UE_LOG(LogTemp, Log, TEXT("last is %d"), count - (maxRow - minRow) * (maxColumn - minColumn)/2);
		if (count == (maxRow - minRow) * (maxColumn - minColumn) / 2) { bUpdateTiles = false; }
	}
}

bool AGrid::AddToSelection(ATile* tileToAdd)
{
	if (bUpdateTiles) return false; //Donot let selection happen while tiles are falling

	bool bVerticalAdjacent, bHorizontalAdjacent;
	if (SelectedTiles.Num() < 1) 
	{
		SelectedTiles.Add(tileToAdd);
		tileToAdd->SelectTile();
		return true;
	}
	else if (SelectedTiles.Num() < 2)
	{
		FVector2D firstAddress = IndexToGridAddress(SelectedTiles[0]->CurrentIndex);
		FVector2D newAddress = IndexToGridAddress(tileToAdd->CurrentIndex);

		bVerticalAdjacent = (firstAddress.Y == newAddress.Y) // selected in same direction
			&& FMath::Abs(firstAddress.X - newAddress.X) == 1; // new item is adjacent

		bHorizontalAdjacent = (firstAddress.X == newAddress.X)  // selected in same direction
			&& FMath::Abs(firstAddress.Y - newAddress.Y) == 1; // new item is adjacent
	}
	else
	{
		FVector2D firstAddress = IndexToGridAddress(SelectedTiles[0]->CurrentIndex);
		FVector2D lastAddress = IndexToGridAddress(SelectedTiles.Last()->CurrentIndex);
		FVector2D newAddress = IndexToGridAddress(tileToAdd->CurrentIndex);

		//UE_LOG(LogTemp, Log, TEXT("1st is %s"), *firstAddress.ToString());
		//UE_LOG(LogTemp, Log, TEXT("last is %s"), *lastAddress.ToString());
		//UE_LOG(LogTemp, Log, TEXT("Tnew is %s"), *newAddress.ToString());

		bVerticalAdjacent = (firstAddress.Y == lastAddress.Y && lastAddress.Y == newAddress.Y) // selected in same direction
									&& FMath::Abs(lastAddress.X- newAddress.X) == 1; // new item is adjacent

		bHorizontalAdjacent = (firstAddress.X == lastAddress.X && lastAddress.X == newAddress.X)  // selected in same direction
									&& FMath::Abs(lastAddress.Y - newAddress.Y) == 1; // new item is adjacent		
	}

	if( !(!bVerticalAdjacent && !bHorizontalAdjacent)&& 
		SelectedTiles[0]->TileTypeID == tileToAdd->TileTypeID)
	{
		SelectedTiles.Add(tileToAdd);
		tileToAdd->SelectTile();

		return true;
	}

	for (ATile* tile : SelectedTiles)
	{
		tile->DeselectTile();
	}
	SelectedTiles.Empty();
	return false;
}

void AGrid::PointerUp()
{
	if (SelectedTiles.Num() >= MinSelectionLength)
	{
		GameMode->Score += SelectedTiles.Num() * ScoreMult;
		GameMode->ScoreValueUpdate(GameMode->Score);
		for (ATile* tile : SelectedTiles)
		{
			SpawnedTiles[tile->CurrentIndex] = nullptr;
			FVector2D gridAddress = IndexToGridAddress(tile->CurrentIndex);
			int32 tileTypeID = GetRandomTileID();
			FVector spawnLocation = GridAddressToLocation(gridAddress.X + MaxRows, gridAddress.Y);
			CreateTile(TileLibrary[tileTypeID].TileClass, spawnLocation, GridAddressToIndex(gridAddress.X + MaxRows, gridAddress.Y), tileTypeID, 0);
			tile->Destroy();
			bUpdateTiles = true;
		}
		SelectedTiles.Empty();
	}
	else
	{
		for (ATile* tile : SelectedTiles)
		{
			tile->DeselectTile();
		}
		SelectedTiles.Empty();
	}
}

int32 AGrid::GetRandomTileID()
{
	float probabilitySum = 0;
	for (auto tile : TileLibrary)
	{
		probabilitySum += tile.Probability;
	}
	float randomNum = FMath::FRandRange(0.0f, probabilitySum);
	float subDivision = 0;
	for (int32 i = 0; i < TileLibrary.Num(); i++)
	{
		subDivision += TileLibrary[i].Probability;
		if (randomNum <= subDivision)
		{
			return i;
		}
	}
	return 0;
}

#pragma region Convert A to B

FVector2D AGrid::IndexToGridAddress(int32 index)
{
	FVector2D address;
	address.Y = index % MaxColumns;
	address.X = FMath::FloorToInt(index / (float)MaxColumns);
	//UE_LOG(LogTemp, Log, TEXT("index is %d"), index);
	return address;
}

FVector2D AGrid::LocationToGridAddress(FVector location)
{//address.Y is address of column and address.X is for row
	FVector2D address = FVector2D(-1, -1);

	//Calculating whether pointer is in bounds or not
	bool bInXBounds = location.X >= GetActorLocation().X - TileSize.X / 2 &&
		location.X < (MaxColumns - 1)* TileSize.X + GetActorLocation().X + TileSize.X / 2;
	bool bInYBounds = location.Z >= GetActorLocation().Z - TileSize.Y / 2 &&
		location.Z < (MaxRows - 1)* TileSize.Y + GetActorLocation().Z + TileSize.Y / 2;

	//UE_LOG(LogTemp, Warning, TEXT("The boolean value is %s"), (bInXBounds && bInYBounds ? TEXT("true") : TEXT("false")));

	if (bInXBounds && bInYBounds)
	{
		address.Y = FMath::FloorToInt(0.5f + (location.X - GetActorLocation().X) / TileSize.X);
		address.X = FMath::FloorToInt(0.5f + (location.Z - GetActorLocation().Z) / TileSize.Y);
	}
	return address;
}

FVector AGrid::IndexToLocation(int32 index)
{// X represents row and Y column
	FVector2D address = IndexToGridAddress(index);
	FVector location = GetActorLocation();
	location.X += address.Y * TileSize.X;
	location.Z += address.X * TileSize.Y;
	return location;
}

FVector AGrid::GridAddressToLocation(int32 row, int32 column)
{
	FVector location = GetActorLocation();
	location.X += column * TileSize.X;
	location.Z += row * TileSize.Y;
	return location;
}

int32 AGrid::GridAddressToIndex(int32 row, int32 column)
{
	return (column + MaxColumns * row);
}

#pragma endregion


