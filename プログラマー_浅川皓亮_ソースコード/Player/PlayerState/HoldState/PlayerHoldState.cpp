// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/State/PlayerHoldState.h"
#include "Manager/LevelManager.h"
#include "Sound/SoundManager.h"
#include "InputActionValue.h"
#include "Interface/StateControllable.h"
#include "Logic/Movement/PlayerMoveLogic.h"
#include "Components/Color/ColorReactiveComponent.h"
#include "Components/MoveComponent.h"

UPlayerHoldState::UPlayerHoldState()
	: HoldTarget(nullptr)
{
}

// 処理の流れ:
// 1. Ownerを内部に保持
// 2. MoveComponentが未生成なら生成
// 3. 移動ロジックを初期化
bool UPlayerHoldState::OnEnter(APawn* Owner, UWorld* World)
{
	if (Owner == nullptr)
		return false;

	mOwner = Owner;

	if (!MoveComp)
	{
		MoveComp = NewObject<UMoveComponent>(mOwner);
		UPlayerMoveLogic* PlayerLogic = NewObject<UPlayerMoveLogic>(this);
		MoveComp->Init(PlayerLogic);
	}

	return true;
}

// 処理の流れ:
// 1. 必要な参照が揃っているか確認
// 2. 掴み対象が非表示になったら解除
// 3. 掴み開始時との距離をチェック
// 4. 一定距離以上離れたら解除
bool UPlayerHoldState::OnUpdate(float DeltaTime)
{
	if (HoldTarget == nullptr || mOwner == nullptr || TargetComp == nullptr)
		return false;

	if (TargetComp->IsHidden())
	{
		// 掴み解除 → Default ステートへ
		if (IStateControllable* Player = Cast<IStateControllable>(mOwner))
		{
			Player->ChangeState(EPlayerStateType::Default);
		}
		return true;
	}

	const float Distance =
		FVector::Dist(mOwner->GetActorLocation(), HoldTarget->GetActorLocation());

	const float MaxDistance = 20.f;

	if (Distance > InitialHoldDistance + MaxDistance)
	{
		// 距離が離れすぎたため解除
		if (IStateControllable* Player = Cast<IStateControllable>(mOwner))
		{
			Player->ChangeState(EPlayerStateType::Default);
		}
	}

	return true;
}

// 処理の流れ:
// 1. 掴み対象と関連コンポーネントをクリア
// 2. 置くSEを再生
bool UPlayerHoldState::OnExit(APawn* Owner)
{
	HoldTarget = nullptr;
	TargetComp = nullptr;

	ALevelManager::GetInstance(GetWorld())
		->GetSoundManager()
		->PlaySound("SE", "Put");

	return true;
}

// 処理の流れ:
// 1. 入力が押されたかを確認
// 2. 押されたら掴み解除
bool UPlayerHoldState::OnSkill(const FInputActionValue& Value)
{
	if (Value.Get<bool>())
	{
		if (IStateControllable* Player = Cast<IStateControllable>(mOwner))
		{
			Player->ChangeState(EPlayerStateType::Default);
		}
	}

	return true;
}

// 処理の流れ:
// 1. 掴み対象と移動コンポーネントの有効性確認
// 2. 入力から移動方向を取得
// 3. 移動方向に応じて向き情報を更新
// 4. 掴んだ向きを考慮した移動量を計算
// 5. プレイヤーと掴み対象を同時に移動
void UPlayerHoldState::Movement(const FInputActionValue& Value)
{
	if (!HoldTarget || !MoveComp)
		return;

	FVector Direction = MoveComp->Movement(0, mOwner, Value);
	if (Direction.IsNearlyZero())
	{
		MoveDelta = FVector::ZeroVector;
		return;
	}

	if (Direction.Y < 0)
	{
		MoveDirection = -1;
	}
	else if (Direction.Y > 0)
	{
		MoveDirection = 1;
	}
	else
	{
		MoveDirection = 0;
	}

	MoveDelta =
		Direction * 700.f * GetWorld()->GetDeltaSeconds() * GrabDirection;

	mOwner->AddActorWorldOffset(MoveDelta, true);
	HoldTarget->AddActorWorldOffset(MoveDelta, true);
}

// 処理の流れ:
// 1. 掴み対象を保持
// 2. 掴み開始時の距離を保存
// 3. 色反応コンポーネントを取得
// 4. 掴んだ向きを固定値として保存
void UPlayerHoldState::SetUp(AActor* Target, bool bGrabDirection)
{
	HoldTarget = Target;

	if (HoldTarget && mOwner)
	{
		InitialHoldDistance =
			FVector::Dist(mOwner->GetActorLocation(), HoldTarget->GetActorLocation());

		TargetComp =
			HoldTarget->GetComponentByClass<UColorReactiveComponent>();
	}

	GrabDirection = bGrabDirection ? 1 : -1;
}
