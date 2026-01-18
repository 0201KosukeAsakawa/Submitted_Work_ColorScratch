// プロジェクト設定の Description ページに著作権情報を記入

#include "Player/State/PlayerDefaultState.h"
#include "Player/State/LadderClimberState.h"
#include "Player/State/PlayerHoldState.h"
#include "Player/PlayerCharacter.h"
#include "Player/InGameController.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/MoveComponent.h"
#include "Components/BoxComponent.h"
#include "Components/PhysicsCalculator.h"
#include "FunctionLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputActionValue.h"
#include "Interface/StateControllable.h"
#include "Interface/Soundable.h"
#include "Kismet/KismetMathLibrary.h"
#include "Logic/Movement/PlayerMoveLogic.h"
#include "Manager/LevelManager.h"
#include "UI/UIManager.h"
#include "Objects/Color/LadderActor.h"

UPlayerDefaultState::UPlayerDefaultState()
{
}

// 処理の流れ:
// 1. OwnerとWorldの有効性を確認
// 2. OwnerとWorldを内部に保持
// 3. MoveComponentと移動ロジックを初期化
// 4. 物理・当たり判定コンポーネントを取得
// 5. プレイヤー用マテリアルを適用
// 6. 着地モンタージュ終了イベントを登録
// 7. 入力モードをゲーム専用に設定
// 8. 移動関連パラメータを初期化
bool UPlayerDefaultState::OnEnter(APawn* Owner, UWorld* World)
{
	if (Owner == nullptr || World == nullptr)
		return false;

	if (!mOwner)
		mOwner = Owner;
	if (!pWorld)
		pWorld = World;

	if (!MoveComp)
	{
		MoveComp = NewObject<UMoveComponent>(mOwner);
		UPlayerMoveLogic* PlayerLogic = NewObject<UPlayerMoveLogic>(this);
		MoveComp->Init(PlayerLogic);
	}

	if (!Physics)
	{
		Physics = GetOwner()->GetComponentByClass<UPhysicsCalculator>();
	}

	if (!HitBox)
	{
		HitBox = GetOwner()->GetComponentByClass<UCapsuleComponent>();
	}

	UStaticMeshComponent* StaticMeshComp =
		UFunctionLibrary::FindComponentByName<UStaticMeshComponent>(Owner, "StaticMesh");

	UMaterialInterface* Material = NewMaterial.LoadSynchronous();
	if (Material && StaticMeshComp)
	{
		StaticMeshComp->SetMaterial(0, Material);
	}

	APlayerCharacter* Player = Cast<APlayerCharacter>(mOwner);
	if (!Player)
		return false;

	if (USkeletalMeshComponent* MeshComp = Player->FindComponentByClass<USkeletalMeshComponent>())
	{
		if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
		{
			AnimInstance->OnMontageEnded.AddDynamic(
				this, &UPlayerDefaultState::OnLandingMontageEnded);
		}
	}

	bIsPlayingLandingAnimation = false;

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;
	}

	mMoveSpeed = 100.f;
	CurrentDirection = mOwner->GetActorForwardVector();

	return true;
}

// 処理の流れ:
// 1. WorldとPhysicsの有効性を確認
// 2. 着地終了フラグをリセット
// 3. 着地判定を行う
// 4. 着地時にSEとアニメーションを再生
// 5. アニメーション中は移動を停止
bool UPlayerDefaultState::OnUpdate(float DeltaTime)
{
	if (GetWorld() == nullptr || Physics == nullptr)
		return false;

	bLandingAnimationJustEnded = false;

	if (Physics->HasLanded() && !bIsPlayingLandingAnimation)
	{
		ISoundable* Sound =
			ALevelManager::GetInstance(GetWorld())->GetSoundManager().GetInterface();
		if (Sound)
		{
			Sound->PlaySound("SE", "Land");
		}

		PlayLandingAnimation();
	}

	if (bIsPlayingLandingAnimation)
	{
		MoveDelta = FVector::ZeroVector;
	}

	return true;
}

// 処理の流れ:
// 1. 着地アニメーション再生中かを確認
// 2. 再生中の場合はモンタージュを停止
// 3. 入力を再有効化
bool UPlayerDefaultState::OnExit(APawn* Owner)
{
	if (bIsPlayingLandingAnimation && LandingMontage)
	{
		APlayerCharacter* Player = Cast<APlayerCharacter>(mOwner);
		if (Player)
		{
			if (USkeletalMeshComponent* MeshComp =
				Player->FindComponentByClass<USkeletalMeshComponent>())
			{
				if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
				{
					AnimInstance->Montage_Stop(0.2f, LandingMontage);
				}
			}
		}

		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			mOwner->EnableInput(PC);
		}

		bIsPlayingLandingAnimation = false;
	}

	return true;
}

