#include "NeonPulseSphere.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"

ANeonPulseSphere::ANeonPulseSphere()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	MeshPivot = CreateDefaultSubobject<USceneComponent>(TEXT("MeshPivot"));
	MeshPivot->SetupAttachment(SceneRoot);

	SphereMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SphereMesh"));
	SphereMesh->SetupAttachment(MeshPivot);

	SceneRoot->SetMobility(EComponentMobility::Movable);
	MeshPivot->SetMobility(EComponentMobility::Movable);
	SphereMesh->SetMobility(EComponentMobility::Movable);
}

void ANeonPulseSphere::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	CenterMeshOnPivot();
}

void ANeonPulseSphere::BeginPlay()
{
	Super::BeginPlay();

	// L'Actor et le pivot restent neutres.
	// On animera uniquement le SphereMesh avec compensation de position.
	SetActorScale3D(FVector::OneVector);

	if (SceneRoot)
	{
		SceneRoot->SetRelativeScale3D(FVector::OneVector);
	}

	if (MeshPivot)
	{
		MeshPivot->SetRelativeLocation(FVector::ZeroVector);
		MeshPivot->SetRelativeRotation(FRotator::ZeroRotator);
		MeshPivot->SetRelativeScale3D(FVector::OneVector);
	}

	CenterMeshOnPivot();

	InitialLocation = GetWantedInitialLocation();
	SetActorLocation(InitialLocation);

	if (bUsePlacedScaleAsStartScale && SphereMesh)
	{
		StartBaseScale = SphereMesh->GetRelativeScale3D().X;
	}

	BaseScale = StartBaseScale;
	PulseTime = 0.0f;

	bRevealTriggeredThisVanish = false;
	NextMannequinRevealIndex = 0;
	ActiveMannequinMoves.Empty();

	if (bHideMannequinsAtBeginPlay)
	{
		HideAllMannequinsInRevealOrder();
	}

	CreateDynamicMaterials();

	SetMaterialScalar(EmissionStrengthParameterName, NormalEmissionStrength);
	SetMaterialScalar(OpacityParameterName, 1.0f);

	if (bAutoEnableInput)
	{
		APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);

		if (PlayerController)
		{
			EnableInput(PlayerController);

			if (InputComponent)
			{
				InputComponent->BindKey(EKeys::T, IE_Pressed, this, &ANeonPulseSphere::GoToNextState);
			}
		}
	}

	EnterState(CurrentState);
	ApplyConstantPulseScale();
}

void ANeonPulseSphere::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	StateTime += DeltaTime;
	PulseTime += DeltaTime;

	switch (CurrentState)
	{
	case ENeonSphereState::Grow:
		UpdateGrowState(DeltaTime);
		break;

	case ENeonSphereState::Move:
		UpdateMoveState(DeltaTime);
		break;

	case ENeonSphereState::MovedPulse:
		UpdateMovedPulseState(DeltaTime);
		break;

	case ENeonSphereState::Vanish:
		UpdateVanishState(DeltaTime);
		break;

	default:
		break;
	}

	UpdateActiveMannequinMoves(DeltaTime);
	ApplyConstantPulseScale();
}

void ANeonPulseSphere::GoToNextState()
{
	if (CurrentState == ENeonSphereState::Grow)
	{
		EnterState(ENeonSphereState::Move);
	}
	else if (CurrentState == ENeonSphereState::Move)
	{
		EnterState(ENeonSphereState::MovedPulse);
	}
	else if (CurrentState == ENeonSphereState::MovedPulse)
	{
		EnterState(ENeonSphereState::Vanish);
	}
	else if (CurrentState == ENeonSphereState::Vanish)
	{
		SetActorHiddenInGame(false);
		SetActorEnableCollision(true);
		SetActorLocation(InitialLocation);
		SetActorScale3D(FVector::OneVector);

		if (MeshPivot)
		{
			MeshPivot->SetRelativeLocation(FVector::ZeroVector);
			MeshPivot->SetRelativeRotation(FRotator::ZeroRotator);
			MeshPivot->SetRelativeScale3D(FVector::OneVector);
		}

		BaseScale = StartBaseScale;
		PulseTime = 0.0f;

		SetMaterialScalar(OpacityParameterName, 1.0f);
		SetMaterialScalar(EmissionStrengthParameterName, NormalEmissionStrength);

		EnterState(ENeonSphereState::Grow);
	}
}

