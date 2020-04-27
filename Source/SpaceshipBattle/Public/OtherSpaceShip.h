// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "NetworkPlay.h"
#include "OtherSpaceShip.generated.h"

class USphereComponent;
class UCameraComponent;
class USpringArmComponent;
class ABullet;
class USoundCue;

UCLASS()
class SPACESHIPBATTLE_API AOtherSpaceShip : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AOtherSpaceShip();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Component")
		USphereComponent* CollisionComp;

	UPROPERTY(VisibleAnywhere, Category = "Component")
		UStaticMeshComponent* ShipSM;

	UPROPERTY(EditAnywhere, Category = "Fire")
		TSubclassOf<ABullet> Bullet;

	UPROPERTY(VisibleAnywhere, Category = "Component")
		USceneComponent* SpawnPoint;

	UPROPERTY(BlueprintReadWrite, Category = "Move")
		float SpeedRate;

	FTimerHandle TimerHandle_BetweenShot;

	UPROPERTY(EditAnywhere, Category = "Fire")
		float TimeBetweenShot;

	UPROPERTY(EditAnywhere, Category = "Sound")
		USoundCue* GameOverCue;
	UPROPERTY(EditAnywhere, Category = "Sound")
		USoundCue* ShootCue;

	UPROPERTY(VisibleAnywhere, Category = "Component")
		UParticleSystemComponent* ThrusterParticleComp;

	UPROPERTY(EditAnywhere, Category = "Particle")
		UParticleSystem* ExplosionParticle;

	bool bUpMove;
	bool bRightMove;

	void OnDeath();

public:
	// For Internet Data

	UFUNCTION(BlueprintCallable, category = "MongoPlay")
	void MoveUp(int32 Value);
	UFUNCTION(BlueprintCallable, category = "MongoPlay")
	void MoveRight(int32 Value);
	UFUNCTION(BlueprintCallable, category = "MongoPlay")
	void SetAngle(int32 Angle);
	UFUNCTION(BlueprintCallable, category = "MongoPlay")
	void Fireing(bool fireing);

private:

	bool IsFireing;
	void Fire();
	void StartFire();
	void EndFire();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;


public:

	TArray<FPlayerMoveData> ParsedPackage;
	void AddParsedPackage(FPlayerMoveData Data);

};