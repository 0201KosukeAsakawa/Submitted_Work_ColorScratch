#include "Player/State/StateManager.h"
#include "Player/State/PlayerDefaultState.h"
#include "Components/Player/PlayerStateComponent.h"

// 処理の流れ:
// 1. コンポーネントが毎フレームTick可能になるよう設定
UStateManager::UStateManager()
{
	PrimaryComponentTick.bCanEverTick = true;
}

// 処理の流れ:
// 1. OwnerとWorldの有効性を確認
// 2. 内部メンバにOwnerとWorldを保持
// 3. 初期ステートとしてDefaultステートに遷移
void UStateManager::Init(APawn* Owner, UWorld* World)
{
	if (!Owner || !World)
		return;

	mOwner = Owner;
	pWorld = World;

	ChangeState(EPlayerStateType::Default);
}

// 処理の流れ:
// 1. 現在のステートが存在するか確認
// 2. 存在する場合、ステートのOnUpdateを呼び出す
void UStateManager::Update(float DeltaTime)
{
	if (CurrentState != nullptr)
	{
		CurrentState->OnUpdate(DeltaTime);
	}
}

// 処理の流れ:
// 1. ステートマップ・Owner・Worldの有効性を確認
// 2. 遷移先ステートクラスを取得
// 3. 現在のステートが存在する場合、OnExitを呼び出し破棄
// 4. 新しいステートを生成
// 5. 新しいステートのOnEnterを実行
// 6. 生成したステートを返却
UPlayerStateComponent* UStateManager::ChangeState(EPlayerStateType NextStateTag)
{
	if (StateClassMap.IsEmpty() ||
		!StateClassMap.Contains(NextStateTag) ||
		!mOwner ||
		!pWorld)
	{
		return nullptr;
	}

	TSubclassOf<UPlayerStateComponent> StateClass =
		StateClassMap[NextStateTag];

	if (CurrentState)
	{
		CurrentState->OnExit(mOwner);
		CurrentState->ConditionalBeginDestroy();
		CurrentState = nullptr;
	}

	CurrentState = NewObject<UPlayerStateComponent>(mOwner, StateClass);
	if (!CurrentState)
		return nullptr;

	CurrentState->OnEnter(mOwner, pWorld);
	return CurrentState;
}

// 処理の流れ:
// 1. 指定されたステート種別に対応するクラスを取得
// 2. 現在のステートと取得したクラスの有効性を確認
// 3. 現在のステートが指定クラスかを判定して返却
bool UStateManager::IsStateMatch(EPlayerStateType StateTag)
{
	TSubclassOf<UPlayerStateComponent> StateClass =
		StateClassMap.FindRef(StateTag);

	if (StateClass == nullptr || CurrentState == nullptr)
	{
		return false;
	}

	return CurrentState->IsA(StateClass);
}
