#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ColorControllerComponent.generated.h"

// Blueprint からバインド可能な色変更通知デリゲート（対象タイプも含む）
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnColorChanged, FLinearColor, NewColor, EColorTargetType, TargetType);

DECLARE_DELEGATE_OneParam(FColorAnimationDelegate, float);

class IColorReactiveInterface;
/**
 * アクターにアタッチして色の制御を行うコンポーネント
 * カラーモードの切り替え、色の調整、ターゲット選択などを管理
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PACHIO_API UColorControllerComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    /**
     * コンストラクタ
     * カラーマップの初期化を行う
     */
    UColorControllerComponent();

    /**
     * 毎フレーム呼ばれる更新処理
     * @param DeltaTime 前フレームからの経過時間
     * @param TickType Tickのタイプ
     * @param ThisTickFunction このTickの関数情報
     */
    void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    /**
     * HSV色空間での色相を調整
     * 彩度と明度は固定範囲にクランプされる
     * @param Delta 色相の変化量（-1.0 〜 1.0で360度分）
     */
    UFUNCTION(BlueprintCallable)
    void AdjustColor(float Delta);

    /**
     * HSV色空間で色相を直接設定
     * @param value 色相の値（0〜360度）
     */
    UFUNCTION(BlueprintCallable)
    void SetColor(float value);

    /**
     * 現在のカラーモードの色を取得
     * @return 現在の色
     */
    UFUNCTION(BlueprintCallable)
    FLinearColor GetCurrentColor() const { return ColorMap[CurrentColorMode]; }

    /**
     * カラーモードを切り替える
     * ObjectColorモードの場合は最も近い変更可能なターゲットを自動選択
     * @param Direction 切り替え方向（+1で次、-1で前）
     */
    void ChangeMode(int Direction);

public:
    /**
     * Blueprintから購読可能な色変更イベント
     * 色と対象タイプを通知
     */
    UPROPERTY(BlueprintAssignable)
    FOnColorChanged OnColorChanged;

    /**
     * カラーモード変更時のアニメーションデリゲート
     */
    FColorAnimationDelegate AnimationDelegate;

private:
    /**
     * ObjectColorモードへの切り替え処理
     * 最も近い変更可能なターゲットを検索して設定
     * @param Direction 切り替え方向
     * @param NextMode 次のモード
     */
    void HandleObjectColorMode(int Direction, EColorTargetType NextMode);

    /**
     * 通常モードへの切り替え処理
     * ColorTargetをリセットしてマーカーを非表示
     * @param Direction 切り替え方向
     * @param NextMode 次のモード
     */
    void HandleSimpleMode(int Direction, EColorTargetType NextMode);

    /**
     * 最も近い色変更可能なターゲットを検索
     * @param OutTarget 見つかったターゲット（出力）
     * @param OutActor 見つかったアクター（出力）
     * @return ターゲットが見つかった場合true
     */
    bool FindClosestColorTarget(IColorReactiveInterface*& OutTarget, AActor*& OutActor);

    /**
     * 次のカラーモードを取得（右回り）
     * @param CurrentMode 現在のモード
     * @return 次のモード
     */
    EColorTargetType GetNextMode(EColorTargetType CurrentMode);

    /**
     * 前のカラーモードを取得（左回り）
     * @param CurrentMode 現在のモード
     * @return 前のモード
     */
    EColorTargetType GetPreviousMode(EColorTargetType CurrentMode);

    /**
     * 隣接するカラーモードを取得
     * Responders と Event を除外したモードリストから取得
     * @param CurrentMode 現在のモード
     * @param Direction 方向（+1で次、-1で前）
     * @return 隣接するモード
     */
    EColorTargetType GetAdjacentMode(EColorTargetType CurrentMode, int Direction);

private:
    /**
     * カラーモードごとの色を保持するマップ
     */
    TMap<EColorTargetType, FLinearColor> ColorMap;

    /**
     * 現在のカラーモード（どの対象に色を適用するか）
     */
    UPROPERTY(EditAnywhere)
    EColorTargetType CurrentColorMode;
};