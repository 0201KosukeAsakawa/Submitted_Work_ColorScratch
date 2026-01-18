// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Sound/SoundWave.h"
#include "Components/AudioComponent.h"
#include "Components/ActorComponent.h"
#include "Interface/Soundable.h"
#include "DataContainer/EffectMatchResult.h"
#include "fmod_studio.hpp"     // FMOD Studio APIのC++ラッパー
#include "SoundManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBeatDetected);  
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnConfirmedBeat);  // マーカーで受けた正確なビート

class UAudioComponent;
class UFMODAudioComponent;
class UFMODEvent;

/*
* サウンドのデータを保持する構造体
* 各サウンドのアセットと再生用AudioComponentを管理
*/
USTRUCT()
struct FSoundData : public FTableRowBase
{
    GENERATED_USTRUCT_BODY()

public:
    /* サウンドのアセットを保持（SoundWaveやSoundCue） */
    UPROPERTY(EditAnywhere, Category = "Sound")
    TMap<FName, USoundBase*> SoundAssetMap;

    /* 再生用AudioComponentを保持（生成時に作成） */
    UPROPERTY(Transient)
    TMap<FName, UAudioComponent*> AudioComponentMap;
};

/*
* サウンド管理コンポーネント
* BGM/SEの再生、音量調整、Beat判定などを管理
*/
UCLASS(ClassGroup = (Custom), Blueprintable, meta = (BlueprintSpawnableComponent))
class PACHIO_API USoundManager : public UActorComponent, public ISoundable
{
    GENERATED_BODY()

    friend class ALevelManager;

public:
    /*
    * コンストラクタ：デフォルト値を初期化
    */
    USoundManager();

    /*
    * サウンドマネージャを初期化
    * AudioComponentの生成やVolumeロードを行う
    */
    void Init();

    /*
    * カラーに応じてBPMを変更
    * @param Mode 使用されるカラーターゲットタイプ
    * @param NewColor 新しいカラー値
    */
    UFUNCTION(Category = "Beat")
    void SetTmp(EColorTargetType Mode, FLinearColor NewColor);

    /* Beat検知イベント */
    UPROPERTY(BlueprintAssignable, Category = "Beat")
    FOnBeatDetected OnBeatDetected;

    /*
    * FMODのMarkerでBeatを検知したとき呼ばれる
    * @param MarkerPositionMs Marker位置（ミリ秒）
    */
    void OnMarkerBeat(int64 MarkerPositionMs);

private:

    /* サウンドボリュームをロードまたは初期化 */
    void LoadOrCreateVolumeSave();

    /*
    * BGMとSEの音量を設定
    * @param NewBGM BGM音量
    * @param NewSE SE音量
    */
    UFUNCTION(BlueprintCallable)
    void SetVolume(float NewBGM, float NewSE);

    /*
    * 指定したサウンドを再生
    * @param DataID "BGM"や"SE"などサウンド種別
    * @param SoundID サウンドの識別子
    * @param SetVolume 音量を固定値で設定するか
    * @param Volume 音量倍率
    * @param IsSpecifyLocation 位置指定で再生するか
    * @param place 位置
    * @return 成功したか
    */
    UFUNCTION(BlueprintCallable)
    bool PlaySound(FName DataID, FName SoundID, const bool SetVolume = false, float Volume = 1.f, bool IsSpecifyLocation = false, FVector place = FVector::ZeroVector) override;

    /* BGM音量を設定 */
    UFUNCTION(BlueprintCallable)
    void SetBGMVolume(float Volume) override;

    /* SE音量を設定 */
    UFUNCTION(BlueprintCallable)
    virtual void SetSEVolume(float Volume);

    /* BGMを停止 */
    void StopBGM() override;

    /* 現在のBGM音量を取得 */
    UFUNCTION(BlueprintCallable)
    float GetBGMVolume() const override { return BGMVolume; }

    /* 現在のSE音量を取得 */
    UFUNCTION(BlueprintCallable)
    float GetSEVolume() const override { return SEVolume; }

    /* サウンドをフェードインして再生 */
    void PlaySoundWithFadeIn(FName DataID, FName SoundID, float Volume, float FadeDuration) override;

    /*
    * Envelope値を受け取るコールバック
    * @param SoundWave 対象のSoundWave
    * @param EnvelopeValue エンベロープ値
    */
    UFUNCTION()
    void OnEnvelopeValue(const USoundWave* SoundWave, const float EnvelopeValue);

    /* BGM再生 */
    bool PlayBGM();

    /* Beatタイマー処理 */
    UFUNCTION()
    void OnBeatTimerElapsed();

    /* テスト用BGM初期化 */
    void InitTestSound();

private:
    /* サウンドデータを保持するマップ（DataID -> FSoundData） */
    UPROPERTY(EditAnywhere, Category = "Sound")
    TMap<FName, FSoundData> SoundDataMap;

    /* 現在再生中のBGM */
    UPROPERTY()
    UFMODAudioComponent* CurrentBGMComponent;

    /* BGM音量（0〜4） */
    float BGMVolume;

    /* SE音量（0〜4） */
    float SEVolume;

    /* FMOD BGMコンポーネント */
    UPROPERTY()
    UFMODAudioComponent* BGM;

    /* FMOD Eventアセット */
    UPROPERTY(EditAnywhere, Category = "FMOD")
    UFMODEvent* BGMEventAsset;

    /* 曲のBPM */
    UPROPERTY(EditAnywhere, Category = "BPM")
    float MusicBPM = 166.0f;

    /* Beat判定用タイマー */
    FTimerHandle BeatTimerHandle;

    /* Beat間隔（秒） */
    float BeatInterval = 0.5f;

    /* 再生開始時間 */
    float StartTime = 0.0f;

    /* 最後に予測したBeat番号 */
    int32 LastPredictedBeat = -1;

    /* 最後に確定したBeat時間 */
    float LastConfirmedBeatTime = 0.0f;

    /* Beat確定時の通知イベント */
    UPROPERTY(BlueprintAssignable)
    FOnConfirmedBeat OnConfirmedBeat;

    /* FMOD AudioComponent */
    UPROPERTY()
    UFMODAudioComponent* FMODAudioComponent;

    /* FMOD EventInstance */
    FMOD::Studio::EventInstance* EventInstance;
};
