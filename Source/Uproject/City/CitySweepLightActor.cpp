#include "CitySweepLightActor.h"

#include "Components/SceneComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

ACitySweepLightActor::ACitySweepLightActor()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	SweepYawPivot = CreateDefaultSubobject<USceneComponent>(TEXT("SweepYawPivot"));
	SweepYawPivot->SetupAttachment(SceneRoot);

	BeamDirectionPivot = CreateDefaultSubobject<USceneComponent>(TEXT("BeamDirectionPivot"));
	BeamDirectionPivot->SetupAttachment(SweepYawPivot);

	SearchLightComponent = CreateDefaultSubobject<USpotLightComponent>(TEXT("SearchLight"));
	SearchLightComponent->SetupAttachment(BeamDirectionPivot);
	SearchLightComponent->SetMobility(EComponentMobility::Movable);

	BeamMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisibleBeam"));
	BeamMeshComponent->SetupAttachment(BeamDirectionPivot);
	BeamMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BeamMeshComponent->SetCastShadow(false);
	BeamMeshComponent->SetMobility(EComponentMobility::Movable);
	BeamMeshComponent->bSelectable = false;

	LensMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LensGlow"));
	LensMeshComponent->SetupAttachment(BeamDirectionPivot);
	LensMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LensMeshComponent->SetCastShadow(false);
	LensMeshComponent->SetMobility(EComponentMobility::Movable);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultConeMesh(TEXT("/Engine/BasicShapes/Cone.Cone"));
	if (DefaultConeMesh.Succeeded())
	{
		BeamMesh = DefaultConeMesh.Object;
		BeamMeshComponent->SetStaticMesh(BeamMesh);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultSphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (DefaultSphereMesh.Succeeded())
	{
		LensMesh = DefaultSphereMesh.Object;
		LensMeshComponent->SetStaticMesh(LensMesh);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultEmissiveMaterial(TEXT("/Engine/EngineMaterials/EmissiveMeshMaterial.EmissiveMeshMaterial"));
	if (DefaultEmissiveMaterial.Succeeded())
	{
		BeamMaterial = DefaultEmissiveMaterial.Object;
		LensMaterial = DefaultEmissiveMaterial.Object;
	}

	ResetSweep();
}

void ACitySweepLightActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ApplyAllSettings();
}

void ACitySweepLightActor::BeginPlay()
{
	Super::BeginPlay();
	ResetSweep();
	ApplyAllSettings();
}

void ACitySweepLightActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const UWorld* World = GetWorld();
	const bool bIsEditorPreview = World && World->WorldType == EWorldType::Editor;
	if (!bSweepEnabled || (bIsEditorPreview && !bAnimateInEditor))
	{
		return;
	}

	AdvanceSweep(DeltaSeconds);
}

bool ACitySweepLightActor::ShouldTickIfViewportsOnly() const
{
	return bAnimateInEditor;
}

void ACitySweepLightActor::SetSweepEnabled(bool bNewEnabled)
{
	bSweepEnabled = bNewEnabled;
}

void ACitySweepLightActor::SetLightEnabled(bool bNewEnabled)
{
	bLightEnabled = bNewEnabled;
	ApplyLightSettings();
}

void ACitySweepLightActor::ResetSweep()
{
	CurrentYawDeg = bPingPongSweep ? FMath::Clamp(SweepStartYawDeg, SweepYawMinDeg, SweepYawMaxDeg) : SweepStartYawDeg;
	PingPongDirection = SweepSpeedDegPerSec >= 0.0f ? 1.0f : -1.0f;
	ApplySweepTransform();
}

void ACitySweepLightActor::ApplyAllSettings()
{
	ApplySweepTransform();
	ApplyLightSettings();
	ApplyBeamVisualSettings();
	ApplyLensVisualSettings();
}

void ACitySweepLightActor::ApplySweepTransform()
{
	if (SweepYawPivot)
	{
		SweepYawPivot->SetRelativeRotation(FRotator(0.0f, CurrentYawDeg, 0.0f));
	}

	if (BeamDirectionPivot)
	{
		BeamDirectionPivot->SetRelativeRotation(FRotator(BeamPitchDeg, 0.0f, BeamRollDeg));
	}
}

void ACitySweepLightActor::ApplyLightSettings()
{
	if (!SearchLightComponent)
	{
		return;
	}

	SearchLightComponent->SetVisibility(bLightEnabled);
	SearchLightComponent->SetHiddenInGame(!bLightEnabled);
	SearchLightComponent->SetLightColor(LightColor);
	SearchLightComponent->SetIntensity(LightIntensity);
	SearchLightComponent->SetAttenuationRadius(AttenuationRadius);
	SearchLightComponent->SetInnerConeAngle(FMath::Min(InnerConeAngle, OuterConeAngle));
	SearchLightComponent->SetOuterConeAngle(FMath::Max(InnerConeAngle + 0.1f, OuterConeAngle));
	SearchLightComponent->SetSourceRadius(SourceRadius);
	SearchLightComponent->SetSoftSourceRadius(SoftSourceRadius);
	SearchLightComponent->SetVolumetricScatteringIntensity(VolumetricScatteringIntensity);
	SearchLightComponent->SetAffectTranslucentLighting(bAffectTranslucentLighting);
	SearchLightComponent->SetCastShadows(bCastLightShadows);
}

