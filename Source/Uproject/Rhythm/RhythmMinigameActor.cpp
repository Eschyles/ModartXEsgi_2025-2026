#include "RhythmMinigameActor.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "RhythmButtonActor.h"
#include "RhythmNoteActor.h"
#include "RhythmSongData.h"
#include "Components/AudioComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SplineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/LocalPlayer.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/PlayerController.h"
#include "Materials/MaterialInterface.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"

ARhythmMinigameActor::ARhythmMinigameActor()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	NoteSpline = CreateDefaultSubobject<USplineComponent>(TEXT("NoteSpline"));
	NoteSpline->SetupAttachment(SceneRoot);
	NoteSpline->ClearSplinePoints(false);
	NoteSpline->AddSplinePoint(FVector(SpawnDistanceAlongSpline, 0.0f, 0.0f), ESplineCoordinateSpace::Local, false);
	NoteSpline->AddSplinePoint(FVector(HitDistanceAlongSpline, 0.0f, 0.0f), ESplineCoordinateSpace::Local, true);

	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	AudioComponent->SetupAttachment(SceneRoot);
	AudioComponent->bAutoActivate = false;

	ScoreTextComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("ScoreTextComponent"));
	ScoreTextComponent->SetupAttachment(SceneRoot);
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
	HologramTitleTextComponent->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
	HologramTitleTextComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 70.0f));

	HologramStatusTextComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("HologramStatusTextComponent"));
	HologramStatusTextComponent->SetupAttachment(HologramRootComponent);
	HologramStatusTextComponent->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);

	HologramStoryTextComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("HologramStoryTextComponent"));
	HologramStoryTextComponent->SetupAttachment(HologramRootComponent);
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

	HologramIdleTitle = FText::FromString(TEXT("RHYTHM LINK"));
	HologramRunningTitle = FText::FromString(TEXT("SYNC ACTIVE"));
	HologramPassedTitle = FText::FromString(TEXT("SYNC PASSED"));
	HologramFailedTitle = FText::FromString(TEXT("SYNC FAILED"));
	HologramStoryText = FText::FromString(TEXT("Follow the lanes and hit the notes on the beat."));
}

void ARhythmMinigameActor::BeginPlay()
{
	Super::BeginPlay();

	if (bSpawnButtonsOnBeginPlay)
	{
		SpawnButtonActors();
	}

	RefreshScoreText();
	RefreshHologramScreen();

	if (bSpawnOnBeginPlay)
	{
		StartMinigame(OverrideSongData);
	}
}

void ARhythmMinigameActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	TeardownInput();
	DestroyRuntimeActors();
	Super::EndPlay(EndPlayReason);
}

void ARhythmMinigameActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bRunning)
	{
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
	URhythmSongData* SelectedSongData = InOverrideSongData;
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
		return;
	}

	FString Error;
	if (!ActiveSongData->GetSortedNotes(SortedNotes, Error))
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not start rhythm minigame: %s"), *Error);
		return;
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

	for (int32 LaneIndex = 0; LaneIndex < LaneCount; ++LaneIndex)
	{
		const FVector LocalOffset(0.0f, (static_cast<float>(LaneIndex) - 1.0f) * LaneSpacing, 0.0f);
		const FTransform SpawnTransform(GetActorRotation(), GetActorLocation() + GetActorRotation().RotateVector(LocalOffset));
		ARhythmButtonActor* ButtonActor = GetWorld()->SpawnActor<ARhythmButtonActor>(ButtonActorClass, SpawnTransform);
		if (!ButtonActor)
		{
			continue;
		}

		ButtonActor->ConfigureButton(LaneIndex, GetLaneColor(LaneIndex));
		if (ButtonMesh && ButtonActor->ButtonMeshComponent)
		{
			ButtonActor->ButtonMeshComponent->SetStaticMesh(ButtonMesh);
		}

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

	while (SortedNotes.IsValidIndex(NextSpawnNoteIndex))
	{
		const FRhythmResolvedNote& ResolvedNote = SortedNotes[NextSpawnNoteIndex];
		if (ResolvedNote.Note.TimeSec - CachedSongTimeSec > NoteTravelTimeSec)
		{
			break;
		}

		ARhythmNoteActor* NoteActor = GetWorld()->SpawnActor<ARhythmNoteActor>(NoteActorClass, GetActorTransform());
		if (NoteActor)
		{
			NoteActor->ConfigureNote(ResolvedNote.Note.Lane, ResolvedNote.Note.TimeSec, ResolvedNote.Note.DurationSec);
			if (NoteMeshOverride && NoteActor->NoteMeshComponent)
			{
				NoteActor->NoteMeshComponent->SetStaticMesh(NoteMeshOverride);
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
		const float Distance = FMath::Lerp(SpawnDistanceAlongSpline, HitDistanceAlongSpline, Alpha);
		NoteActor->UpdateTransformOnSpline(NoteSpline, Distance, NoteHeightOffset, LaneSpacing);

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
	StopMinigame(false);
	OnMinigameFinished.Broadcast(ScoreStats);
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
	ScoreTextComponent->SetText(FText::FromString(FString::Printf(
		TEXT("Perfect %d  Good %d  Miss %d  Accuracy %.0f%%"),
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
		const FText Title = bRunning ? HologramRunningTitle : (ScoreStats.bPassed ? HologramPassedTitle : HologramIdleTitle);
		HologramTitleTextComponent->SetText(Title);
		HologramTitleTextComponent->SetWorldSize(HologramTitleWorldSize);
	}

	if (HologramStatusTextComponent)
	{
		HologramStatusTextComponent->SetText(FText::FromString(FString::Printf(TEXT("%.0f%%"), ScoreStats.Accuracy * 100.0f)));
		HologramStatusTextComponent->SetWorldSize(HologramStatusWorldSize);
	}

	if (HologramStoryTextComponent)
	{
		HologramStoryTextComponent->SetText(HologramStoryText);
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
