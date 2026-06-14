#include "HighwayTrafficSystem.h"

#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Math/RandomStream.h"
#include "UObject/ConstructorHelpers.h"

AHighwayTrafficCarActor::AHighwayTrafficCarActor()
{
	PrimaryActorTick.bCanEverTick = false;

	VehicleRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("VehicleRootComponent"));
	SetRootComponent(VehicleRootComponent);

	CarMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CarMeshComponent"));
	CarMeshComponent->SetupAttachment(VehicleRootComponent);
	CarMeshComponent->SetCollisionProfileName(TEXT("Vehicle"));

	FrontLeftHeadlight = CreateDefaultSubobject<USpotLightComponent>(TEXT("FrontLeftHeadlight"));
	FrontLeftHeadlight->SetupAttachment(VehicleRootComponent);

	FrontRightHeadlight = CreateDefaultSubobject<USpotLightComponent>(TEXT("FrontRightHeadlight"));
	FrontRightHeadlight->SetupAttachment(VehicleRootComponent);

	RearLeftLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("RearLeftLight"));
	RearLeftLight->SetupAttachment(VehicleRootComponent);

	RearRightLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("RearRightLight"));
	RearRightLight->SetupAttachment(VehicleRootComponent);

	FrontLeftLightSourceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrontLeftLightSourceMesh"));
	FrontLeftLightSourceMesh->SetupAttachment(VehicleRootComponent);
	FrontLeftLightSourceMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FrontLeftLightSourceMesh->SetCastShadow(false);

	FrontRightLightSourceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrontRightLightSourceMesh"));
	FrontRightLightSourceMesh->SetupAttachment(VehicleRootComponent);
	FrontRightLightSourceMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FrontRightLightSourceMesh->SetCastShadow(false);

	RearLeftLightSourceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RearLeftLightSourceMesh"));
	RearLeftLightSourceMesh->SetupAttachment(VehicleRootComponent);
	RearLeftLightSourceMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RearLeftLightSourceMesh->SetCastShadow(false);

	RearRightLightSourceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RearRightLightSourceMesh"));
	RearRightLightSourceMesh->SetupAttachment(VehicleRootComponent);
	RearRightLightSourceMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RearRightLightSourceMesh->SetCastShadow(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> EmissiveMaterial(TEXT("/Engine/EngineMaterials/EmissiveMeshMaterial.EmissiveMeshMaterial"));
	if (EmissiveMaterial.Succeeded())
	{
		LightSourceMaterial = EmissiveMaterial.Object;
	}

	if (SphereMesh.Succeeded())
	{
		FrontLeftLightSourceMesh->SetStaticMesh(SphereMesh.Object);
		FrontRightLightSourceMesh->SetStaticMesh(SphereMesh.Object);
		RearLeftLightSourceMesh->SetStaticMesh(SphereMesh.Object);
		RearRightLightSourceMesh->SetStaticMesh(SphereMesh.Object);
	}
}

void AHighwayTrafficCarActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ApplyVehicleSettings();
	ApplyLightSettings();
}

void AHighwayTrafficCarActor::SetLightsEnabled(bool bNewLightsEnabled)
{
	bLightsEnabled = bNewLightsEnabled;
	ApplyLightSettings();
}

void AHighwayTrafficCarActor::SetModelRotationOffset(FRotator NewModelRotationOffset)
{
	ModelRotationOffset = NewModelRotationOffset;
	ApplyVehicleSettings();
}

void AHighwayTrafficCarActor::SetVehicleMesh(UStaticMesh* NewMesh)
{
	VehicleMesh = NewMesh;
	ApplyVehicleSettings();
}

void AHighwayTrafficCarActor::ApplyVehicleSettings()
{
	if (!VehicleRootComponent || !CarMeshComponent)
	{
		return;
	}

	VehicleRootComponent->SetRelativeLocation(VehicleRelativeLocation + FVector(0.0f, 0.0f, VehicleHeightOffset));
	VehicleRootComponent->SetRelativeRotation(VehicleRelativeRotation + ModelRotationOffset);
	VehicleRootComponent->SetRelativeScale3D(VehicleRelativeScale);

	if (VehicleMesh)
	{
		CarMeshComponent->SetStaticMesh(VehicleMesh);
	}

	CarMeshComponent->SetRelativeLocation(MeshRelativeLocation);
	CarMeshComponent->SetRelativeRotation(MeshRelativeRotation);
	CarMeshComponent->SetRelativeScale3D(MeshRelativeScale);
}

