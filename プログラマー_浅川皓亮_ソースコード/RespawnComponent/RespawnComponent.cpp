// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/RespawnComponent.h"


URespawnComponent::URespawnComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

// 処理の流れ:
// 1. Ownerを取得
// 2. Ownerの現在位置をInitialLocationに記録
void URespawnComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* Owner = GetOwner())
	{
		InitialLocation = Owner->GetActorLocation();
	}
}

void URespawnComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

// 処理の流れ:
// 1. Ownerを取得
// 2. Ownerの位置を記録しておいたInitialLocationに設定
void URespawnComponent::RespawnOwnerAtInitialLocation()
{
	if (AActor* Owner = GetOwner())
	{
		Owner->SetActorLocation(InitialLocation);
	}
}