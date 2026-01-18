// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Color/ColorControllerComponent.h"
#include "DataContainer/EffectMatchResult.h"
#include "FunctionLibrary.h"
#include "UI/ColorLens.h"
#include "UI/UIManager.h"
#include "Manager/LevelManager.h"
#include "Manager/ColorManager.h"

// 処理の流れ:
// 1. Tick設定を有効化
// 2. 全てのEColorTargetTypeを取得
// 3. Responders と Event を除外してカラーマップに白色で初期化
UColorControllerComponent::UColorControllerComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    // カラーマップ初期化（Responders/Event は除外）
    const TArray<EColorTargetType> AllModes = UFunctionLibrary::GetAllEnumValues<EColorTargetType>();
    for (EColorTargetType Mode : AllModes)
    {
        if (Mode == EColorTargetType::Responders || Mode == EColorTargetType::Event)
            continue;

        ColorMap.Add(Mode, FLinearColor::White);
    }
}

void UColorControllerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
}

// 処理の流れ:
// 1. 現在のモードの色をHSVに変換
// 2. 彩度と明度を固定範囲にクランプ
// 3. 色相をDelta分回転（360度ループ）
// 4. HSVからRGBに変換し、アルファ値を保持して更新
// 5. 色変更イベントをブロードキャスト
void UColorControllerComponent::AdjustColor(float Delta)
{
    FLinearColor HSV = ColorMap[CurrentColorMode].LinearRGBToHSV();

    float Hue = HSV.R;
    //float Saturation = FMath::Clamp(HSV.G, 0.2f, 0.6f); // 彩度
    float Saturation = FMath::Clamp(HSV.G, 0.1f, 0.3f); // 彩度
    float Value = FMath::Clamp(HSV.B, 0.8f, 1.0f);      // 明度

    Hue = FMath::Fmod(Hue + Delta * 360.0f, 360.0f);
    if (Hue < 0.f) Hue += 360.f;

    FLinearColor NewColor = FLinearColor(Hue, Saturation, Value).HSVToLinearRGB();
    ColorMap[CurrentColorMode] = FLinearColor(NewColor.R, NewColor.G, NewColor.B, ColorMap[CurrentColorMode].A);

    OnColorChanged.Broadcast(ColorMap[CurrentColorMode], CurrentColorMode);
}

// 処理の流れ:
// 1. 現在のモードの色をHSVに変換
// 2. 彩度と明度を固定範囲にクランプ
// 3. 色相を指定値に直接設定（360度ループ）
// 4. HSVからRGBに変換し、アルファ値を保持して更新
// 5. 色変更イベントをブロードキャスト
void UColorControllerComponent::SetColor(float value)
{
    FLinearColor HSV = ColorMap[CurrentColorMode].LinearRGBToHSV();

    float Hue = HSV.R;
    float Saturation = FMath::Clamp(HSV.G, 0.2f, 0.6f); // 彩度
    float Value = FMath::Clamp(HSV.B, 0.8f, 1.0f);      // 明度

    Hue = FMath::Fmod(value, 360.0f);
    if (Hue < 0.f) Hue += 360.f;

    FLinearColor NewColor = FLinearColor(Hue, Saturation, Value).HSVToLinearRGB();
    ColorMap[CurrentColorMode] = FLinearColor(NewColor.R, NewColor.G, NewColor.B, ColorMap[CurrentColorMode].A);

    OnColorChanged.Broadcast(ColorMap[CurrentColorMode], CurrentColorMode);
}

// 処理の流れ:
// 1. Direction を +1 または -1 に正規化
// 2. 次のモードを取得（Direction に応じて前後）
// 3. 次のモードが ObjectColor の場合は専用処理へ
// 4. それ以外の場合は通常のモード切り替え処理へ
void UColorControllerComponent::ChangeMode(int Direction)
{
    Direction = (Direction >= 1) ? 1 : -1;
    EColorTargetType NextMode = (Direction > 0)
        ? GetNextMode(CurrentColorMode)
        : GetPreviousMode(CurrentColorMode);

    UE_LOG(LogTemp, Warning, TEXT("Trying Mode Change: %d -> %d"), static_cast<int32>(CurrentColorMode), static_cast<int32>(NextMode));

    if (NextMode == EColorTargetType::ObjectColor)
    {
        HandleObjectColorMode(Direction, NextMode);
        return;
    }

    HandleSimpleMode(Direction, NextMode);
}