void AHighwayTrafficCarActor::ApplyLightSettings()
{
	const bool bFrontVisible = bLightsEnabled && bHeadlightsEnabled;
	const bool bRearVisible = bLightsEnabled && bRearLightsEnabled;
	const float LightScale = GetMeshLightScale();
	const float RadiusScale = bScaleLightRadiusWithMesh ? LightScale : 1.0f;
	const float IntensityScale = bScaleLightIntensityWithMesh ? FMath::Pow(LightScale, LightIntensityScalePower) : 1.0f;

	if (FrontLeftHeadlight)
	{
		FrontLeftHeadlight->SetVisibility(bFrontVisible);
		FrontLeftHeadlight->SetRelativeLocation(GetLightOffsetRelativeToVehicle(FrontLeftLightOffset, true, false));
		FrontLeftHeadlight->SetRelativeRotation(GetLightRotationRelativeToVehicle(FrontLightRotation));
		FrontLeftHeadlight->SetLightColor(FrontLightColor);
		FrontLeftHeadlight->SetIntensity(FrontLightIntensity * IntensityScale);
		FrontLeftHeadlight->SetAttenuationRadius(FrontLightAttenuationRadius * RadiusScale);
		FrontLeftHeadlight->SetSourceRadius(FrontLightSourceRadius * RadiusScale);
		FrontLeftHeadlight->SetSoftSourceRadius(FrontLightSourceRadius * RadiusScale);
		FrontLeftHeadlight->SetInnerConeAngle(FrontLightInnerConeAngle);
		FrontLeftHeadlight->SetOuterConeAngle(FrontLightOuterConeAngle);
	}

	if (FrontRightHeadlight)
	{
		FrontRightHeadlight->SetVisibility(bFrontVisible);
		FrontRightHeadlight->SetRelativeLocation(GetLightOffsetRelativeToVehicle(FrontRightLightOffset, true, true));
		FrontRightHeadlight->SetRelativeRotation(GetLightRotationRelativeToVehicle(FrontLightRotation));
		FrontRightHeadlight->SetLightColor(FrontLightColor);
		FrontRightHeadlight->SetIntensity(FrontLightIntensity * IntensityScale);
		FrontRightHeadlight->SetAttenuationRadius(FrontLightAttenuationRadius * RadiusScale);
		FrontRightHeadlight->SetSourceRadius(FrontLightSourceRadius * RadiusScale);
		FrontRightHeadlight->SetSoftSourceRadius(FrontLightSourceRadius * RadiusScale);
		FrontRightHeadlight->SetInnerConeAngle(FrontLightInnerConeAngle);
		FrontRightHeadlight->SetOuterConeAngle(FrontLightOuterConeAngle);
	}

	if (RearLeftLight)
	{
		RearLeftLight->SetVisibility(bRearVisible);
		RearLeftLight->SetRelativeLocation(GetLightOffsetRelativeToVehicle(RearLeftLightOffset, false, false));
		RearLeftLight->SetLightColor(RearLightColor);
		RearLeftLight->SetIntensity(RearLightIntensity * IntensityScale);
		RearLeftLight->SetAttenuationRadius(RearLightAttenuationRadius * RadiusScale);
		RearLeftLight->SetSourceRadius(RearLightSourceRadius * RadiusScale);
		RearLeftLight->SetSoftSourceRadius(RearLightSourceRadius * RadiusScale);
	}

	if (RearRightLight)
	{
		RearRightLight->SetVisibility(bRearVisible);
		RearRightLight->SetRelativeLocation(GetLightOffsetRelativeToVehicle(RearRightLightOffset, false, true));
		RearRightLight->SetLightColor(RearLightColor);
		RearRightLight->SetIntensity(RearLightIntensity * IntensityScale);
		RearRightLight->SetAttenuationRadius(RearLightAttenuationRadius * RadiusScale);
		RearRightLight->SetSourceRadius(RearLightSourceRadius * RadiusScale);
		RearRightLight->SetSoftSourceRadius(RearLightSourceRadius * RadiusScale);
	}

	ApplyLightSourceMesh(FrontLeftLightSourceMesh, FrontLeftLightOffset, true, false, FrontLightColor, bFrontVisible, LightScale);
	ApplyLightSourceMesh(FrontRightLightSourceMesh, FrontRightLightOffset, true, true, FrontLightColor, bFrontVisible, LightScale);
	ApplyLightSourceMesh(RearLeftLightSourceMesh, RearLeftLightOffset, false, false, RearLightColor, bRearVisible, LightScale);
	ApplyLightSourceMesh(RearRightLightSourceMesh, RearRightLightOffset, false, true, RearLightColor, bRearVisible, LightScale);
}

