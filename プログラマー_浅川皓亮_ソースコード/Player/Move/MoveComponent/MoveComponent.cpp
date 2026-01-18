// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/MoveComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/PlayerController.h"
#include "Math/UnrealMathUtility.h"
#include "Interface/MoveLogic.h"

// 処理の流れ:
// 1. コンポーネント生成時にデフォルト速度を設定
UMoveComponent::UMoveComponent()
{
	Speed = 600.0f;
}

// 処理の流れ:
// 1. 移動速度を設定
// 2. 使用する移動ロジックを保持
// 3. 移動ロジック側の初期化を実行
void UMoveComponent::Init(
	TScriptInterface<IMoveLogic> MoveLogicInterface,
	const float InSpeed,
	const FVector NewDirection
)
{
	Speed = InSpeed;
	MoveLogic = MoveLogicInterface;

	if (MoveLogic)
	{
		MoveLogic->Init(InSpeed, NewDirection);
	}
}

// 処理の流れ:
// 1. 移動ロジックが設定されているか確認
// 2. 未設定の場合はゼロベクトルを返却
// 3. 設定されている場合は移動ロジックに処理を委譲
FVector UMoveComponent::Movement(
	float DeltaTime,
	AActor* Owner,
	const FInputActionValue& Value
)
{
	if (!MoveLogic)
	{
		return FVector(0.f, 0.f, 0.f);
	}

	return MoveLogic->Movement(DeltaTime, Owner, Value);
}

// 処理の流れ:
// 1. 現在の速度を保持したまま移動方向を更新
// 2. 移動ロジック側を再初期化
void UMoveComponent::SetDirection(FVector NewDirection)
{
	if (MoveLogic)
	{
		MoveLogic->Init(Speed, NewDirection);
	}
}

// 処理の流れ:
// 1. 新しい移動ロジックの有効性を確認
// 2. 無効な場合は失敗を返却
// 3. 有効な場合は移動ロジックを差し替え
bool UMoveComponent::SetMoveLogic(TScriptInterface<IMoveLogic> Logic)
{
	if (!Logic)
	{
		return false;
	}

	MoveLogic = Logic;
	return true;
}
