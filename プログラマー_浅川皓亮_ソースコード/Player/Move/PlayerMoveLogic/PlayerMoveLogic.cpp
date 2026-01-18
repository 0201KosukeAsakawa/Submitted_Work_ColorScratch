#include "Logic/Movement/PlayerMoveLogic.h"
#include "InputAction.h"
#include "GameFramework/Character.h"


// 1. オーナーの有効性を確認
// 2. 入力値を取得しY軸(左右)のデッドゾーンをチェック
// 3. オーナーの右方向ベクトルに入力を適用
// 4. 移動方向ベクトルを返す
FVector UPlayerMoveLogic::Movement(float DeltaTime, AActor* Owner, const FInputActionValue& Value)
{
    if (!Owner)
        return FVector::ZeroVector;

    FVector2D MoveInput = Value.Get<FVector2D>();
    const float DeadZone = 0.2f;

    if (FMath::Abs(MoveInput.Y) < DeadZone)
        return FVector::ZeroVector;

    FVector MoveDir = Owner->GetActorRightVector() * MoveInput.Y;

    return MoveDir;
}

void UPlayerMoveLogic::Init(float Speed, const FVector Direction)
{
    // 現在未実装
}
