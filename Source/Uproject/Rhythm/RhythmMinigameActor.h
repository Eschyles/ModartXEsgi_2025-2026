#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RhythmTypes.h"
#include "RhythmMinigameActor.generated.h"

class ARhythmButtonActor;
class ARhythmNoteActor;
class UAudioComponent;
class UInputAction;
class UInputMappingContext;
class UMaterialInterface;
class UStaticMesh;
class UStaticMeshComponent;
class USplineComponent;
class UTextRenderComponent;
class URhythmSongData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRhythmGameEventSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRhythmGameFinishedSignature, FRhythmScoreStats, Stats);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FRhythmNoteJudgedSignature, int32, LaneIndex, ERhythmHitRating, Rating, float, TimingErrorSec, FRhythmScoreStats, Stats);

UCLASS(Blueprintable)
class UPROJECT_API ARhythmMinigameActor : public AActor
{
	GENERATED_BODY()

public:
	ARhythmMinigameActor();

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintAssignable, Category = "Rhythm")
	FRhythmGameEventSignature OnMinigameStarted;

	UPROPERTY(BlueprintAssignable, Category = "Rhythm")
	FRhythmGameEventSignature OnMinigameStopped;

	UPROPERTY(BlueprintAssignable, Category = "Rhythm")
	FRhythmGameFinishedSignature OnMinigameFinished;

	UPROPERTY(BlueprintAssignable, Category = "Rhythm")
	FRhythmNoteJudgedSignature OnNoteJudged;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USplineComponent> NoteSpline = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAudioComponent> AudioComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UTextRenderComponent> ScoreTextComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> HologramRootComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> HologramPanelComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UTextRenderComponent> HologramTitleTextComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UTextRenderComponent> HologramStatusTextComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UTextRenderComponent> HologramStoryTextComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Song")
	TObjectPtr<URhythmSongData> SongData = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Song")
	TObjectPtr<URhythmSongData> ActiveSongData = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Song")
	TObjectPtr<URhythmSongData> OverrideSongData = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Actors")
	TSubclassOf<ARhythmNoteActor> NoteActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Actors")
	TSubclassOf<ARhythmButtonActor> ButtonActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<UStaticMesh> NoteMeshOverride = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<UStaticMesh> ButtonMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<UStaticMesh> TrackSegmentMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<UMaterialInterface> TrackSegmentMaterial = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
	TArray<TObjectPtr<UStaticMeshComponent>> TrackVisualSegments;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<UMaterialInterface> HologramMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> RhythmInputMappingContext = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> Lane0Action = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> Lane1Action = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> Lane2Action = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rhythm")
	TArray<TObjectPtr<ARhythmButtonActor>> SpawnedButtons;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout", meta = (ClampMin = "1"))
	int32 LaneCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	float LaneSpacing = 160.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	float SpawnDistanceAlongSpline = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	float HitDistanceAlongSpline = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	float NoteHeightOffset = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	float TrackHeightOffset = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	float TrackSegmentWidth = 140.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	float TrackSegmentHeight = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	float TrackEmissiveStrength = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	float TrackPulseEmissiveBoost = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	bool bBuildNeonTrack = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Runtime")
	bool bSpawnButtonsOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Runtime")
	bool bSpawnOnBeginPlay = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Runtime")
	bool bDestroySpawnedActorsOnStop = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	FLinearColor PerfectColor = FLinearColor(0.0f, 1.0f, 0.8f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	FLinearColor GoodColor = FLinearColor(0.2f, 0.5f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	FLinearColor MissColor = FLinearColor(1.0f, 0.05f, 0.1f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	TArray<FLinearColor> LaneColors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score")
	bool bShowScoreText = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score")
	bool bScoreTextFacesPlayer = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score")
	FVector ScoreTextRelativeLocation = FVector(0.0f, 0.0f, 260.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score")
	float ScoreTextWorldSize = 28.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hologram")
	bool bShowHologramScreen = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hologram")
	bool bHologramFacesPlayer = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hologram")
	FVector HologramRelativeLocation = FVector(0.0f, -260.0f, 260.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hologram")
	FRotator HologramRelativeRotation = FRotator(0.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hologram")
	FVector HologramRelativeScale = FVector(1.0f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hologram")
	float HologramTitleWorldSize = 22.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hologram")
	float HologramStatusWorldSize = 16.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hologram")
	float HologramStoryWorldSize = 13.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hologram")
	FText HologramIdleTitle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hologram")
	FText HologramRunningTitle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hologram")
	FText HologramPassedTitle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hologram")
	FText HologramFailedTitle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hologram")
	FText HologramStoryText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hologram")
	FLinearColor HologramIdleColor = FLinearColor(0.0f, 0.4f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hologram")
	FLinearColor HologramRunningColor = FLinearColor(0.0f, 1.0f, 0.8f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hologram")
	float HologramPulseDurationSec = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float PerfectWindowSec = 0.075f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float GoodWindowSec = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float MissWindowSec = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float NoteTravelTimeSec = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float CountdownLeadInSec = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float SongEndTimeSec = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PassAccuracyThreshold = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	int32 RhythmInputPriority = 0;

	UFUNCTION(BlueprintCallable, Category = "Rhythm")
	void StartMinigame(URhythmSongData* InOverrideSongData);

	UFUNCTION(BlueprintCallable, Category = "Rhythm")
	void StopMinigame(bool bResetScore);

	UFUNCTION(BlueprintPure, Category = "Rhythm")
	bool IsRunning() const;

	UFUNCTION(BlueprintCallable, Category = "Rhythm")
	void HandleLaneInput(int32 LaneIndex);

	UFUNCTION(BlueprintPure, Category = "Rhythm")
	float GetSongTimeSec() const;

	UFUNCTION(BlueprintPure, Category = "Rhythm")
	FRhythmScoreStats GetScoreStats() const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(Transient)
	TArray<TObjectPtr<ARhythmNoteActor>> ActiveNoteActors;

	TArray<FRhythmResolvedNote> SortedNotes;
	TSet<FName> JudgedRows;
	FRhythmScoreStats ScoreStats;
	float StartWorldTimeSec = 0.0f;
	float CachedSongTimeSec = 0.0f;
	int32 NextSpawnNoteIndex = 0;
	bool bRunning = false;

	void SetupInput();
	void TeardownInput();
	void HandleLane0Action();
	void HandleLane1Action();
	void HandleLane2Action();
	void SpawnButtonActors();
	void DestroyRuntimeActors();
	void SpawnDueNotes();
	void UpdateActiveNotes();
	void UpdateTrackVisualPulse();
	void FinishIfComplete();
	void FinishMinigame();
	ERhythmHitRating ResolveActiveNote(int32 LaneIndex, float& OutTimingErrorSec);
	void PulseButton(int32 LaneIndex, ERhythmHitRating Rating);
	void RefreshScoreText();
	void RefreshHologramScreen();
	void RecalculateStats();
	FLinearColor GetLaneColor(int32 LaneIndex) const;
};
