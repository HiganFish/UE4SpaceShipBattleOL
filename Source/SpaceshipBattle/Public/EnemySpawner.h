// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemySpawner.generated.h"

class AEnemy;
class UBoxComponent;
class ASpaceShip;

UCLASS()
class SPACESHIPBATTLE_API AEnemySpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEnemySpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere,Category="Enemy")
	TSubclassOf<AEnemy> EnemyObj;

	UPROPERTY(VisibleAnywhere,Category="Component")
	UBoxComponent* SpawnArea;

	int MaxEnemyNum;
	int CurrentEnemyCount;

	FRandomStream SpawnEnemyRamdomStream;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void DecreaseEnemyCount();

	int32 GetGenerateLocationAndId(FVector* Location);
	void SpawnEnemy(int32 X, int32 Y, int32 Z, int32 EnemyId);
};
