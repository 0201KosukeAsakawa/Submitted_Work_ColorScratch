// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/State/LadderClimberState.h"
#include "Player/State/StateManager.h"
#include "Player/PlayerCharacter.h"
#include "Objects/Color/LadderActor.h"
#include "Components/MoveComponent.h"
#include "Components/Color/ColorConfigurator.h"
#include "Components/CapsuleComponent.h"
#include "Components/PhysicsCalculator.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/OverlapResult.h"
#include "Logic/Movement/LadderMoveLogic.h"



// 処理の流れ:
// 1. Ladderの有効性を確認
// 2. 梯子アクターのバウンディング情報を取得
// 3. X,Yを梯子中心、Zを0とした位置を返却
FVector GetLadderCenterXZ(const AActor* Ladder)
{
	if (!Ladder)
		return FVector::ZeroVector;

	FVector Origin;
	FVector BoxExtent;
	Ladder->GetActorBounds(true, Origin, BoxExtent);

	return FVector(Origin.X, Origin.Y, 0.f);
}

// 処理の流れ:
// 1. 引数の梯子の有効性を確認
// 2. 昇降対象の梯子をメンバに保持
// 3. 梯子に対する固定位置を取得
// 4. 梯子のColorConfiguratorを取得
// 5. プレイヤー位置を梯子位置へ補正
void ULadderClimberState::SetTargetLadder(ALadderActor* LadderActor)
{
	if (LadderActor == nullptr)
		return;

	Ladder = LadderActor;

	FixedPosition = Ladder->GetFixedPositionForActor(GetOwner());

	UColorConfigurator* Comp = Ladder->GetComponentByClass<UColorConfigurator>();
	if (Comp != nullptr)
	{
		TargetComp = Comp;
	}

	GetOwner()->SetActorLocation(FixedPosition);
}

