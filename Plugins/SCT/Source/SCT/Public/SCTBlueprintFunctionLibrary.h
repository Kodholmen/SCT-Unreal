/*
MIT License

Copyright (c) 2020 Kodholmen

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#pragma once

#include "CoreMinimal.h"
#include "SCTReplaySkeletonPawn.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SCTBlueprintFunctionLibrary.generated.h"


UENUM(BlueprintType)
enum class EJointIndex : uint8
{
	Root,
	Pelvis,
	Spine_01,
	Spine_02,
	Spine_03,
	Neck_01,
	Head,
	Clavicle_l,
	Upperarm_l,
	Lowerarm_l,
	Hand_l,
	Clavicle_r,
	Upperarm_r,
	Lowerarm_r,
	Hand_r,
	Thigh_l,
	Calf_l,
	Foot_l,
	Thigh_r,
	Calf_r,
	Foot_r
};

/**
 * 
 */
UCLASS()
class SCT_API USCTBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	UFUNCTION(BlueprintCallable, Category = "Skeleton")
	static FVector GetJointLocationFromPawnByEnum(ASCTReplaySkeletonPawn* pawn, EJointIndex Joint);
	UFUNCTION(BlueprintCallable, Category = "Skeleton")
	static FTransform GetCameraTransformFromPawn(ASCTReplaySkeletonPawn* pawn);
};
