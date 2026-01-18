// Fill out your copyright notice in the Description page of Project Settings.


#include "Logic/Movement/LadderMoveLogic.h"

// 1. オーナーの有効性を確認
// 2. 入力値を取得しデッドゾーンをチェック
// 3. オーナーの上方向ベクトルに入力を適用
// 4. 正規化された移動方向を返す
FVector ULadderMoveLogic::Movement(float DeltaTime, AActor* Owner, const FInputActionValue& Value)
{
    if (!Owner)
        return FVector::ZeroVector;

    FVector2D MoveInput = Value.Get<FVector2D>();
    const float DeadZone = 0.2f;

    if (MoveInput.Size() < DeadZone)
        return FVector::ZeroVector;

    FVector UpVector = Owner->GetActorUpVector();
    FVector MoveDir = UpVector * MoveInput.X;
    MoveDir = MoveDir.GetClampedToMaxSize(1.0f);

    return MoveDir;
}

void ULadderMoveLogic::Init(float Speed, const FVector Direction)
{
    // 現在未実装
}