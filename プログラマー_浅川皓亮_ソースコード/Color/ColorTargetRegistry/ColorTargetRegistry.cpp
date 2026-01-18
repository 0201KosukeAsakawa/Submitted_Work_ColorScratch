// Fill out your copyright notice in the Description page of Project Settings.


#include "Logic/ColorManager/ColorTargetRegistry.h"
#include "Logic/ColorManager/EffectColorMatcher.h"
#include "Interface/ColorFilterInterface.h"
#include "Kismet/GameplayStatics.h"

// 1. モードに応じた色適用処理を実行
// 2. WorldColorの場合: ポストプロセスに色を設定し、ターゲットに通知
// 3. ObjectColorの場合: 選択中のオブジェクトに色を適用
// 4. 色適用完了をデリゲートで通知
void UColorTargetRegistry::ApplyColor(FLinearColor NewColor, EColorTargetType Mode, FEffectMatchResult Effect)
{


    switch (Mode)
    {
    case EColorTargetType::WorldColor:
        if (PostProcessMID)
        {
            PostProcessMID->SetVectorParameterValue(TEXT("FilterColor"), NewColor);
        }
        NotifyTargets(Mode, NewColor, Effect);
        NotifyTargets(EColorTargetType::Responders, NewColor, Effect);
        break;

    case EColorTargetType::ObjectColor:
        if (!TargetObject)
            return;
        TargetObject->ColorAction(NewColor, Effect);
        break;

    default:
        break;
    }

    OnColorApplied.Broadcast(Mode, NewColor);
}

// 1. イベントタイプのターゲットが登録されているか確認
// 2. 登録されたターゲットの中から一致するEventIDを持つものを検索
// 3. 該当するターゲットに色を適用
void UColorTargetRegistry::ColorEvent(FName EventID, FLinearColor NewColor, FEffectMatchResult Effect)
{
    if (!ColorResponseTargets.Contains(EColorTargetType::Event))
    {
        return;
    }

    auto& Instances = ColorResponseTargets[EColorTargetType::Event].Instances;
    if (Instances.Num() == 0)
        return;

    for (auto& TargetInstance : Instances)
    {
        if (TargetInstance->GetColorEventID() != EventID)
            continue;

        TargetInstance->ColorAction(NewColor, Effect);
    }
}

// 1. インターフェースからUObjectとして取得
// 2. TargetObjectに設定
// 3. 選択モードを有効化
void UColorTargetRegistry::SetColorTarget(IColorReactiveInterface* InInterface)
{
    TargetObject.SetObject(Cast<UObject>(InInterface));
    TargetObject.SetInterface(InInterface);
    InInterface->SetSelectMode(true);
}

void UColorTargetRegistry::ResetColorTarget()
{
    // 選択モードを解除

    if (TargetObject)
    {
        TargetObject->SetSelectMode(false);
    }
}

// 1. 有効なターゲットか確認
// 2. モードに対応する配列を取得または作成
// 3. 重複登録を防ぎながらターゲットを追加
void UColorTargetRegistry::RegisterTarget(EColorTargetType Mode, TScriptInterface<IColorReactiveInterface> Target)
{
    if (!Target)
        return;

    FColorTargetInstanceArray& TargetArray = ColorResponseTargets.FindOrAdd(Mode);
    if (!TargetArray.Instances.Contains(Target))
    {
        TargetArray.Instances.Add(Target);
    }
}

// 1. 指定モードのターゲット配列を検索
// 2. 配列内の各ターゲットに対してColorActionを呼び出し
void UColorTargetRegistry::NotifyTargets(EColorTargetType Mode, const FLinearColor& Color, FEffectMatchResult Effect)
{
    if (FColorTargetInstanceArray* TargetArray = ColorResponseTargets.Find(Mode))
    {
        for (const TScriptInterface<IColorReactiveInterface>& Target : TargetArray->Instances)
        {
            if (Target)
            {
                Target->ColorAction(Color, Effect);
            }
        }
    }
}

// 1. ワールド内の全PostProcessVolumeを検索
// 2. 最初に見つかったVolumeを使用
// 3. PostProcessMaterialから動的インスタンスを作成
// 4. Volumeの設定に動的インスタンスを追加
void UColorTargetRegistry::InitializePostEffect()
{
    TArray<AActor*> FoundVolumes;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APostProcessVolume::StaticClass(), FoundVolumes);

    if (FoundVolumes.Num() <= 0)
        return;

    APostProcessVolume* PostProcessVolume = Cast<APostProcessVolume>(FoundVolumes[0]);
    if (PostProcessVolume && PostProcessMaterial)
    {
        PostProcessMID = UMaterialInstanceDynamic::Create(PostProcessMaterial, this);
        PostProcessVolume->Settings.WeightedBlendables.Array.Add(FWeightedBlendable(1.0f, PostProcessMID));
    }
}

// 1. PostProcessMIDが有効か確認
// 2. FilterColorパラメータの値を取得
// 3. 取得失敗時はデフォルト色(黒)を返す
FLinearColor UColorTargetRegistry::GetPostProcessColor() const
{
    if (!PostProcessMID)
        return FLinearColor::Black;

    FLinearColor CurrentColor;
    if (PostProcessMID->GetVectorParameterValue(FName("FilterColor"), CurrentColor))
    {
        return CurrentColor;
    }

    return FLinearColor::Black;
}