void ANeonPulseSphere::EnterState(ENeonSphereState NewState)
{
	CurrentState = NewState;
	StateTime = 0.0f;

	if (CurrentState == ENeonSphereState::Grow)
	{
		BaseScale = StartBaseScale;

		SetActorLocation(InitialLocation);
		SetActorScale3D(FVector::OneVector);

		if (MeshPivot)
		{
			MeshPivot->SetRelativeLocation(FVector::ZeroVector);
			MeshPivot->SetRelativeRotation(FRotator::ZeroRotator);
			MeshPivot->SetRelativeScale3D(FVector::OneVector);
		}

		SetMaterialScalar(OpacityParameterName, 1.0f);
		SetMaterialScalar(EmissionStrengthParameterName, NormalEmissionStrength);
	}
	else if (CurrentState == ENeonSphereState::Move)
	{
		MoveStartLocation = GetActorLocation();

		const FVector SafeDirection = MoveDirection.IsNearlyZero()
			? FVector(0.0f, 0.0f, 1.0f)
			: MoveDirection.GetSafeNormal();

		MoveTargetLocation = MoveStartLocation + SafeDirection * MoveDistance;

		BaseScale = GrowTargetScale;
	}
	else if (CurrentState == ENeonSphereState::MovedPulse)
	{
		BaseScale = GrowTargetScale;
		SetActorLocation(MoveTargetLocation);
	}
	else if (CurrentState == ENeonSphereState::Vanish)
	{
		BaseScale = GrowTargetScale;

		// On autorise un seul reveal pendant CE Vanish.
		// Le reveal se fait plus tard dans UpdateVanishState(),
		// quand Alpha >= RevealVanishProgress.
		bRevealTriggeredThisVanish = false;
	}
}

void ANeonPulseSphere::UpdateGrowState(float DeltaTime)
{
	const float Alpha = FMath::Clamp(StateTime / GetSafeDuration(GrowDuration), 0.0f, 1.0f);
	const float SmoothAlpha = FMath::SmoothStep(0.0f, 1.0f, Alpha);

	BaseScale = FMath::Lerp(StartBaseScale, GrowTargetScale, SmoothAlpha);

	if (bAutoAdvanceStates && Alpha >= 1.0f)
	{
		EnterState(ENeonSphereState::Move);
	}
}

void ANeonPulseSphere::UpdateMoveState(float DeltaTime)
{
	const float Alpha = FMath::Clamp(StateTime / GetSafeDuration(MoveDuration), 0.0f, 1.0f);
	const float SmoothAlpha = FMath::SmoothStep(0.0f, 1.0f, Alpha);

	const FVector NewLocation = FMath::Lerp(MoveStartLocation, MoveTargetLocation, SmoothAlpha);

	SetActorLocation(NewLocation);

	BaseScale = GrowTargetScale;

	if (bAutoAdvanceStates && Alpha >= 1.0f)
	{
		EnterState(ENeonSphereState::MovedPulse);
	}
}

void ANeonPulseSphere::UpdateMovedPulseState(float DeltaTime)
{
	BaseScale = GrowTargetScale;
	SetActorLocation(MoveTargetLocation);

	if (bAutoAdvanceStates && StateTime >= MovedPulseDuration)
	{
		EnterState(ENeonSphereState::Vanish);
	}
}

