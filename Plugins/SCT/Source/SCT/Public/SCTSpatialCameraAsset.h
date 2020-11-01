// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SCTSpatialCameraAsset.generated.h"

/**
 * 
 */
UCLASS()
class SCT_API USCTSpatialCameraAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Header")
	int32 Version;
	UPROPERTY(EditDefaultsOnly, Category = "Header")
	int32 FrameCount;
	UPROPERTY(EditDefaultsOnly, Category = "Header")
	int32 DeviceOrientation;
	UPROPERTY(EditDefaultsOnly, Category = "Header")
	float HorizontalFOV;
	UPROPERTY(EditDefaultsOnly, Category = "Header")
	float VerticalFOV;
	UPROPERTY(EditDefaultsOnly, Category = "Header")
	float FocalLengthX;
	UPROPERTY(EditDefaultsOnly, Category = "Header")
	float FocalLengthY;
	UPROPERTY(EditDefaultsOnly, Category = "Header")
	int CaptureType;
	UPROPERTY(EditDefaultsOnly, Category = "Header")
	TArray<FVector> UserAnchors;

	UPROPERTY(EditDefaultsOnly, Category = "Data")
	TArray<uint8> FrameData;
};
