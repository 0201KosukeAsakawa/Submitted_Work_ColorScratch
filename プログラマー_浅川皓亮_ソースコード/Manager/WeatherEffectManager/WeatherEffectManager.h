// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataContainer/EffectMatchResult.h"
#include "UObject/NoExportTypes.h"
#include "WeatherEffectManager.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;

/**
 * 天候の種類
 */
UENUM(BlueprintType)
enum class EWeatherType : uint8
{
    Clear      UMETA(DisplayName = "Clear"),      // 晴れ
    Rain       UMETA(DisplayName = "Rain"),       // 雨
    Thunder    UMETA(DisplayName = "Thunder"),    // 雷
    Wind       UMETA(DisplayName = "Wind")        // 風
};

/**
 * 天候エフェクトを管理するコンポーネント
 * 色の変化に応じて雨、雷、風などの天候エフェクトを切り替える
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PACHIO_API UWeatherComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UWeatherComponent();

    virtual void BeginPlay() override;

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    /**
     * 色の変化に応じて天候を設定する
     * @param Mode 色の適用モード
     * @param NewColor 新しい色
     */
    UFUNCTION(BlueprintCallable)
    void SetWeather(EColorTargetType Mode, FLinearColor NewColor);

private:
    /**
     * 各天候のNiagaraエフェクトを初期化する
     */
    void InitializeEffects();

protected:
    // 雨エフェクト用のNiagaraシステム(エディタで設定)
    UPROPERTY(EditAnywhere, Category = "Weather Effects")
    UNiagaraSystem* RainSystem;

    // 雷エフェクト用のNiagaraシステム(エディタで設定)
    UPROPERTY(EditAnywhere, Category = "Weather Effects")
    UNiagaraSystem* ThunderSystem;

    // 風エフェクト用のNiagaraシステム(エディタで設定)
    UPROPERTY(EditAnywhere, Category = "Weather Effects")
    UNiagaraSystem* WindSystem;

private:
    // 雨エフェクトのコンポーネント
    UPROPERTY()
    UNiagaraComponent* RainEffect;

    // 雷エフェクトのコンポーネント
    UPROPERTY()
    UNiagaraComponent* ThunderEffect;

    // 風エフェクトのコンポーネント
    UPROPERTY()
    UNiagaraComponent* WindEffect;

    // 現在の天候タイプ
    EWeatherType CurrentWeather = EWeatherType::Clear;
};