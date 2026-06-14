#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RhythmTypes.h"
#include "RhythmSongData.generated.h"

class UDataTable;
class USoundBase;

UCLASS(BlueprintType)
class UPROJECT_API URhythmSongData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Song")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Song")
	TObjectPtr<USoundBase> SongAudio = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Song", meta = (RowType = "/Script/Uproject.RhythmNoteRow"))
	TObjectPtr<UDataTable> NoteTable = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Song")
	float AudioStartOffsetSec = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timing")
	float CountdownLeadInSec = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timing")
	float NoteTravelTimeSec = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timing")
	float PerfectWindowSec = 0.075f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timing")
	float GoodWindowSec = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timing")
	float MissWindowSec = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timing")
	float SongEndTimeSec = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scoring", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PassAccuracyThreshold = 0.7f;

	UFUNCTION(BlueprintCallable, Category = "Rhythm")
	bool GetSortedNotes(TArray<FRhythmResolvedNote>& OutNotes, FString& OutError) const;
};
