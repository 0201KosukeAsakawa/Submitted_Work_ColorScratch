#include "Components/Color/ColorConfigurator.h"
#include "Components/Color/ColorReactiveComponent.h"
#include "Interface/ColorFilterInterface.h"
#include "Manager/LevelManager.h"
#include "Manager/ColorManager.h"
#include "Sound/SoundManager.h"
#include "FunctionLibrary.h"


UColorConfigurator::UColorConfigurator()
{

}

// 処理の流れ:
// 1. ColorLogicの初期化
// 2. ColorManagerへの登録
// 3. マテリアルの設定
void UColorConfigurator::Init()
{
	InitializeColorLogic();
	RegisterToColorManager();
	SetupMaterial();
}

// 処理の流れ:
// 1. ReactiveComponentClassの有効性確認
// 2. 現在色を初期色に設定
// 3. ColorReactiveComponentの生成
// 4. コンポーネントの登録とアクティベート
// 5. エフェクトとNiagaraの初期化
void UColorConfigurator::InitializeColorLogic()
{
	if (!ReactiveComponentClass) return;

	CurrentColor = StartColor;
	ColorReactiveComponent = NewObject<UColorReactiveComponent>(this, ReactiveComponentClass);
	if (!ColorReactiveComponent) return;

	ColorReactiveComponent->RegisterComponent();
	ColorReactiveComponent->Activate(true);
	ColorReactiveComponent->InitColorEffectAndNiagara(StartColor, Effect, Niagaras);
	ColorReactiveComponent->Init(bColorVariable);
}

// 処理の流れ:
// 1. ColorManagerの取得
// 2. ColorTargetTypeとOwnerを使って登録
void UColorConfigurator::RegisterToColorManager()
{
	if (UColorManager* ColorManager = GetColorManager())
	{
		ColorManager->RegisterTarget(ColorTargetType, GetOwner());
	}
}

// 処理の流れ:
// 1. bSetColorフラグの確認
// 2. LevelManagerからエフェクト色を取得してStartColorに設定
// 3. SkeletalMeshの取得
// 4. カスタムデプスの設定
// 5. マテリアルインスタンスの作成と色の適用
void UColorConfigurator::SetupMaterial()
{
	if (!bSetColor) return;
	StartColor = ALevelManager::GetInstance(GetWorld())
		->GetColorManager()
		->GetEffectColor(Effect);
	if (USkeletalMeshComponent* Mesh = GetStaticMesh())
	{
		Mesh->SetRenderCustomDepth(true);
		Mesh->SetCustomDepthStencilValue(10);

		if (UMaterialInstanceDynamic* DynMaterial = Mesh->CreateAndSetMaterialInstanceDynamic(0))
		{
			DynMaterial->SetVectorParameterValue(FName("BaseColor"), StartColor);
		}
	}
}

// 処理の流れ:
// 1. bPlayColorActionとColorReactiveComponentの確認
// 2. bColorVariableがtrueの場合、マテリアルに色を適用
// 3. 色の一致チェックを実行
void UColorConfigurator::ColorAction(FLinearColor NewColor, FEffectMatchResult result)
{
	if (!bPlayColorAction || !ColorReactiveComponent) return;

	if (bColorVariable)
	{
		ApplyColorToMaterial(NewColor);
	}

	bColorMuch = ColorReactiveComponent->CheckColorMatch(result, NewColor, bUseComplementaryColor);
}

// 処理の流れ:
// 1. 現在色を新しい色に更新
// 2. bSetColorがtrueの場合、マテリアルに色を適用
// 3. ColorReactiveComponentが存在する場合、エフェクトとNiagaraを初期化
// 4. ColorManagerからワールド色を取得してColorActionを実行
void UColorConfigurator::SetColor(FLinearColor NewColor, FEffectMatchResult result)
{
	CurrentColor = NewColor;

	if (bSetColor)
	{
		ApplyColorToMaterial(CurrentColor);
	}

	if (ColorReactiveComponent)
	{
		ColorReactiveComponent->InitColorEffectAndNiagara(CurrentColor, result.ClosestEffect, Niagaras);
	}

	if (const UColorManager* ColorManager = GetColorManager())
	{
		ColorAction(ColorManager->GetWorldColor(), result);
	}
}

// 処理の流れ:
// StartColorを使ってSetColorを呼び出し、初期状態に戻す
void UColorConfigurator::ResetColor(FEffectMatchResult result)
{
	SetColor(StartColor, result);
}

void UColorConfigurator::SetCurrentColor(FLinearColor NewColor)
{
	CurrentColor = NewColor;
}

void UColorConfigurator::SetColorMatch(bool bInColorMuch)
{
	bColorMuch = bInColorMuch;
}

// 処理の流れ:
// 1. 選択状態を更新
// 2. ColorReactiveComponentが存在する場合、選択モードを設定
void UColorConfigurator::SetSelectMode(bool bInIsSelected)
{
	bIsSelected = bInIsSelected;
	if (ColorReactiveComponent)
	{
		ColorReactiveComponent->SetSelectMode(bIsSelected);
	}
}

bool UColorConfigurator::IsColorChange() const
{
	return ColorReactiveComponent && ColorReactiveComponent->IsColorMatch(StartColor);
}

bool UColorConfigurator::IsColorChange(FLinearColor Color) const
{
	return ColorReactiveComponent && ColorReactiveComponent->IsColorMatch(Color);
}

bool UColorConfigurator::CheckColorMatch(FEffectMatchResult result, const FLinearColor& FilterColor, bool buseComplementaryColor) const
{
	return ColorReactiveComponent && ColorReactiveComponent->CheckColorMatch(result, FilterColor, buseComplementaryColor);
}

bool UColorConfigurator::IsColorMatch() const
{
	return bColorMuch;
}

bool UColorConfigurator::IsColorMatch(const FLinearColor& FilterColor, const FLinearColor& TargetColor, float Tolerance) const
{
	return ColorReactiveComponent && ColorReactiveComponent->IsColorMatch(FilterColor, TargetColor, Tolerance);
}

bool UColorConfigurator::IsColorMatch(const FLinearColor& FilterColor, float Tolerance) const
{
	return ColorReactiveComponent && ColorReactiveComponent->IsColorMatch(FilterColor, Tolerance);
}

bool UColorConfigurator::IsChangeable() const
{
	return bColorChangeable;
}

bool UColorConfigurator::IsHidden() const
{
	return ColorReactiveComponent && ColorReactiveComponent->IsHidden();
}

// 処理の流れ:
// ColorReactiveComponentが存在する場合、マテリアルに色を適用
void UColorConfigurator::ApplyColorToMaterial(FLinearColor InColor)
{
	if (ColorReactiveComponent)
	{
		ColorReactiveComponent->ApplyColorToMaterial(InColor);
	}
}

// =======================
// 補助関数（共通処理）
// =======================

USkeletalMeshComponent* UColorConfigurator::GetStaticMesh() const
{
	return UFunctionLibrary::FindComponentByName<USkeletalMeshComponent>(GetOwner(), TEXT("Mesh"));
}

ALevelManager* UColorConfigurator::GetLevelManager() const
{
	return ALevelManager::GetInstance(GetWorld());
}

UColorManager* UColorConfigurator::GetColorManager() const
{
	const ALevelManager* LevelManager = GetLevelManager();
	return LevelManager ? LevelManager->GetColorManager() : nullptr;
}