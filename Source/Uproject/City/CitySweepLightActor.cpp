#include "CitySweepLightActor.h"

#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "WorldCollision.h"
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

	BeamHaloMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisibleBeamHalo"));
	BeamHaloMeshComponent->SetupAttachment(BeamDirectionPivot);
	BeamHaloMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BeamHaloMeshComponent->SetCastShadow(false);
	BeamHaloMeshComponent->SetMobility(EComponentMobility::Movable);
	BeamHaloMeshComponent->bSelectable = false;

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
		BeamHaloMeshComponent->SetStaticMesh(BeamMesh);
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

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultBeamMaterial(TEXT("/Game/Assets/Mathiasprovot/Lighting/M_CitySweepBeam_Clip.M_CitySweepBeam_Clip"));
	if (DefaultBeamMaterial.Succeeded())
	{
		BeamMaterial = DefaultBeamMaterial.Object;
	}

	BeamIgnoredActorTags.AddUnique(TEXT("BeamIgnore"));
	BeamIgnoredComponentTags.AddUnique(TEXT("BeamIgnore"));

	ResetSweep();
	CurrentBeamClipDistance = BeamLength;
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
	UpdateBeamCollision(DeltaSeconds);
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
	CurrentBeamClipDistance = ResolveBeamLengthFromCollision();
	ApplySweepTransform();
}

void ACitySweepLightActor::ApplyAllSettings()
{
	CurrentBeamClipDistance = ResolveBeamLengthFromCollision();
	ApplySweepTransform();
	ApplyLightSettings();
	if (SearchLightComponent)
	{
		const float RuntimeRadius = bClipBeamOnWorldHit
			? FMath::Min(AttenuationRadius, BeamStartOffset + CurrentBeamClipDistance + BeamClipPadding)
			: AttenuationRadius;
		SearchLightComponent->SetAttenuationRadius(FMath::Max(1.0f, RuntimeRadius));
	}
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
	SearchLightComponent->SetCastVolumetricShadow(bCastVolumetricShadow);
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

	const float SafeBeamLength = FMath::Max(1.0f, BeamLength);
	const float ClipAlpha = FMath::Clamp(CurrentBeamClipDistance / SafeBeamLength, 0.0f, 1.0f);
	const bool bUseMaterialClip = BeamClipMode == ECitySweepBeamClipMode::MaterialMask;
	const bool bUseFullLengthMesh = bUseMaterialClip || bKeepBeamMeshFullLengthWhenClipped;
	const float VisualLength = bUseFullLengthMesh ? SafeBeamLength : CurrentBeamClipDistance;
	const float VisualRadius = bUseFullLengthMesh ? BeamRadius : BeamRadius * ClipAlpha;

	ApplyBeamMeshTransform(BeamMeshComponent, VisualLength, VisualRadius, BeamStartOffset);
	ApplyColorMaterialParameters(BeamMeshComponent, LightColor, BeamOpacity, BeamEmissiveStrength);

	if (BeamHaloMeshComponent)
	{
		BeamHaloMeshComponent->SetVisibility(bShowBeamMesh && bShowBeamHalo);
		BeamHaloMeshComponent->SetHiddenInGame(!(bShowBeamMesh && bShowBeamHalo));
		if (BeamMesh)
		{
			BeamHaloMeshComponent->SetStaticMesh(BeamMesh);
		}
		if (BeamMaterial)
		{
			BeamHaloMeshComponent->SetMaterial(0, BeamMaterial);
		}

		ApplyBeamMeshTransform(BeamHaloMeshComponent, VisualLength, VisualRadius * BeamHaloRadiusMultiplier, BeamStartOffset);
		ApplyColorMaterialParameters(
			BeamHaloMeshComponent,
			LightColor,
			BeamOpacity * BeamHaloOpacityMultiplier,
			BeamEmissiveStrength * BeamHaloEmissiveMultiplier);
	}
}

