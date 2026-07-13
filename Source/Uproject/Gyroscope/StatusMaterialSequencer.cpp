#include "StatusMaterialSequencer.h"

#include "../Rhythm/RhythmTransitionStateSubsystem.h"
#include "Components/MeshComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

AStatusMaterialSequencer::AStatusMaterialSequencer()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	bEnableKeyboardToggle = false;
	ToggleKey = EKeys::H;
}

void AStatusMaterialSequencer::BeginPlay()
{
	Super::BeginPlay();

	CacheOriginalMaterials(true);
	bStepActivated.Init(false, StatusSteps.Num());

	if (bEnableKeyboardToggle)
	{
		if (UWorld* World = GetWorld())
		{
			if (APlayerController* PlayerController = World->GetFirstPlayerController())
			{
				EnableInput(PlayerController);

				if (InputComponent)
				{
					FInputKeyBinding& KeyBinding =
						InputComponent->BindKey(ToggleKey, IE_Pressed, this, &AStatusMaterialSequencer::ToggleStatusSequence);

					KeyBinding.bConsumeInput = bConsumeInput;
				}
			}
		}
	}

	if (bStartOnLevelBeginPlay)
	{
		ScheduleStatusSequenceStart(LevelBeginPlayStartDelay, TEXT("level BeginPlay"));
	}

	TryStartFromRhythmTransition();
}

void AStatusMaterialSequencer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RhythmTransitionStartTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void AStatusMaterialSequencer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bSequenceIsPlaying)
	{
		UpdateSequence(DeltaTime);
	}
}

#if WITH_EDITOR
bool AStatusMaterialSequencer::ShouldTickIfViewportsOnly() const
{
	return bPreviewTickInEditor;
}
#endif

void AStatusMaterialSequencer::ScheduleStatusSequenceStart(float DelaySec, const TCHAR* Reason)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[%s] Scheduling status sequence start from %s after %.2f sec."), *GetName(), Reason, DelaySec);

	World->GetTimerManager().ClearTimer(RhythmTransitionStartTimerHandle);

	if (DelaySec <= 0.0f)
	{
		StartStatusSequence();
		return;
	}

	World->GetTimerManager().SetTimer(
		RhythmTransitionStartTimerHandle,
		this,
		&AStatusMaterialSequencer::StartStatusSequence,
		DelaySec,
		false);
}

