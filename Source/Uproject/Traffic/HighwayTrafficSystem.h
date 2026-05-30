#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HighwayTrafficSystem.generated.h"

class AHighwayTrafficCarActor;
class UMaterialInterface;
class UPointLightComponent;
class USplineComponent;
class USpotLightComponent;
class UStaticMesh;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class UPROJECT_API AHighwayTrafficCarActor : public AActor
{
	GENERATED_BODY()

public:
	AHighwayTrafficCarActor();

	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> VehicleRootComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> CarMeshComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USpotLightComponent> FrontLeftHeadlight = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USpotLightComponent> FrontRightHeadlight = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPointLightComponent> RearLeftLight = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPointLightComponent> RearRightLight = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> FrontLeftLightSourceMesh = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> FrontRightLightSourceMesh = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> RearLeftLightSourceMesh = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> RearRightLightSourceMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	TObjectPtr<UStaticMesh> VehicleMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	FVector VehicleRelativeLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	FRotator VehicleRelativeRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	FVector VehicleRelativeScale = FVector::OneVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	FVector MeshRelativeLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	FRotator MeshRelativeRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	FVector MeshRelativeScale = FVector::OneVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	float VehicleHeightOffset = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	FRotator ModelRotationOffset = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lights")
	bool bLightsEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lights")
	bool bHeadlightsEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lights")
	bool bRearLightsEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lights")
	bool bUseMeshTransformForLights = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lights")
	bool bScaleLightRadiusWithMesh = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lights")
	bool bScaleLightIntensityWithMesh = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lights")
	bool bAutoPlaceLightsFromMeshBounds = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lights")
	bool bShowLightSourceMeshes = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lights", meta = (ClampMin = "0.0"))
	float LightBoundsExteriorOffset = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lights", meta = (ClampMin = "0.0", ClampMax = "0.5"))
	float LightBoundsSideInset = 0.22f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lights", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LightBoundsHeightPercent = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lights", meta = (ClampMin = "0.0"))
	float LightIntensityScalePower = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lights", meta = (ClampMin = "0.0"))
	float LightSourceMeshScale = 0.12f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lights", meta = (ClampMin = "0.0"))
	float LightSourceEmissiveStrength = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lights")
	TObjectPtr<UMaterialInterface> LightSourceMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lights")
	FLinearColor FrontLightColor = FLinearColor(0.85f, 0.95f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lights")
	FLinearColor RearLightColor = FLinearColor(1.0f, 0.0f, 0.08f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lights")
	float FrontLightIntensity = 3500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lights")
	float RearLightIntensity = 550.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lights")
	float FrontLightAttenuationRadius = 2200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lights")
	float RearLightAttenuationRadius = 450.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lights")
	float FrontLightSourceRadius = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lights")
	float RearLightSourceRadius = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lights")
	float FrontLightInnerConeAngle = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lights")
	float FrontLightOuterConeAngle = 32.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lights")
	FRotator FrontLightRotation = FRotator(-8.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lights")
	FVector FrontLeftLightOffset = FVector(190.0f, -55.0f, 45.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lights")
	FVector FrontRightLightOffset = FVector(190.0f, 55.0f, 45.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lights")
	FVector RearLeftLightOffset = FVector(-190.0f, -55.0f, 45.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lights")
	FVector RearRightLightOffset = FVector(-190.0f, 55.0f, 45.0f);

	UFUNCTION(BlueprintCallable, Category = "Lights")
	void SetLightsEnabled(bool bNewLightsEnabled);

	UFUNCTION(BlueprintCallable, Category = "Vehicle")
	void SetModelRotationOffset(FRotator NewModelRotationOffset);

	UFUNCTION(BlueprintCallable, Category = "Vehicle")
	void SetVehicleMesh(UStaticMesh* NewMesh);

private:
	void ApplyVehicleSettings();
	void ApplyLightSettings();
	FVector GetLightOffsetRelativeToVehicle(FVector ManualLocalLightOffset, bool bFrontLight, bool bRightLight) const;
	FRotator GetLightRotationRelativeToVehicle(FRotator LocalLightRotation) const;
	FVector GetBoundsLightOffset(FVector ManualLocalLightOffset, bool bFrontLight, bool bRightLight) const;
	float GetMeshLightScale() const;
	void ApplyLightSourceMesh(UStaticMeshComponent* SourceMesh, FVector ManualLocalLightOffset, bool bFrontLight, bool bRightLight, FLinearColor SourceColor, bool bSourceVisible, float LightScale);
};

USTRUCT(BlueprintType)
struct UPROJECT_API FHighwayTrafficRuntimeCar
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traffic")
	TObjectPtr<AHighwayTrafficCarActor> CarActor = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traffic")
	float DistanceAlongSpline = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traffic")
	float SpeedCmPerSec = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traffic")
	int32 Lane = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traffic")
	FVector Velocity = FVector::ZeroVector;
};

UCLASS(Blueprintable)
class UPROJECT_API AHighwayTrafficSplineActor : public AActor
{
	GENERATED_BODY()

public:
	AHighwayTrafficSplineActor();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USplineComponent> TrafficSpline = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traffic")
	TSubclassOf<AHighwayTrafficCarActor> CarActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traffic")
	TObjectPtr<UStaticMesh> VehicleMeshOverride = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic", meta = (ClampMin = "0"))
	int32 CarsPerLane = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic", meta = (ClampMin = "1"))
	int32 LaneCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic")
	float LaneSpacing = 360.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic")
	float TrafficSpeedKmh = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic")
	float SpeedVariance = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic")
	float DistanceJitter = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic")
	int32 RandomSeed = 1337;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic")
	bool bLoopTraffic = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic")
	bool bReverseDirection = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic")
	bool bCarsUseLights = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic")
	bool bSpawnOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic")
	FRotator CarModelRotationOffset = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traffic")
	TArray<FHighwayTrafficRuntimeCar> RuntimeCars;

	UFUNCTION(BlueprintCallable, Category = "Traffic")
	void SpawnTraffic();

	UFUNCTION(BlueprintCallable, Category = "Traffic")
	void ClearTraffic();

	UFUNCTION(BlueprintCallable, Category = "Traffic")
	void RebuildTraffic();

private:
	void UpdateCarTransform(FHighwayTrafficRuntimeCar& RuntimeCar);
};
