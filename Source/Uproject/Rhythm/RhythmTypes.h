#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "RhythmTypes.generated.h"

UENUM(BlueprintType)
enum class ERhythmHitRating : uint8
{
	None UMETA(DisplayName = "None"),
	Perfect UMETA(DisplayName = "Perfect"),
	Good UMETA(DisplayName = "Good"),
	Miss UMETA(DisplayName = "Miss")
};

USTRUCT(BlueprintType)
struct UPROJECT_API FRhythmNoteRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
	float TimeSec = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
	int32 Lane = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
	float DurationSec = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
	int32 Velocity = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
	float Pitch = 1.0f;
};

USTRUCT(BlueprintType)
struct UPROJECT_API FRhythmResolvedNote
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
	FName RowName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
	FRhythmNoteRow Note;
};

USTRUCT(BlueprintType)
struct UPROJECT_API FRhythmScoreStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
	int32 Perfect = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
	int32 Good = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
	int32 Miss = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
	int32 BadPress = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
	int32 TotalNotes = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
	float Accuracy = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
	bool bPassed = false;
};
