#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "DataContainer/EffectMatchResult.h"
#include "ColorManager.generated.h"

class IColorReactiveInterface;
class UEffectColorMatcher;
class UColorTargetRegistry;

/**
 * 色管理を統括するマネージャークラス
 * 色の適用、バフエフェクトとの対応付け、ターゲット登録を一元管理する
 */
UCLASS(Blueprintable)
class PACHIO_API UColorManager : public UObject
{
    GENERATED_BODY()

public:
    /**
     * ColorManagerの初期化処理
     * レジストリとマッチャーのインスタンス生成、コントローラーバインド、ポストエフェクト初期化を実行
     */
    void Init();

    /**
     * 指定された色を適用する
     * @param NewColor 適用する色
     * @param Mode 適用モード(ワールド全体/特定オブジェクト等)
     */
    UFUNCTION()
    void ApplyColor(FLinearColor NewColor, EColorTargetType Mode);

    /**
     * イベントIDに基づいて特定のターゲットに色を適用する
     * @param EventID 発火するイベントの識別子
     * @param NewColor 適用する色
     */
    UFUNCTION()
    void ColorEvent(FName EventID, FLinearColor NewColor);

    /**
     * 色変更の対象オブジェクトを設定する
     * @param Target 色反応インターフェースを実装したオブジェクト
     */
    void SetColorTarget(IColorReactiveInterface* Target);

    /**
     * 現在の色変更対象をリセットする
     */
    void ResetColorTarget();

    /**
     * 色変更の通知を受け取るターゲットを登録する
     * @param Mode 登録するターゲットのタイプ
     * @param Target 登録するターゲットオブジェクト
     */
    void RegisterTarget(EColorTargetType Mode, TScriptInterface<IColorReactiveInterface> Target);

    /**
     * 2色間の色相角度距離を計算する(ワールド色との比較)
     * @param ColorA 比較する色
     * @return 色相角度の差(度数法)
     */
    float GetColorDistanceRGB(const FLinearColor& ColorA);

    /**
     * 2色間の色相角度距離を計算する
     * @param ColorA 比較する色1
     * @param ColorB 比較する色2
     * @return 色相角度の差(度数法)
     */
    float GetColorDistanceRGB(const FLinearColor& ColorA, const FLinearColor& ColorB);

    /**
     * 現在のワールド色から最も近いエフェクトを判定する
     * @return マッチング結果(エフェクトタイプ、距離、強度比率)
     */
    FEffectMatchResult GetClosestEffectByHue();

    /**
     * 指定された色から最も近いエフェクトを判定する
     * @param InputColor 判定対象の色
     * @return マッチング結果(エフェクトタイプ、距離、強度比率)
     */
    FEffectMatchResult GetClosestEffectByHue(const FLinearColor& InputColor);

    /**
     * ColorTargetRegistryのインスタンスを取得する
     * @return ColorTargetRegistryへのポインタ
     */
    UColorTargetRegistry* GetColorTargetRegistry() const { return ColorTargetRegistry; }

    /**
     * 現在のワールド全体に適用されている色を取得する
     * @return 現在のワールド色
     */
    FLinearColor GetWorldColor() const;

    /**
     * 指定されたエフェクトタイプに対応する色を取得する
     * @param Effect エフェクトタイプ
     * @return 対応する色
     */
    FLinearColor GetEffectColor(EBuffEffect Effect) const;

private:
    /**
     * プレイヤーの色コントローラーとイベントをバインドする
     */
    void BindController();

    /**
     * ポストプロセスエフェクトを初期化する
     */
    void InitializePostEffect();

private:
    // 色とエフェクトのマッチング処理を行うマッチャー
    UPROPERTY()
    UEffectColorMatcher* EffectColorMatcher;

    // 色の適用とターゲット管理を行うレジストリ
    UPROPERTY()
    UColorTargetRegistry* ColorTargetRegistry;

    // ColorTargetRegistryのクラス参照(エディタで設定)
    UPROPERTY(EditAnywhere, Category = "Color")
    TSubclassOf<UColorTargetRegistry> ColorTargetRegistryClass;

    // 色に反応するオブジェクトを格納するマップ(非推奨: ColorTargetRegistryに移行)
    UPROPERTY()
    TMap<EColorTargetType, FColorTargetInstanceArray> ColorResponseTargets;
};