// 処理の流れ:
// 1. LevelManager と ColorManager を取得
// 2. 最も近い変更可能なターゲットを検索
// 3. ターゲットが見つからない場合はモード変更をキャンセル
// 4. モードを NextMode に変更
// 5. ColorManager にターゲットを設定
// 6. UIマーカーを表示
// 7. アニメーションデリゲートを実行
void UColorControllerComponent::HandleObjectColorMode(int Direction, EColorTargetType NextMode)
{
    ALevelManager* LevelManager = ALevelManager::GetInstance(GetWorld());
    UColorManager* ColorManager = LevelManager ? LevelManager->GetColorManager() : nullptr;
    if (!LevelManager || !ColorManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("LevelManager または ColorManager が取得できませんでした"));
        return;
    }

    IColorReactiveInterface* ClosestTarget = nullptr;
    AActor* TargetActor = nullptr;
    if (!FindClosestColorTarget(ClosestTarget, TargetActor))
    {
        UE_LOG(LogTemp, Warning, TEXT("対象が見つからなかったため、モードを変更しませんでした"));
        return;
    }

    CurrentColorMode = NextMode;
    UE_LOG(LogTemp, Warning, TEXT("New Mode: %d"), static_cast<int32>(CurrentColorMode));

    ColorManager->SetColorTarget(ClosestTarget);
    UE_LOG(LogTemp, Warning, TEXT("ColorTarget を ColorManager に設定しました"));

    if (TargetActor)
        LevelManager->GetUIManager()->ShowMarker(TEXT("ChangeColorTarget"), TargetActor);

    if (AnimationDelegate.IsBound())
        AnimationDelegate.Execute(Direction);
}

// 処理の流れ:
// 1. LevelManager と ColorManager を取得
// 2. モードを NextMode に変更
// 3. ColorManager のターゲットをリセット
// 4. UIマーカーを非表示
// 5. アニメーションデリゲートを実行
void UColorControllerComponent::HandleSimpleMode(int Direction, EColorTargetType NextMode)
{
    ALevelManager* LevelManager = ALevelManager::GetInstance(GetWorld());
    UColorManager* ColorManager = LevelManager ? LevelManager->GetColorManager() : nullptr;

    CurrentColorMode = NextMode;
    UE_LOG(LogTemp, Warning, TEXT("New Mode: %d"), static_cast<int32>(CurrentColorMode));

    if (ColorManager) ColorManager->ResetColorTarget();
    if (LevelManager) LevelManager->GetUIManager()->HideMarker(TEXT("ChangeColorTarget"));
    if (AnimationDelegate.IsBound())
        AnimationDelegate.Execute(Direction);
}

// 処理の流れ:
// 1. オーナーの位置を中心に1000ユニットのボックスでSweep
// 2. ヒットした全てのアクターを取得
// 3. ColorReactiveInterfaceを実装し、変更可能なアクターをフィルタ
// 4. 最も近いアクターを選択
// 5. 見つかったターゲットとアクターを出力パラメータに設定
bool UColorControllerComponent::FindClosestColorTarget(IColorReactiveInterface*& OutTarget, AActor*& OutActor)
{
    FVector Start = GetOwner()->GetActorLocation();
    FVector BoxExtent(1000.f, 1000.f, 1000.f);

    TArray<FHitResult> HitResults;
    FCollisionShape Box = FCollisionShape::MakeBox(BoxExtent);
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetOwner());

    bool bHit = GetWorld()->SweepMultiByChannel(
        HitResults, Start, Start, FQuat::Identity, ECC_Visibility, Box, Params
    );
    if (!bHit) return false;

    float ClosestDistSq = TNumericLimits<float>::Max();
    OutTarget = nullptr;
    OutActor = nullptr;

    for (const FHitResult& Hit : HitResults)
    {
        AActor* HitActor = Hit.GetActor();
        if (!HitActor || !HitActor->GetClass()->ImplementsInterface(UColorReactiveInterface::StaticClass()))
            continue;

        IColorReactiveInterface* IR = Cast<IColorReactiveInterface>(HitActor);
        if (!IR || !IR->IsChangeable())
            continue;

        float DistSq = FVector::DistSquared(HitActor->GetActorLocation(), Start);
        if (DistSq < ClosestDistSq)
        {
            ClosestDistSq = DistSq;
            OutTarget = IR;
            OutActor = HitActor;
        }
    }
    return OutTarget != nullptr;
}

EColorTargetType UColorControllerComponent::GetNextMode(EColorTargetType CurrentMode)
{
    return GetAdjacentMode(CurrentMode, +1);
}

EColorTargetType UColorControllerComponent::GetPreviousMode(EColorTargetType CurrentMode)
{
    return GetAdjacentMode(CurrentMode, -1);
}

// 処理の流れ:
// 1. 全てのEColorTargetTypeからResponders と Event を除外
// 2. フィルタ済みリストから現在のモードのインデックスを取得
// 3. Direction分移動した新しいインデックスを計算（ループ処理）
// 4. 新しいインデックスのモードを返す
EColorTargetType UColorControllerComponent::GetAdjacentMode(EColorTargetType CurrentMode, int Direction)
{
    TArray<EColorTargetType> FilteredModes;
    for (EColorTargetType Mode : UFunctionLibrary::GetAllEnumValues<EColorTargetType>())
    {
        if (Mode != EColorTargetType::Responders && Mode != EColorTargetType::Event)
            FilteredModes.Add(Mode);
    }

    int32 CurrentIndex = FilteredModes.IndexOfByKey(CurrentMode);
    if (CurrentIndex == INDEX_NONE)
        return EColorTargetType::WorldColor;

    int32 NewIndex = (CurrentIndex + Direction + FilteredModes.Num()) % FilteredModes.Num();
    return FilteredModes[NewIndex];
}