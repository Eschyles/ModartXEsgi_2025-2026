#include "RhythmMinigameActor.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "RhythmButtonActor.h"
#include "RhythmNoteActor.h"
#include "RhythmSongData.h"
#include "RhythmTransitionStateSubsystem.h"
#include "../Gyroscope/StatusMaterialSequencer.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Components/InputComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SplineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/LocalPlayer.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

ARhythmMinigameActor::ARhythmMinigameActor()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	NoteSpline = CreateDefaultSubobject<USplineComponent>(TEXT("NoteSpline"));
	NoteSpline->SetupAttachment(SceneRoot);
	NoteSpline->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	NoteSpline->SetGenerateOverlapEvents(false);
	NoteSpline->ClearSplinePoints(false);
	NoteSpline->AddSplinePoint(FVector(HitDistanceAlongSpline, 0.0f, 0.0f), ESplineCoordinateSpace::Local, false);
	NoteSpline->AddSplinePoint(FVector(SpawnDistanceAlongSpline, 0.0f, 0.0f), ESplineCoordinateSpace::Local, true);

	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	AudioComponent->SetupAttachment(SceneRoot);
	AudioComponent->bAutoActivate = false;

	TriggerVolumeComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
	TriggerVolumeComponent->SetupAttachment(SceneRoot);
	TriggerVolumeComponent->SetBoxExtent(FVector(260.0f, 260.0f, 180.0f));
	TriggerVolumeComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TriggerVolumeComponent->SetCollisionObjectType(ECC_WorldDynamic);
	TriggerVolumeComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerVolumeComponent->SetGenerateOverlapEvents(false);

	ScoreTextComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("ScoreTextComponent"));
	ScoreTextComponent->SetupAttachment(SceneRoot);
	ScoreTextComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ScoreTextComponent->SetGenerateOverlapEvents(false);
	ScoreTextComponent->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
	ScoreTextComponent->SetRelativeLocation(ScoreTextRelativeLocation);
	ScoreTextComponent->SetWorldSize(ScoreTextWorldSize);

	HologramRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("HologramRootComponent"));
	HologramRootComponent->SetupAttachment(SceneRoot);
	HologramRootComponent->SetRelativeLocation(HologramRelativeLocation);
	HologramRootComponent->SetRelativeRotation(HologramRelativeRotation);
	HologramRootComponent->SetRelativeScale3D(HologramRelativeScale);

	HologramPanelComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HologramPanelComponent"));
	HologramPanelComponent->SetupAttachment(HologramRootComponent);
	HologramPanelComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	HologramTitleTextComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("HologramTitleTextComponent"));
	HologramTitleTextComponent->SetupAttachment(HologramRootComponent);
	HologramTitleTextComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HologramTitleTextComponent->SetGenerateOverlapEvents(false);
	HologramTitleTextComponent->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
	HologramTitleTextComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 70.0f));

	HologramStatusTextComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("HologramStatusTextComponent"));
	HologramStatusTextComponent->SetupAttachment(HologramRootComponent);
	HologramStatusTextComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HologramStatusTextComponent->SetGenerateOverlapEvents(false);
	HologramStatusTextComponent->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);

	HologramStoryTextComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("HologramStoryTextComponent"));
	HologramStoryTextComponent->SetupAttachment(HologramRootComponent);
	HologramStoryTextComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HologramStoryTextComponent->SetGenerateOverlapEvents(false);
	HologramStoryTextComponent->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
	HologramStoryTextComponent->SetRelativeLocation(FVector(0.0f, 0.0f, -60.0f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (PlaneMesh.Succeeded())
	{
		HologramPanelComponent->SetStaticMesh(PlaneMesh.Object);
		HologramPanelComponent->SetRelativeScale3D(FVector(2.6f, 1.4f, 1.0f));
	}

	NoteActorClass = ARhythmNoteActor::StaticClass();
	ButtonActorClass = ARhythmButtonActor::StaticClass();

	LaneColors = {
		FLinearColor(0.0f, 0.9f, 1.0f),
		FLinearColor(1.0f, 0.0f, 0.8f),
		FLinearColor(0.8f, 1.0f, 0.0f)
	};

	Lane0FallbackKey = EKeys::A;
	Lane1FallbackKey = EKeys::S;
	Lane2FallbackKey = EKeys::D;

	HologramIdleTitle = FText::FromString(TEXT("RHYTHM LINK"));
	HologramRunningTitle = FText::FromString(TEXT("SYNC ACTIVE"));
	HologramPassedTitle = FText::FromString(TEXT("SYNC PASSED"));
	HologramFailedTitle = FText::FromString(TEXT("SYNC FAILED"));
	HologramStoryText = FText::FromString(TEXT("Follow the lanes and hit the notes on the beat."));
}

void ARhythmMinigameActor::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Log, TEXT("[%s] Rhythm minigame C++ sequence patch loaded. SongSequence=%d SpawnOnBeginPlay=%s StartSequenceOnBeginPlay=%s TriggerStart=%s"),
		*GetName(),
		SongSequence.Num(),
		bSpawnOnBeginPlay ? TEXT("true") : TEXT("false"),
		bStartSongSequenceOnBeginPlay ? TEXT("true") : TEXT("false"),
		bStartSongSequenceOnTriggerOverlap ? TEXT("true") : TEXT("false"));

	if (TriggerVolumeComponent)
	{
		TriggerVolumeComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		TriggerVolumeComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
		TriggerVolumeComponent->SetGenerateOverlapEvents(false);
	}

	if (bSpawnButtonsOnBeginPlay)
	{
		SpawnButtonActors();
	}

	RefreshScoreText();
	RefreshHologramScreen();
	UpdatePlayerFacingVisuals();

	if (bStartSongSequenceOnBeginPlay)
	{
		StartSongSequence();
	}
	else if (bSpawnOnBeginPlay)
	{
		StartMinigame(OverrideSongData);
	}

	if (bCheckInitialTriggerOverlapsOnBeginPlay && GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ARhythmMinigameActor::CheckInitialTriggerOverlaps);
	}
}

void ARhythmMinigameActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(StartNextSequenceSongTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(OpenPassedLevelTimerHandle);
	}
	TeardownInput();
	DestroyRuntimeActors();
	Super::EndPlay(EndPlayReason);
}

void ARhythmMinigameActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdatePlayerFacingVisuals();

	if (!bRunning)
	{
		PollTriggerVolumeForPlayer();
		return;
	}

	CachedSongTimeSec = GetSongTimeSec();
	SpawnDueNotes();
	UpdateActiveNotes();
	UpdateTrackVisualPulse();
	FinishIfComplete();
}

void ARhythmMinigameActor::StartMinigame(URhythmSongData* InOverrideSongData)
{
	bSequenceActive = false;
	CurrentSequenceSongIndex = INDEX_NONE;
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(StartNextSequenceSongTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(OpenPassedLevelTimerHandle);
	}

	StartSongInternal(InOverrideSongData);
}

void ARhythmMinigameActor::StartSongSequence()
{
	UE_LOG(LogTemp, Log, TEXT("[%s] StartSongSequence called. SongSequence=%d Override=%s SongData=%s"),
		*GetName(),
		SongSequence.Num(),
		*GetNameSafe(OverrideSongData.Get()),
		*GetNameSafe(SongData.Get()));

	if (SongSequence.Num() == 0)
	{
		bSequenceActive = false;
		UE_LOG(LogTemp, Warning, TEXT("[%s] SongSequence is empty, falling back to single song mode. Fill Song Sequence with both tracks."), *GetName());
		if (!StartSongInternal(OverrideSongData))
		{
			OnRhythmGateFailed.Broadcast();
		}
		return;
	}

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(StartNextSequenceSongTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(OpenPassedLevelTimerHandle);
	}

	bSequenceActive = true;
	bSequenceGateUnlocked = false;
	CurrentSequenceSongIndex = 0;
	PassedSequenceSongCount = 0;
	CompletedSequenceStats.Reset();
	StartSequenceSongAtIndex(CurrentSequenceSongIndex);
}

void ARhythmMinigameActor::ResetSongSequenceProgress()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(StartNextSequenceSongTimerHandle);
	}

	bSequenceActive = false;
	bSequenceGateUnlocked = false;
	bTriggerAlreadyUsed = false;
	bTriggerNeedsExitBeforeRestart = false;
	CurrentSequenceSongIndex = INDEX_NONE;
	PassedSequenceSongCount = 0;
	CompletedSequenceStats.Reset();
	LastStartError.Reset();
	LastTriggerActorName = NAME_None;
	RefreshScoreText();
	RefreshHologramScreen();
}

bool ARhythmMinigameActor::StartSongInternal(URhythmSongData* InSongData)
{
	LastStartError.Reset();

	URhythmSongData* SelectedSongData = InSongData;
	if (!SelectedSongData)
	{
		SelectedSongData = OverrideSongData.Get();
	}
	if (!SelectedSongData)
	{
		SelectedSongData = SongData.Get();
	}

	ActiveSongData = SelectedSongData;
	if (!ActiveSongData)
	{
		LastStartError = TEXT("No RhythmSongData assigned. Fill Song Sequence, Override Song Data, or Song Data.");
		UE_LOG(LogTemp, Warning, TEXT("%s"), *LastStartError);
		RefreshHologramScreen();
		return false;
	}

	FString Error;
	if (!ActiveSongData->GetSortedNotes(SortedNotes, Error))
	{
		LastStartError = FString::Printf(TEXT("Could not start rhythm minigame: %s"), *Error);
		UE_LOG(LogTemp, Warning, TEXT("%s"), *LastStartError);
		RefreshHologramScreen();
		return false;
	}

	if (SortedNotes.Num() == 0)
	{
		LastStartError = FString::Printf(TEXT("Could not start rhythm minigame: %s has no notes. Check the NoteTable/DataTable import."), *GetNameSafe(ActiveSongData));
		UE_LOG(LogTemp, Warning, TEXT("%s"), *LastStartError);
		RefreshHologramScreen();
		return false;
	}

	DestroyRuntimeActors();
	SpawnButtonActors();

	JudgedRows.Reset();
	ScoreStats = FRhythmScoreStats();
	ScoreStats.TotalNotes = SortedNotes.Num();
	NextSpawnNoteIndex = 0;
	StartWorldTimeSec = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	CachedSongTimeSec = -CountdownLeadInSec;
	bRunning = true;

	PerfectWindowSec = ActiveSongData->PerfectWindowSec;
	GoodWindowSec = ActiveSongData->GoodWindowSec;
	MissWindowSec = ActiveSongData->MissWindowSec;
	NoteTravelTimeSec = ActiveSongData->NoteTravelTimeSec;
	CountdownLeadInSec = ActiveSongData->CountdownLeadInSec;
	PassAccuracyThreshold = ActiveSongData->PassAccuracyThreshold;
	SongEndTimeSec = ActiveSongData->SongEndTimeSec;

	if (AudioComponent)
	{
		AudioComponent->SetSound(ActiveSongData->SongAudio);
		if (ActiveSongData->SongAudio)
		{
			AudioComponent->Play(FMath::Max(0.0f, ActiveSongData->AudioStartOffsetSec));
		}
	}

	SetupInput();
	RefreshScoreText();
	RefreshHologramScreen();
	OnMinigameStarted.Broadcast();
	return true;
}

