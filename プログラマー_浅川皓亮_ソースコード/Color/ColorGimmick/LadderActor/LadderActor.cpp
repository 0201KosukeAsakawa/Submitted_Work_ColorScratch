// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/Color/LadderActor.h"
#include "Components/BoxComponent.h"

ALadderActor::ALadderActor()
{
    // Tickを有効化
    PrimaryActorTick.bCanEverTick = true;

    // はしごのボリュームコンポーネントを作成
    LadderVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
}

void ALadderActor::Init()
{
    // 親クラスの初期化を実行
    AColorReactiveObject::Init();
}

void ALadderActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

FVector ALadderActor::GetTopWorldPosition() const
{
    // 1. LadderVolumeの有効性を確認
    // 2. Boxの範囲と中心座標を取得
    // 3. 中心座標にZ軸の範囲を加算して最上部の座標を返す
    // 4. LadderVolumeがない場合はアクターの座標を返す

    if (LadderVolume)
    {
        FVector Origin, Extent;
        Extent = LadderVolume->GetScaledBoxExtent();
        Origin = LadderVolume->GetComponentLocation();
        return Origin + FVector(0, 0, Extent.Z);
    }

    return GetActorLocation();
}

FVector ALadderActor::GetBottomWorldPosition() const
{
    // 1. LadderVolumeの有効性を確認
    // 2. Boxの範囲と中心座標を取得
    // 3. 中心座標からZ軸の範囲を減算して最下部の座標を返す
    // 4. LadderVolumeがない場合はアクターの座標を返す

    if (LadderVolume)
    {
        FVector Origin, Extent;
        Extent = LadderVolume->GetScaledBoxExtent();
        Origin = LadderVolume->GetComponentLocation();
        return Origin - FVector(0, 0, Extent.Z);
    }

    return GetActorLocation();
}

FVector ALadderActor::GetFixedPositionForActor(AActor* OtherActor) const
{
    // 1. OtherActorの有効性を確認
    // 2. はしごとアクターの座標を取得
    // 3. はしごの右方向ベクトルとアクターへの方向ベクトルの内積を計算
    // 4. 内積の正負によって左右を判定し、適切なオフセットを適用
    // 5. 補正された座標を返す

    if (!OtherActor)
        return GetActorLocation();

    FVector LadderLocation = GetActorLocation();
    FVector OtherLocation = OtherActor->GetActorLocation();

    FVector LadderRight = GetActorRightVector();
    FVector DirToOther = (OtherLocation - LadderLocation).GetSafeNormal();
    float Dot = FVector::DotProduct(DirToOther, LadderRight);

    FVector FixedLocation = OtherLocation;

    if (Dot > 0)
    {
        FixedLocation.Y = LadderLocation.Y + Offset; // 右側
    }
    else
    {
        FixedLocation.Y = LadderLocation.Y - Offset; // 左側
    }

    return FixedLocation;
}

FRotator ALadderActor::GetRotationForActorToFaceThis(AActor* OtherActor) const
{
    // 1. OtherActorの有効性を確認
    // 2. はしごとアクターの座標を取得
    // 3. はしごの前方ベクトルとアクターへの方向ベクトルを正規化(Z成分は無視)
    // 4. 内積を計算してアクターがはしごの前後どちらにいるか判定
    // 5. 前にいる場合は回転不要(0度)、後ろにいる場合は180度回転を返す

    if (!OtherActor)
    {
        return FRotator::ZeroRotator;
    }

    FVector LadderLocation = GetActorLocation();
    FVector OtherLocation = OtherActor->GetActorLocation();

    FVector Forward = GetActorForwardVector();
    Forward.Z = 0;
    Forward.Normalize();

    FVector ToOther = (OtherLocation - LadderLocation);
    ToOther.Z = 0;
    ToOther.Normalize();

    float Dot = FVector::DotProduct(Forward, ToOther);

    if (Dot >= 0.0f)
    {
        return FRotator(0.0f, 0.0f, 0.0f);
    }
    else
    {
        return FRotator(0.0f, 180.0f, 0.0f);
    }
}