// Fill out your copyright notice in the Description page of Project Settings.


#include "Logic/Color/ColorTriggerStopComponent.h"
#include "FunctionLibrary.h"
#include "Manager/LevelManager.h"
#include "Manager/ColorManager.h"
#include "Components/BoxComponent.h"



UColorTriggerStopComponent::UColorTriggerStopComponent()
{
}

// 処理の流れ:
// 1. 既に非表示の場合は処理をスキップしてfalseを返す
// 2. Ownerを取得
// 3. HideTargetタグを持つ全てのコンポーネントをループ
// 4. PrimitiveComponentの場合、表示とコリジョンを無効化
// 5. アクティブなコンポーネントを非アクティブ化
// 6. Tickを無効化
// 7. 出現エフェクトを再生
// 8. エフェクトを有効化
// 9. bIsHiddenをtrueに設定して返す
bool UColorTriggerStopComponent::OnColorMatched(const FLinearColor& FilterColor)
{
	if (bIsHidden) return false;

	AActor* Owner = GetOwner();
	if (!Owner) return false;

	for (UActorComponent* Component : Owner->GetComponents())
	{
		if (Component->ComponentHasTag("HideTarget"))
		{
			if (UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(Component))
			{
				Primitive->SetVisibility(false, false);
				Primitive->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}

			if (Component->IsActive())
			{
				Component->Deactivate();
			}

			Component->PrimaryComponentTick.SetTickFunctionEnable(false);
		}
	}

	PlayAppearEffect();
	ActiveEffect(true);
	bIsHidden = true;
	return bIsHidden;
}

// 処理の流れ:
// 1. 既に表示されている場合は処理をスキップしてfalseを返す
// 2. Ownerを取得
// 3. 全てのエフェクトを無効化
// 4. HideTargetタグを持つ全てのコンポーネントをループ
// 5. PrimitiveComponentの場合、表示とコリジョンを有効化
// 6. アクティブなコンポーネントをアクティブ化
// 7. Tickを有効化
// 8. エフェクトを無効化
// 9. bIsHiddenをfalseに設定して返す
bool UColorTriggerStopComponent::OnColorMismatched(const FLinearColor& FilterColor)
{
	if (!bIsHidden) return false;

	AActor* Owner = GetOwner();
	if (!Owner) return false;

	DeactivateAllEffects();

	for (UActorComponent* Component : Owner->GetComponents())
	{
		if (Component->ComponentHasTag("HideTarget"))
		{
			if (UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(Component))
			{
				Primitive->SetVisibility(true, true);
				Primitive->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			}

			if (Component->IsActive())
			{
				Component->Activate(true);
			}

			Component->PrimaryComponentTick.SetTickFunctionEnable(true);
		}
	}

	ActiveEffect(false);
	bIsHidden = false;
	return bIsHidden;
}