void ARhythmMinigameActor::StartSequenceSongAtIndex(int32 SongIndex)
{
	if (!SongSequence.IsValidIndex(SongIndex) || !SongSequence[SongIndex])
	{
		UE_LOG(LogTemp, Warning, TEXT("Rhythm sequence cannot start song index %d."), SongIndex);
		bSequenceActive = false;
		OnRhythmGateFailed.Broadcast();
		RefreshHologramScreen();
		return;
	}

	CurrentSequenceSongIndex = SongIndex;
	UE_LOG(LogTemp, Log, TEXT("[%s] Rhythm sequence starting track %d/%d: %s"),
		*GetName(),
		SongIndex + 1,
		SongSequence.Num(),
		*GetNameSafe(SongSequence[SongIndex].Get()));

	if (!StartSongInternal(SongSequence[SongIndex].Get()))
	{
		bSequenceActive = false;
		CurrentSequenceSongIndex = INDEX_NONE;
		OnRhythmGateFailed.Broadcast();
	}
}

void ARhythmMinigameActor::StartNextSequenceSong()
{
	StartSequenceSongAtIndex(CurrentSequenceSongIndex);
}

void ARhythmMinigameActor::HandleSequenceSongFinished(const FRhythmScoreStats& FinishedStats)
{
	if (!bSequenceActive)
	{
		return;
	}

	const int32 FinishedSongIndex = CurrentSequenceSongIndex;
	CompletedSequenceStats.Add(FinishedStats);
	UE_LOG(LogTemp, Log, TEXT("[%s] Rhythm sequence finished track %d/%d. Passed=%s Accuracy=%.2f Threshold=%.2f ActiveSong=%s"),
		*GetName(),
		FinishedSongIndex + 1,
		SongSequence.Num(),
		FinishedStats.bPassed ? TEXT("true") : TEXT("false"),
		FinishedStats.Accuracy,
		PassAccuracyThreshold,
		*GetNameSafe(ActiveSongData.Get()));

	if (!FinishedStats.bPassed)
	{
		bSequenceActive = false;
		bSequenceGateUnlocked = false;
		CurrentSequenceSongIndex = INDEX_NONE;
		OnRhythmTrackFailed.Broadcast(FinishedSongIndex, FinishedStats);
		OnRhythmGateFailed.Broadcast();
		RefreshScoreText();
		RefreshHologramScreen();
		return;
	}

	++PassedSequenceSongCount;
	OnRhythmTrackPassed.Broadcast(FinishedSongIndex, FinishedStats);

	if (PassedSequenceSongCount >= SongSequence.Num())
	{
		bSequenceActive = false;
		CurrentSequenceSongIndex = INDEX_NONE;
		HandleRhythmGatePassed();
		return;
	}

	CurrentSequenceSongIndex = PassedSequenceSongCount;
	RefreshScoreText();
	RefreshHologramScreen();

	UE_LOG(LogTemp, Log, TEXT("[%s] Rhythm sequence advancing immediately to track %d/%d."),
		*GetName(),
		CurrentSequenceSongIndex + 1,
		SongSequence.Num());
	StartNextSequenceSong();
}

void ARhythmMinigameActor::HandleRhythmGatePassed()
{
	bSequenceGateUnlocked = true;
	RefreshScoreText();
	RefreshHologramScreen();
	UE_LOG(LogTemp, Log, TEXT("[%s] Rhythm gate passed. OpenLevel=%s Level=%s Delay=%.2f"),
		*GetName(),
		bOpenLevelOnRhythmPassed ? TEXT("true") : TEXT("false"),
		*LevelToOpenOnRhythmPassed.ToString(),
		OpenLevelDelaySec);
	StartPassedStatusSequencers();
	OnRhythmPassed.Broadcast();
	ScheduleOpenPassedLevel();
}

void ARhythmMinigameActor::StartPassedStatusSequencers()
{
	for (AStatusMaterialSequencer* Sequencer : StatusSequencersToStartOnRhythmPassed)
	{
		if (!Sequencer)
		{
			continue;
		}

		UE_LOG(LogTemp, Log, TEXT("[%s] Starting status material sequencer on rhythm pass: %s"), *GetName(), *Sequencer->GetName());
		Sequencer->StartStatusSequence();
	}
}