void ANeonPulseSphere::UpdateVanishState(float DeltaTime)
{
	const float Alpha = FMath::Clamp(StateTime / GetSafeDuration(VanishDuration), 0.0f, 1.0f);
	const float SmoothAlpha = FMath::SmoothStep(0.0f, 1.0f, Alpha);

	BaseScale = FMath::Lerp(GrowTargetScale, FinalGrowScale, SmoothAlpha);

	const float NewOpacity = FMath::Lerp(1.0f, 0.0f, SmoothAlpha);
	const float NewEmission = FMath::Lerp(NormalEmissionStrength, VanishEmissionStrength, SmoothAlpha);

	SetMaterialScalar(OpacityParameterName, NewOpacity);
	SetMaterialScalar(EmissionStrengthParameterName, NewEmission);

	// Reveal un seul mannequin pendant ce final grow.
	// Les mannequins déjà révélés restent visibles.
	if (!bRevealTriggeredThisVanish && Alpha >= RevealVanishProgress)
	{
		RevealNextMannequinOnFinalGrow();
	}

	if (Alpha >= 1.0f)
	{
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);
	}
}

void ANeonPulseSphere::ApplyConstantPulseScale()
{
	float FinalScale = BaseScale;

	// Pulse visible tout le temps, y compris pendant le Grow.
	const float Pulse01 = (FMath::Sin(PulseTime * PulseSpeed) + 1.0f) * 0.5f;

	const float PulseMultiplier = FMath::Lerp(
		1.0f - PulseStrength,
		1.0f + PulseStrength,
		Pulse01
	);

	FinalScale = BaseScale * PulseMultiplier;

	ApplyVisualScale(FinalScale);
}

void ANeonPulseSphere::ApplyVisualScale(float NewScale)
{
	if (!SphereMesh)
	{
		return;
	}

	UStaticMesh* Mesh = SphereMesh->GetStaticMesh();

	if (!Mesh)
	{
		return;
	}

	const FBoxSphereBounds MeshBounds = Mesh->GetBounds();

	// Sécurité : le parent reste neutre.
	if (MeshPivot)
	{
		MeshPivot->SetRelativeLocation(FVector::ZeroVector);
		MeshPivot->SetRelativeRotation(FRotator::ZeroRotator);
		MeshPivot->SetRelativeScale3D(FVector::OneVector);
	}

	// On scale vraiment le mesh.
	SphereMesh->SetRelativeScale3D(FVector(NewScale));

	// Et on compense son pivot bas.
	// Sans ça, la sphere scale depuis le sol.
	SphereMesh->SetRelativeLocation(-MeshBounds.Origin * NewScale);
	SphereMesh->SetRelativeRotation(FRotator::ZeroRotator);
}

void ANeonPulseSphere::CenterMeshOnPivot()
{
	if (!bAutoCenterMeshOnPivot || !SphereMesh)
	{
		return;
	}

	UStaticMesh* Mesh = SphereMesh->GetStaticMesh();

	if (!Mesh)
	{
		return;
	}

	const FBoxSphereBounds MeshBounds = Mesh->GetBounds();

	if (MeshPivot)
	{
		MeshPivot->SetRelativeLocation(FVector::ZeroVector);
		MeshPivot->SetRelativeRotation(FRotator::ZeroRotator);
		MeshPivot->SetRelativeScale3D(FVector::OneVector);
	}

	SphereMesh->SetRelativeScale3D(FVector::OneVector);
	SphereMesh->SetRelativeLocation(-MeshBounds.Origin);
	SphereMesh->SetRelativeRotation(FRotator::ZeroRotator);
}

void ANeonPulseSphere::CreateDynamicMaterials()
{
	DynamicMaterials.Empty();

	if (!SphereMesh)
	{
		return;
	}

	const int32 MaterialCount = SphereMesh->GetNumMaterials();

	for (int32 Index = 0; Index < MaterialCount; ++Index)
	{
		UMaterialInstanceDynamic* MID = SphereMesh->CreateDynamicMaterialInstance(Index);

		if (MID)
		{
			DynamicMaterials.Add(MID);
		}
	}
}

void ANeonPulseSphere::SetMaterialScalar(FName ParameterName, float Value)
{
	for (UMaterialInstanceDynamic* MID : DynamicMaterials)
	{
		if (MID)
		{
			MID->SetScalarParameterValue(ParameterName, Value);
		}
	}
}

