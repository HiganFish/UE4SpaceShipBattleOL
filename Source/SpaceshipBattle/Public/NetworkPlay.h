// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Proto/PlayerMoveData.pb.h"
#include "Proto/SpawnEnemyData.pb.h"
#include "Networking.h"
#include "NetworkPlay.generated.h"

USTRUCT(BlueprintType)
struct FPlayerMoveData
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, Category = "MongoPlay")
		FString Username;
	UPROPERTY(BlueprintReadWrite, Category = "MongoPlay")
		FString UserUid;
	UPROPERTY(BlueprintReadWrite, Category = "MongoPlay")
		int64 TimeStamp;
	UPROPERTY(BlueprintReadWrite, Category = "MongoPlay")
		TArray<int32> MoveForward;
	UPROPERTY(BlueprintReadWrite, Category = "MongoPlay")
		TArray<int32> MoveRight;
	UPROPERTY(BlueprintReadWrite, Category = "MongoPlay")
		TArray<int32> Angle;
	UPROPERTY(BlueprintReadWrite, Category = "MongoPlay")
		TArray<bool> Fireing;
};


class AOtherSpaceShip;
class ASpaceShip;
class AEnemySpawner;

UCLASS()
class SPACESHIPBATTLE_API ANetworkPlay : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANetworkPlay();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	bool LastFireing;

	UFUNCTION(BlueprintCallable, category = "MongoPlay")
	void SendInputPackage(int32 MoveForward, int32 MoveRight, int32 Angle, bool Fireing);

	UFUNCTION(BlueprintCallable, category = "MongoPlay")
		bool ConnectToServer();

	UFUNCTION(BlueprintCallable, category = "MongoPlay")
		int64 GetTimeStamp();

	UPROPERTY(BlueprintReadWrite, Category = "MongoPlay")
		float SendInterval = 0.08f;

	UPROPERTY(BlueprintReadWrite, category = "MongoPlay")
		FString IpStr;

	UPROPERTY(BlueprintReadWrite, category = "MongoPlay")
		int32 Port;

	UPROPERTY(BlueprintReadWrite, category = "MongoPlay")
		FString Username;
	UPROPERTY(BlueprintReadWrite, category = "MongoPlay")
		FString UserUid;

	UPROPERTY(BlueprintReadWrite, category = "MongoPlay")
		TArray<FPlayerMoveData> ParsedPackage;
private:
	FSocket* PlayFd;
	void BigLittleSwap32(int32* A);
	// 1 big 0 little -1 default
	int Endian;
	bool CheckCPUBigEndian();
	int32 Htonl(int32 h);
	int32 Ntohl(int32 h);

private:
	enum ParseStatus { PARSE_ERROR, PARSE_HEADER, PARSE_BODY };
	void ParseData();
	ParseStatus Status;
	int32 DataLen;
	int32 NameLen;

private:
	// Per DelayTimes To Send Once
	float DelayTimes;
	float SpawnEnemyInterval;
	bool SendOnce;
	
	FTimerHandle SpawnEnemyHandler;
	void SpawnNewEnemy();

	mongo::PlayerMoveData* PlayerMoveDataForSend;
	mongo::PlayerMoveData* PlayerMoveDataForRecv;
	mongo::SpawnEnemyData* SpawnEnemyDataForRecv;

	uint8* EncodeToBuffer(int32* len, const google::protobuf::Message& message);
	void RecvAndSaveData();
	TArray<uint8> DataBuffer;

	int32 BufferLength;
	uint8* Buffer;
	int64 Time;


public:

	UPROPERTY(EditAnywhere, Category = "MongoPlay")
	TSubclassOf<ASpaceShip> MainSpaceShipObj;

	UPROPERTY(EditAnywhere, Category = "MongoPlay")
	TSubclassOf<AOtherSpaceShip> OtherSpaceShipObj;

	UPROPERTY(EditAnywhere, Category = "MongoPlay")
	TSubclassOf<AEnemySpawner> EnemySpawnerObj;

private:

	void ProcessInput();

	ASpaceShip* MainSpaceShip;
	AOtherSpaceShip* OtherSpaceShip;
	AEnemySpawner* EnemySpawner;

	TMap<FString, AOtherSpaceShip*> OtherShipMap;

	AOtherSpaceShip* GetOtherSpaceShipByUid(FString Uid);
	AOtherSpaceShip* SpawnNewOtherSpaceShip(int32 X, int32 Y, int32 Z);


	void ProcessInputPackage();
	void ProcessSpawnEnemyPackage();
};
