// Fill out your copyright notice in the Description page of Project Settings.


#include "Manager/WeatherEffectManager.h"
#include "Manager/ColorManager.h"
#include "Manager/LevelManager.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Logic/ColorManager/ColorTargetRegistry.h"

UWeatherComponent::UWeatherComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

// 1. 親クラスのBeginPlayを呼び出し
// 2. 天候エフェクトを初期化
// 3. ColorManagerのOnColorAppliedデリゲートにSetWeatherをバインド
void UWeatherComponent::BeginPlay()
{
    Super::BeginPlay();

    InitializeEffects();

    ALevelManager* LevelManager = ALevelManager::GetInstance(GetWorld());
    if (LevelManager && LevelManager->GetColorManager() && LevelManager->GetColorManager()->GetColorTargetRegistry())
    {
        LevelManager->GetColorManager()->GetColorTargetRegistry()->OnColorApplied.AddDynamic(this, &UWeatherComponent::SetWeather);
    }
}

// 1. 親クラスのTickComponentを呼び出し
// 2. オーナーとプレイヤーPawnを取得
// 3. プレイヤーの位置を取得
// 4. 現在アクティブな天候エフェクトをプレイヤー位置に追従させる
void UWeatherComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    AActor* Owner = GetOwner();
    if (!Owner)
        return;

    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn)
        return;

    FVector PlayerLocation = PlayerPawn->GetActorLocation();

    switch (CurrentWeather)
    {
    case EWeatherType::Rain:
        if (RainEffect)
            RainEffect->SetWorldLocation(PlayerLocation);
        break;

    case EWeatherType::Thunder:
        if (ThunderEffect)
            ThunderEffect->SetWorldLocation(PlayerLocation);
        break;

    case EWeatherType::Wind:
        if (WindEffect)
            WindEffect->SetWorldLocation(PlayerLocation);
        break;

    default:
        break;
    }
}

// 1. オーナーの有効性を確認
// 2. 各NiagaraSystemが設定されている場合にコンポーネントを生成
// 3. エフェクトをオーナーにアタッチしスケールを設定
// 4. 初期状態は非アクティブ化
void UWeatherComponent::InitializeEffects()
{
    if (!GetOwner())
        return;

    if (RainSystem && !RainEffect)
    {
        RainEffect = UNiagaraFunctionLibrary::SpawnSystemAttached(
            RainSystem,
            GetOwner()->GetRootComponent(),
            NAME_None,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::KeepRelativeOffset,
            false
        );
        RainEffect->SetWorldScale3D(FVector(20.0f));
        RainEffect->Deactivate();
    }

    if (ThunderSystem && !ThunderEffect)
    {
        ThunderEffect = UNiagaraFunctionLibrary::SpawnSystemAttached(
            ThunderSystem,
            GetOwner()->GetRootComponent(),
            NAME_None,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::KeepRelativeOffset,
            false
        );
        ThunderEffect->SetWorldScale3D(FVector(20.0f));
        ThunderEffect->Deactivate();
    }

    if (WindSystem && !WindEffect)
    {
        WindEffect = UNiagaraFunctionLibrary::SpawnSystemAttached(
            WindSystem,
            GetOwner()->GetRootComponent(),
            NAME_None,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::KeepRelativeOffset,
            false
        );
        WindEffect->SetWorldScale3D(FVector(20.0f));
        WindEffect->Deactivate();
    }
}

// 1. WorldColorモード以外は処理しない
// 2. 新しい色から最も近いエフェクトを判定
// 3. 現在アクティブな全エフェクトを非アクティブ化
// 4. 判定されたエフェクトに応じて対応する天候エフェクトをアクティブ化
// 5. 現在の天候タイプを更新
void UWeatherComponent::SetWeather(EColorTargetType Mode, FLinearColor NewColor)
{
    if (Mode != EColorTargetType::WorldColor)
        return;

    FEffectMatchResult Match = ALevelManager::GetInstance(GetWorld())
        ->GetColorManager()
        ->GetClosestEffectByHue(NewColor);

    if (RainEffect)
        RainEffect->Deactivate();
    if (ThunderEffect)
        ThunderEffect->Deactivate();
    if (WindEffect)
        WindEffect->Deactivate();

    EWeatherType NewWeather = EWeatherType::Clear;

    switch (Match.ClosestEffect)
    {
    case EBuffEffect::Red:
        if (ThunderEffect)
        {
            ThunderEffect->Activate();
            NewWeather = EWeatherType::Thunder;
        }
        break;

    case EBuffEffect::Green:
        if (WindEffect)
        {
            WindEffect->Activate();
            NewWeather = EWeatherType::Wind;
        }
        break;

    case EBuffEffect::Blue:
        if (RainEffect)
        {
            RainEffect->Activate();
            NewWeather = EWeatherType::Rain;
        }
        break;

    default:
        break;
    }

    CurrentWeather = NewWeather;
}