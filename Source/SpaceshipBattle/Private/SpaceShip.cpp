// Fill out your copyright notice in the Description page of Project Settings.

#include "Public/SpaceShip.h"
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
#include "Engine/Engine.h"

// Sets default values
ASpaceShip::ASpaceShip()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
	RootComponent = CollisionComp;

	ShipSM = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipSM"));
	ShipSM->SetupAttachment(RootComponent);

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->SetupAttachment(RootComponent);

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);

	SpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("SpawnPoint"));
	SpawnPoint->SetupAttachment(ShipSM);

	ThrusterParticleComp = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("TrusterParticle"));
	ThrusterParticleComp->SetupAttachment(RootComponent);

	SpeedRate = 1;
	BuffTime = 3;
	BulltScaleRate = 1;

	ShotInterval = 0.2f;

	IsFireing = false;

}

// Called when the game starts or when spawned
void ASpaceShip::BeginPlay()
{
	Super::BeginPlay();
	
	PC =Cast<APlayerController>(GetController());
	PC->bShowMouseCursor = true;
}

void ASpaceShip::MoveUp(int32 Value)
{
	if (Value != 0) {
		bUpMove = true;
	}
	else {
		bUpMove = false;
	}
	AddMovementInput(FVector::ForwardVector,Value);
}

void ASpaceShip::MoveRight(int32 Value)
{
	if (Value != 0) {
		bRightMove = true;
	}
	else {
		bRightMove = false;
	}
	AddMovementInput(FVector::RightVector,Value);
}


void ASpaceShip::Fire()
{
	if (Bullet)
	{
		FTransform Transform(SpawnPoint->GetComponentLocation());
		Transform.SetRotation(SpawnPoint->GetComponentRotation().Quaternion());
		Transform.SetScale3D(FVector::OneVector * BulltScaleRate);

		GetWorld()->SpawnActor<ABullet>(Bullet, Transform);
		if(ShootCue)
		UGameplayStatics::PlaySoundAtLocation(this, ShootCue, GetActorLocation());
	}
}

void ASpaceShip::StartFire()
{
	GetWorldTimerManager().SetTimer(TimerHandle_BetweenShot, this, &ASpaceShip::Fire, ShotInterval, true, 0.0f);
}

void ASpaceShip::EndFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_BetweenShot);
}

void ASpaceShip::OnDeath()
{
	/*bDead = true;
	CollisionComp->SetVisibility(false,true);
	
	GetWorldTimerManager().SetTimer(TimerHandle_Restart,this,&ASpaceShip::RestartLevel,2.0f,false);*/

	if (GameOverCue)
	UGameplayStatics::PlaySoundAtLocation(this, GameOverCue, GetActorLocation());
	if (ExplosionParticle)
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionParticle, GetActorLocation(), FRotator::ZeroRotator, true);
}

void ASpaceShip::SetAngle(int32 Angle)
{
	SetActorRotation(FRotator(0.0f, Angle, 0.0f));
}

void ASpaceShip::Fireing(bool fireing)
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

int32 ASpaceShip::GetAngle()
{
	FVector MouseLocation, MouseDirection;
	PC->DeprojectMousePositionToWorld(MouseLocation, MouseDirection);
	FVector TargetLocation = FVector(MouseLocation.X, MouseLocation.Y, GetActorLocation().Z);
	FRotator Rotator = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), TargetLocation);
	return Rotator.Yaw;
}

// Called every frame
void ASpaceShip::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (bUpMove || bRightMove) {
		ThrusterParticleComp->Activate();
	}
	else {
		ThrusterParticleComp->Deactivate();
	}

	AddActorWorldOffset(ConsumeMovementInputVector() * 1.5f * SpeedRate, true);

	GEngine->AddOnScreenDebugMessage(11, 0.2f, FColor::Red, FString::Printf(TEXT("SpeedRate - %d"), SpeedRate));
	GEngine->AddOnScreenDebugMessage(12, 0.2f, FColor::Blue, FString::Printf(TEXT("ShotInterval - %f"), ShotInterval));
	GEngine->AddOnScreenDebugMessage(13, 0.2f, FColor::Yellow, FString::Printf(TEXT("BulltScaleRate - %d"), BulltScaleRate));

}

// Called to bind functionality to input
void ASpaceShip::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ASpaceShip::NotifyActorBeginOverlap(AActor * OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);
	AEnemy* Enemy = Cast<AEnemy>(OtherActor);
	if (Enemy) {

		int32 EnemyId = Enemy->GetEnemyId();

		if (EnemyId == 1)
		{
			SetCoinBulltSpeedUp();
		}
		else if (EnemyId == 2)
		{
			SetCoinBulltBig();
		}
		else if (EnemyId == 3)
		{
			SetCoinSpeedUp();
		}
		else if (EnemyId == 0)
		{
			OnDeath();
		}
		Enemy->OnDeath();
		Enemy->Destroy();
	}
}

void ASpaceShip::AddParsedPackage(FPlayerMoveData Data)
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

void ASpaceShip::SetCoinSpeedUp()
{
	SpeedRate = 3;

	GetWorldTimerManager().SetTimer(TimerHandlerCoinSpeedUp, this, &ASpaceShip::RemoveCoinSpeedUp, BuffTime);
}

void ASpaceShip::SetCoinBulltSpeedUp()
{
	ShotInterval = 0.1f;
	GetWorldTimerManager().SetTimer(TimerHandlerCoinBulltSpeedUp, this, &ASpaceShip::RemoveCoinBulltSpeedUp, BuffTime);
}

void ASpaceShip::SetCoinBulltBig()
{
	BulltScaleRate = 3;
	GetWorldTimerManager().SetTimer(TimerHandlerCoinBulltBig, this, &ASpaceShip::RemoveCoinBulltBig, BuffTime);
}

void ASpaceShip::RemoveCoinSpeedUp()
{
	SpeedRate = 1;
	EndFire();
}

void ASpaceShip::RemoveCoinBulltSpeedUp()
{
	ShotInterval = 0.2f;
}

void ASpaceShip::RemoveCoinBulltBig()
{
	BulltScaleRate = 1;
}