void ARhythmMinigameActor::ScheduleOpenPassedLevel()
{
	if (!bOpenLevelOnRhythmPassed)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Rhythm gate passed but Open Level On Rhythm Passed is unchecked."), *GetName());
		return;
	}

	if (LevelToOpenOnRhythmPassed.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Rhythm gate passed but Level To Open On Rhythm Passed is empty."), *GetName());
		return;
	}

	if (bStartStatusSequencersWhenOpenedLevelLoads)
	{
		if (UWorld* World = GetWorld())
		{
			if (UGameInstance* GameInstance = World->GetGameInstance())
			{
				if (URhythmTransitionStateSubsystem* TransitionState = GameInstance->GetSubsystem<URhythmTransitionStateSubsystem>())
				{
					TransitionState->RequestStatusSequencersOnNextLevel();
					UE_LOG(LogTemp, Log, TEXT("[%s] Requested destination level status sequencer auto-start."), *GetName());
				}
			}
		}
	}

	if (!GetWorld())
	{
		return;
	}

	if (OpenLevelDelaySec <= 0.0f)
	{
		OpenPassedLevel();
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(
		OpenPassedLevelTimerHandle,
		this,
		&ARhythmMinigameActor::OpenPassedLevel,
		OpenLevelDelaySec,
		false);
}

void ARhythmMinigameActor::OpenPassedLevel()
{
	if (LevelToOpenOnRhythmPassed.IsNone())
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[%s] Opening rhythm passed level: %s"), *GetName(), *LevelToOpenOnRhythmPassed.ToString());
	UGameplayStatics::OpenLevel(this, LevelToOpenOnRhythmPassed);
}

bool ARhythmMinigameActor::TryStartSongSequenceFromTriggerActor(AActor* OtherActor)
{
	if (!bStartSongSequenceOnTriggerOverlap || !OtherActor || OtherActor == this || bRunning || bSequenceActive || bSequenceGateUnlocked)
	{
		return false;
	}

	if (bTriggerOnlyOnce && bTriggerAlreadyUsed)
	{
		return false;
	}

	if (!RequiredTriggerActorTag.IsNone() && !OtherActor->ActorHasTag(RequiredTriggerActorTag))
	{
		if (bLogTriggerAttempts)
		{
			UE_LOG(LogTemp, Log, TEXT("Rhythm trigger ignored %s because it does not have required tag %s."), *OtherActor->GetName(), *RequiredTriggerActorTag.ToString());
		}
		return false;
	}

	LastTriggerActorName = OtherActor->GetFName();
	if (bLogTriggerAttempts)
	{
		UE_LOG(LogTemp, Log, TEXT("[%s] Rhythm trigger starting sequence from actor %s. SongSequence=%d"),
			*GetName(),
			*OtherActor->GetName(),
			SongSequence.Num());
	}

	bTriggerAlreadyUsed = true;
	bTriggerNeedsExitBeforeRestart = true;
	StartSongSequence();
	return true;
}

void ARhythmMinigameActor::CheckInitialTriggerOverlaps()
{
	PollTriggerVolumeForPlayer();
}

void ARhythmMinigameActor::HandleTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	TryStartSongSequenceFromTriggerActor(OtherActor);
}

void ARhythmMinigameActor::PollTriggerVolumeForPlayer()
{
	if (!bPollTriggerVolumeForPlayer || !TriggerVolumeComponent || !bStartSongSequenceOnTriggerOverlap || bRunning || bSequenceActive || bSequenceGateUnlocked)
	{
		return;
	}

	APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PlayerController)
	{
		return;
	}

	bool bInsideTrigger = false;
	AActor* Pawn = PlayerController->GetPawn();
	if (Pawn && IsWorldLocationInsideTrigger(Pawn->GetActorLocation()))
	{
		bInsideTrigger = true;
	}

	FVector ViewLocation = FVector::ZeroVector;
	FRotator ViewRotation = FRotator::ZeroRotator;
	PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);
	if (!bInsideTrigger && IsWorldLocationInsideTrigger(ViewLocation))
	{
		bInsideTrigger = true;
	}

	if (!bInsideTrigger)
	{
		bTriggerNeedsExitBeforeRestart = false;
		return;
	}

	if (bTriggerNeedsExitBeforeRestart)
	{
		return;
	}

	TryStartSongSequenceFromTriggerActor(Pawn ? Pawn : PlayerController);
}

bool ARhythmMinigameActor::IsWorldLocationInsideTrigger(const FVector& WorldLocation) const
{
	if (!TriggerVolumeComponent)
	{
		return false;
	}

	const FVector LocalLocation = TriggerVolumeComponent->GetComponentTransform().InverseTransformPosition(WorldLocation);
	const FVector Extent = TriggerVolumeComponent->GetUnscaledBoxExtent();
	return FMath::Abs(LocalLocation.X) <= Extent.X
		&& FMath::Abs(LocalLocation.Y) <= Extent.Y
		&& FMath::Abs(LocalLocation.Z) <= Extent.Z;
}