// 処理の流れ:
// 1. 入力が有効か確認
// 2. 着地アニメーション中は無効
// 3. 前方レイで持てるオブジェクトを判定
// 4. Holdステートへ遷移
bool UPlayerDefaultState::OnSkill(const FInputActionValue& Value)
{
	if (!Value.Get<bool>())
		return false;

	if (bIsPlayingLandingAnimation)
		return false;

	FVector Start = mOwner->GetActorLocation();
	FVector End = Start + CurrentDirection * 200.f;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(mOwner);

	if (!GetWorld()->LineTraceSingleByChannel(
		Hit, Start, End, ECC_Visibility, Params))
		return false;

	AActor* Target = Hit.GetActor();
	if (!Target || !Target->ActorHasTag("Holdable"))
		return false;

	IStateControllable* Player = Cast<IStateControllable>(mOwner);
	if (!Player)
		return false;

	UPlayerStateComponent* NewState =
		Player->ChangeState(EPlayerStateType::Hold);
	if (!NewState)
		return false;

	if (UPlayerHoldState* HoldState = Cast<UPlayerHoldState>(NewState))
	{
		HoldState->SetUp(Target, CurrentDirection.Y > 0);
	}

	return true;
}

// 処理の流れ:
// 1. 着地アニメーション中かを確認
// 2. 入力値を取得
// 3. 梯子遷移条件を判定
// 4. 移動方向と向きを更新
// 5. 移動入力を反映
void UPlayerDefaultState::Movement(const FInputActionValue& Value)
{
	if (bIsPlayingLandingAnimation)
		return;

	FVector2D MoveInput = Value.Get<FVector2D>();

	if (MoveInput.X >= 0.2f && TryEnterLadderOnJump())
	{
		Physics->AddForce(FVector::ZeroVector, 0.f);
		Physics->SetGravityScale(false);
		return;
	}

	FVector DirectionVec = MoveComp->Movement(0, mOwner, Value);
	DirectionVec.Normalize();

	if (DirectionVec != FVector::ZeroVector)
	{
		float TargetYaw = (MoveInput.Y < 0) ? 180.f : 0.f;
		CurrentDirection = FVector(0, DirectionVec.Y, 0);
		mOwner->SetActorRotation(FRotator(0, TargetYaw, 0));
	}

	MoveDelta = DirectionVec * MoveSpeed * GetWorld()->GetDeltaSeconds();
	mOwner->AddMovementInput(DirectionVec, MoveSpeed);
}

// 処理の流れ:
// 1. 着地アニメーション中かを確認
// 2. 接地状態を確認
// 3. 上方向へ力を加える
// 4. ジャンプSEを再生
bool UPlayerDefaultState::Jump(float jumpForce)
{
	if (bIsPlayingLandingAnimation)
		return false;

	if (!GetOwner() || !Physics || !Physics->OnGround())
		return false;

	Physics->AddForce(GetOwner()->GetActorUpVector(), jumpForce);

	ISoundable* Sound =
		ALevelManager::GetInstance(GetWorld())->GetSoundManager().GetInterface();
	if (Sound)
	{
		Sound->PlaySound("SE", "Jump");
	}

	return true;
}

// 処理の流れ:
// 1. OwnerとHitBoxの有効性を確認
// 2. 梯子とのオーバーラップを取得
// 3. 梯子が見つかればClimbステートへ遷移
bool UPlayerDefaultState::TryEnterLadderOnJump() const
{
	if (!mOwner || !HitBox)
		return false;

	TArray<AActor*> Overlaps;
	HitBox->GetOverlappingActors(Overlaps, ALadderActor::StaticClass());

	IStateControllable* Player = Cast<IStateControllable>(GetOwner());
	if (!Player)
		return false;

	for (AActor* Actor : Overlaps)
	{
		if (!Actor->ActorHasTag("Ladder"))
			continue;

		if (ALadderActor* Ladder = Cast<ALadderActor>(Actor))
		{
			if (UPlayerStateComponent* NewState =
				Player->ChangeState(EPlayerStateType::Climb))
			{
				if (ULadderClimberState* ClimbState =
					Cast<ULadderClimberState>(NewState))
				{
					ClimbState->SetTargetLadder(Ladder);
					return true;
				}
			}
		}
	}

	return false;
}

// 処理の流れ:
// 1. 対象モンタージュかを確認
// 2. 着地アニメーション終了フラグを更新
// 3. 入力と物理移動をリセット
void UPlayerDefaultState::OnLandingMontageEnded(
	UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != LandingMontage)
		return;

	bIsPlayingLandingAnimation = false;
	bLandingAnimationJustEnded = true;

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		mOwner->EnableInput(PC);

		if (APlayerCharacter* Player = Cast<APlayerCharacter>(mOwner))
		{
			if (UCharacterMovementComponent* Move =
				Player->GetCharacterMovement())
			{
				Move->Velocity = FVector::ZeroVector;
				Move->StopMovementImmediately();
			}
			Player->ConsumeMovementInputVector();
		}
	}
}

// 処理の流れ:
// 1. 必要なコンポーネントの有効性を確認
// 2. 入力を無効化
// 3. 着地モンタージュを再生
// 4. 再生中フラグを更新
void UPlayerDefaultState::PlayLandingAnimation()
{
	if (!LandingMontage)
		return;

	APlayerCharacter* Player = Cast<APlayerCharacter>(mOwner);
	if (!Player)
		return;

	USkeletalMeshComponent* MeshComp =
		Player->FindComponentByClass<USkeletalMeshComponent>();
	if (!MeshComp)
		return;

	UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
	if (!AnimInstance)
		return;

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		mOwner->DisableInput(PC);
	}

	if (AnimInstance->Montage_Play(LandingMontage, 1.f) > 0.f)
	{
		bIsPlayingLandingAnimation = true;
	}
}
