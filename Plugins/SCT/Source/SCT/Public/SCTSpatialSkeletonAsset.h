// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SCTSpatialCameraAsset.h"
#include "SCTSpatialSkeletonAsset.generated.h"

USTRUCT()
struct FSCTSkeletonDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Data")
	TArray<FName> JointNames;
	UPROPERTY(EditDefaultsOnly, Category = "Data")
	TArray<int32> ParentIndices;
	UPROPERTY(EditDefaultsOnly, Category = "Data")
	TArray<FTransform> NeutralTransforms;
};

/**
 * 
 */
UCLASS()
class SCT_API USCTSpatialSkeletonAsset : public USCTSpatialCameraAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, Category = "Data")
	FSCTSkeletonDefinition SkeletonDefinition;
};