void ARhythmMinigameActor::StopMinigame(bool bResetScore)
{
	bRunning = false;
	TeardownInput();

	if (AudioComponent)
	{
		AudioComponent->Stop();
	}

	if (bDestroySpawnedActorsOnStop)
	{
		DestroyRuntimeActors();
	}

	if (bResetScore)
	{
		ScoreStats = FRhythmScoreStats();
	}

	RefreshScoreText();
	RefreshHologramScreen();
	OnMinigameStopped.Broadcast();
}

bool ARhythmMinigameActor::IsRunning() const
{
	return bRunning;
}

void ARhythmMinigameActor::HandleLaneInput(int32 LaneIndex)
{
	if (!bRunning)
	{
		return;
	}

	float TimingErrorSec = 0.0f;
	const ERhythmHitRating Rating = ResolveActiveNote(LaneIndex, TimingErrorSec);

	if (Rating == ERhythmHitRating::Perfect)
	{
		++ScoreStats.Perfect;
	}
	else if (Rating == ERhythmHitRating::Good)
	{
		++ScoreStats.Good;
	}
	else
	{
		if (Rating == ERhythmHitRating::Miss)
		{
			++ScoreStats.Miss;
		}
		else
		{
			++ScoreStats.BadPress;
		}
	}

	RecalculateStats();
	PulseButton(LaneIndex, Rating);
	RefreshScoreText();
	RefreshHologramScreen();
	OnNoteJudged.Broadcast(LaneIndex, Rating, TimingErrorSec, ScoreStats);
}

float ARhythmMinigameActor::GetSongTimeSec() const
{
	if (!GetWorld())
	{
		return CachedSongTimeSec;
	}

	return GetWorld()->GetTimeSeconds() - StartWorldTimeSec - CountdownLeadInSec;
}

FRhythmScoreStats ARhythmMinigameActor::GetScoreStats() const
{
	return ScoreStats;
}

int32 ARhythmMinigameActor::GetRequiredSongCount() const
{
	return SongSequence.Num() > 0 ? SongSequence.Num() : 1;
}

void ARhythmMinigameActor::SetupInput()
{
	APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PlayerController)
	{
		return;
	}

	if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (RhythmInputMappingContext)
			{
				Subsystem->AddMappingContext(RhythmInputMappingContext, RhythmInputPriority);
			}
		}
	}

	EnableInput(PlayerController);

	if (bEnableFallbackKeyboardInput && InputComponent)
	{
		if (Lane0FallbackKey.IsValid())
		{
			InputComponent->BindKey(Lane0FallbackKey, IE_Pressed, this, &ARhythmMinigameActor::HandleLane0Action);
		}

		if (Lane1FallbackKey.IsValid())
		{
			InputComponent->BindKey(Lane1FallbackKey, IE_Pressed, this, &ARhythmMinigameActor::HandleLane1Action);
		}

		if (Lane2FallbackKey.IsValid())
		{
			InputComponent->BindKey(Lane2FallbackKey, IE_Pressed, this, &ARhythmMinigameActor::HandleLane2Action);
		}
	}

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EnhancedInputComponent)
	{
		return;
	}

	if (Lane0Action)
	{
		EnhancedInputComponent->BindAction(Lane0Action, ETriggerEvent::Started, this, &ARhythmMinigameActor::HandleLane0Action);
	}

	if (Lane1Action)
	{
		EnhancedInputComponent->BindAction(Lane1Action, ETriggerEvent::Started, this, &ARhythmMinigameActor::HandleLane1Action);
	}

	if (Lane2Action)
	{
		EnhancedInputComponent->BindAction(Lane2Action, ETriggerEvent::Started, this, &ARhythmMinigameActor::HandleLane2Action);
	}
}

void ARhythmMinigameActor::TeardownInput()
{
	APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PlayerController)
	{
		return;
	}

	if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (RhythmInputMappingContext)
			{
				Subsystem->RemoveMappingContext(RhythmInputMappingContext);
			}
		}
	}

	DisableInput(PlayerController);
}

void ARhythmMinigameActor::HandleLane0Action()
{
	HandleLaneInput(0);
}

void ARhythmMinigameActor::HandleLane1Action()
{
	HandleLaneInput(1);
}

void ARhythmMinigameActor::HandleLane2Action()
{
	HandleLaneInput(2);
}

