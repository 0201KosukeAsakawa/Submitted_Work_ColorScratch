// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/Color/ColorReactiveComponent.h"
#include "ColorTriggerStopComponent.generated.h"


/**
 * 色の一致/不一致に応じてオブジェクトの表示を切り替えるコンポーネント
 * ProximitySpawnerとは逆の動作（一致時に非表示、不一致時に表示）
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PACHIO_API UColorTriggerStopComponent : public UColorReactiveComponent
{
	GENERATED_BODY()
public:
	/**
	 * コンストラクタ
	 */
	UColorTriggerStopComponent();

	/**
	 * 色が一致したときの処理
	 * HideTargetタグを持つコンポーネントを非表示にしてエフェクトを再生
	 * @param FilterColor フィルター色
	 * @return 非表示状態（bIsHidden）
	 */
	bool OnColorMatched(const FLinearColor& FilterColor) override;

	/**
	 * 色が一致しなかったときの処理
	 * HideTargetタグを持つコンポーネントを表示してエフェクトを停止
	 * @param FilterColor フィルター色
	 * @return 非表示状態（bIsHidden）
	 */
	bool OnColorMismatched(const FLinearColor& FilterColor) override;
};