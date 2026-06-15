#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CitySweepLightActor.generated.h"

class UMaterialInterface;
class USceneComponent;
class USpotLightComponent;
class UStaticMesh;
class UStaticMeshComponent;

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
	bool bCastLightShadows = false;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Visual")
	TObjectPtr<UStaticMesh> BeamMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Visual")
	TObjectPtr<UMaterialInterface> BeamMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Visual", meta = (ClampMin = "1.0"))
	float BeamLength = 45000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Visual", meta = (ClampMin = "1.0"))
	float BeamRadius = 2200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Visual")
	float BeamStartOffset = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Visual")
	FRotator BeamMeshRotationOffset = FRotator(-90.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Visual", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BeamOpacity = 0.16f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Visual", meta = (ClampMin = "0.0"))
	float BeamEmissiveStrength = 1.8f;

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

	void ApplyAllSettings();
	void ApplySweepTransform();
	void ApplyLightSettings();
	void ApplyBeamVisualSettings();
	void ApplyLensVisualSettings();
	void ApplyColorMaterialParameters(UStaticMeshComponent* MeshComponent, FLinearColor Color, float Opacity, float EmissiveStrength) const;
	void AdvanceSweep(float DeltaSeconds);
};