void ARhythmMinigameActor::SpawnButtonActors()
{
	if (!GetWorld() || !ButtonActorClass || SpawnedButtons.Num() > 0)
	{
		return;
	}

	const float HitDistanceOnSpline = GetHitDistanceOnSpline();
	const float SplineLength = NoteSpline ? NoteSpline->GetSplineLength() : 0.0f;
	const float ButtonDistanceOnSpline = NoteSpline
		? FMath::Clamp(HitDistanceOnSpline + ButtonDistanceOffsetAlongSpline, 0.0f, SplineLength)
		: HitDistanceOnSpline;
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int32 LaneIndex = 0; LaneIndex < LaneCount; ++LaneIndex)
	{
		const FTransform SpawnTransform = GetLaneTransformOnSpline(ButtonDistanceOnSpline, LaneIndex, ButtonHeightOffset);
		ARhythmButtonActor* ButtonActor = GetWorld()->SpawnActor<ARhythmButtonActor>(ButtonActorClass, SpawnTransform, SpawnParameters);
		if (!ButtonActor)
		{
			continue;
		}

		ButtonActor->SetActorEnableCollision(false);
		if (ButtonMesh && ButtonActor->ButtonMeshComponent)
		{
			ButtonActor->ButtonMeshComponent->SetStaticMesh(ButtonMesh);
			ButtonActor->ButtonMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			ButtonActor->ButtonMeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
			ButtonActor->ButtonMeshComponent->SetGenerateOverlapEvents(false);
		}
		ButtonActor->ConfigureButton(LaneIndex, GetLaneColor(LaneIndex));

		SpawnedButtons.Add(ButtonActor);
	}
}

void ARhythmMinigameActor::DestroyRuntimeActors()
{
	for (ARhythmNoteActor* NoteActor : ActiveNoteActors)
	{
		if (IsValid(NoteActor))
		{
			NoteActor->Destroy();
		}
	}
	ActiveNoteActors.Reset();

	for (ARhythmButtonActor* ButtonActor : SpawnedButtons)
	{
		if (IsValid(ButtonActor))
		{
			ButtonActor->Destroy();
		}
	}
	SpawnedButtons.Reset();
}

void ARhythmMinigameActor::SpawnDueNotes()
{
	if (!GetWorld() || !NoteActorClass)
	{
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	while (SortedNotes.IsValidIndex(NextSpawnNoteIndex))
	{
		const FRhythmResolvedNote& ResolvedNote = SortedNotes[NextSpawnNoteIndex];
		if (ResolvedNote.Note.TimeSec - CachedSongTimeSec > NoteTravelTimeSec)
		{
			break;
		}

		ARhythmNoteActor* NoteActor = GetWorld()->SpawnActor<ARhythmNoteActor>(NoteActorClass, GetActorTransform(), SpawnParameters);
		if (NoteActor)
		{
			NoteActor->SetActorEnableCollision(false);
			NoteActor->ConfigureNote(ResolvedNote.Note.Lane, ResolvedNote.Note.TimeSec, ResolvedNote.Note.DurationSec);
			if (NoteMeshOverride && NoteActor->NoteMeshComponent)
			{
				NoteActor->NoteMeshComponent->SetStaticMesh(NoteMeshOverride);
				NoteActor->NoteMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				NoteActor->NoteMeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
				NoteActor->NoteMeshComponent->SetGenerateOverlapEvents(false);
			}
			NoteActor->SetVisualColor(GetLaneColor(ResolvedNote.Note.Lane));
			ActiveNoteActors.Add(NoteActor);
		}

		++NextSpawnNoteIndex;
	}
}

void ARhythmMinigameActor::UpdateActiveNotes()
{
	for (int32 NoteIndex = ActiveNoteActors.Num() - 1; NoteIndex >= 0; --NoteIndex)
	{
		ARhythmNoteActor* NoteActor = ActiveNoteActors[NoteIndex];
		if (!IsValid(NoteActor))
		{
			ActiveNoteActors.RemoveAtSwap(NoteIndex);
			continue;
		}

		const float TimeUntilHit = NoteActor->NoteTimeSec - CachedSongTimeSec;
		const float Alpha = 1.0f - FMath::Clamp(TimeUntilHit / FMath::Max(0.01f, NoteTravelTimeSec), 0.0f, 1.0f);
		const float SpawnDistanceOnSpline = GetSpawnDistanceOnSpline();
		const float HitDistanceOnSpline = GetHitDistanceOnSpline();
		const float Distance = FMath::Lerp(SpawnDistanceOnSpline, HitDistanceOnSpline, Alpha);
		NoteActor->SetActorTransform(GetLaneTransformOnSpline(Distance, NoteActor->Lane, NoteHeightOffset));

		if (CachedSongTimeSec - NoteActor->NoteTimeSec > MissWindowSec)
		{
			NoteActor->SetHitRating(ERhythmHitRating::Miss);
			++ScoreStats.Miss;
			JudgedRows.Add(FName(*FString::Printf(TEXT("Missed_%d_%f"), NoteActor->Lane, NoteActor->NoteTimeSec)));
			RecalculateStats();
			NoteActor->Destroy();
			ActiveNoteActors.RemoveAtSwap(NoteIndex);
		}
	}
}

void ARhythmMinigameActor::UpdateTrackVisualPulse()
{
}

void ARhythmMinigameActor::UpdatePlayerFacingVisuals()
{
	APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PlayerController)
	{
		return;
	}

	FVector ViewLocation = FVector::ZeroVector;
	FRotator ViewRotation = FRotator::ZeroRotator;
	PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);

	if (bScoreTextFacesPlayer && ScoreTextComponent)
	{
		const FRotator LookAtRotation = (ViewLocation - ScoreTextComponent->GetComponentLocation()).Rotation();
		ScoreTextComponent->SetWorldRotation(LookAtRotation);
	}

	if (bHologramFacesPlayer && HologramRootComponent)
	{
		const FRotator LookAtRotation = (ViewLocation - HologramRootComponent->GetComponentLocation()).Rotation();
		HologramRootComponent->SetWorldRotation(LookAtRotation);
	}
}

