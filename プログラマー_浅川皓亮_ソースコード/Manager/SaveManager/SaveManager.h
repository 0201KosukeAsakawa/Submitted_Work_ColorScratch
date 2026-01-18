// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DataContainer/SaveData.h"
#include "SaveManager.generated.h"

/**
 * セーブデータの管理を行うマネージャークラス
 * ステージクリア情報と音量設定をJSON形式で保存・読み込みする
 */
UCLASS()
class PACHIO_API USaveManager : public UObject
{
    GENERATED_BODY()

public:
    /**
     * 指定されたステージのセーブデータを保存する
     * @param StageKey ステージの識別キー
     * @param NewData 保存するセーブデータ
     */
    UFUNCTION(BlueprintCallable, Category = "Save")
    static void SaveStageData(const FString& StageKey, FSaveData NewData);

    /**
     * ステージセーブデータをJSON形式で保存する
     * @param Data 保存するステージセーブデータ
     */
    UFUNCTION(BlueprintCallable, Category = "Save")
    static void SaveToJson(const FStageSaveData& Data);

    /**
     * ステージセーブデータをJSON形式で読み込む
     * @return 読み込まれたステージセーブデータ
     */
    UFUNCTION(BlueprintCallable, Category = "Save")
    static FStageSaveData LoadFromJson();

    /**
     * 指定されたステージのクリアランクを取得する
     * @param StageKey ステージの識別キー
     * @return ステージのクリアランク
     */
    UFUNCTION(BlueprintCallable, Category = "Save")
    EStageRank GetStageRank(const FString& StageKey);

    /**
     * 音量設定をJSON形式で保存する
     * @param InData 保存する音量設定データ
     */
    UFUNCTION(BlueprintCallable, Category = "Save")
    static void SaveVolumeToJson(const FVolumeSaveData& InData);

    /**
     * 音量設定をJSON形式で読み込む
     * @return 読み込まれた音量設定データ
     */
    UFUNCTION(BlueprintCallable, Category = "Save")
    static FVolumeSaveData LoadVolumeFromJson();

    /**
     * BGMの音量を取得する
     * @return BGM音量(0.0〜1.0)
     */
    UFUNCTION(BlueprintCallable, Category = "Save")
    static float GetBGMVolume();

    /**
     * SEの音量を取得する
     * @return SE音量(0.0〜1.0)
     */
    UFUNCTION(BlueprintCallable, Category = "Save")
    static float GetSEVolume();

    /**
     * BGMとSEの音量を設定して保存する
     * @param NewBGM 新しいBGM音量
     * @param NewSE 新しいSE音量
     */
    UFUNCTION(BlueprintCallable, Category = "Save")
    static void SetVolume(float NewBGM, float NewSE);
};