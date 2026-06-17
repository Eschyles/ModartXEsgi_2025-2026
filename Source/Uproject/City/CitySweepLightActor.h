#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CitySweepLightActor.generated.h"

class UMaterialInterface;
class USceneComponent;
class USpotLightComponent;
class UStaticMesh;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class ECitySweepBeamClipMode : uint8
{
	GeometryPreserveAngle UMETA(DisplayName = "Geometry - Preserve Cone Angle"),
	MaterialMask UMETA(DisplayName = "Material Mask - Full Cone")
};

UCLASS(Blueprintable)
class UPROJECT_API ACitySweepLightActor : public AActor
{
	GENERATED_BODY()

public:
	ACitySweepLightActor();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual bool ShouldTickIfViewportsOnly() const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SweepYawPivot = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> BeamDirectionPivot = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USpotLightComponent> SearchLightComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BeamMeshComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BeamHaloMeshComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> LensMeshComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep")
	bool bSweepEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep")
	bool bAnimateInEditor = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep")
	bool bPingPongSweep = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep", meta = (EditCondition = "bPingPongSweep"))
	float SweepYawMinDeg = -85.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep", meta = (EditCondition = "bPingPongSweep"))
	float SweepYawMaxDeg = 85.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep")
	float SweepStartYawDeg = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep")
	float SweepSpeedDegPerSec = 18.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep")
	float BeamPitchDeg = -4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep")
	float BeamRollDeg = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light")
	bool bLightEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light")
	bool bCastLightShadows = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light")
	bool bCastVolumetricShadow = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light")
	bool bAffectTranslucentLighting = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light")
	FLinearColor LightColor = FLinearColor(0.82f, 0.92f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light", meta = (ClampMin = "0.0"))
	float LightIntensity = 650000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light", meta = (ClampMin = "1.0"))
	float AttenuationRadius = 90000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light", meta = (ClampMin = "0.0", ClampMax = "80.0"))
	float InnerConeAngle = 2.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light", meta = (ClampMin = "0.1", ClampMax = "89.0"))
	float OuterConeAngle = 9.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light", meta = (ClampMin = "0.0"))
	float SourceRadius = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light", meta = (ClampMin = "0.0"))
	float SoftSourceRadius = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light", meta = (ClampMin = "0.0"))
	float VolumetricScatteringIntensity = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Visual")
	bool bShowBeamMesh = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Collision")
	bool bClipBeamOnWorldHit = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Collision")
	ECitySweepBeamClipMode BeamClipMode = ECitySweepBeamClipMode::MaterialMask;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Collision")
	TEnumAsByte<ECollisionChannel> BeamTraceChannel = ECC_Visibility;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Collision")
	bool bBeamTraceComplex = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Collision")
	bool bUseBeamTraceRadius = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Collision", meta = (ClampMin = "0.0"))
	float BeamTraceRadius = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Collision", meta = (ClampMin = "0.0"))
	float BeamTraceStartOffset = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Collision", meta = (ClampMin = "1", ClampMax = "16"))
	int32 BeamProbeRayCount = 7;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Collision", meta = (ClampMin = "0.0", ClampMax = "1.5"))
	float BeamProbeConeFraction = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Collision", meta = (ClampMin = "0.0"))
	float BeamClipPadding = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Collision", meta = (ClampMin = "0.0"))
	float BeamClipFadeDistance = 2400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Collision", meta = (ClampMin = "0.0"))
	float BeamLengthInterpSpeed = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Collision")
	TArray<FName> BeamIgnoredActorTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Collision")
	TArray<FName> BeamIgnoredComponentTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Collision|Debug")
	bool bDebugBeamTrace = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Beam Collision|Debug")
	FName LastBeamHitActorName = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Beam Collision|Debug")
	FName LastBeamHitComponentName = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Beam Collision|Debug")
	FVector LastBeamHitLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Visual")
	TObjectPtr<UStaticMesh> BeamMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Visual")
	TObjectPtr<UMaterialInterface> BeamMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Visual", meta = (ClampMin = "1.0"))
	float BeamLength = 45000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Visual", meta = (ClampMin = "1.0"))
	float BeamRadius = 2200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Visual")
	bool bKeepBeamMeshFullLengthWhenClipped = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Visual")
	float BeamStartOffset = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Visual")
	FRotator BeamMeshRotationOffset = FRotator(-90.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Visual", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BeamOpacity = 0.28f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Visual", meta = (ClampMin = "0.0"))
	float BeamEmissiveStrength = 1.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Visual", meta = (ClampMin = "0.1"))
	float BeamEdgePower = 1.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Visual")
	bool bBeamMaterialUseUAxis = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Visual")
	bool bBeamMaterialInvertAxis = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Visual")
	bool bShowBeamHalo = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Visual", meta = (ClampMin = "1.0"))
	float BeamHaloRadiusMultiplier = 2.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Visual", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BeamHaloOpacityMultiplier = 0.55f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Visual", meta = (ClampMin = "0.0"))
	float BeamHaloEmissiveMultiplier = 0.65f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lens Visual")
	bool bShowLensMesh = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lens Visual")
	TObjectPtr<UStaticMesh> LensMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lens Visual")
	TObjectPtr<UMaterialInterface> LensMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lens Visual", meta = (ClampMin = "0.0"))
	float LensRadius = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lens Visual", meta = (ClampMin = "0.0"))
	float LensEmissiveStrength = 8.0f;

	UFUNCTION(BlueprintCallable, Category = "Sweep Light")
	void SetSweepEnabled(bool bNewEnabled);

	UFUNCTION(BlueprintCallable, Category = "Sweep Light")
	void SetLightEnabled(bool bNewEnabled);

	UFUNCTION(BlueprintCallable, Category = "Sweep Light")
	void ResetSweep();

private:
	float CurrentYawDeg = 0.0f;
	float PingPongDirection = 1.0f;
	float CurrentBeamClipDistance = 45000.0f;

	void ApplyAllSettings();
	void ApplySweepTransform();
	void ApplyLightSettings();
	void ApplyBeamVisualSettings();
	void ApplyBeamMeshTransform(UStaticMeshComponent* MeshComponent, float VisualLength, float VisualRadius, float InBeamStartOffset) const;
	void ApplyLensVisualSettings();
	void ApplyColorMaterialParameters(UStaticMeshComponent* MeshComponent, FLinearColor Color, float Opacity, float EmissiveStrength) const;
	void UpdateBeamCollision(float DeltaSeconds);
	float ResolveBeamLengthFromCollision();
	bool ShouldIgnoreBeamHit(const FHitResult& Hit) const;
	void AdvanceSweep(float DeltaSeconds);
};