FVector AHighwayTrafficCarActor::GetLightOffsetRelativeToVehicle(FVector ManualLocalLightOffset, bool bFrontLight, bool bRightLight) const
{
	const FVector LocalLightOffset = GetBoundsLightOffset(ManualLocalLightOffset, bFrontLight, bRightLight);

	if (!bUseMeshTransformForLights)
	{
		return LocalLightOffset;
	}

	const FVector ScaledOffset(
		LocalLightOffset.X * MeshRelativeScale.X,
		LocalLightOffset.Y * MeshRelativeScale.Y,
		LocalLightOffset.Z * MeshRelativeScale.Z);

	return MeshRelativeLocation + MeshRelativeRotation.RotateVector(ScaledOffset);
}

FRotator AHighwayTrafficCarActor::GetLightRotationRelativeToVehicle(FRotator LocalLightRotation) const
{
	return bUseMeshTransformForLights ? (MeshRelativeRotation + LocalLightRotation) : LocalLightRotation;
}

FVector AHighwayTrafficCarActor::GetBoundsLightOffset(FVector ManualLocalLightOffset, bool bFrontLight, bool bRightLight) const
{
	if (!bAutoPlaceLightsFromMeshBounds || !CarMeshComponent || !CarMeshComponent->GetStaticMesh())
	{
		return ManualLocalLightOffset;
	}

	const FBox MeshBox = CarMeshComponent->GetStaticMesh()->GetBoundingBox();
	if (!MeshBox.IsValid)
	{
		return ManualLocalLightOffset;
	}

	const float X = bFrontLight ? MeshBox.Max.X + LightBoundsExteriorOffset : MeshBox.Min.X - LightBoundsExteriorOffset;
	const float SideAlpha = bRightLight ? 1.0f - LightBoundsSideInset : LightBoundsSideInset;
	const float Y = FMath::Lerp(MeshBox.Min.Y, MeshBox.Max.Y, SideAlpha);
	const float Z = FMath::Lerp(MeshBox.Min.Z, MeshBox.Max.Z, LightBoundsHeightPercent);

	return FVector(X, Y, Z);
}

float AHighwayTrafficCarActor::GetMeshLightScale() const
{
	if (!bUseMeshTransformForLights)
	{
		return 1.0f;
	}

	const float MaxMeshScale = FMath::Max3(FMath::Abs(MeshRelativeScale.X), FMath::Abs(MeshRelativeScale.Y), FMath::Abs(MeshRelativeScale.Z));
	return FMath::Max(0.01f, MaxMeshScale);
}

void AHighwayTrafficCarActor::ApplyLightSourceMesh(UStaticMeshComponent* SourceMesh, FVector ManualLocalLightOffset, bool bFrontLight, bool bRightLight, FLinearColor SourceColor, bool bSourceVisible, float LightScale)
{
	if (!SourceMesh)
	{
		return;
	}

	const bool bVisible = bShowLightSourceMeshes && bSourceVisible;
	SourceMesh->SetVisibility(bVisible);
	SourceMesh->SetHiddenInGame(!bVisible);
	SourceMesh->SetRelativeLocation(GetLightOffsetRelativeToVehicle(ManualLocalLightOffset, bFrontLight, bRightLight));
	SourceMesh->SetRelativeRotation(GetLightRotationRelativeToVehicle(FRotator::ZeroRotator));
	SourceMesh->SetRelativeScale3D(FVector(LightSourceMeshScale * FMath::Max(0.01f, LightScale)));

	if (LightSourceMaterial)
	{
		SourceMesh->SetMaterial(0, LightSourceMaterial);
	}

	if (UMaterialInstanceDynamic* DynamicMaterial = SourceMesh->CreateAndSetMaterialInstanceDynamic(0))
	{
		const FLinearColor EmissiveColor = SourceColor * LightSourceEmissiveStrength;
		DynamicMaterial->SetVectorParameterValue(TEXT("Color"), EmissiveColor);
		DynamicMaterial->SetVectorParameterValue(TEXT("EmissiveColor"), EmissiveColor);
	}
}

AHighwayTrafficSplineActor::AHighwayTrafficSplineActor()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	TrafficSpline = CreateDefaultSubobject<USplineComponent>(TEXT("TrafficSpline"));
	TrafficSpline->SetupAttachment(SceneRoot);

	CarActorClass = AHighwayTrafficCarActor::StaticClass();
}

void AHighwayTrafficSplineActor::BeginPlay()
{
	Super::BeginPlay();

	if (bSpawnOnBeginPlay)
	{
		SpawnTraffic();
	}
}

void AHighwayTrafficSplineActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!TrafficSpline)
	{
		return;
	}

	const float SplineLength = FMath::Max(1.0f, TrafficSpline->GetSplineLength());
	const float Direction = bReverseDirection ? -1.0f : 1.0f;

	for (FHighwayTrafficRuntimeCar& RuntimeCar : RuntimeCars)
	{
		if (!IsValid(RuntimeCar.CarActor))
		{
			continue;
		}

		RuntimeCar.DistanceAlongSpline += RuntimeCar.SpeedCmPerSec * Direction * DeltaTime;
		if (bLoopTraffic)
		{
			RuntimeCar.DistanceAlongSpline = FMath::Fmod(RuntimeCar.DistanceAlongSpline + SplineLength, SplineLength);
		}
		else
		{
			RuntimeCar.DistanceAlongSpline = FMath::Clamp(RuntimeCar.DistanceAlongSpline, 0.0f, SplineLength);
		}

		UpdateCarTransform(RuntimeCar);
	}
}

void AHighwayTrafficSplineActor::SpawnTraffic()
{
	if (!GetWorld() || !TrafficSpline || !CarActorClass)
	{
		return;
	}

	ClearTraffic();

	FRandomStream RandomStream(RandomSeed);
	const float SplineLength = FMath::Max(1.0f, TrafficSpline->GetSplineLength());
	const float BaseSpeedCmPerSec = TrafficSpeedKmh * 100000.0f / 3600.0f;
	const int32 SafeLaneCount = FMath::Max(1, LaneCount);
	const int32 SafeCarsPerLane = FMath::Max(0, CarsPerLane);

	for (int32 LaneIndex = 0; LaneIndex < SafeLaneCount; ++LaneIndex)
	{
		for (int32 CarIndex = 0; CarIndex < SafeCarsPerLane; ++CarIndex)
		{
			FHighwayTrafficRuntimeCar RuntimeCar;
			RuntimeCar.Lane = LaneIndex;
			RuntimeCar.DistanceAlongSpline = (SplineLength / FMath::Max(1, SafeCarsPerLane)) * CarIndex + RandomStream.FRandRange(-DistanceJitter, DistanceJitter);
			RuntimeCar.DistanceAlongSpline = FMath::Fmod(RuntimeCar.DistanceAlongSpline + SplineLength, SplineLength);
			RuntimeCar.SpeedCmPerSec = BaseSpeedCmPerSec * RandomStream.FRandRange(1.0f - SpeedVariance, 1.0f + SpeedVariance);

			RuntimeCar.CarActor = GetWorld()->SpawnActor<AHighwayTrafficCarActor>(CarActorClass, GetActorTransform());
			if (!RuntimeCar.CarActor)
			{
				continue;
			}

			RuntimeCar.CarActor->SetVehicleMesh(VehicleMeshOverride);
			RuntimeCar.CarActor->SetLightsEnabled(bCarsUseLights);
			RuntimeCar.CarActor->SetModelRotationOffset(CarModelRotationOffset);

			UpdateCarTransform(RuntimeCar);
			RuntimeCars.Add(RuntimeCar);
		}
	}
}

void AHighwayTrafficSplineActor::ClearTraffic()
{
	for (FHighwayTrafficRuntimeCar& RuntimeCar : RuntimeCars)
	{
		if (IsValid(RuntimeCar.CarActor))
		{
			RuntimeCar.CarActor->Destroy();
		}
	}

	RuntimeCars.Reset();
}

void AHighwayTrafficSplineActor::RebuildTraffic()
{
	ClearTraffic();
	SpawnTraffic();
}

void AHighwayTrafficSplineActor::UpdateCarTransform(FHighwayTrafficRuntimeCar& RuntimeCar)
{
	if (!TrafficSpline || !IsValid(RuntimeCar.CarActor))
	{
		return;
	}

	const float LaneCenter = (static_cast<float>(LaneCount - 1) * 0.5f);
	const float LaneOffset = (static_cast<float>(RuntimeCar.Lane) - LaneCenter) * LaneSpacing;
	const FVector SplineLocation = TrafficSpline->GetLocationAtDistanceAlongSpline(RuntimeCar.DistanceAlongSpline, ESplineCoordinateSpace::World);
	const FRotator SplineRotation = TrafficSpline->GetRotationAtDistanceAlongSpline(RuntimeCar.DistanceAlongSpline, ESplineCoordinateSpace::World);
	const FVector RightVector = FRotationMatrix(SplineRotation).GetUnitAxis(EAxis::Y);
	const FVector ForwardVector = FRotationMatrix(SplineRotation).GetUnitAxis(EAxis::X);
	RuntimeCar.Velocity = ForwardVector * RuntimeCar.SpeedCmPerSec * (bReverseDirection ? -1.0f : 1.0f);

	RuntimeCar.CarActor->SetActorLocation(SplineLocation + RightVector * LaneOffset);
	RuntimeCar.CarActor->SetActorRotation(SplineRotation);
}