void ACitySweepLightActor::ApplyBeamMeshTransform(UStaticMeshComponent* MeshComponent, float VisualLength, float VisualRadius, float InBeamStartOffset) const
{
	if (!MeshComponent)
	{
		return;
	}

	const float SafeLength = FMath::Max(1.0f, VisualLength);
	const float SafeRadius = FMath::Max(1.0f, VisualRadius);
	MeshComponent->SetRelativeLocation(FVector(InBeamStartOffset + SafeLength * 0.5f, 0.0f, 0.0f));
	MeshComponent->SetRelativeRotation(BeamMeshRotationOffset);
	MeshComponent->SetRelativeScale3D(FVector(
		SafeRadius / 100.0f,
		SafeRadius / 100.0f,
		SafeLength / 100.0f));
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
		DynamicMaterial->SetScalarParameterValue(TEXT("SoftEdge"), 1.0f);
		DynamicMaterial->SetScalarParameterValue(TEXT("DepthFade"), 1.0f);
		DynamicMaterial->SetScalarParameterValue(TEXT("BeamLength"), FMath::Max(1.0f, BeamLength));
		DynamicMaterial->SetScalarParameterValue(TEXT("BeamClipDistance"), FMath::Max(1.0f, CurrentBeamClipDistance));
		DynamicMaterial->SetScalarParameterValue(TEXT("BeamClipAlpha"), FMath::Clamp(CurrentBeamClipDistance / FMath::Max(1.0f, BeamLength), 0.0f, 1.0f));
		DynamicMaterial->SetScalarParameterValue(TEXT("BeamClipFadeDistance"), FMath::Max(0.0f, BeamClipFadeDistance));
		DynamicMaterial->SetScalarParameterValue(TEXT("BeamUsesMaterialClip"), BeamClipMode == ECitySweepBeamClipMode::MaterialMask ? 1.0f : 0.0f);
		DynamicMaterial->SetScalarParameterValue(TEXT("BeamEdgePower"), FMath::Max(0.1f, BeamEdgePower));
		DynamicMaterial->SetScalarParameterValue(TEXT("BeamUseUAxis"), bBeamMaterialUseUAxis ? 1.0f : 0.0f);
		DynamicMaterial->SetScalarParameterValue(TEXT("BeamInvertAxis"), bBeamMaterialInvertAxis ? 1.0f : 0.0f);

		if (BeamDirectionPivot)
		{
			const FVector Forward = BeamDirectionPivot->GetForwardVector();
			const FVector Origin = BeamDirectionPivot->GetComponentLocation() + Forward * FMath::Max(0.0f, BeamStartOffset);
			DynamicMaterial->SetVectorParameterValue(TEXT("BeamWorldOrigin"), FLinearColor(Origin.X, Origin.Y, Origin.Z, 1.0f));
			DynamicMaterial->SetVectorParameterValue(TEXT("BeamWorldDirection"), FLinearColor(Forward.X, Forward.Y, Forward.Z, 0.0f));
		}
	}
}

void ACitySweepLightActor::UpdateBeamCollision(float DeltaSeconds)
{
	const float TargetBeamLength = ResolveBeamLengthFromCollision();
	CurrentBeamClipDistance = BeamLengthInterpSpeed > 0.0f
		? FMath::FInterpTo(CurrentBeamClipDistance, TargetBeamLength, DeltaSeconds, BeamLengthInterpSpeed)
		: TargetBeamLength;

	if (SearchLightComponent)
	{
		const float RuntimeRadius = bClipBeamOnWorldHit
			? FMath::Min(AttenuationRadius, BeamStartOffset + CurrentBeamClipDistance + BeamClipPadding)
			: AttenuationRadius;
		SearchLightComponent->SetAttenuationRadius(FMath::Max(1.0f, RuntimeRadius));
	}

	ApplyBeamVisualSettings();
}

