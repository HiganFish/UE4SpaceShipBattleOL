// Fill out your copyright notice in the Description page of Project Settings.


#include "NetworkPlay.h"
#include "Misc/DateTime.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

#include "OtherSpaceShip.h"
#include "EnemySpawner.h"
#include "SpaceShip.h"

// Sets default values
ANetworkPlay::ANetworkPlay()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PlayFd = nullptr;
	DelayTimes = 0;
	SpawnEnemyInterval = 0.8f;
	SendOnce = false;
	Endian = -1;

	BufferLength = 200;
	Buffer = new uint8[BufferLength];
	Status = PARSE_HEADER;

	// IpStr = "182.92.232.240";
	IpStr = "192.168.80.127";

	Port = 8010;
	Username = "123";
	UserUid = "123-1587878300";
	LastFireing = false;
}

// Called when the game starts or when spawned
void ANetworkPlay::BeginPlay()
{
	Super::BeginPlay();

	PlayerMoveDataForSend = new mongo::PlayerMoveData;
	PlayerMoveDataForRecv = new mongo::PlayerMoveData;
	SpawnEnemyDataForRecv = new mongo::SpawnEnemyData;

	if (MainSpaceShipObj)
	{
		TArray<AActor*> Objs;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), MainSpaceShipObj, Objs);
		MainSpaceShip = Cast<ASpaceShip>(Objs[0]);
	}

	if (EnemySpawnerObj)
	{
		TArray<AActor*> Objs;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), EnemySpawnerObj, Objs);
		EnemySpawner = Cast<AEnemySpawner>(Objs[0]);
		if (EnemySpawner)
		{
			GetWorldTimerManager().SetTimer(SpawnEnemyHandler, this, &ANetworkPlay::SpawnNewEnemy, SpawnEnemyInterval, true, 0.0f);
		}
	}

}

void ANetworkPlay::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (DelayTimes > SendInterval)
	{
		SendOnce = true;
		DelayTimes = 0.0f;
	}
	DelayTimes += DeltaTime;


	if (Status != PARSE_ERROR)
	{
		RecvAndSaveData();
		ParseData();
	}
	else
	{
		PlayFd->Close();
	}

	ProcessInput();
}

void ANetworkPlay::SendInputPackage(int32 MoveForward, int32 MoveRight, int32 Angle, bool Fireing)
{
	if (PlayFd == nullptr)
	{
		return;
	}

	PlayerMoveDataForSend->add_move_forward(MoveForward);
	PlayerMoveDataForSend->add_move_right(MoveRight);
	PlayerMoveDataForSend->add_angle(Angle);

	if (LastFireing != Fireing)
	{
		PlayerMoveDataForSend->add_fireing(Fireing);
		LastFireing = Fireing;
	}
	
	if (SendOnce)
	{
		SendOnce = false;
	
		PlayerMoveDataForSend->set_username(TCHAR_TO_ANSI(*Username));
		PlayerMoveDataForSend->set_uuid(TCHAR_TO_ANSI(*UserUid));
		PlayerMoveDataForSend->set_timestamp(GetTimeStamp());

		int32 Len = 0;
		uint8* Data = EncodeToBuffer(&Len, *PlayerMoveDataForSend);

		int32 SendBytes = 0;
		if (PlayFd->GetConnectionState() == ESocketConnectionState::SCS_Connected)
		{
			PlayFd->Send(Data, Len, SendBytes);
		}
		else
		{
			PlayFd = nullptr;
		}
		PlayerMoveDataForSend->Clear();

	}
}

bool ANetworkPlay::ConnectToServer()
{
	if (PlayFd == nullptr)
	{
		// 将字符串ip转换为点分十进制ip

		FIPv4Address Ip;
		FIPv4Address::Parse(IpStr, Ip);

		// 将十进制ip 转化成网络地址
		TSharedPtr<FInternetAddr> addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
		addr->SetIp(Ip.Value);
		addr->SetPort(Port);

		// 创建TCP Socket 文件描述符
		PlayFd = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("TCP SOCKET"), false);

		if (!PlayFd->Connect(*addr))
		{
			return false;
		}
	}
	return true;
}

int64 ANetworkPlay::GetTimeStamp()
{
	return FDateTime::Now().ToUnixTimestamp();
}

// 长整型大小端互换
void ANetworkPlay::BigLittleSwap32(int32* A)
{
	uint8* Temp = (uint8*)A;

	uint8 Temp1 = Temp[0];
	Temp[0] = Temp[3];
	Temp[3] = Temp1;

	uint8 Temp2 = Temp[1];
	Temp[1] = Temp[2];
	Temp[2] = Temp2;
}

