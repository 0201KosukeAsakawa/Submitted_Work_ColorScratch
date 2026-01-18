#include "Objects/Color/ColorReactiveBeltConveyor.h"
#include "Components/Color/ColorConfigurator.h"
#include "Components/PhysicsCalculator.h"
#include "Components/BoxComponent.h"
#include "DataContainer/EffectMatchResult.h"
#include "Manager/LevelManager.h"
#include "Manager/ColorManager.h"
#include "Sound/SoundManager.h"

AColorReactiveBeltConveyor::AColorReactiveBeltConveyor()
{
    // Tickを有効化
    PrimaryActorTick.bCanEverTick = true;

    // Boxコンポーネントを作成してルートにアタッチ
    BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision"));
    BoxComponent->SetupAttachment(RootComponent);

    // オーバーラップイベントをバインド
    BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &AColorReactiveBeltConveyor::OnOverlapBegin);
    BoxComponent->OnComponentEndOverlap.AddDynamic(this, &AColorReactiveBeltConveyor::OnOverlapEnd);
}

// 1. 親クラスの初期化を実行
// 2. 初期方向と推進力を設定
void AColorReactiveBeltConveyor::Init()
{
    AColorReactiveObject::Init();

    CurrentDirection = direction;
    CurrentPower = DefaultPower;
}

// 1. コリジョンが無効な場合はオブジェクトリストをクリアして終了
// 2. bOnlyClosestがtrueの場合、最も近いオブジェクトのみに力を加える
//    - レイキャストで遮蔽物チェックを行い、遮蔽物がない場合のみ力を加える
// 3. bOnlyClosestがfalseの場合、すべてのオブジェクトに力を加える
void AColorReactiveBeltConveyor::Tick(float DeltaTime)
{
    if (!BoxComponent->IsCollisionEnabled())
    {
        hitObject.Empty();
        return;
    }

    if (bOnlyClosest)
    {
        FVector MyLocation = GetActorLocation();

        for (UPhysicsCalculator* Target : hitObject)
        {
            if (Target)
            {
                AActor* TargetActor = Target->GetOwner();
                if (!TargetActor)
                    continue;

                FVector TargetLocation = TargetActor->GetActorLocation();

                FHitResult HitResult;
                FCollisionQueryParams Params;
                Params.AddIgnoredActor(this);
                Params.AddIgnoredActor(TargetActor);

                bool bHit = GetWorld()->LineTraceSingleByChannel(
                    HitResult,
                    MyLocation,
                    TargetLocation,
                    ECC_Visibility,
                    Params
                );

                if (!bHit)
                {
                    Target->AddForce(CurrentDirection, CurrentPower, true, bUseLocalOffset);
                }
            }
        }
    }
    else
    {
        for (UPhysicsCalculator* Target : hitObject)
        {
            if (Target)
            {
                Target->AddForce(CurrentDirection, CurrentPower, true, bUseLocalOffset);
            }
        }
    }
}

// 1. ColorConfiguratorの有効性を確認
// 2. 親クラスのColorActionを実行
// 3. 色の一致状態を設定
// 4. 色一致時:
//    - IsReversがtrueなら逆方向に設定
//    - IsReversがfalseなら停止(ゼロベクトル)
// 5. 色不一致時: 通常方向に設定
void AColorReactiveBeltConveyor::ColorAction(const FLinearColor InColor, FEffectMatchResult Result)
{
    if (!ColorConfigurator)
        return;

    AColorReactiveObject::ColorAction(InColor, Result);

    ColorConfigurator->SetColorMatch(ColorConfigurator->CheckColorMatch(Result, InColor));

    if (ColorConfigurator->IsColorMatch())
    {
        if (IsRevers)
        {
            CurrentDirection = -direction;
        }
        else
        {
            CurrentDirection = FVector::ZeroVector;
        }
    }
    else
    {
        CurrentDirection = direction;
    }
}

// 1. アクターの有効性を確認
// 2. PhysicsCalculatorコンポーネントを検索
// 3. 見つかった場合、まだリストに含まれていなければ追加
void AColorReactiveBeltConveyor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor == nullptr || OtherActor == this)
        return;

    UPhysicsCalculator* PhysicsCalculator = OtherActor->FindComponentByClass<UPhysicsCalculator>();
    if (PhysicsCalculator == nullptr)
        return;

    if (!hitObject.Contains(PhysicsCalculator))
    {
        hitObject.Add(PhysicsCalculator);
    }
}

// 1. アクターの有効性を確認
// 2. PhysicsCalculatorコンポーネントを検索
// 3. 見つかった場合、リストから削除
void AColorReactiveBeltConveyor::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor == nullptr || OtherActor == this)
        return;

    UPhysicsCalculator* PhysicsCalculator = OtherActor->FindComponentByClass<UPhysicsCalculator>();
    if (PhysicsCalculator == nullptr)
        return;

    hitObject.Remove(PhysicsCalculator);
}