// Fill out your copyright notice in the Description page of Project Settings.


#include "Logic/ColorManager/EffectColorMatcher.h"

UEffectColorMatcher::UEffectColorMatcher()
{
    // 各エフェクトタイプの基準色を初期化(パステルトーン)
    EffectColorMap = {
        { EBuffEffect::Green,  FLinearColor(0.65f, 1.00f, 0.78f, 1.0f) },
        { EBuffEffect::Blue,   FLinearColor(0.65f, 0.78f, 1.00f, 1.0f) },
        { EBuffEffect::Red,    FLinearColor(1.00f, 0.75f, 0.65f, 1.0f) },
        { EBuffEffect::Yellow, FLinearColor(1.00f, 1.00f, 0.65f, 1.0f) },
        { EBuffEffect::Black,  FLinearColor(0.0f, 0.0f, 0.0f, 1.0f) },
    };
}

// 1. 入力色をHSV色空間に変換し明度を取得
// 2. 各エフェクト色との色相角度差を計算
// 3. 明度差が大きい場合は補正値を加算
// 4. 最小距離のエフェクトを特定し結果を返す
FEffectMatchResult UEffectColorMatcher::GetClosestEffectByHue(const FLinearColor& InputColor)
{
    FEffectMatchResult Result;

    FLinearColor InputHSV = InputColor.LinearRGBToHSV();
    float InputValue = FMath::Clamp(InputHSV.B, 0.0f, 1.0f);

    float MinDistance = TNumericLimits<float>::Max();
    EBuffEffect ClosestEffect = EBuffEffect::Red;

    for (const auto& Elem : EffectColorMap)
    {
        const FLinearColor& EffectColor = Elem.Value;

        FLinearColor EffectHSV = EffectColor.LinearRGBToHSV();
        float EffectValue = FMath::Clamp(EffectHSV.B, 0.0f, 1.0f);

        float HueDistance = GetHueAngleDistance(InputColor, EffectColor);

        // 明度差による補正(差が0.5以上の場合、最大30度加算)
        float ValueDifference = FMath::Abs(InputValue - EffectValue);
        if (ValueDifference > 0.5f)
        {
            HueDistance += (ValueDifference - 0.5f) * 60.0f;
        }

        if (HueDistance + KINDA_SMALL_NUMBER < MinDistance)
        {
            MinDistance = HueDistance;
            ClosestEffect = Elem.Key;
        }
    }

    Result.ClosestEffect = ClosestEffect;
    Result.Distance = MinDistance;
    Result.StrengthRatio = FMath::Clamp(1.0f - (MinDistance / 180.0f), 0.0f, 1.0f);

    return Result;
}

// 1. 両色をHSV色空間に変換
// 2. 色相値(0〜360度)の差分を計算
// 3. 最短の角度差を返す(0〜180度)
float UEffectColorMatcher::GetHueAngleDistance(const FLinearColor& ColorA, const FLinearColor& ColorB)
{
    FLinearColor HSV_A = ColorA.LinearRGBToHSV();
    FLinearColor HSV_B = ColorB.LinearRGBToHSV();

    float HueA = HSV_A.R;
    float HueB = HSV_B.R;

    float Delta = FMath::Abs(HueA - HueB);
    float Distance = FMath::Min(Delta, 360.0f - Delta);

    return Distance;
}

// 1. マップから指定エフェクトの色を検索
// 2. 見つかればその色を返す
// 3. 見つからなければデフォルト色(白)を返す
FLinearColor UEffectColorMatcher::GetEffectColor(EBuffEffect Effect) const
{
    if (const FLinearColor* FoundColor = EffectColorMap.Find(Effect))
    {
        return *FoundColor;
    }

    return FLinearColor::White;
}