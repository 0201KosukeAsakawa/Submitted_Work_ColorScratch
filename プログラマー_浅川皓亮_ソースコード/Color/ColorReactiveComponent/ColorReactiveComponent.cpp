// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Color/ColorReactiveComponent.h"
#include "NiagaraActor.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Manager/LevelManager.h"
#include "Manager/ColorManager.h"
#include "FunctionLibrary.h"


// 処理の流れ:
// 1. Tick設定を有効化
// 2. 各Niagaraシステム（FireflyBurst、ParticlesOfLight、LightCube）をロード
UColorReactiveComponent::UColorReactiveComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> FireflyBurst(TEXT("/Game/Niagara/FireflyBurst.FireflyBurst"));
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> ParticlesOfLight(TEXT("/Game/Niagara/ParticlesOfLight.ParticlesOfLight"));
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> LightCube(TEXT("/Game/Niagara/ParticleCube.ParticleCube"));

	if (FireflyBurst.Succeeded())
	{
		FireflyBurstNiagaraSystem = FireflyBurst.Object;
	}

	if (ParticlesOfLight.Succeeded())
	{
		ParticlesOfLightNiagaraSystem = ParticlesOfLight.Object;
	}

	if (LightCube.Succeeded())
	{
		LightCubeNiagaraSystem = LightCube.Object;
	}
}

// 処理の流れ:
// 1. bSetStartColorフラグの確認
// 2. Ownerの取得
// 3. SkeletalMeshComponentの取得
// 4. 動的マテリアルインスタンスの作成
// 5. ColorManagerから色を取得
// 6. bIsColorVariableがfalseの場合、マテリアルに色を適用
void UColorReactiveComponent::Init(bool bIsColorVariable)
{
	if (!bSetStartColor)
		return;

	AActor* Owner = GetOwner();
	if (!Owner)
		return;

	USkeletalMeshComponent* MeshComp =
		UFunctionLibrary::FindComponentByName<USkeletalMeshComponent>(Owner, TEXT("Mesh"));
	if (!MeshComp)
		return;

	DynMesh = MeshComp->CreateAndSetMaterialInstanceDynamic(0);
	if (!DynMesh)
		return;

	CurrentColor = ALevelManager::GetInstance(GetWorld())
		->GetColorManager()
		->GetEffectColor(Effect);

	if (!bIsColorVariable)
	{
		DynMesh->SetVectorParameterValue(FName("BaseColor"), CurrentColor);
	}
}

void UColorReactiveComponent::InitColorEffectAndNiagara(const FLinearColor& FilterColor, EBuffEffect NewEffect, TArray<ANiagaraActor*> NiagaraComponents)
{
	CurrentColor = FilterColor;
	Effect = NewEffect;
	Niagaras = NiagaraComponents;
}

// 処理の流れ:
// 1. bUseComplementaryColorがtrueの場合、補色を取得
// 2. ColorManagerから色の距離を計算
// 3. 距離が30.0f以下の場合、OnColorMatched を呼び出し
// 4. それ以外の場合、OnColorMismatched を呼び出し
// 5. 一致結果を返す
bool UColorReactiveComponent::CheckColorMatch(FEffectMatchResult MatchResult, const FLinearColor& FilterColor, const bool bUseComplementaryColor)
{
	FLinearColor CheckColor = FilterColor;

	if (bUseComplementaryColor)
	{
		CheckColor = GetComplementaryColor(CheckColor);
		UE_LOG(LogTemp, Log, TEXT("CheckColor: %s"), *CheckColor.ToString());
	}

	float Distance = ALevelManager::GetInstance(GetWorld())
		->GetColorManager()
		->GetColorDistanceRGB(CurrentColor, FilterColor);

	bool bMatch;
	if (Distance <= 30.0f)
	{
		bMatch = OnColorMatched(CheckColor);
	}
	else
	{
		bMatch = OnColorMismatched(CheckColor);
	}
	return bMatch;
}

// 処理の流れ:
// 1. RGBの各成分の差分を計算
// 2. Max/Minを求めてDeltaを計算
// 3. 輝度(L)を計算
// 4. Deltaが0の場合、無彩色として処理
// 5. それ以外の場合、彩度(S)と色相(H)を計算
FHSLColor RGBToHSL(const FLinearColor& Color)
{
	float R = Color.R;
	float G = Color.G;
	float B = Color.B;

	float Max = FMath::Max3(R, G, B);
	float Min = FMath::Min3(R, G, B);
	float Delta = Max - Min;

	FHSLColor HSL;
	HSL.L = (Max + Min) / 2.0f;

	if (Delta == 0)
	{
		HSL.H = 0.0f;
		HSL.S = 0.0f;
	}
	else
	{
		HSL.S = (HSL.L < 0.5f) ? (Delta / (Max + Min)) : (Delta / (2.0f - Max - Min));

		if (Max == R)
			HSL.H = (G - B) / Delta + (G < B ? 6.0f : 0.0f);
		else if (Max == G)
			HSL.H = (B - R) / Delta + 2.0f;
		else
			HSL.H = (R - G) / Delta + 4.0f;

		HSL.H /= 6.0f;
	}
	return HSL;
}

