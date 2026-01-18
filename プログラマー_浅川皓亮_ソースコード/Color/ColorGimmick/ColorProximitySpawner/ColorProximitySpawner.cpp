// Fill out your copyright notice in the Description page of Project Settings.


#include "Logic/Color/ColorProximitySpawner.h"
#include "FunctionLibrary.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "NiagaraActor.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

UColorProximitySpawner::UColorProximitySpawner()
{
}

// 処理の流れ:
// 1. Niagaraを有効化
// 2. メッシュを表示
bool UColorProximitySpawner::OnColorMatched(const FLinearColor& FilterColor)
{
	ToggleNiagaraActiveState(true);
	ShowMesh();
	return false;
}

// 処理の流れ:
// 1. Niagaraを無効化
// 2. メッシュを非表示
bool UColorProximitySpawner::OnColorMismatched(const FLinearColor& FilterColor)
{
	ToggleNiagaraActiveState(false);
	HideMesh();
	return true;
}

// 処理の流れ:
// 1. 親クラスの初期化を呼び出し
// 2. Niagaraを非表示状態に設定
// 3. bIsHiddenをfalseに設定
// 4. メッシュを非表示にする
void UColorProximitySpawner::Init(bool bIsColorVariable)
{
	UColorReactiveComponent::Init(bIsColorVariable);
	ToggleNiagaraActiveState(false);
	bIsHidden = false;
	HideMesh();
}

// 処理の流れ:
// 1. 既に非表示の場合は処理をスキップ
// 2. Ownerを取得
// 3. HideTargetタグを持つ全てのコンポーネントをループ
// 4. PrimitiveComponentの場合、表示とコリジョンを無効化
// 5. アクティブなコンポーネントを非アクティブ化
// 6. Tickを無効化
// 7. 出現エフェクトを再生
// 8. bIsHiddenをtrueに設定
void UColorProximitySpawner::HideMesh()
{
	if (bIsHidden) return;

	AActor* Owner = GetOwner();
	if (!Owner) return;

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
	bIsHidden = true;
}

// 処理の流れ:
// 1. 既に表示されている場合は処理をスキップ
// 2. Ownerを取得
// 3. HideTargetタグを持つ全てのコンポーネントをループ
// 4. PrimitiveComponentの場合、表示とコリジョンを有効化
// 5. アクティブなコンポーネントをアクティブ化
// 6. Tickを有効化
// 7. 全てのエフェクトを無効化
// 8. bIsHiddenをfalseに設定
void UColorProximitySpawner::ShowMesh()
{
	if (!bIsHidden) return;

	AActor* Owner = GetOwner();
	if (!Owner) return;

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

	DeactivateAllEffects();
	bIsHidden = false;
}