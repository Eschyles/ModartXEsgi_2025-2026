#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RhythmTransitionStateSubsystem.generated.h"

UCLASS()
class UPROJECT_API URhythmTransitionStateSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Rhythm|Transition")
	void RequestStatusSequencersOnNextLevel();

	UFUNCTION(BlueprintCallable, Category = "Rhythm|Transition")
	bool HasPendingStatusSequencerRequest() const;

	UFUNCTION(BlueprintCallable, Category = "Rhythm|Transition")
	bool ConsumeStatusSequencerRequest();

private:
	UPROPERTY(Transient)
	bool bPendingStatusSequencerRequest = false;
};
