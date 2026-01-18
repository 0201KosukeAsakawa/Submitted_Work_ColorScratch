// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/Color/ColorReactiveSwitch.h"
#include "Components/Color/ColorConfigurator.h"
#include "Manager/LevelManager.h"
#include "Manager/ColorManager.h"
#include "Components/Color/ColorReactiveComponent.h"
#include "Components/BoxComponent.h"

AColorReactiveSwitch::AColorReactiveSwitch()
{
    // Boxコンポーネントを作成してルートにアタッチ
    BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision"));
    BoxComponent->SetupAttachment(RootComponent);
}

// 1. 親クラスの初期化を実行
// 2. セカンドエフェクトタイプから対応する色を取得
void AColorReactiveSwitch::Init()
{
    AColorReactiveObject::Init();

    SecondColor = ALevelManager::GetInstance(GetWorld())
        ->GetColorManager()
        ->GetEffectColor(Second);
}

// 1. ColorConfiguratorの有効性を確認
// 2. 親クラスのColorActionを実行
// 3. メイン色と一致した場合:
//    - マテリアルに色を適用
//    - ColorManagerのColorEventを発火
// 4. セカンド色と一致した場合:
//    - マテリアルに色を適用
//    - ColorManagerのColorEventを発火
void AColorReactiveSwitch::ColorAction(const FLinearColor InColor, FEffectMatchResult Result)
{
    if (!ColorConfigurator)
        return;

    AColorReactiveObject::ColorAction(InColor, Result);

    if (ColorConfigurator->CheckColorMatch(Result, InColor))
    {
        ColorConfigurator->ApplyColorToMaterial(InColor);

        ALevelManager* LevelManager = ALevelManager::GetInstance(GetWorld());
        if (LevelManager == nullptr)
            return;
        if (LevelManager->GetColorManager() == nullptr)
            return;

        LevelManager->GetColorManager()->ColorEvent(
            ColorConfigurator->GetColorEventID(),
            InColor
        );
    }
    else if (ColorConfigurator->IsColorMatch(SecondColor, InColor))
    {
        ColorConfigurator->ApplyColorToMaterial(InColor);

        ALevelManager* LevelManager = ALevelManager::GetInstance(GetWorld());
        if (LevelManager == nullptr)
            return;
        if (LevelManager->GetColorManager() == nullptr)
            return;

        LevelManager->GetColorManager()->ColorEvent(
            ColorConfigurator->GetColorEventID(),
            InColor
        );
    }
}