void ANeonPulseSphere::HideAllMannequinsInRevealOrder()
{
	for (const FNeonMannequinRevealStep& Step : MannequinRevealOrder)
	{
		AActor* Mannequin = Step.Mannequin;

		if (!IsValid(Mannequin) || Mannequin == this)
		{
			continue;
		}

		Mannequin->SetActorHiddenInGame(true);

		if (bDisableMannequinCollisionWhileHidden)
		{
			Mannequin->SetActorEnableCollision(false);
		}
	}
}

void ANeonPulseSphere::RevealNextMannequinOnFinalGrow()
{
	bRevealTriggeredThisVanish = true;

	while (NextMannequinRevealIndex < MannequinRevealOrder.Num())
	{
		const FNeonMannequinRevealStep& Step = MannequinRevealOrder[NextMannequinRevealIndex];
		NextMannequinRevealIndex++;

		AActor* Mannequin = Step.Mannequin;

		if (!IsValid(Mannequin) || Mannequin == this)
		{
			continue;
		}

		Mannequin->SetActorHiddenInGame(false);

		if (bDisableMannequinCollisionWhileHidden)
		{
			Mannequin->SetActorEnableCollision(true);
		}

		if (Step.bMoveAfterReveal)
		{
			StartMannequinMove(
				Mannequin,
				Step.TargetWorldLocation,
				Step.MoveDuration
			);
		}

		// Important : un seul mannequin par final grow.
		return;
	}
}

void ANeonPulseSphere::StartMannequinMove(AActor* Mannequin, const FVector& TargetLocation, float Duration)
{
	if (!IsValid(Mannequin))
	{
		return;
	}

	// Si ce mannequin était déjà en train de bouger, on remplace son mouvement.
	for (int32 Index = ActiveMannequinMoves.Num() - 1; Index >= 0; --Index)
	{
		if (ActiveMannequinMoves[Index].Actor.Get() == Mannequin)
		{
			ActiveMannequinMoves.RemoveAt(Index);
		}
	}

	if (Duration <= 0.0f)
	{
		Mannequin->SetActorLocation(TargetLocation);
		return;
	}

	FActiveMannequinMove NewMove;
	NewMove.Actor = Mannequin;
	NewMove.StartLocation = Mannequin->GetActorLocation();
	NewMove.TargetLocation = TargetLocation;
	NewMove.ElapsedTime = 0.0f;
	NewMove.Duration = Duration;

	ActiveMannequinMoves.Add(NewMove);
}

void ANeonPulseSphere::UpdateActiveMannequinMoves(float DeltaTime)
{
	for (int32 Index = ActiveMannequinMoves.Num() - 1; Index >= 0; --Index)
	{
		FActiveMannequinMove& Move = ActiveMannequinMoves[Index];

		if (!Move.Actor.IsValid())
		{
			ActiveMannequinMoves.RemoveAt(Index);
			continue;
		}

		AActor* Mannequin = Move.Actor.Get();

		Move.ElapsedTime += DeltaTime;

		const float Alpha = FMath::Clamp(Move.ElapsedTime / GetSafeDuration(Move.Duration), 0.0f, 1.0f);
		const float SmoothAlpha = FMath::SmoothStep(0.0f, 1.0f, Alpha);

		const FVector NewLocation = FMath::Lerp(
			Move.StartLocation,
			Move.TargetLocation,
			SmoothAlpha
		);

		Mannequin->SetActorLocation(NewLocation);

		if (Alpha >= 1.0f)
		{
			Mannequin->SetActorLocation(Move.TargetLocation);
			ActiveMannequinMoves.RemoveAt(Index);
		}
	}
}

FVector ANeonPulseSphere::GetWantedInitialLocation() const
{
	if (bUseTargetActorLocation && TargetCenterActor)
	{
		return TargetCenterActor->GetActorLocation() + TargetCenterOffset;
	}

	return GetActorLocation();
}

float ANeonPulseSphere::GetSafeDuration(float Duration) const
{
	return FMath::Max(Duration, 0.001f);
}