// 本机大端返回1，小端返回0
bool ANetworkPlay::CheckCPUBigEndian()
{
	if (Endian == -1)
	{
		union
		{
			unsigned long int i;
			unsigned char s[4];
		}c;

		c.i = 0x12345678;
		Endian = 0x12 == c.s[0] ? 1 : 0;
	}
	return Endian == 1;
}

// 模拟htonl函数，本机字节序转网络字节序
int32 ANetworkPlay::Htonl(int32 h)
{
	if (!CheckCPUBigEndian())
	{
		BigLittleSwap32(&h);
	}
	return h;
}

int32 ANetworkPlay::Ntohl(int32 h)
{
	if (!CheckCPUBigEndian())
	{
		BigLittleSwap32(&h);
	}
	return h;
}

void ANetworkPlay::RecvAndSaveData()
{
	if (PlayFd == nullptr)
	{
		return;
	}

	uint32 PendingSize = 0;
	if (PlayFd->HasPendingData(PendingSize))
	{
		int32 RecvSize = 0;
		bool Result = PlayFd->Recv(Buffer, BufferLength, RecvSize);
		if (!Result)
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("Recv Errror"));
			return;
		}

		for (int i = 0; i < RecvSize; ++i)
		{
			DataBuffer.Add(Buffer[i]);
		}
	}

}

void ANetworkPlay::ParseData()
{
	int32 ParsedData = 0;
	while (DataBuffer.Num() - ParsedData > 0)
	{
		if (Status == PARSE_HEADER)
		{
			if (DataBuffer.Num() - ParsedData < 8)
			{
				break;
			}

			NameLen = Ntohl(*(int32*)&DataBuffer[ParsedData + 4]);
			DataLen = Ntohl(*(int32*)&DataBuffer[ParsedData + 0]);

			Status = PARSE_BODY;
		}

		if (Status == PARSE_BODY)
		{
			if (DataLen > 1500)
			{
				Status = PARSE_ERROR;
				GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("The Data Len is too big"));
				break;
			}
			if (DataBuffer.Num() - ParsedData < DataLen + 4)
			{
				break;
			}

			int32 MessageSum = Ntohl(*(int32*)&DataBuffer[ParsedData + DataLen]);
			// int32 TrueSum = adler32(&DataBuffer[4], DataLen - 4);
			int32 TrueSum = 1609676181;
			if (MessageSum != TrueSum)
			{
				Status = PARSE_ERROR;
				GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, FString::Printf(TEXT("InValid CheckSum Msg: %d, True: %d"), MessageSum, TrueSum));
				break;
			}

			bool Result;

			if (!memcmp("mongo.PlayerMoveData\0", &DataBuffer[ParsedData + 8], NameLen))
			{
				Result = PlayerMoveDataForRecv->ParseFromArray(&DataBuffer[ParsedData + 8 + NameLen], DataLen - 8 - NameLen);
				if (!Result)
				{
					Status = PARSE_ERROR;
					GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Can't parse data"));
					break;
				}
				ProcessInputPackage();
			}
			else if (!memcmp("mongo.SpawnEnemyData\0", &DataBuffer[ParsedData + 8], NameLen))
			{
				Result = SpawnEnemyDataForRecv->ParseFromArray(&DataBuffer[ParsedData + 8 + NameLen], DataLen - 8 - NameLen);
				if (!Result)
				{
					Status = PARSE_ERROR;
					GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Can't parse data"));
					break;
				}
				ProcessSpawnEnemyPackage();
			}
			else
			{
				Status = PARSE_ERROR;
				GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Invalid Data Name"));
				break;
			}

			ParsedData += DataLen + 4;
			Status = PARSE_HEADER;
		}
	}
	DataBuffer.RemoveAt(0, ParsedData);
}

void ANetworkPlay::SpawnNewEnemy()
{
	FVector Location;
	int32 EnemyId = EnemySpawner->GetGenerateLocationAndId(&Location);
	if (EnemyId == -1)
	{
		return;
	}

	mongo::SpawnEnemyData SpawnEnemyDataPack;
	SpawnEnemyDataPack.set_username(TCHAR_TO_ANSI(*Username));
	SpawnEnemyDataPack.set_uuid(TCHAR_TO_ANSI(*UserUid));
	SpawnEnemyDataPack.set_timestamp(GetTimeStamp());
	SpawnEnemyDataPack.set_x(Location.X);
	SpawnEnemyDataPack.set_y(Location.Y);
	SpawnEnemyDataPack.set_z(Location.Z);
	SpawnEnemyDataPack.set_enemy_id(EnemyId);

	int32 Length = 0;
	int32 SendBytes = 0;
	uint8* Data = EncodeToBuffer(&Length, SpawnEnemyDataPack);

	if (PlayFd->GetConnectionState() == ESocketConnectionState::SCS_Connected)
	{
		PlayFd->Send(Data, Length, SendBytes);
	}
	else
	{
		PlayFd = nullptr;
	}
}

