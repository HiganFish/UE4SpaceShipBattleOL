// Fill out your copyright notice in the Description page of Project Settings.

#include "Public/ShipGameMode.h"

AShipGameMode::AShipGameMode() {
	Score = 0;
}

void AShipGameMode::IncreaseScore()
{
	Score++;
}
