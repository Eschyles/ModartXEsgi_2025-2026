#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NeonPulseSphere.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;

UENUM(BlueprintType)
enum class ENeonSphereState : uint8
{
	Grow       UMETA(DisplayName = "Grow"),
	Move       UMETA(DisplayName = "Move"),
	MovedPulse UMETA(DisplayName = "Moved Pulse"),
	Vanish     UMETA(DisplayName = "Vanish")
};

USTRUCT(BlueprintType)
struct FNeonMannequinRevealStep
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neon|Final Pulse Reveal")
	AActor* Mannequin = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neon|Final Pulse Reveal")
	bool bMoveAfterReveal = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neon|Final Pulse Reveal")
	FVector TargetWorldLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neon|Final Pulse Reveal", meta = (ClampMin = "0.0"))
	float MoveDuration = 1.5f;
};

struct FActiveMannequinMove
{
	TWeakObjectPtr<AActor> Actor;
	FVector StartLocation = FVector::ZeroVector;
	FVector TargetLocation = FVector::ZeroVector;
	float ElapsedTime = 0.0f;
	float Duration = 1.0f;
};

UCLASS(Blueprintable)
class UPROJECT_API ANeonPulseSphere : public AActor
{
	GENERATED_BODY()

public:
	ANeonPulseSphere();

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

public:
	virtual void Tick(float DeltaTime) override;

protected:
	// ---------------- COMPONENTS ----------------

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* MeshPivot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* SphereMesh;

	// ---------------- GENERAL ----------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neon|General")
	ENeonSphereState CurrentState = ENeonSphereState::Grow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neon|General")
	bool bAutoEnableInput = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neon|General")
	bool bWaitForFirstInputToStart = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neon|General")
	bool bAutoAdvanceStates = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neon|General")
	bool bUsePlacedScaleAsStartScale = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neon|General")
	bool bAutoCenterMeshOnPivot = true;

	// ---------------- TARGET CENTER ----------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neon|Target Center")
	bool bUseTargetActorLocation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neon|Target Center")
	AActor* TargetCenterActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neon|Target Center")
	FVector TargetCenterOffset = FVector::ZeroVector;

	// ---------------- CONSTANT PULSE ----------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neon|Pulse")
	float PulseSpeed = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neon|Pulse")
	float PulseStrength = 0.25f;

	// ---------------- STATE 1 : GROW ----------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neon|State 1 Grow")
	float StartBaseScale = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neon|State 1 Grow")
	float GrowTargetScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neon|State 1 Grow")
	float GrowDuration = 3.0f;

	// ---------------- STATE 2 : MOVE ----------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neon|State 2 Move")
	FVector MoveDirection = FVector(0.0f, 0.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neon|State 2 Move")
	float MoveDistance = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neon|State 2 Move")
	float MoveDuration = 3.0f;

	// ---------------- STATE 3 : PULSE AFTER MOVE ----------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neon|State 3 Moved Pulse")
	float MovedPulseDuration = 2.0f;

	// ---------------- STATE 4 : VANISH ----------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neon|State 4 Vanish")
	float FinalGrowScale = 2.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neon|State 4 Vanish")
	float VanishDuration = 2.5f;

	// ---------------- FINAL PULSE REVEAL ----------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neon|Final Pulse Reveal")
	TArray<FNeonMannequinRevealStep> MannequinRevealOrder;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neon|Final Pulse Reveal")
	bool bHideMannequinsAtBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neon|Final Pulse Reveal")
	bool bDisableMannequinCollisionWhileHidden = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neon|Final Pulse Reveal", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RevealVanishProgress = 0.65f;

	// ---------------- MATERIAL ----------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neon|Material")
	FName EmissionStrengthParameterName = "EmissionStrength";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neon|Material")
	FName OpacityParameterName = "Opacity";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neon|Material")
	float NormalEmissionStrength = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neon|Material")
	float VanishEmissionStrength = 60.0f;

private:
	float StateTime = 0.0f;
	float PulseTime = 0.0f;
	float BaseScale = 1.0f;

	bool bRevealTriggeredThisVanish = false;
	bool bHasStarted = false;
	int32 NextMannequinRevealIndex = 0;

	FVector InitialLocation = FVector::ZeroVector;
	FVector MoveStartLocation = FVector::ZeroVector;
	FVector MoveTargetLocation = FVector::ZeroVector;

	UPROPERTY()
	TArray<UMaterialInstanceDynamic*> DynamicMaterials;

	TArray<FActiveMannequinMove> ActiveMannequinMoves;

	void GoToNextState();
	void EnterState(ENeonSphereState NewState);

	void UpdateGrowState(float DeltaTime);
	void UpdateMoveState(float DeltaTime);
	void UpdateMovedPulseState(float DeltaTime);
	void UpdateVanishState(float DeltaTime);

	void ApplyConstantPulseScale();
	void ApplyVisualScale(float NewScale);
	void CenterMeshOnPivot();

	void CreateDynamicMaterials();
	void SetMaterialScalar(FName ParameterName, float Value);

	void HideAllMannequinsInRevealOrder();
	void RevealNextMannequinOnFinalGrow();
	void StartMannequinMove(AActor* Mannequin, const FVector& TargetLocation, float Duration);
	void UpdateActiveMannequinMoves(float DeltaTime);

	FVector GetWantedInitialLocation() const;
	float GetSafeDuration(float Duration) const;
};
