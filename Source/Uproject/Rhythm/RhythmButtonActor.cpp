#include "RhythmButtonActor.h"
#include "RhythmMinigameActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

ARhythmButtonActor::ARhythmButtonActor()
{
	PrimaryActorTick.bCanEverTick = false;
	SetActorEnableCollision(true);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	ButtonMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ButtonMeshComponent"));
	ButtonMeshComponent->SetupAttachment(SceneRoot);
	// Collision de requete uniquement : le bouton peut etre vise par un line trace (canal Visibility).
	ButtonMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ButtonMeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	ButtonMeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	ButtonMeshComponent->SetGenerateOverlapEvents(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylinderMesh.Succeeded())
	{
		ButtonMeshComponent->SetStaticMesh(CylinderMesh.Object);
		ButtonMeshComponent->SetRelativeScale3D(FVector(0.7f, 0.7f, 0.12f));
	}
}

void ARhythmButtonActor::BeginPlay()
{
	Super::BeginPlay();

	if (ButtonMeshComponent)
	{
		InitialMeshLocation = ButtonMeshComponent->GetRelativeLocation();
		InitialMeshScale = ButtonMeshComponent->GetRelativeScale3D();
	}
}

void ARhythmButtonActor::ConfigureButton(int32 InLane, FLinearColor InColor)
{
	Lane = InLane;
	SetActorEnableCollision(true);
	if (ButtonMeshComponent)
	{
		ButtonMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		ButtonMeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
		ButtonMeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		ButtonMeshComponent->SetGenerateOverlapEvents(false);
	}
	SetVisualColor(InColor);
}

void ARhythmButtonActor::Interact()
{
	// La lane est fixee au spawn (ConfigureButton) a partir de la meme valeur que celle utilisee pour
	// placer le bouton : la note jouee correspond donc toujours a la position du bouton.
	if (OwnerMinigame)
	{
		OwnerMinigame->HandleLaneInput(Lane);
	}
}

void ARhythmButtonActor::PulseButton(ERhythmHitRating Rating)
{
	LastRating = Rating;

	if (ButtonMeshComponent)
	{
		ButtonMeshComponent->SetRelativeLocation(InitialMeshLocation - FVector(0.0f, 0.0f, PressDepth));
		ButtonMeshComponent->SetRelativeScale3D(InitialMeshScale * (1.0f + PressScaleBoost));
	}

	switch (Rating)
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

	GetWorldTimerManager().ClearTimer(ResetPulseTimerHandle);
	GetWorldTimerManager().SetTimer(ResetPulseTimerHandle, this, &ARhythmButtonActor::ResetPulse, PressDurationSec, false);
}

void ARhythmButtonActor::SetVisualColor(FLinearColor InColor)
{
	if (!ButtonMeshComponent)
	{
		return;
	}

	UMaterialInstanceDynamic* DynamicMaterial = ButtonMeshComponent->CreateAndSetMaterialInstanceDynamic(0);
	if (!DynamicMaterial)
	{
		return;
	}

	DynamicMaterial->SetVectorParameterValue(TEXT("Color"), InColor);
	DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), InColor);
	DynamicMaterial->SetVectorParameterValue(TEXT("EmissiveColor"), InColor);
	DynamicMaterial->SetScalarParameterValue(TEXT("EmissiveStrength"), EmissiveStrength);
	DynamicMaterial->SetScalarParameterValue(TEXT("Pulse"), 1.0f);
}

void ARhythmButtonActor::ResetPulse()
{
	if (ButtonMeshComponent)
	{
		ButtonMeshComponent->SetRelativeLocation(InitialMeshLocation);
		ButtonMeshComponent->SetRelativeScale3D(InitialMeshScale);
	}
}
