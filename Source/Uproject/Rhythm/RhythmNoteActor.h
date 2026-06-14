#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RhythmTypes.h"
#include "RhythmNoteActor.generated.h"

class UMaterialInterface;
class UStaticMeshComponent;
class USplineComponent;

UCLASS(Blueprintable)
class UPROJECT_API ARhythmNoteActor : public AActor
{
	GENERATED_BODY()

public:
	ARhythmNoteActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> NoteMeshComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
	int32 Lane = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
	float NoteTimeSec = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
	float DurationSec = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
	ERhythmHitRating HitRating = ERhythmHitRating::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	float EmissiveStrength = 6.0f;

	UFUNCTION(BlueprintCallable, Category = "Rhythm")
	void ConfigureNote(int32 InLane, float InNoteTimeSec, float InDurationSec);

	UFUNCTION(BlueprintCallable, Category = "Rhythm")
	void UpdateTransformOnSpline(USplineComponent* Spline, float DistanceAlongSpline, float HeightOffset, float LaneSpacing);

	UFUNCTION(BlueprintCallable, Category = "Visual")
	void SetVisualColor(FLinearColor InColor);

	UFUNCTION(BlueprintCallable, Category = "Rhythm")
	void SetHitRating(ERhythmHitRating InHitRating);

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInterface> BaseMaterial = nullptr;
};