void ACitySweepLightActor::ApplyBeamVisualSettings()
{
	if (!BeamMeshComponent)
	{
		return;
	}

	BeamMeshComponent->SetVisibility(bShowBeamMesh);
	BeamMeshComponent->SetHiddenInGame(!bShowBeamMesh);

	if (BeamMesh)
	{
		BeamMeshComponent->SetStaticMesh(BeamMesh);
	}
	if (BeamMaterial)
	{
		BeamMeshComponent->SetMaterial(0, BeamMaterial);
	}

	BeamMeshComponent->SetRelativeLocation(FVector(BeamStartOffset + BeamLength * 0.5f, 0.0f, 0.0f));
	BeamMeshComponent->SetRelativeRotation(BeamMeshRotationOffset);
	BeamMeshComponent->SetRelativeScale3D(FVector(
		FMath::Max(1.0f, BeamRadius) / 100.0f,
		FMath::Max(1.0f, BeamRadius) / 100.0f,
		FMath::Max(1.0f, BeamLength) / 100.0f));

	ApplyColorMaterialParameters(BeamMeshComponent, LightColor, BeamOpacity, BeamEmissiveStrength);
}

void ACitySweepLightActor::ApplyLensVisualSettings()
{
	if (!LensMeshComponent)
	{
		return;
	}

	LensMeshComponent->SetVisibility(bShowLensMesh);
	LensMeshComponent->SetHiddenInGame(!bShowLensMesh);

	if (LensMesh)
	{
		LensMeshComponent->SetStaticMesh(LensMesh);
	}
	if (LensMaterial)
	{
		LensMeshComponent->SetMaterial(0, LensMaterial);
	}

	LensMeshComponent->SetRelativeLocation(FVector::ZeroVector);
	LensMeshComponent->SetRelativeRotation(FRotator::ZeroRotator);
	LensMeshComponent->SetRelativeScale3D(FVector(FMath::Max(0.0f, LensRadius) / 50.0f));
	ApplyColorMaterialParameters(LensMeshComponent, LightColor, 1.0f, LensEmissiveStrength);
}

void ACitySweepLightActor::ApplyColorMaterialParameters(UStaticMeshComponent* MeshComponent, FLinearColor Color, float Opacity, float EmissiveStrength) const
{
	if (!MeshComponent)
	{
		return;
	}

	const int32 MaterialCount = FMath::Max(1, MeshComponent->GetNumMaterials());
	for (int32 Index = 0; Index < MaterialCount; ++Index)
	{
		UMaterialInstanceDynamic* DynamicMaterial = Cast<UMaterialInstanceDynamic>(MeshComponent->GetMaterial(Index));
		if (!DynamicMaterial)
		{
			DynamicMaterial = MeshComponent->CreateAndSetMaterialInstanceDynamic(Index);
		}
		if (!DynamicMaterial)
		{
			continue;
		}

		DynamicMaterial->SetVectorParameterValue(TEXT("Color"), Color * EmissiveStrength);
		DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), Color);
		DynamicMaterial->SetVectorParameterValue(TEXT("BeamColor"), Color);
		DynamicMaterial->SetVectorParameterValue(TEXT("EmissiveColor"), Color * EmissiveStrength);
		DynamicMaterial->SetScalarParameterValue(TEXT("Opacity"), Opacity);
		DynamicMaterial->SetScalarParameterValue(TEXT("Alpha"), Opacity);
		DynamicMaterial->SetScalarParameterValue(TEXT("BeamOpacity"), Opacity);
		DynamicMaterial->SetScalarParameterValue(TEXT("EmissiveStrength"), EmissiveStrength);
	}
}

void ACitySweepLightActor::AdvanceSweep(float DeltaSeconds)
{
	if (bPingPongSweep)
	{
		const float MinYaw = FMath::Min(SweepYawMinDeg, SweepYawMaxDeg);
		const float MaxYaw = FMath::Max(SweepYawMinDeg, SweepYawMaxDeg);
		CurrentYawDeg += FMath::Abs(SweepSpeedDegPerSec) * PingPongDirection * DeltaSeconds;

		if (CurrentYawDeg > MaxYaw)
		{
			CurrentYawDeg = MaxYaw;
			PingPongDirection = -1.0f;
		}
		else if (CurrentYawDeg < MinYaw)
		{
			CurrentYawDeg = MinYaw;
			PingPongDirection = 1.0f;
		}
	}
	else
	{
		CurrentYawDeg = FMath::UnwindDegrees(CurrentYawDeg + SweepSpeedDegPerSec * DeltaSeconds);
	}

	ApplySweepTransform();
}
