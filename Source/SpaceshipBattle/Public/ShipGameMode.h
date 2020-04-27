// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ShipGameMode.generated.h"

/**
 * 
 */
UCLASS()
class SPACESHIPBATTLE_API AShipGameMode : public AGameModeBase
{
	GENERATED_BODY()


protected:

	AShipGameMode();

	UPROPERTY(BlueprintReadOnly)
	int Score;
public:

	void IncreaseScore();
};