// 処理の流れ:
// 1. 彩度(S)が0の場合、無彩色として処理
// 2. それ以外の場合、HueToRGBラムダ関数を使ってRGB変換
// 3. FLinearColorを返す
FLinearColor HSLToRGB(const FHSLColor& HSL)
{
	float R, G, B;

	if (HSL.S == 0)
	{
		R = G = B = HSL.L;
	}
	else
	{
		auto HueToRGB = [](float p, float q, float t) -> float
			{
				if (t < 0.0f) t += 1.0f;
				if (t > 1.0f) t -= 1.0f;
				if (t < 1.0f / 6.0f) return p + (q - p) * 6.0f * t;
				if (t < 1.0f / 2.0f) return q;
				if (t < 2.0f / 3.0f) return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
				return p;
			};

		float q = (HSL.L < 0.5f) ? (HSL.L * (1 + HSL.S)) : (HSL.L + HSL.S - HSL.L * HSL.S);
		float p = 2 * HSL.L - q;

		R = HueToRGB(p, q, HSL.H + 1.0f / 3.0f);
		G = HueToRGB(p, q, HSL.H);
		B = HueToRGB(p, q, HSL.H - 1.0f / 3.0f);
	}

	return FLinearColor(R, G, B, 1.0f);
}

// 処理の流れ:
// 1. RGB→HSLに変換
// 2. 色相を180度反転（補色化）
// 3. 彩度を0.3fに設定（パステル調）
// 4. 輝度を0.8~1.0の範囲にクランプ
// 5. HSL→RGBに変換
// 6. 最大成分を1.0に強制
// 7. 他の成分を0.8~1.0にクランプ
FLinearColor UColorReactiveComponent::GetComplementaryColor(const FLinearColor& InColor)
{
	FHSLColor HSL = RGBToHSL(InColor);

	HSL.H += 0.5f;
	if (HSL.H > 1.0f) HSL.H -= 1.0f;

	HSL.S = 0.3f;
	HSL.L = FMath::Clamp(HSL.L, 0.8f, 1.0f);

	FLinearColor Complementary = HSLToRGB(HSL);

	float MaxComponent = FMath::Max3(Complementary.R, Complementary.G, Complementary.B);

	if (Complementary.R == MaxComponent) Complementary.R = 1.0f;
	if (Complementary.G == MaxComponent) Complementary.G = 1.0f;
	if (Complementary.B == MaxComponent) Complementary.B = 1.0f;

	if (Complementary.R != 1.0f) Complementary.R = FMath::Clamp(Complementary.R, 0.8f, 1.0f);
	if (Complementary.G != 1.0f) Complementary.G = FMath::Clamp(Complementary.G, 0.8f, 1.0f);
	if (Complementary.B != 1.0f) Complementary.B = FMath::Clamp(Complementary.B, 0.8f, 1.0f);

	Complementary.A = 1.0f;
	return Complementary;
}

// 処理の流れ:
// 1. 選択状態を更新
// 2. DynMeshが存在するか確認
// 3. 選択解除の場合、発光色を黒に設定
void UColorReactiveComponent::SetSelectMode(bool bIsNowSelected)
{
	bIsSelected = bIsNowSelected;

	if (!DynMesh) return;

	if (!bIsSelected)
	{
		DynMesh->SetVectorParameterValue(FName("EmissiveColor"), FLinearColor::Black);
	}
}

// 処理の流れ:
// 1. 全てのNiagaraアクターをループ
// 2. 各Niagaraの表示/非表示、コリジョン、Tickを設定
void UColorReactiveComponent::ActiveEffect(bool bActivate)
{
	for (ANiagaraActor* Niagara : Niagaras)
	{
		if (!Niagara) continue;

		Niagara->SetActorHiddenInGame(bActivate);
		Niagara->SetActorEnableCollision(bActivate);
		Niagara->SetActorTickEnabled(bActivate);
	}
}

// 処理の流れ:
// 1. Ownerの存在確認
// 2. 全てのNiagaraアクターをループ
// 3. 各Niagaraアクターとコンポーネントの表示状態を設定
void UColorReactiveComponent::ToggleNiagaraActiveState(bool bVisible)
{
	if (GetOwner() == nullptr)
		return;

	TArray<ANiagaraActor*> NiagaraComponents = Niagaras;

	for (ANiagaraActor* NiagaraActor : NiagaraComponents)
	{
		if (NiagaraActor == nullptr)
			continue;

		NiagaraActor->SetActorHiddenInGame(!bVisible);
		NiagaraActor->SetActorEnableCollision(bVisible);

		UNiagaraComponent* NiagaraComp = NiagaraActor->GetNiagaraComponent();

		NiagaraComp->SetVisibility(bVisible, true);
		NiagaraComp->SetPaused(!bVisible);
	}
}