FTransform ARhythmMinigameActor::GetLaneTransformOnSpline(float DistanceAlongSpline, int32 LaneIndex, float HeightOffset) const
{
	const FVector SplineLocation = NoteSpline
		? NoteSpline->GetLocationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World)
		: GetActorLocation();
	const FRotator SplineRotation = NoteSpline
		? NoteSpline->GetRotationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World)
		: GetActorRotation();
	const FRotationMatrix RotationMatrix(SplineRotation);
	const FVector RightVector = RotationMatrix.GetUnitAxis(EAxis::Y);
	const FVector UpVector = RotationMatrix.GetUnitAxis(EAxis::Z);
	const FVector LaneLocation = SplineLocation + RightVector * GetLaneOffset(LaneIndex) + UpVector * HeightOffset;

	return FTransform(SplineRotation, LaneLocation);
}

float ARhythmMinigameActor::GetLaneOffset(int32 LaneIndex) const
{
	const float Direction = bInvertLaneSides ? -1.0f : 1.0f;
	return (static_cast<float>(LaneIndex) - 1.0f) * LaneSpacing * Direction;
}

float ARhythmMinigameActor::GetSpawnDistanceOnSpline() const
{
	if (!NoteSpline)
	{
		return 0.0f;
	}

	if (bUseSplineEndsForNoteTravel)
	{
		return bSpawnNotesAtSplineEnd ? NoteSpline->GetSplineLength() : 0.0f;
	}

	return GetSplineDistanceForConfiguredLocalX(SpawnDistanceAlongSpline);
}

float ARhythmMinigameActor::GetHitDistanceOnSpline() const
{
	if (!NoteSpline)
	{
		return 0.0f;
	}

	if (bUseSplineEndsForNoteTravel)
	{
		return bSpawnNotesAtSplineEnd ? 0.0f : NoteSpline->GetSplineLength();
	}

	return GetSplineDistanceForConfiguredLocalX(HitDistanceAlongSpline);
}

float ARhythmMinigameActor::GetSplineDistanceForConfiguredLocalX(float LocalX) const
{
	if (!NoteSpline)
	{
		return 0.0f;
	}

	const FVector LocalEndpoint(LocalX, 0.0f, 0.0f);
	const FVector WorldEndpoint = NoteSpline->GetComponentTransform().TransformPosition(LocalEndpoint);
	const float InputKey = NoteSpline->FindInputKeyClosestToWorldLocation(WorldEndpoint);
	return NoteSpline->GetDistanceAlongSplineAtSplineInputKey(InputKey);
}

void ARhythmMinigameActor::FinishIfComplete()
{
	const bool bAllNotesSpawned = NextSpawnNoteIndex >= SortedNotes.Num();
	const bool bNoActiveNotes = ActiveNoteActors.Num() == 0;
	const bool bPastSongEnd = SongEndTimeSec > 0.0f && CachedSongTimeSec >= SongEndTimeSec;

	if ((bAllNotesSpawned && bNoActiveNotes) || bPastSongEnd)
	{
		FinishMinigame();
	}
}

void ARhythmMinigameActor::FinishMinigame()
{
	RecalculateStats();
	ScoreStats.bPassed = ScoreStats.Accuracy >= PassAccuracyThreshold;
	const FRhythmScoreStats FinishedStats = ScoreStats;
	const bool bWasSequenceActive = bSequenceActive;
	const int32 FinishedSequenceSongIndex = CurrentSequenceSongIndex;
	StopMinigame(false);

	if (bWasSequenceActive)
	{
		bSequenceActive = true;
		CurrentSequenceSongIndex = FinishedSequenceSongIndex;
		HandleSequenceSongFinished(FinishedStats);
	}
	else if (FinishedStats.bPassed)
	{
		HandleRhythmGatePassed();
	}
	else
	{
		OnRhythmGateFailed.Broadcast();
	}

	OnMinigameFinished.Broadcast(FinishedStats);
}

ERhythmHitRating ARhythmMinigameActor::ResolveActiveNote(int32 LaneIndex, float& OutTimingErrorSec)
{
	OutTimingErrorSec = 0.0f;
	ARhythmNoteActor* BestNoteActor = nullptr;
	float BestAbsError = TNumericLimits<float>::Max();

	for (ARhythmNoteActor* NoteActor : ActiveNoteActors)
	{
		if (!IsValid(NoteActor) || NoteActor->Lane != LaneIndex)
		{
			continue;
		}

		const float Error = CachedSongTimeSec - NoteActor->NoteTimeSec;
		const float AbsError = FMath::Abs(Error);
		if (AbsError < BestAbsError)
		{
			BestAbsError = AbsError;
			OutTimingErrorSec = Error;
			BestNoteActor = NoteActor;
		}
	}

	if (!BestNoteActor || BestAbsError > MissWindowSec)
	{
		return ERhythmHitRating::None;
	}

	ERhythmHitRating Rating = ERhythmHitRating::Miss;
	if (BestAbsError <= PerfectWindowSec)
	{
		Rating = ERhythmHitRating::Perfect;
	}
	else if (BestAbsError <= GoodWindowSec)
	{
		Rating = ERhythmHitRating::Good;
	}

	BestNoteActor->SetHitRating(Rating);
	ActiveNoteActors.Remove(BestNoteActor);
	BestNoteActor->Destroy();

	return Rating;
}

