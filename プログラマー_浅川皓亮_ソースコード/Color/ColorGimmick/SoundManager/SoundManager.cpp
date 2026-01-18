#include "Sound/SoundManager.h"
#include "Kismet/GameplayStatics.h"
#include "Logic/ColorManager/ColorTargetRegistry.h"
#include "Components/AudioComponent.h"
#include "Manager/LevelManager.h"
#include "Manager/ColorManager.h"
#include "Manager/SaveManager.h"

#define FMOD_API_TRUE
#define FMOD_STUDIO_API_TRUE 

#include "fmod_studio.h"
#include "fmod.h"
#include "fmod_common.h"      
#include "fmod_studio.hpp"    
#include "fmod_studio_common.h" 

#include "FMODAudioComponent.h"


static FMOD_RESULT OnTimelineMarker(FMOD_STUDIO_EVENT_CALLBACK_TYPE type, FMOD_STUDIO_EVENTINSTANCE* eventInstance, void* parameters)
{
	// 処理の流れ:
	// 1. タイムラインマーカーイベントを判定
	// 2. Beatマーカーの場合、UserDataからUSoundManagerを取得
	// 3. Managerが存在すればOnMarkerBeatを呼ぶ
	if (type == FMOD_STUDIO_EVENT_CALLBACK_TIMELINE_MARKER)
	{
		auto* Marker = static_cast<FMOD_STUDIO_TIMELINE_MARKER_PROPERTIES*>(parameters);
		FString MarkerName = UTF8_TO_TCHAR(Marker->name);

		if (MarkerName == "Beat")
		{
			void* RawUserData = nullptr;
			((FMOD::Studio::EventInstance*)eventInstance)->getUserData(&RawUserData);
			USoundManager* Manager = static_cast<USoundManager*>(RawUserData);
			if (Manager)
			{
				Manager->OnMarkerBeat(Marker->position);
			}
		}
	}
	return FMOD_OK;
}

// コンストラクタ
// 処理の流れ:
// 1. BGM, SE 音量を初期化
// 2. 再生中BGMは nullptr に設定
USoundManager::USoundManager()
	: BGMVolume(1)
	, SEVolume(1)
	, CurrentBGMComponent(nullptr)
{
}

// 初期化
// 処理の流れ:
// 1. 保存音量をロード
// 2. 各SoundDataのAudioComponentを生成
// 3. BGM用にEnvelopeイベントをバインド
// 4. Beat検知用デリゲートを登録
// 5. テストBGMを初期化
void USoundManager::Init()
{
	LoadOrCreateVolumeSave();

	for (auto& soundMap : SoundDataMap)
	{
		const FName dataTag = soundMap.Key;
		FSoundData& soundData = soundMap.Value;
		soundData.AudioComponentMap.Reset();

		for (const auto& soundAssetPair : soundData.SoundAssetMap)
		{
			const FName waveTag = soundAssetPair.Key;
			USoundBase* sound = soundAssetPair.Value;
			if (waveTag.IsNone() || !sound) continue;
			if (soundData.AudioComponentMap.Contains(waveTag)) continue;

			UAudioComponent* AudioComponent = UGameplayStatics::CreateSound2D(this, sound);
			if (!AudioComponent) continue;

			AudioComponent->bAutoDestroy = false;

			if (dataTag == "BGM")
			{
				AudioComponent->OnAudioSingleEnvelopeValue.AddDynamic(this, &USoundManager::OnEnvelopeValue);
			}

			soundData.AudioComponentMap.Add(waveTag, AudioComponent);
		}
	}

	ALevelManager::GetInstance(GetWorld())->GetColorManager()->GetColorTargetRegistry()->OnColorApplied.AddDynamic(this, &USoundManager::SetTmp);
	InitTestSound();
}

// 音量ロード
void USoundManager::LoadOrCreateVolumeSave()
{
	FVolumeSaveData LoadedData = USaveManager::LoadVolumeFromJson();
	BGMVolume = LoadedData.BGMVolume;
	SEVolume = LoadedData.SEVolume;
}

// 音量設定
void USoundManager::SetVolume(float NewBGM, float NewSE)
{
	BGMVolume = NewBGM;
	SEVolume = NewSE;
	USaveManager::SetVolume(BGMVolume, SEVolume);
}

