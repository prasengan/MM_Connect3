// Fill out your copyright notice in the Description page of Project Settings.


#include "Tile.h"
#include "Grid.h"
#include "PaperSpriteComponent.h"
#include "Kismet/GameplayStatics.h"

ATile::ATile()
{
	bIsSelected = false;
	if (RootComponent)
	{
		RootComponent->SetMobility(EComponentMobility::Movable);
	}
}

void ATile::SelectTile()
{
	GetRenderComponent()->CreateAndSetMaterialInstanceDynamic(0)->SetScalarParameterValue("HighlightLerp", 0.15f);
	FVector scaleVector = FVector(1.15f);
	SetActorScale3D(scaleVector);
}

void ATile::DeselectTile()
{
	GetRenderComponent()->CreateAndSetMaterialInstanceDynamic(0)->SetScalarParameterValue("HighlightLerp", 0.0f);
	FVector scaleVector = FVector(1.f);
	SetActorScale3D(scaleVector);
	bIsSelected = false;
}

// Called when the game starts or when spawned
void ATile::BeginPlay()
{
	Super::BeginPlay();
	GetRenderComponent()->SetMobility(EComponentMobility::Movable);
}
