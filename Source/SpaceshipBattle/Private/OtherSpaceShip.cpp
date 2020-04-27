// Fill out your copyright notice in the Description page of Project Settings.

#include "Public/OtherSpaceShip.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/SpringArmComponent.h"
#include "Public/Misc/App.h"
#include "Engine/World.h"
#include "Public/Bullet.h"
#include "TimerManager.h"
#include "Public/Enemy.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"

// Sets default values
AOtherSpaceShip::AOtherSpaceShip()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
	RootComponent = CollisionComp;

	ShipSM = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipSM"));
	ShipSM->SetupAttachment(RootComponent);

	SpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("SpawnPoint"));
	SpawnPoint->SetupAttachment(ShipSM);

	ThrusterParticleComp = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("TrusterParticle"));
	ThrusterParticleComp->SetupAttachment(RootComponent);

	TimeBetweenShot = 0.2f;
	SpeedRate = 1;
}

// Called when the game starts or when spawned
void AOtherSpaceShip::BeginPlay()
{
	Super::BeginPlay();

}

void AOtherSpaceShip::MoveUp(int32 Value)
{
	if (Value != 0) {
		bUpMove = true;
	}
	else {
		bUpMove = false;
	}
	AddMovementInput(FVector::ForwardVector, Value);
}

void AOtherSpaceShip::MoveRight(int32 Value)
{
	if (Value != 0) {
		bRightMove = true;
	}
	else {
		bRightMove = false;
	}
	AddMovementInput(FVector::RightVector, Value);
}


void AOtherSpaceShip::Fire()
{
	if (Bullet)
	{
		FActorSpawnParameters SpawnParams;
		GetWorld()->SpawnActor<ABullet>(Bullet, SpawnPoint->GetComponentLocation(), SpawnPoint->GetComponentRotation(), SpawnParams);
		if (ShootCue)
			UGameplayStatics::PlaySoundAtLocation(this, ShootCue, GetActorLocation());
	}
}

void AOtherSpaceShip::StartFire()
{
	GetWorldTimerManager().SetTimer(TimerHandle_BetweenShot, this, &AOtherSpaceShip::Fire, TimeBetweenShot, true, 0.0f);
}

void AOtherSpaceShip::EndFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_BetweenShot);
}

void AOtherSpaceShip::OnDeath()
{
	/*bDead = true;
	CollisionComp->SetVisibility(false,true);

	GetWorldTimerManager().SetTimer(TimerHandle_Restart,this,&AOtherSpaceShip::RestartLevel,2.0f,false);*/

	if (GameOverCue)
		UGameplayStatics::PlaySoundAtLocation(this, GameOverCue, GetActorLocation());
	if (ExplosionParticle)
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionParticle, GetActorLocation(), FRotator::ZeroRotator, true);
}

void AOtherSpaceShip::SetAngle(int32 Angle)
{
	SetActorRotation(FRotator(0.0f, Angle, 0.0f));
}

void AOtherSpaceShip::Fireing(bool fireing)
{
	if (fireing)
	{
		if (!IsFireing)
		{
			IsFireing = true;
			StartFire();
		}
	}
	else
	{
		if (IsFireing)
		{
			IsFireing = false;
			EndFire();
		}
	}
}

// Called every frame
void AOtherSpaceShip::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (bUpMove || bRightMove)
	{
		ThrusterParticleComp->Activate();
	}
	else
	{
		ThrusterParticleComp->Deactivate();
	}

	AddActorWorldOffset(ConsumeMovementInputVector() * 1.5f, true);
}

// Called to bind functionality to input
void AOtherSpaceShip::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AOtherSpaceShip::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);
	AEnemy* Enemy = Cast<AEnemy>(OtherActor);
	if (Enemy) {
		Enemy->Destroy();
		OnDeath();
		//	Destroy();
	}
}

void AOtherSpaceShip::AddParsedPackage(FPlayerMoveData Data)
{
	for (int i = 0; i < Data.Fireing.Num(); ++i)
	{
		Fireing(Data.Fireing[i]);
	}
	for (int i = 0; i < Data.MoveForward.Num(); ++i)
	{
		MoveUp(Data.MoveForward[i]);
	}
	for (int i = 0; i < Data.MoveRight.Num(); ++i)
	{
		MoveRight(Data.MoveRight[i]);
	}
	SetAngle(Data.Angle[0]);
}

