// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Color/ColorReactiveComponent.h"
#include "ColorProximitySpawner.generated.h"

/**
 * 色の一致/不一致に応じてオブジェクトの表示を切り替えるコンポーネント
 * 色が一致している間はオブジェクトを表示し、不一致の場合は非表示にする
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PACHIO_API UColorProximitySpawner : public UColorReactiveComponent
{
	GENERATED_BODY()
public:
	/**
	 * コンストラクタ
	 */
	UColorProximitySpawner();

private:
	/**
	 * コンポーネントの初期化
	 * Niagaraを非表示にし、メッシュを非表示状態で開始
	 * @param bIsColorVariable 色が可変かどうか
	 */
	void Init(bool bIsColorVariable) override;

	/**
	 * メッシュを非表示にする
	 * HideTargetタグを持つコンポーネントの表示とコリジョンを無効化
	 */
	void HideMesh();

	/**
	 * メッシュを表示する
	 * HideTargetタグを持つコンポーネントの表示とコリジョンを有効化
	 */
	void ShowMesh();

	/**
	 * 色が一致したときの処理
	 * Niagaraとメッシュを表示
	 * @param FilterColor フィルター色
	 * @return 処理結果（false）
	 */
	bool OnColorMatched(const FLinearColor& FilterColor) override;

	/**
	 * 色が一致しなかったときの処理
	 * Niagaraとメッシュを非表示
	 * @param FilterColor フィルター色
	 * @return 処理結果（true）
	 */
	bool OnColorMismatched(const FLinearColor& FilterColor) override;
};