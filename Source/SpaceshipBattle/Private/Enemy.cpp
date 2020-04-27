// Fill out your copyright notice in the Description page of Project Settings.

#include "Public/Enemy.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Public/SpaceShip.h"
#include "Kismet/KismetMathLibrary.h"
#include "Public/ShipGameMode.h"
#include "Public/EnemySpawner.h"
#include "Particles/ParticleSystem.h"

// Sets default values
AEnemy::AEnemy()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
	RootComponent = CollisionComp;

	ShipSM = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipSM"));
	ShipSM->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	MyGameMode= Cast<AShipGameMode>(UGameplayStatics::GetGameMode(this));
	TArray<AActor*> EnemySpawnerArray;
	UGameplayStatics::GetAllActorsOfClass(this, AEnemySpawner::StaticClass(), EnemySpawnerArray);
	EnemySpawner = Cast<AEnemySpawner>(EnemySpawnerArray[0]);

	SetColor();
}

void AEnemy::OnDeath()
{
	MyGameMode->IncreaseScore();
	EnemySpawner->DecreaseEnemyCount();
	SpawnExplosion();
	Destroy();
}

void AEnemy::SetEnemyId(int32 Id)
{
	EnemyId = Id;
}
int32 AEnemy::GetEnemyId()
{
	return EnemyId;
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

