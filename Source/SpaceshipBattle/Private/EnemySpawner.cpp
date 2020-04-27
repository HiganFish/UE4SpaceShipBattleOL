// Fill out your copyright notice in the Description page of Project Settings.

#include "Public/EnemySpawner.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Public/SpaceShip.h"
#include "Engine/World.h"
#include "Public/Enemy.h"
#include "TimerManager.h"

// Sets default values
AEnemySpawner::AEnemySpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpawnArea = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnArea"));
	RootComponent = SpawnArea;
	MaxEnemyNum = 10;
	CurrentEnemyCount = 0;

	SpawnEnemyRamdomStream.Initialize(0);
}

// Called when the game starts or when spawned
void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();

	SpawnEnemy(200, 200, 100, 0);
	SpawnEnemy(-200, 200, 100, 1);
	SpawnEnemy(200, -200, 100, 2);
	SpawnEnemy(-200, -200, 100, 3);
}

int32 AEnemySpawner::GetGenerateLocationAndId(FVector* Location)
{
	if (CurrentEnemyCount >= MaxEnemyNum)
	{
		return -1;
	}

	*Location = UKismetMathLibrary::RandomPointInBoundingBox(SpawnArea->Bounds.Origin, SpawnArea->Bounds.BoxExtent);
	
	int32 RandInt32 = SpawnEnemyRamdomStream.RandRange(0, 10);
	if (RandInt32 == 8)
	{
		return 1;
	}
	if (RandInt32 == 9)
	{
		return 2;
	}
	if (RandInt32 == 10)
	{
		return 3;
	}
	return 0;
}



void AEnemySpawner::SpawnEnemy(int32 X, int32 Y, int32 Z, int32 EnemyId)
{
	if (!EnemyObj)
	{
		return;
	}
	FTransform Transform(FVector(X, Y, Z));
	AEnemy* Enemy = GetWorld()->SpawnActorDeferred<AEnemy>(EnemyObj, Transform);
	if (Enemy)
	{
		Enemy->SetEnemyId(EnemyId);

		UGameplayStatics::FinishSpawningActor(Enemy, Enemy->GetTransform());
		CurrentEnemyCount++;
	}
}

// Called every frame
void AEnemySpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentEnemyCount < MaxEnemyNum)
	{
		// Spawn
	}
}

void AEnemySpawner::DecreaseEnemyCount()
{
	if (CurrentEnemyCount > 0)
	{
		CurrentEnemyCount--;
		UE_LOG(LogTemp,Warning,TEXT("%s"),*FString::SanitizeFloat(CurrentEnemyCount));
	}
}

