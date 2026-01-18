#include "Manager/ColorManager.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Manager/LevelManager.h"
#include "UI/UIManager.h"
#include "UI/ColorLens.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/PostProcessVolume.h"
#include "Interface/ColorFilterInterface.h"
#include "Components/Color/ColorControllerComponent.h"
#include "Components/PostProcessComponent.h"
#include "Logic/ColorManager/EffectColorMatcher.h"
#include "Logic/ColorManager/ColorTargetRegistry.h"

// 1. ColorTargetRegistryClassが設定されている場合にインスタンス化
// 2. EffectColorMatcherをインスタンス化
// 3. プレイヤーコントローラーとイベントをバインド
// 4. ポストプロセスエフェクトを初期化
void UColorManager::Init()
{
    if (ColorTargetRegistryClass)
    {
        ColorTargetRegistry = NewObject<UColorTargetRegistry>(this, ColorTargetRegistryClass);
    }

    EffectColorMatcher = NewObject<UEffectColorMatcher>();

    BindController();
    InitializePostEffect();
}

// 1. ColorTargetRegistryの有効性を確認
// 2. 入力色から最も近いエフェクトを判定
// 3. ColorTargetRegistryに色適用を委譲
void UColorManager::ApplyColor(FLinearColor NewColor, EColorTargetType Mode)
{
    if (!ColorTargetRegistry)
        return;

    FEffectMatchResult Result = GetClosestEffectByHue(NewColor);
    ColorTargetRegistry->ApplyColor(NewColor, Mode, Result);
}

// 1. ColorTargetRegistryの有効性を確認
// 2. 入力色から最も近いエフェクトを判定
// 3. イベントIDに基づいてターゲットに色を適用
void UColorManager::ColorEvent(FName EventID, FLinearColor NewColor)
{
    if (!ColorTargetRegistry)
        return;

    FEffectMatchResult Result = GetClosestEffectByHue(NewColor);
    ColorTargetRegistry->ColorEvent(EventID, NewColor, Result);
}

// ColorTargetRegistryに色変更対象を設定
void UColorManager::SetColorTarget(IColorReactiveInterface* Target)
{

    if (ColorTargetRegistry)
    {
        ColorTargetRegistry->SetColorTarget(Target);
    }
}

// ColorTargetRegistryの色変更対象をリセット
void UColorManager::ResetColorTarget()
{

    if (ColorTargetRegistry)
    {
        ColorTargetRegistry->ResetColorTarget();
    }
}

// 1. 自身とColorTargetRegistryの有効性を確認
// 2. ターゲットをColorTargetRegistryに登録
void UColorManager::RegisterTarget(EColorTargetType Mode, TScriptInterface<IColorReactiveInterface> Target)
{
    if (!this || !ColorTargetRegistry)
        return;

    ColorTargetRegistry->RegisterTarget(Mode, Target);
}

// 1. 指定された色とワールド色の色相角度距離を計算
// 2. EffectColorMatcherに距離計算を委譲
float UColorManager::GetColorDistanceRGB(const FLinearColor& ColorA)
{
    if (!EffectColorMatcher || !ColorTargetRegistry)
        return 0.0f;

    return EffectColorMatcher->GetHueAngleDistance(ColorA, ColorTargetRegistry->GetPostProcessColor());
}

// 2色間の色相角度距離を計算
float UColorManager::GetColorDistanceRGB(const FLinearColor& ColorA, const FLinearColor& ColorB)
{
    if (!EffectColorMatcher)
        return 0.0f;

    return EffectColorMatcher->GetHueAngleDistance(ColorA, ColorB);
}

// 1. EffectColorMatcherとColorTargetRegistryの有効性を確認
// 2. 現在のワールド色から最も近いエフェクトを判定
FEffectMatchResult UColorManager::GetClosestEffectByHue()
{
    if (!EffectColorMatcher || !ColorTargetRegistry)
        return FEffectMatchResult();

    return EffectColorMatcher->GetClosestEffectByHue(ColorTargetRegistry->GetPostProcessColor());
}

// 1. EffectColorMatcherの有効性を確認
// 2. 指定された色から最も近いエフェクトを判定
FEffectMatchResult UColorManager::GetClosestEffectByHue(const FLinearColor& InputColor)
{
    if (!EffectColorMatcher)
        return FEffectMatchResult();

    return EffectColorMatcher->GetClosestEffectByHue(InputColor);
}


// ColorTargetRegistryからワールド色を取得
FLinearColor UColorManager::GetWorldColor() const
{
    if (!ColorTargetRegistry)
        return FLinearColor::Black;

    return ColorTargetRegistry->GetPostProcessColor();
}

// EffectColorMatcherから指定エフェクトの色を取得
FLinearColor UColorManager::GetEffectColor(EBuffEffect Effect) const
{
    if (!EffectColorMatcher)
        return FLinearColor::White;

    return EffectColorMatcher->GetEffectColor(Effect);
}

// 1. プレイヤーPawnを取得
// 2. ColorControllerComponentを検索
// 3. OnColorChangedデリゲートにApplyColorをバインド
// 4. LevelManagerからUIManagerを取得
// 5. ColorLensのAnimationDelegateにアニメーション関数をバインド
void UColorManager::BindController()
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn == nullptr)
        return;

    UColorControllerComponent* ColorController = PlayerPawn->FindComponentByClass<UColorControllerComponent>();
    if (ColorController == nullptr)
        return;

    if (!ColorController->OnColorChanged.IsAlreadyBound(this, &UColorManager::ApplyColor))
    {
        ColorController->OnColorChanged.AddDynamic(this, &UColorManager::ApplyColor);
    }

    ALevelManager* LevelManager = ALevelManager::GetInstance(GetWorld());
    if (LevelManager == nullptr)
        return;
    if (LevelManager->GetUIManager() == nullptr)
        return;
    if (LevelManager->GetUIManager()->GetColorLens() == nullptr)
        return;

    ColorController->AnimationDelegate.BindUObject(
        LevelManager->GetUIManager()->GetColorLens(),
        &UColorLens::Animation
    );
}

// ColorTargetRegistryのポストエフェクト初期化を実行
void UColorManager::InitializePostEffect()
{
    if (!ColorTargetRegistry)
        return;

    ColorTargetRegistry->InitializePostEffect();
}