// 処理の流れ:
// 1. OwnerとWorldを保持
// 2. MoveComponentと梯子移動ロジックを初期化
// 3. 物理挙動を停止
// 4. キャラクターの速度をリセット
// 5. 重力を無効化しフライングモードに変更
// 6. コリジョンを無効化
bool ULadderClimberState::OnEnter(APawn* Owner, UWorld* World)
{
	if (!Owner)
		return false;

	if (!mOwner)
		mOwner = Owner;

	if (!pWorld)
		pWorld = World;

	if (!MoveComp)
	{
		MoveComp = NewObject<UMoveComponent>(mOwner);
		ULadderMoveLogic* PlayerLogic = NewObject<ULadderMoveLogic>(this);
		MoveComp->Init(PlayerLogic);
	}

	UPhysicsCalculator* Physics = GetOwner()->GetComponentByClass<UPhysicsCalculator>();
	if (Physics == nullptr)
		return false;

	Physics->AddForce(FVector::ZeroVector, 0);

	if (ACharacter* Character = Cast<ACharacter>(mOwner))
	{
		if (UCharacterMovementComponent* CharMove = Character->GetCharacterMovement())
		{
			CharMove->Velocity = FVector::ZeroVector;
			CharMove->GravityScale = 0.f;
			CharMove->SetMovementMode(EMovementMode::MOVE_Flying);

			if (UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
			{
				Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
		}
	}

	return true;
}

// 処理の流れ:
// 1. Ownerおよび梯子の有効性を確認
// 2. 梯子が非表示状態の場合は通常ステートへ遷移
// 3. プレイヤーのX,Y位置を梯子中心に固定
// 4. 梯子の上下限を超えたかを判定
// 5. 周囲に別の梯子があるかを探索
// 6. 新しい梯子が見つかれば切り替え
// 7. 見つからなければ通常ステートへ遷移
bool ULadderClimberState::OnUpdate(float DeltaTime)
{
	if (!mOwner || !Ladder)
		return false;

	if (TargetComp && TargetComp->IsHidden())
	{
		if (IStateControllable* Player = Cast<IStateControllable>(mOwner))
		{
			Player->ChangeState(EPlayerStateType::Default);
		}
		return true;
	}

	FixedPosition = Ladder->GetFixedPositionForActor(GetOwner());
	const FVector OwnerLocation = GetOwner()->GetActorLocation();
	const FVector NewLocation(FixedPosition.X, FixedPosition.Y, OwnerLocation.Z);
	GetOwner()->SetActorLocation(NewLocation);

	const float LadderTopZ = Ladder->GetTopWorldPosition().Z;
	const float LadderBottomZ = Ladder->GetBottomWorldPosition().Z;
	const float PlayerZ = mOwner->GetActorLocation().Z;

	if (PlayerZ > LadderTopZ || PlayerZ < LadderBottomZ)
	{
		const FVector PlayerLocation = mOwner->GetActorLocation();
		const FVector BoxExtent(20.f, 20.f, 50.f);
		TArray<FOverlapResult> Overlaps;

		FCollisionQueryParams Params;
		Params.AddIgnoredActor(mOwner);
		Params.AddIgnoredActor(Ladder);

		bool bFoundNewLadder = false;

		if (UWorld* World = mOwner->GetWorld())
		{
			if (World->OverlapMultiByObjectType(
				Overlaps,
				PlayerLocation,
				FQuat::Identity,
				FCollisionObjectQueryParams(ECC_WorldDynamic),
				FCollisionShape::MakeBox(BoxExtent),
				Params))
			{
				for (const FOverlapResult& Result : Overlaps)
				{
					if (AActor* HitActor = Result.GetActor())
					{
						if (HitActor->IsA(Ladder->GetClass()))
						{
							Ladder = Cast<ALadderActor>(HitActor);
							bFoundNewLadder = true;
							break;
						}
					}
				}
			}
		}

		if (!bFoundNewLadder)
		{
			if (UStateManager* StateManager = mOwner->FindComponentByClass<UStateManager>())
			{
				if (PlayerZ > LadderTopZ)
				{
					float Direction = 1.f;

					FVector ToPlayer = mOwner->GetActorLocation() - Ladder->GetActorLocation();
					ToPlayer.Normalize();

					if (FVector::DotProduct(Ladder->GetActorRightVector(), ToPlayer) < 0.f)
					{
						Direction *= -1.f;
					}

					FVector ExitLocation = Ladder->GetActorLocation();
					ExitLocation.Z = LadderTopZ + 300.f;
					ExitLocation += Ladder->GetActorForwardVector() * Direction * 150.f;

					mOwner->SetActorLocation(
						ExitLocation,
						false,
						nullptr,
						ETeleportType::TeleportPhysics);

					StateManager->ChangeState(EPlayerStateType::Default);
				}
			}
		}
	}

	return true;
}

// 処理の流れ:
// 1. Ownerの有効性を確認
// 2. キャラクターの重力と移動モードを元に戻す
// 3. コリジョンを有効化
bool ULadderClimberState::OnExit(APawn* Owner)
{
	if (!Owner)
		return false;

	APlayerCharacter* Player = Cast<APlayerCharacter>(Owner);
	if (Player == nullptr)
		return false;

	Player->SetGravityScale(true);

	if (ACharacter* Character = Cast<ACharacter>(Owner))
	{
		if (UCharacterMovementComponent* Comp = Character->GetCharacterMovement())
		{
			Comp->GravityScale = 1.f;
			Comp->SetMovementMode(EMovementMode::MOVE_Walking);

			if (UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
			{
				Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			}
		}
	}

	return true;
}

// 処理の流れ:
// 1. OwnerをStateControllableとして取得
// 2. 通常ステートへ遷移
bool ULadderClimberState::OnSkill(const FInputActionValue& Input)
{
	IStateControllable* OwnerState = Cast<IStateControllable>(mOwner);
	if (OwnerState == nullptr)
		return false;

	return OwnerState->ChangeState(EPlayerStateType::Default) != nullptr;
}

// 処理の流れ:
// 1. Ownerと梯子の有効性を確認
// 2. 入力から移動方向を算出
// 3. 上下移動方向を判定
// 4. 下方向移動時は地面との衝突をチェック
// 5. 地面に接触した場合は通常ステートへ遷移
// 6. 梯子に固定した位置へ移動
void ULadderClimberState::Movement(const FInputActionValue& Value)
{
	if (!mOwner || !Ladder)
		return;

	FVector Direction = MoveComp->Movement(0, mOwner, Value);

	if (Direction.Z < 0.f)
	{
		MoveDirection = -1;
	}
	else if (Direction.Z > 0.f)
	{
		MoveDirection = 1;
	}
	else
	{
		MoveDirection = 0;
	}

	FVector NewLocation = mOwner->GetActorLocation() + Direction * 10.f;

	if (Direction.Z < 0.f)
	{
		FVector Start = NewLocation;
		FVector End = Start - FVector(0.f, 0.f, 500.f);

		FHitResult HitResult;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(mOwner);

		if (UWorld* World = mOwner->GetWorld())
		{
			if (World->LineTraceSingleByChannel(
				HitResult, Start, End, ECC_Visibility, Params))
			{
				if (IStateControllable* OwnerState = Cast<IStateControllable>(mOwner))
				{
					OwnerState->ChangeState(EPlayerStateType::Default);
					return;
				}
			}
		}
	}

	MoveDelta = Direction * 10.f;
	GetOwner()->AddActorLocalOffset(Direction * 10.f, false);

	FixedPosition = Ladder->GetFixedPositionForActor(GetOwner());
	GetOwner()->SetActorLocation(FixedPosition);
}
