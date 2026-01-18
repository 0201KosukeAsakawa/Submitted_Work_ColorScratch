#include "Objects/Color/ColorReactiveObject.h"
#include "Components/Color/ColorConfigurator.h"
#include "Manager/ColorManager.h"
#include "Manager/LevelManager.h"

AColorReactiveObject::AColorReactiveObject()
{
    // Tickをデフォルトで無効化
    PrimaryActorTick.bCanEverTick = false;

    // ColorConfiguratorを作成
    ColorConfigurator = CreateDefaultSubobject<UColorConfigurator>(TEXT("ColorConfigurator"));
}

// 1. 親クラスのBeginPlayを呼び出し
// 2. ColorConfiguratorを初期化
// 3. 色反応オブジェクトの初期化を実行
void AColorReactiveObject::BeginPlay()
{
    Super::BeginPlay();
    ColorConfigurator->Init();
    Init();
}

// 派生クラスでオーバーライドして使用
void AColorReactiveObject::Init()
{

}

// 1. ColorConfiguratorの有効性を確認
// 2. 色反応ロジックを初期化
void AColorReactiveObject::InitializeColorLogic()
{
    if (ColorConfigurator == nullptr)
        return;

    ColorConfigurator->InitializeColorLogic();
}

// 1. ColorConfiguratorの有効性を確認
// 2. ColorManagerへ自身を登録
void AColorReactiveObject::RegisterToColorManager()
{
    if (ColorConfigurator == nullptr)
        return;

    ColorConfigurator->RegisterToColorManager();
}

// 1. ColorConfiguratorの有効性を確認
// 2. マテリアルとカスタムデプスを設定
void AColorReactiveObject::SetupMaterial()
{
    if (ColorConfigurator == nullptr)
        return;

    ColorConfigurator->SetupMaterial();
}

// 1. ColorConfiguratorの有効性を確認
// 2. ColorConfiguratorに色アクションを委譲
void AColorReactiveObject::ColorAction(FLinearColor NewColor, FEffectMatchResult Result)
{
    if (ColorConfigurator == nullptr)
        return;

    ColorConfigurator->ColorAction(NewColor, Result);
}

// 1. ColorConfiguratorの有効性を確認
// 2. 色を設定
void AColorReactiveObject::SetColor(FLinearColor NewColor, FEffectMatchResult Result)
{
    if (ColorConfigurator == nullptr)
        return;

    ColorConfigurator->SetColor(NewColor, Result);
}

// 1. ColorConfiguratorの有効性を確認
// 2. 選択モードを設定
void AColorReactiveObject::SetSelectMode(bool bIsSelected)
{
    if (!ColorConfigurator)
        return;

    ColorConfigurator->SetSelectMode(bIsSelected);
}

// 1. ColorConfiguratorの有効性を確認
// 2. マテリアルに色を適用
void AColorReactiveObject::ApplyColorToMaterial(FLinearColor InColor)
{
    if (ColorConfigurator == nullptr)
        return;

    ColorConfigurator->ApplyColorToMaterial(InColor);
}

// 1. ColorConfiguratorの有効性を確認
// 2. ロック状態を変更
void AColorReactiveObject::ChangeLock(bool bLock)
{
    if (ColorConfigurator == nullptr)
        return;

    ColorConfigurator->ChangeLock(bLock);
}

// ColorConfiguratorから色変更状態を取得
bool AColorReactiveObject::IsColorChange() const
{
    return ColorConfigurator && ColorConfigurator->IsColorChange();
}

// ColorConfiguratorから変更可能状態を取得
bool AColorReactiveObject::IsChangeable() const
{
    return ColorConfigurator && ColorConfigurator->IsChangeable();
}

// 1. ColorConfiguratorの有効性を確認
// 2. 色編集可能状態を取得
bool AColorReactiveObject::IsColorModifiable() const
{
    if (ColorConfigurator == nullptr)
        return false;

    return ColorConfigurator->IsColorModifiable();
}

// ColorConfiguratorから色一致状態を取得
bool AColorReactiveObject::IsColorMatch() const
{
    return ColorConfigurator && ColorConfigurator->IsColorMatch();
}

// 1. ColorConfiguratorの有効性を確認
// 2. カラーイベントIDを取得
FName AColorReactiveObject::GetColorEventID() const
{
    if (ColorConfigurator == nullptr)
        return FName(" ");

    return ColorConfigurator->GetColorEventID();
}