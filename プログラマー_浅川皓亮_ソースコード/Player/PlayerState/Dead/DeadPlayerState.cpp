// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/State/DeadPlayerState.h"
#include "Player/PlayerCharacter.h"
#include "Interface/StateControllable.h"
#include "Manager/LevelManager.h"
#include "Sound/SoundManager.h"
#include "Kismet/GameplayStatics.h"

// 処理の流れ:
// 1. Ownerの有効性を確認
// 2. OwnerをPlayerCharacterとして取得
// 3. プレイヤー入力を無効化
// 4. カメラを暗転させる
// 5. リスポーン状態と経過時間を初期化
// 6. LevelManagerを取得
// 7. 死亡SEを再生
bool UDeadPlayerState::OnEnter(APawn* Owner, UWorld* World)
{
    if (!Owner)
        return false;

    Player = Cast<APlayerCharacter>(Owner);

    // 入力停止および暗転処理
    if (APlayerController* PC = Cast<APlayerController>(Player->GetController()))
    {
        Player->DisableInput(PC);

        if (PC->PlayerCameraManager)
        {
            PC->PlayerCameraManager->StartCameraFade(
                0.f, 1.f, 1.f, FLinearColor::Black, false, true
            );
        }

        bIsRespawn = false;
        ElapsedTime = 0.f;
    }

    ALevelManager* LevelManager = ALevelManager::GetInstance(GetWorld());
    if (LevelManager == nullptr)
        return false;

    LevelManager->GetSoundManager()->PlaySound(TEXT("SE"), TEXT("Dead"));

    return true;
}



// 処理の流れ:
// 1. Playerの有効性を確認
// 2. 経過時間を加算
// 3. 一定時間経過後、未リスポーンの場合はリスポーン処理を実行
// 4. リスポーン遅延時間を超えた場合、通常ステートへ遷移
bool UDeadPlayerState::OnUpdate(float DeltaTime)
{
    if (Player == nullptr)
        return false;

    ElapsedTime += DeltaTime;

    if (ElapsedTime >= 1.5f && !bIsRespawn)
    {
        Player->Respawn();
        bIsRespawn = true;
    }

    if (ElapsedTime >= RespawnDelay)
    {
        if (IStateControllable* IS = Cast<IStateControllable>(Player))
        {
            IS->ChangeState(EPlayerStateType::Default);
        }
    }

    return true;
}


// 処理の流れ:
// 1. PlayerControllerを取得
// 2. カメラの暗転を解除
// 3. プレイヤー入力を再有効化
bool UDeadPlayerState::OnExit(APawn* Owner)
{
    if (APlayerController* PC = Cast<APlayerController>(Player->GetController()))
    {
        if (PC->PlayerCameraManager)
        {
            PC->PlayerCameraManager->StartCameraFade(
                1.f, 0.f, 1.f, FLinearColor::Black, false, false
            );
        }

        Player->EnableInput(PC);
    }

    return true;
}
