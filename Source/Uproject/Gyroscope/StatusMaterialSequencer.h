#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InputCoreTypes.h"
#include "TimerManager.h"
#include "StatusMaterialSequencer.generated.h"

class UMeshComponent;
class UMaterialInterface;

USTRUCT(BlueprintType)
struct FStatusMaterialStep
{
	GENERATED_BODY()

	// Actor à illuminer : tu drag & drop ta statue/mannequin ici
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	TObjectPtr<AActor> TargetActor = nullptr;

	// Optionnel : si tu veux viser un mesh précis.
	// Tu peux laisser vide, dans ce cas tous les MeshComponents du TargetActor seront utilisés.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	TObjectPtr<UMeshComponent> TargetMeshOverride = nullptr;

	// Matériau appliqué après le délai
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	TObjectPtr<UMaterialInterface> ActivatedMaterial = nullptr;

	// Délai avant activation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status", meta = (ClampMin = "0.0"))
	float Delay = 0.0f;

	// Slot matériau à remplacer si bApplyToAllMaterialSlots = false
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status", meta = (ClampMin = "0"))
	int32 MaterialSlot = 0;

	// Remplace tous les slots matériaux du mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	bool bApplyToAllMaterialSlots = true;

	// Si aucun TargetMeshOverride n'est donné, applique à tous les MeshComponents du TargetActor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	bool bApplyToAllMeshComponents = true;

	// Inclure aussi les child actors éventuels
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	bool bIncludeChildActors = false;

	// Optionnel : cache la statue jusqu'à son activation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	bool bHideUntilActivated = false;
};

USTRUCT()
struct FStatusMeshOriginalMaterials
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	TObjectPtr<UMeshComponent> Mesh = nullptr;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInterface>> Materials;

	UPROPERTY(Transient)
	bool bWasVisible = true;
};

USTRUCT()
struct FStatusOriginalStepMaterials
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	TArray<FStatusMeshOriginalMaterials> Meshes;
};

UCLASS(Blueprintable)
class UPROJECT_API AStatusMaterialSequencer : public AActor
{
	GENERATED_BODY()

public:
	AStatusMaterialSequencer();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;

#if WITH_EDITOR
	virtual bool ShouldTickIfViewportsOnly() const override;
#endif

public:
	// Liste des statues / statuses à illuminer
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Sequencer")
	TArray<FStatusMaterialStep> StatusSteps;

	// Si false : Delay = temps depuis l'appui sur H.
	// Exemple : 0.0, 0.5, 1.0, 1.5
	//
	// Si true : Delay = temps après le step précédent.
	// Exemple : 0.5, 0.5, 0.5, 0.5
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Sequencer")
	bool bDelaysAreRelativeToPreviousStep = false;

	// Permet de prévisualiser avec les boutons dans l'éditeur
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Sequencer")
	bool bPreviewTickInEditor = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Sequencer")
	bool bStartOnLevelBeginPlay = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Sequencer", meta = (EditCondition = "bStartOnLevelBeginPlay", ClampMin = "0.0"))
	float LevelBeginPlayStartDelay = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Sequencer")
	bool bStartWhenRhythmTransitionPending = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Sequencer", meta = (EditCondition = "bStartWhenRhythmTransitionPending", ClampMin = "0.0"))
	float RhythmTransitionStartDelay = 0.2f;

	// Active le toggle clavier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	bool bEnableKeyboardToggle = false;

	// Touche de test
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	FKey ToggleKey;

	// Si true, la touche H est consommée par cet actor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	bool bConsumeInput = false;

	// Bouton visible dans l'inspector
	UFUNCTION(CallInEditor, BlueprintCallable, Category = "Status Sequencer")
	void StartStatusSequence();

	// Bouton visible dans l'inspector
	UFUNCTION(CallInEditor, BlueprintCallable, Category = "Status Sequencer")
	void ResetStatusSequence();

	UFUNCTION(BlueprintCallable, Category = "Status Sequencer")
	void ToggleStatusSequence();

private:
	UPROPERTY(Transient)
	TArray<FStatusOriginalStepMaterials> CachedOriginalMaterials;

	UPROPERTY(Transient)
	TArray<bool> bStepActivated;

	bool bOriginalsCached = false;
	bool bSequenceIsActive = false;
	bool bSequenceIsPlaying = false;

	float SequenceElapsedTime = 0.0f;
	FTimerHandle RhythmTransitionStartTimerHandle;

private:
	void UpdateSequence(float DeltaTime);
	void ScheduleStatusSequenceStart(float DelaySec, const TCHAR* Reason);
	void TryStartFromRhythmTransition();

	void ActivateStep(int32 StepIndex);
	void ApplyPreActivationVisibility();

	void CacheOriginalMaterials(bool bForceRecache);
	void RestoreOriginalMaterials();

	void ResolveMeshesForStep(int32 StepIndex, TArray<UMeshComponent*>& OutMeshes) const;

	float GetEffectiveDelayForStep(int32 StepIndex) const;
};
