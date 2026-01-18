// Fill out your copyright notice in the Description page of Project Settings.


#include "Manager/SaveManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "DataContainer/SaveData.h"


// 1. 現在のセーブデータを読み込む
// 2. 指定されたステージキーのデータを更新または追加
// 3. JSONファイルに保存
void USaveManager::SaveStageData(const FString& StageKey, FSaveData NewData)
{
    FStageSaveData CurrentData = LoadFromJson();
    CurrentData.Stages.FindOrAdd(StageKey) = NewData;
    SaveToJson(CurrentData);
}

// 1. セーブファイルのパスを取得
// 2. データをJSON形式にシリアライズ
// 3. ファイルに書き込み
void USaveManager::SaveToJson(const FStageSaveData& InData)
{
    FString SavePath = FPaths::ProjectSavedDir() + "StageSave.json";
    FStageSaveData Data = InData;

    if (!FPaths::FileExists(SavePath))
    {
        UE_LOG(LogTemp, Warning, TEXT("Save file not found. Creating default."));
    }

    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(Data.ToJson().ToSharedRef(), Writer);
    FFileHelper::SaveStringToFile(OutputString, *SavePath);
}

// 1. セーブファイルのパスを取得
// 2. ファイルから文字列を読み込み
// 3. JSON形式をデシリアライズしてデータ構造に変換
// 4. 読み込まれたデータを返す
FStageSaveData USaveManager::LoadFromJson()
{
    FString SavePath = FPaths::ProjectSavedDir() + "StageSave.json";
    FString Input;
    FStageSaveData LoadedData;

    if (FFileHelper::LoadFileToString(Input, *SavePath))
    {
        TSharedPtr<FJsonObject> Json;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Input);
        if (FJsonSerializer::Deserialize(Reader, Json))
        {
            LoadedData = FStageSaveData::FromJson(Json);
        }
    }

    return LoadedData;
}

// 1. セーブデータを読み込む
// 2. 指定されたステージのランクを取得して返す
EStageRank USaveManager::GetStageRank(const FString& StageKey)
{
    FStageSaveData Data = LoadFromJson();
    return Data.GetStageRank(StageKey);
}

// 1. 音量セーブファイルのパスを取得
// 2. データをJSON形式にシリアライズ
// 3. ファイルに書き込み
void USaveManager::SaveVolumeToJson(const FVolumeSaveData& InData)
{
    FString SavePath = FPaths::ProjectSavedDir() + "VolumeSave.json";
    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(InData.ToJson().ToSharedRef(), Writer);
    FFileHelper::SaveStringToFile(OutputString, *SavePath);
}

// 1. 音量セーブファイルのパスを取得
// 2. ファイルから文字列を読み込み
// 3. JSON形式をデシリアライズ
// 4. ファイルが存在しない場合はデフォルト値を保存
// 5. 読み込まれたデータを返す
FVolumeSaveData USaveManager::LoadVolumeFromJson()
{
    FString SavePath = FPaths::ProjectSavedDir() + "VolumeSave.json";
    FString Input;
    FVolumeSaveData LoadedData;

    if (FFileHelper::LoadFileToString(Input, *SavePath))
    {
        TSharedPtr<FJsonObject> Json;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Input);
        if (FJsonSerializer::Deserialize(Reader, Json))
        {
            LoadedData = FVolumeSaveData::FromJson(Json);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Volume save file not found, creating new with default values."));
        SaveVolumeToJson(LoadedData);
    }

    return LoadedData;
}

// 音量データを読み込んでBGM音量を返す
float USaveManager::GetBGMVolume()
{
    return LoadVolumeFromJson().BGMVolume;
}

// 音量データを読み込んでSE音量を返す
float USaveManager::GetSEVolume()
{
    return LoadVolumeFromJson().SEVolume;
}

// 1. 新しい音量データを作成
// 2. JSON形式で保存
void USaveManager::SetVolume(float NewBGM, float NewSE)
{
    FVolumeSaveData VolumeData;
    VolumeData.BGMVolume = NewBGM;
    VolumeData.SEVolume = NewSE;
    SaveVolumeToJson(VolumeData);
}