float ACitySweepLightActor::ResolveBeamLengthFromCollision()
{
	if (!bClipBeamOnWorldHit || !BeamDirectionPivot || !GetWorld())
	{
		LastBeamHitActorName = NAME_None;
		LastBeamHitComponentName = NAME_None;
		LastBeamHitLocation = FVector::ZeroVector;
		return FMath::Max(1.0f, BeamLength);
	}

	const FVector Origin = BeamDirectionPivot->GetComponentLocation();
	const FVector Forward = BeamDirectionPivot->GetForwardVector();
	const FVector Right = BeamDirectionPivot->GetRightVector();
	const FVector Up = BeamDirectionPivot->GetUpVector();
	const float VisualStartOffset = FMath::Max(0.0f, BeamStartOffset);
	const float TraceStartOffset = VisualStartOffset + FMath::Max(0.0f, BeamTraceStartOffset);
	const FVector VisualStart = Origin + Forward * VisualStartOffset;
	const FVector TraceStart = Origin + Forward * TraceStartOffset;
	const float SafeBeamLength = FMath::Max(1.0f, BeamLength);
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(CitySweepLightBeamTrace), false, this);
	QueryParams.AddIgnoredActor(this);
	QueryParams.bTraceComplex = bBeamTraceComplex;
	QueryParams.bReturnFaceIndex = bBeamTraceComplex;

	const int32 SafeProbeRayCount = FMath::Clamp(BeamProbeRayCount, 1, 16);
	const float ConeSlope = SafeBeamLength > 0.0f
		? (FMath::Max(0.0f, BeamRadius) / SafeBeamLength) * FMath::Clamp(BeamProbeConeFraction, 0.0f, 1.5f)
		: 0.0f;
	const float SafeTraceRadius = FMath::Max(0.0f, BeamTraceRadius);

	FHitResult BestHit;
	float BestDistanceFromVisibleStart = SafeBeamLength;
	bool bFoundUsableHit = false;

	for (int32 RayIndex = 0; RayIndex < SafeProbeRayCount; ++RayIndex)
	{
		FVector RayDirection = Forward;
		if (RayIndex > 0 && ConeSlope > 0.0f)
		{
			const int32 RingRayCount = SafeProbeRayCount - 1;
			const float AngleRad = (2.0f * PI * static_cast<float>(RayIndex - 1)) / static_cast<float>(RingRayCount);
			const FVector ConeOffset = Right * FMath::Cos(AngleRad) + Up * FMath::Sin(AngleRad);
			RayDirection = (Forward + ConeOffset * ConeSlope).GetSafeNormal();
		}

		const FVector RayEnd = TraceStart + RayDirection * SafeBeamLength;
		TArray<FHitResult> Hits;
		const bool bHit = bUseBeamTraceRadius && SafeTraceRadius > 0.0f
			? GetWorld()->SweepMultiByChannel(Hits, TraceStart, RayEnd, FQuat::Identity, BeamTraceChannel, FCollisionShape::MakeSphere(SafeTraceRadius), QueryParams)
			: GetWorld()->LineTraceMultiByChannel(Hits, TraceStart, RayEnd, BeamTraceChannel, QueryParams);

		if (!bHit)
		{
			if (bDebugBeamTrace)
			{
				DrawDebugLine(GetWorld(), TraceStart, RayEnd, FColor::Green, false, 0.05f, 0, 3.0f);
			}
			continue;
		}

		Hits.Sort([](const FHitResult& A, const FHitResult& B)
		{
			return A.Distance < B.Distance;
		});

		for (const FHitResult& CandidateHit : Hits)
		{
			if (ShouldIgnoreBeamHit(CandidateHit))
			{
				continue;
			}

			const float CandidateDistanceFromVisibleStart = FVector::DotProduct(CandidateHit.ImpactPoint - VisualStart, Forward);
			if (CandidateDistanceFromVisibleStart >= 0.0f && (!bFoundUsableHit || CandidateDistanceFromVisibleStart < BestDistanceFromVisibleStart))
			{
				BestHit = CandidateHit;
				BestDistanceFromVisibleStart = CandidateDistanceFromVisibleStart;
				bFoundUsableHit = true;
			}
			break;
		}

		if (bDebugBeamTrace)
		{
			const FColor TraceColor = bFoundUsableHit ? FColor::Yellow : FColor::Green;
			DrawDebugLine(GetWorld(), TraceStart, RayEnd, TraceColor, false, 0.05f, 0, RayIndex == 0 ? 7.0f : 3.0f);
		}
	}

	if (!bFoundUsableHit)
	{
		LastBeamHitActorName = NAME_None;
		LastBeamHitComponentName = NAME_None;
		LastBeamHitLocation = FVector::ZeroVector;
		if (bDebugBeamTrace)
		{
			DrawDebugPoint(GetWorld(), TraceStart, 12.0f, FColor::Green, false, 0.05f);
		}
		return FMath::Max(1.0f, BeamLength);
	}

	const AActor* HitActor = BestHit.GetActor();
	const UPrimitiveComponent* HitComponent = BestHit.GetComponent();
	LastBeamHitActorName = HitActor ? HitActor->GetFName() : NAME_None;
	LastBeamHitComponentName = HitComponent ? HitComponent->GetFName() : NAME_None;
	LastBeamHitLocation = BestHit.ImpactPoint;

	if (bDebugBeamTrace)
	{
		DrawDebugLine(GetWorld(), TraceStart, BestHit.ImpactPoint, FColor::Orange, false, 0.05f, 0, 8.0f);
		DrawDebugPoint(GetWorld(), BestHit.ImpactPoint, 18.0f, FColor::Red, false, 0.05f);
		DrawDebugString(GetWorld(), BestHit.ImpactPoint, FString::Printf(TEXT("%s / %s"), *LastBeamHitActorName.ToString(), *LastBeamHitComponentName.ToString()), nullptr, FColor::Yellow, 0.05f, true);
	}

	return FMath::Clamp(BestDistanceFromVisibleStart - BeamClipPadding, 1.0f, BeamLength);
}

bool ACitySweepLightActor::ShouldIgnoreBeamHit(const FHitResult& Hit) const
{
	const AActor* HitActor = Hit.GetActor();
	if (!HitActor || HitActor == this)
	{
		return true;
	}

	for (const FName& IgnoredTag : BeamIgnoredActorTags)
	{
		if (!IgnoredTag.IsNone() && HitActor->ActorHasTag(IgnoredTag))
		{
			return true;
		}
	}

	const UPrimitiveComponent* HitComponent = Hit.GetComponent();
	if (HitComponent)
	{
		for (const FName& IgnoredTag : BeamIgnoredComponentTags)
		{
			if (!IgnoredTag.IsNone() && HitComponent->ComponentHasTag(IgnoredTag))
			{
				return true;
			}
		}
	}

	return false;
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