// BPM変更
void USoundManager::SetTmp(EColorTargetType Mode, FLinearColor NewColor)
{
	ALevelManager* level = ALevelManager::GetInstance(GetWorld());
	if (!level) return;

	UColorManager* colorManager = level->GetColorManager();
	if (!colorManager) return;

	FEffectMatchResult Match = colorManager->GetClosestEffectByHue(NewColor);

	switch (Match.ClosestEffect)
	{
	case EBuffEffect::Red:   MusicBPM = 160.f; break;
	case EBuffEffect::Blue:  MusicBPM = 90.f;  break;
	case EBuffEffect::Green: MusicBPM = 120.f; break;
	default: MusicBPM = 120.f; break;
	}

	BeatInterval = 60.f / MusicBPM;
	StartTime = GetWorld()->GetTimeSeconds();
	LastPredictedBeat = -1;
}

// BGM音量設定
void USoundManager::SetBGMVolume(float vol)
{
	const float previousBGMVolume = BGMVolume;
	BGMVolume = FMath::Clamp(vol, 0.0f, 4.0f);

	if (BGM)
	{
		BGM->SetVolume(BGMVolume);

		if (BGMVolume > 0.0f && !BGM->IsPlaying())
		{
			BGM->Play();
			BGM->SetVolume(BGMVolume);
		}
		else if (BGMVolume == 0.0f)
		{
			BGM->Stop();
		}
	}
}

// SE音量設定
void USoundManager::SetSEVolume(float vol)
{
	SEVolume = FMath::Clamp(vol, 0.0f, 4.0f);
}

// サウンド再生
bool USoundManager::PlaySound(FName DataID, FName SoundID, bool SetVolume, float Volume, bool IsSpecifyLocation, FVector place)
{
	float volume = SetVolume ? Volume : (DataID == "BGM" ? BGMVolume : SEVolume);

	if (DataID == "BGM") return PlayBGM();

	if (!SoundDataMap.Contains(DataID)) return false;
	if (!SoundDataMap[DataID].AudioComponentMap.Contains(SoundID)) return false;

	UAudioComponent* AudioComponent = SoundDataMap[DataID].AudioComponentMap[SoundID];
	if (!AudioComponent) { Init(); return false; }

	AudioComponent->SetVolumeMultiplier(FMath::Clamp(volume, 0.0f, 1.0f));

	if (IsSpecifyLocation)
		AudioComponent->SetWorldLocation(place);

	FSoundAttenuationSettings AttenuationSettings;
	AttenuationSettings.bAttenuate = true;
	AttenuationSettings.FalloffDistance = 2000.0f;

	AudioComponent->Play();
	return true;
}

// BGM停止
void USoundManager::StopBGM()
{
	if (FMODAudioComponent) FMODAudioComponent->Stop();
}

// フェードイン再生
void USoundManager::PlaySoundWithFadeIn(FName DataID, FName SoundID, float Volume, float FadeDuration)
{
	if (!SoundDataMap.Contains(DataID)) return;
	if (!SoundDataMap[DataID].AudioComponentMap.Contains(SoundID)) return;

	UAudioComponent* AudioComponent = SoundDataMap[DataID].AudioComponentMap[SoundID];
	if (!AudioComponent) return;

	AudioComponent->SetVolumeMultiplier(FMath::Clamp(Volume, 0.0f, 1.0f));
	AudioComponent->FadeIn(FadeDuration);
	AudioComponent->Play();
}

// フェードアウト停止（未使用のためコメント化）
// void USoundManager::StopBGMWithFadeOut(float FadeDuration) { ... }

void USoundManager::OnEnvelopeValue(const USoundWave* SoundWave, const float EnvelopeValue) {}

// BGM再生
bool USoundManager::PlayBGM()
{
	if (!BGMEventAsset) return false;

	BeatInterval = 60.0f / MusicBPM;

	if (!BGM)
	{
		BGM = NewObject<UFMODAudioComponent>(this);
		BGM->RegisterComponent();
	}

	BGM->SetEvent(BGMEventAsset);
	BGM->Play();

	EventInstance = BGM->StudioInstance;
	if (EventInstance)
	{
		EventInstance->setUserData(this);
		EventInstance->setCallback(OnTimelineMarker, FMOD_STUDIO_EVENT_CALLBACK_TIMELINE_MARKER);
	}

	StartTime = GetWorld()->GetTimeSeconds();
	LastPredictedBeat = -1;

	return true;
}

void USoundManager::OnBeatTimerElapsed() {}

// テストBGM初期化
void USoundManager::InitTestSound()
{
	if (!BGM)
	{
		BGM = NewObject<UFMODAudioComponent>(this);
		BGM->RegisterComponent();
		if (BGMEventAsset) BGM->SetEvent(BGMEventAsset);
	}
}

// Beatマーカー処理
void USoundManager::OnMarkerBeat(int64 MarkerPositionMs)
{
	LastConfirmedBeatTime = MarkerPositionMs / 1000.0f;
	OnBeatDetected.Broadcast();
}