void AStatusMaterialSequencer::TryStartFromRhythmTransition()
{
	if (!bStartWhenRhythmTransitionPending)
	{
		UE_LOG(LogTemp, Verbose, TEXT("[%s] Rhythm transition auto-start skipped: checkbox disabled."), *GetName());
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	URhythmTransitionStateSubsystem* TransitionState = GameInstance->GetSubsystem<URhythmTransitionStateSubsystem>();
	if (!TransitionState || !TransitionState->ConsumeStatusSequencerRequest())
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Rhythm transition auto-start checked, but no pending rhythm transition request was found."), *GetName());
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[%s] Auto-starting status sequence from rhythm level transition."), *GetName());
	ScheduleStatusSequenceStart(RhythmTransitionStartDelay, TEXT("rhythm transition"));
}

void AStatusMaterialSequencer::StartStatusSequence()
{
	UE_LOG(LogTemp, Log, TEXT("[%s] Starting status material sequence. Steps=%d"), *GetName(), StatusSteps.Num());

	// Si on avait déjà joué une séquence, on remet proprement les matériaux de base
	if (bOriginalsCached)
	{
		RestoreOriginalMaterials();
	}

	// On recache les matériaux actuels comme état "default"
	CacheOriginalMaterials(true);

	// On remet l'état default avant de lancer la séquence
	RestoreOriginalMaterials();

	bStepActivated.Init(false, StatusSteps.Num());

	SequenceElapsedTime = 0.0f;
	bSequenceIsActive = true;
	bSequenceIsPlaying = true;

	ApplyPreActivationVisibility();

	// Active immédiatement les steps qui ont Delay = 0
	UpdateSequence(0.0f);
}

void AStatusMaterialSequencer::ResetStatusSequence()
{
	bSequenceIsActive = false;
	bSequenceIsPlaying = false;
	SequenceElapsedTime = 0.0f;

	bStepActivated.Init(false, StatusSteps.Num());

	if (!bOriginalsCached)
	{
		CacheOriginalMaterials(false);
	}

	RestoreOriginalMaterials();
}

void AStatusMaterialSequencer::ToggleStatusSequence()
{
	if (bSequenceIsActive)
	{
		ResetStatusSequence();
	}
	else
	{
		StartStatusSequence();
	}
}

void AStatusMaterialSequencer::UpdateSequence(float DeltaTime)
{
	SequenceElapsedTime += DeltaTime;

	bool bStillWaitingForAtLeastOneStep = false;

	for (int32 StepIndex = 0; StepIndex < StatusSteps.Num(); ++StepIndex)
	{
		if (!bStepActivated.IsValidIndex(StepIndex))
		{
			continue;
		}

		if (bStepActivated[StepIndex])
		{
			continue;
		}

		const float EffectiveDelay = GetEffectiveDelayForStep(StepIndex);

		if (SequenceElapsedTime >= EffectiveDelay)
		{
			ActivateStep(StepIndex);
			bStepActivated[StepIndex] = true;
		}
		else
		{
			bStillWaitingForAtLeastOneStep = true;
		}
	}

	// Tous les steps ont été activés.
	// Les matériaux restent activés, mais la séquence ne tick plus.
	if (!bStillWaitingForAtLeastOneStep)
	{
		bSequenceIsPlaying = false;
	}
}

void AStatusMaterialSequencer::ActivateStep(int32 StepIndex)
{
	if (!StatusSteps.IsValidIndex(StepIndex))
	{
		return;
	}

	const FStatusMaterialStep& Step = StatusSteps[StepIndex];

	if (!Step.ActivatedMaterial)
	{
		return;
	}

	TArray<UMeshComponent*> Meshes;
	ResolveMeshesForStep(StepIndex, Meshes);

	for (UMeshComponent* Mesh : Meshes)
	{
		if (!Mesh)
		{
			continue;
		}

		Mesh->SetVisibility(true, true);

		if (Step.bApplyToAllMaterialSlots)
		{
			const int32 MaterialCount = Mesh->GetNumMaterials();

			for (int32 SlotIndex = 0; SlotIndex < MaterialCount; ++SlotIndex)
			{
				Mesh->SetMaterial(SlotIndex, Step.ActivatedMaterial);
			}
		}
		else
		{
			const int32 SafeSlot = FMath::Max(0, Step.MaterialSlot);
			Mesh->SetMaterial(SafeSlot, Step.ActivatedMaterial);
		}
	}
}

void AStatusMaterialSequencer::ApplyPreActivationVisibility()
{
	for (int32 StepIndex = 0; StepIndex < StatusSteps.Num(); ++StepIndex)
	{
		if (!StatusSteps.IsValidIndex(StepIndex))
		{
			continue;
		}

		const FStatusMaterialStep& Step = StatusSteps[StepIndex];

		if (!Step.bHideUntilActivated)
		{
			continue;
		}

		TArray<UMeshComponent*> Meshes;
		ResolveMeshesForStep(StepIndex, Meshes);

		for (UMeshComponent* Mesh : Meshes)
		{
			if (Mesh)
			{
				Mesh->SetVisibility(false, true);
			}
		}
	}
}

void AStatusMaterialSequencer::CacheOriginalMaterials(bool bForceRecache)
{
	if (bOriginalsCached && !bForceRecache)
	{
		return;
	}

	CachedOriginalMaterials.Empty();
	CachedOriginalMaterials.SetNum(StatusSteps.Num());

	for (int32 StepIndex = 0; StepIndex < StatusSteps.Num(); ++StepIndex)
	{
		TArray<UMeshComponent*> Meshes;
		ResolveMeshesForStep(StepIndex, Meshes);

		for (UMeshComponent* Mesh : Meshes)
		{
			if (!Mesh)
			{
				continue;
			}

			FStatusMeshOriginalMaterials MeshCache;
			MeshCache.Mesh = Mesh;
			MeshCache.bWasVisible = Mesh->IsVisible();

			const int32 MaterialCount = Mesh->GetNumMaterials();
			MeshCache.Materials.Reserve(MaterialCount);

			for (int32 SlotIndex = 0; SlotIndex < MaterialCount; ++SlotIndex)
			{
				MeshCache.Materials.Add(Mesh->GetMaterial(SlotIndex));
			}

			CachedOriginalMaterials[StepIndex].Meshes.Add(MeshCache);
		}
	}

	bOriginalsCached = true;
}

void AStatusMaterialSequencer::RestoreOriginalMaterials()
{
	if (!bOriginalsCached)
	{
		return;
	}

	for (const FStatusOriginalStepMaterials& StepCache : CachedOriginalMaterials)
	{
		for (const FStatusMeshOriginalMaterials& MeshCache : StepCache.Meshes)
		{
			UMeshComponent* Mesh = MeshCache.Mesh.Get();

			if (!Mesh)
			{
				continue;
			}

			for (int32 SlotIndex = 0; SlotIndex < MeshCache.Materials.Num(); ++SlotIndex)
			{
				Mesh->SetMaterial(SlotIndex, MeshCache.Materials[SlotIndex]);
			}

			Mesh->SetVisibility(MeshCache.bWasVisible, true);
		}
	}
}

void AStatusMaterialSequencer::ResolveMeshesForStep(int32 StepIndex, TArray<UMeshComponent*>& OutMeshes) const
{
	OutMeshes.Reset();

	if (!StatusSteps.IsValidIndex(StepIndex))
	{
		return;
	}

	const FStatusMaterialStep& Step = StatusSteps[StepIndex];

	if (Step.TargetMeshOverride)
	{
		OutMeshes.Add(Step.TargetMeshOverride);
		return;
	}

	if (!Step.TargetActor)
	{
		return;
	}

	Step.TargetActor->GetComponents<UMeshComponent>(OutMeshes, Step.bIncludeChildActors);

	if (!Step.bApplyToAllMeshComponents && OutMeshes.Num() > 1)
	{
		UMeshComponent* FirstMesh = OutMeshes[0];
		OutMeshes.Reset();

		if (FirstMesh)
		{
			OutMeshes.Add(FirstMesh);
		}
	}
}

float AStatusMaterialSequencer::GetEffectiveDelayForStep(int32 StepIndex) const
{
	if (!StatusSteps.IsValidIndex(StepIndex))
	{
		return 0.0f;
	}

	if (!bDelaysAreRelativeToPreviousStep)
	{
		return FMath::Max(0.0f, StatusSteps[StepIndex].Delay);
	}

	float AccumulatedDelay = 0.0f;

	for (int32 Index = 0; Index <= StepIndex; ++Index)
	{
		if (StatusSteps.IsValidIndex(Index))
		{
			AccumulatedDelay += FMath::Max(0.0f, StatusSteps[Index].Delay);
		}
	}

	return AccumulatedDelay;
}
