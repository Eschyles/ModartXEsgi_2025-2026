#include "RhythmTransitionStateSubsystem.h"

void URhythmTransitionStateSubsystem::RequestStatusSequencersOnNextLevel()
{
	bPendingStatusSequencerRequest = true;
	UE_LOG(LogTemp, Log, TEXT("Rhythm transition state: requested status sequencer start on next level."));
}

bool URhythmTransitionStateSubsystem::HasPendingStatusSequencerRequest() const
{
	return bPendingStatusSequencerRequest;
}

bool URhythmTransitionStateSubsystem::ConsumeStatusSequencerRequest()
{
	if (!bPendingStatusSequencerRequest)
	{
		UE_LOG(LogTemp, Log, TEXT("Rhythm transition state: no pending status sequencer request to consume."));
		return false;
	}

	bPendingStatusSequencerRequest = false;
	UE_LOG(LogTemp, Log, TEXT("Rhythm transition state: consumed status sequencer request."));
	return true;
}