void ARhythmMinigameActor::PulseButton(int32 LaneIndex, ERhythmHitRating Rating)
{
	for (ARhythmButtonActor* ButtonActor : SpawnedButtons)
	{
		if (IsValid(ButtonActor) && ButtonActor->Lane == LaneIndex)
		{
			ButtonActor->PulseButton(Rating);
			return;
		}
	}
}

void ARhythmMinigameActor::RefreshScoreText()
{
	if (!ScoreTextComponent)
	{
		return;
	}

	ScoreTextComponent->SetVisibility(bShowScoreText);
	ScoreTextComponent->SetRelativeLocation(ScoreTextRelativeLocation);
	ScoreTextComponent->SetWorldSize(ScoreTextWorldSize);

	const int32 RequiredSongCount = GetRequiredSongCount();
	const bool bShowSequenceLine = RequiredSongCount > 1 || bSequenceActive || bSequenceGateUnlocked || PassedSequenceSongCount > 0;
	const int32 DisplaySongIndex = CurrentSequenceSongIndex != INDEX_NONE ? CurrentSequenceSongIndex + 1 : FMath::Min(PassedSequenceSongCount + 1, RequiredSongCount);
	const FString SequenceLine = bShowSequenceLine
		? FString::Printf(TEXT("Track %d/%d  Gate %d/%d\n"), DisplaySongIndex, RequiredSongCount, PassedSequenceSongCount, RequiredSongCount)
		: FString();

	ScoreTextComponent->SetText(FText::FromString(FString::Printf(
		TEXT("%sPerfect %d  Good %d  Miss %d  Accuracy %.0f%%"),
		*SequenceLine,
		ScoreStats.Perfect,
		ScoreStats.Good,
		ScoreStats.Miss,
		ScoreStats.Accuracy * 100.0f)));
}

void ARhythmMinigameActor::RefreshHologramScreen()
{
	if (HologramRootComponent)
	{
		HologramRootComponent->SetVisibility(bShowHologramScreen, true);
		HologramRootComponent->SetRelativeLocation(HologramRelativeLocation);
		HologramRootComponent->SetRelativeRotation(HologramRelativeRotation);
		HologramRootComponent->SetRelativeScale3D(HologramRelativeScale);
	}

	if (HologramTitleTextComponent)
	{
		const FText Title = bRunning ? HologramRunningTitle : (bSequenceGateUnlocked || ScoreStats.bPassed ? HologramPassedTitle : HologramIdleTitle);
		HologramTitleTextComponent->SetText(Title);
		HologramTitleTextComponent->SetWorldSize(HologramTitleWorldSize);
	}

	if (HologramStatusTextComponent)
	{
		const int32 RequiredSongCount = GetRequiredSongCount();
		if (!LastStartError.IsEmpty())
		{
			HologramStatusTextComponent->SetText(FText::FromString(TEXT("START ERROR")));
		}
		else if (RequiredSongCount > 1 || bSequenceActive || bSequenceGateUnlocked || PassedSequenceSongCount > 0)
		{
			const int32 DisplaySongIndex = CurrentSequenceSongIndex != INDEX_NONE ? CurrentSequenceSongIndex + 1 : FMath::Min(PassedSequenceSongCount + 1, RequiredSongCount);
			HologramStatusTextComponent->SetText(FText::FromString(FString::Printf(
				TEXT("TRACK %d/%d  ACCESS %d/%d  %.0f%%"),
				DisplaySongIndex,
				RequiredSongCount,
				PassedSequenceSongCount,
				RequiredSongCount,
				ScoreStats.Accuracy * 100.0f)));
		}
		else
		{
			HologramStatusTextComponent->SetText(FText::FromString(FString::Printf(TEXT("%.0f%%"), ScoreStats.Accuracy * 100.0f)));
		}
		HologramStatusTextComponent->SetWorldSize(HologramStatusWorldSize);
	}

	if (HologramStoryTextComponent)
	{
		HologramStoryTextComponent->SetText(LastStartError.IsEmpty() ? HologramStoryText : FText::FromString(LastStartError));
		HologramStoryTextComponent->SetWorldSize(HologramStoryWorldSize);
	}
}

void ARhythmMinigameActor::RecalculateStats()
{
	const float Earned = static_cast<float>(ScoreStats.Perfect) + static_cast<float>(ScoreStats.Good) * 0.5f;
	ScoreStats.Accuracy = ScoreStats.TotalNotes > 0 ? Earned / static_cast<float>(ScoreStats.TotalNotes) : 0.0f;
	ScoreStats.bPassed = ScoreStats.Accuracy >= PassAccuracyThreshold;
}

FLinearColor ARhythmMinigameActor::GetLaneColor(int32 LaneIndex) const
{
	if (LaneColors.IsValidIndex(LaneIndex))
	{
		return LaneColors[LaneIndex];
	}

	return FLinearColor::White;
}
