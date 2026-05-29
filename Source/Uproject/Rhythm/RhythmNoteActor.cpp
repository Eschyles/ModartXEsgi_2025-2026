#include "RhythmNoteActor.h"

#include "Components/SceneComponent.h"
#include "Components/SplineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

ARhythmNoteActor::ARhythmNoteActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	NoteMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NoteMeshComponent"));
	NoteMeshComponent->SetupAttachment(SceneRoot);
	NoteMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		NoteMeshComponent->SetStaticMesh(CubeMesh.Object);
		NoteMeshComponent->SetRelativeScale3D(FVector(0.35f, 0.35f, 0.12f));
	}
}

void ARhythmNoteActor::BeginPlay()
{
	Super::BeginPlay();

	if (NoteMeshComponent)
	{
		BaseMaterial = NoteMeshComponent->GetMaterial(0);
	}
}

void ARhythmNoteActor::ConfigureNote(int32 InLane, float InNoteTimeSec, float InDurationSec)
{
	Lane = InLane;
	NoteTimeSec = InNoteTimeSec;
	DurationSec = InDurationSec;
	HitRating = ERhythmHitRating::None;
}

void ARhythmNoteActor::UpdateTransformOnSpline(USplineComponent* Spline, float DistanceAlongSpline, float HeightOffset, float LaneSpacing)
{
	if (!Spline)
	{
		return;
	}

	const float LaneOffset = (static_cast<float>(Lane) - 1.0f) * LaneSpacing;
	const FVector SplineLocation = Spline->GetLocationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World);
	const FRotator SplineRotation = Spline->GetRotationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World);
	const FVector RightVector = FRotationMatrix(SplineRotation).GetUnitAxis(EAxis::Y);
	const FVector UpVector = FRotationMatrix(SplineRotation).GetUnitAxis(EAxis::Z);

	SetActorLocation(SplineLocation + RightVector * LaneOffset + UpVector * HeightOffset);
	SetActorRotation(SplineRotation);
}

void ARhythmNoteActor::SetVisualColor(FLinearColor InColor)
{
	if (!NoteMeshComponent)
	{
		return;
	}

	UMaterialInstanceDynamic* DynamicMaterial = NoteMeshComponent->CreateAndSetMaterialInstanceDynamic(0);
	if (!DynamicMaterial)
	{
		return;
	}

	DynamicMaterial->SetVectorParameterValue(TEXT("Color"), InColor);
	DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), InColor);
	DynamicMaterial->SetVectorParameterValue(TEXT("EmissiveColor"), InColor);
	DynamicMaterial->SetScalarParameterValue(TEXT("EmissiveStrength"), EmissiveStrength);
}

void ARhythmNoteActor::SetHitRating(ERhythmHitRating InHitRating)
{
	HitRating = InHitRating;

	switch (HitRating)
	{
	case ERhythmHitRating::Perfect:
		SetVisualColor(FLinearColor(0.0f, 1.0f, 0.8f));
		break;
	case ERhythmHitRating::Good:
		SetVisualColor(FLinearColor(0.2f, 0.5f, 1.0f));
		break;
	case ERhythmHitRating::Miss:
		SetVisualColor(FLinearColor(1.0f, 0.05f, 0.1f));
		break;
	default:
		break;
	}
}
