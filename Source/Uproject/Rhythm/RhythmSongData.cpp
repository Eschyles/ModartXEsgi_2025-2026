#include "RhythmSongData.h"

#include "Engine/DataTable.h"
#include "Sound/SoundBase.h"

bool URhythmSongData::GetSortedNotes(TArray<FRhythmResolvedNote>& OutNotes, FString& OutError) const
{
	OutNotes.Reset();
	OutError.Reset();

	if (!NoteTable)
	{
		OutError = TEXT("Rhythm song has no note table.");
		return false;
	}

	const UScriptStruct* RowStruct = NoteTable->GetRowStruct();
	if (!RowStruct || !RowStruct->IsChildOf(FRhythmNoteRow::StaticStruct()))
	{
		OutError = TEXT("Rhythm note table must use FRhythmNoteRow.");
		return false;
	}

	for (const TPair<FName, uint8*>& RowPair : NoteTable->GetRowMap())
	{
		const FRhythmNoteRow* Row = reinterpret_cast<const FRhythmNoteRow*>(RowPair.Value);
		if (!Row)
		{
			continue;
		}

		FRhythmResolvedNote ResolvedNote;
		ResolvedNote.RowName = RowPair.Key;
		ResolvedNote.Note = *Row;
		OutNotes.Add(ResolvedNote);
	}

	OutNotes.Sort([](const FRhythmResolvedNote& Left, const FRhythmResolvedNote& Right)
	{
		if (!FMath::IsNearlyEqual(Left.Note.TimeSec, Right.Note.TimeSec))
		{
			return Left.Note.TimeSec < Right.Note.TimeSec;
		}

		return Left.Note.Lane < Right.Note.Lane;
	});

	return true;
}