uint8* ANetworkPlay::EncodeToBuffer(int32* Length, const google::protobuf::Message& Message)
{
	int32 NameLen = Message.GetTypeName().length() + 1;
	int32 ProtoDataBegin = 8 + NameLen;
	int32 ProtoDataLen = Message.ByteSizeLong();
	int32 SumLen = ProtoDataBegin + ProtoDataLen + 4;
	int32 Len = SumLen - 4;

	uint8* Data = new uint8[SumLen];

	Message.SerializePartialToArray(&Data[ProtoDataBegin], ProtoDataLen);

	Len = Htonl(Len);
	memcpy(&Data[0], &Len, 4);

	memcpy(&Data[8], Message.GetTypeName().c_str(), NameLen);

	NameLen = Htonl(NameLen);
	memcpy(&Data[4], &NameLen, 4);

	// int32 CheckSum = adler32(&Data[4], SumLen - 8);
	int32 CheckSum = 1609676181;
	CheckSum = Htonl(CheckSum);
	memcpy(&Data[SumLen - 4], &CheckSum, 4);

	*Length = SumLen;
	return Data;
}

void ANetworkPlay::ProcessInput()
{
	if (ParsedPackage.Num() == 0)
	{
		return;
	}

	int Num = ParsedPackage.Num();
	for (int i = 0; i < Num; ++i)
	{
		if (ParsedPackage[i].UserUid == UserUid)
		{
			MainSpaceShip->AddParsedPackage(ParsedPackage[i]);
		}
		else
		{
			AOtherSpaceShip* OtherShip = GetOtherSpaceShipByUid(ParsedPackage[i].UserUid);
			OtherShip->AddParsedPackage(ParsedPackage[i]);
		}
	}
	ParsedPackage.RemoveAt(0, Num);
}

AOtherSpaceShip* ANetworkPlay::GetOtherSpaceShipByUid(FString Uid)
{
	AOtherSpaceShip** OtherShip = OtherShipMap.Find(Uid);
	AOtherSpaceShip* Result;
	if (OtherShip == nullptr)
	{
		Result = SpawnNewOtherSpaceShip(0, 0, 100);
		OtherShipMap.Add(Uid, Result);
		return Result;
	}
	return *OtherShip;
}

AOtherSpaceShip* ANetworkPlay::SpawnNewOtherSpaceShip(int32 X, int32 Y, int32 Z)
{
	if (OtherSpaceShipObj)
	{
		return OtherSpaceShip = GetWorld()->SpawnActor<AOtherSpaceShip>(OtherSpaceShipObj, FVector(0, 0, 100), FRotator(0.0f, 0.0f, 0.0f));
	}

	return nullptr;
}

void ANetworkPlay::ProcessInputPackage()
{
	FPlayerMoveData Package;
	Package.Username = FString(PlayerMoveDataForRecv->username().c_str());
	Package.UserUid = FString(PlayerMoveDataForRecv->uuid().c_str());
	Package.TimeStamp = PlayerMoveDataForRecv->timestamp();
	GEngine->AddOnScreenDebugMessage(1234, 0.3f, FColor::Green, FString::Printf(TEXT("DelayTime: %ld"), GetTimeStamp() - Package.TimeStamp));

	for (int i = 0; i < PlayerMoveDataForRecv->move_forward_size(); ++i)
	{
		Package.MoveForward.Add(PlayerMoveDataForRecv->move_forward(i));
	}
	for (int i = 0; i < PlayerMoveDataForRecv->move_right_size(); ++i)
	{
		Package.MoveRight.Add(PlayerMoveDataForRecv->move_right(i));
	}
	for (int i = 0; i < PlayerMoveDataForRecv->fireing_size(); ++i)
	{
		Package.Fireing.Add(PlayerMoveDataForRecv->fireing(i));
	}
	for (int i = 0; i < PlayerMoveDataForRecv->angle_size(); ++i)
	{
		Package.Angle.Add(PlayerMoveDataForRecv->angle(i));
	}

	ParsedPackage.Add(Package);
	PlayerMoveDataForRecv->Clear();
}

void ANetworkPlay::ProcessSpawnEnemyPackage()
{
	EnemySpawner->SpawnEnemy(SpawnEnemyDataForRecv->x(),
		SpawnEnemyDataForRecv->y(),
		SpawnEnemyDataForRecv->z(),
		SpawnEnemyDataForRecv->enemy_id());
}