// 処理の流れ:
// 1. Ownerの取得
// 2. SkeletalMeshComponentの取得
// 3. 動的マテリアルインスタンスの作成
// 4. BaseColorパラメータに色を設定
void UColorReactiveComponent::ApplyColorToMaterial(FLinearColor InColor)
{
	AActor* Owner = GetOwner();
	if (!Owner)
		return;

	USkeletalMeshComponent* Mesh = UFunctionLibrary::FindComponentByName<USkeletalMeshComponent>(Owner, TEXT("Mesh"));
	if (!Mesh) return;

	UMaterialInstanceDynamic* DynMaterial = Mesh->CreateAndSetMaterialInstanceDynamic(0);
	if (!DynMaterial) return;

	DynMaterial->SetVectorParameterValue(FName("BaseColor"), InColor);
}

// 処理の流れ:
// 1. RGB各成分の差分を計算
// 2. 輝度ベースの重み付き色差を計算（人間の目に近い）
// 3. 色差が許容誤差以下かを判定
bool UColorReactiveComponent::IsColorMatch(const FLinearColor& FilterColor, const float Tolerance) const
{
	float dR = CurrentColor.R - FilterColor.R;
	float dG = CurrentColor.G - FilterColor.G;
	float dB = CurrentColor.B - FilterColor.B;

	float ColorDifference = 0.299f * dR * dR + 0.587f * dG * dG + 0.114f * dB * dB;

	return ColorDifference <= Tolerance * Tolerance;
}

// 処理の流れ:
// 1. RGB各成分の差分を計算
// 2. 輝度ベースの重み付き色差を計算（人間の目に近い）
// 3. 色差が許容誤差以下かを判定
bool UColorReactiveComponent::IsColorMatch(const FLinearColor& FilterColor, const FLinearColor& TargetColor, const float Tolerance) const
{
	float dR = TargetColor.R - FilterColor.R;
	float dG = TargetColor.G - FilterColor.G;
	float dB = TargetColor.B - FilterColor.B;

	float ColorDifference = 0.299f * dR * dR + 0.587f * dG * dG + 0.114f * dB * dB;

	return ColorDifference <= Tolerance * Tolerance;
}

void UColorReactiveComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool UColorReactiveComponent::OnColorMatched(const FLinearColor& FilterColor)
{
	return true;
}

bool UColorReactiveComponent::OnColorMismatched(const FLinearColor& FilterColor)
{
	return false;
}

void UColorReactiveComponent::PlayAppearEffect()
{
	ActiveNiagaraEffect(FireflyBurstNiagaraSystem);
	ActiveNiagaraEffect(LightCubeNiagaraSystem);
}

void UColorReactiveComponent::PlayDisappearEffect()
{

}

// 処理の流れ:
// 1. NiagaraSystemの有効性確認
// 2. アタッチ先のSkeletalMeshComponentを取得
// 3. Niagaraをメッシュにアタッチして生成
// 4. 現在の色の最大RGB成分を50倍に強調
// 5. Niagaraに色を設定
// 6. アクティブなコンポーネントリストに追加
void UColorReactiveComponent::ActiveNiagaraEffect(UNiagaraSystem* NiagaraSystem)
{
	if (!NiagaraSystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("NiagaraSystem is null"));
		return;
	}

	USkeletalMeshComponent* AttachComponent = UFunctionLibrary::FindComponentByName<USkeletalMeshComponent>(GetOwner(), TEXT("Mesh"));
	if (!AttachComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("AttachComponent is null"));
		return;
	}

	UNiagaraComponent* TargetNiagara = UNiagaraFunctionLibrary::SpawnSystemAttached(
		NiagaraSystem,
		AttachComponent,
		NAME_None,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		EAttachLocation::KeepRelativeOffset,
		true,
		true,
		ENCPoolMethod::None,
		true
	);

	if (TargetNiagara)
	{
		float MaxRGB = FMath::Max3(CurrentColor.R, CurrentColor.G, CurrentColor.B);

		FLinearColor TargetColor = CurrentColor;
		if (CurrentColor.R == MaxRGB)
		{
			TargetColor.R *= 50.f;
		}
		else if (CurrentColor.G == MaxRGB)
		{
			TargetColor.G *= 50.f;
		}
		else
		{
			TargetColor.B *= 50.f;
		}

		TargetNiagara->SetVariableLinearColor(FName("User_Color"), TargetColor);

		ActiveNiagaraComponent.Add(TargetNiagara);
	}
}

// 処理の流れ:
// 1. アクティブな全てのNiagaraコンポーネントをループ
// 2. 各コンポーネントを無効化して破棄
// 3. アクティブリストをクリア
void UColorReactiveComponent::DeactivateAllEffects()
{
	for (UNiagaraComponent* NiagaraComp : ActiveNiagaraComponent)
	{
		if (NiagaraComp && !NiagaraComp->IsBeingDestroyed())
		{
			NiagaraComp->Deactivate();
			NiagaraComp->DestroyComponent();
		}
	}
	ActiveNiagaraComponent.Empty();
}