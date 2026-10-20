#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RhythmTypes.h"
#include "RhythmButtonActor.generated.h"

class UStaticMeshComponent;
class ARhythmMinigameActor;

UCLASS(Blueprintable)
class UPROJECT_API ARhythmButtonActor : public AActor
{
	GENERATED_BODY()

public:
	ARhythmButtonActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ButtonMeshComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
	int32 Lane = 0;

	// Minigame qui a spawn ce bouton (renseigne par ARhythmMinigameActor::SpawnButtonActors).
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Rhythm")
	TObjectPtr<ARhythmMinigameActor> OwnerMinigame = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
	ERhythmHitRating LastRating = ERhythmHitRating::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	float PressDepth = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	float PressDurationSec = 0.12f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	float PressScaleBoost = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	float ShaderPulseDurationSec = 0.24f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	float EmissiveStrength = 8.0f;

	UFUNCTION(BlueprintCallable, Category = "Rhythm")
	void ConfigureButton(int32 InLane, FLinearColor InColor);

	// A appeler quand le joueur vise ce bouton et declenche l'interaction (VR).
	// Declenche la lane de ce bouton dans le minigame.
	UFUNCTION(BlueprintCallable, Category = "Rhythm")
	void Interact();

	UFUNCTION(BlueprintPure, Category = "Rhythm")
	int32 GetLane() const { return Lane; }

	UFUNCTION(BlueprintCallable, Category = "Rhythm")
	void PulseButton(ERhythmHitRating Rating);

	UFUNCTION(BlueprintCallable, Category = "Visual")
	void SetVisualColor(FLinearColor InColor);

protected:
	virtual void BeginPlay() override;

private:
	FTimerHandle ResetPulseTimerHandle;
	FVector InitialMeshLocation = FVector::ZeroVector;
	FVector InitialMeshScale = FVector::OneVector;

	void ResetPulse();
};
