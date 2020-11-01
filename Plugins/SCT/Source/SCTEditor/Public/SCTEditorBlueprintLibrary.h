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
#include "Kismet/BlueprintFunctionLibrary.h"

#include "SCTEditorBlueprintLibrary.generated.h"

class FMRSerializeFromBuffer;
class USCTSpatialCameraAsset;
struct FSCTSkeletonDefinition;

UCLASS()
class SCTEDITOR_API USCTEditorBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Editor Scripting | SCT", meta = (DevelopmentOnly))
	static void ImportEnvironmentProbes();

	UFUNCTION(BlueprintCallable, Category = "Editor Scripting | SCT", meta = (DevelopmentOnly))
	static void ImportSpatialCamera();

	UFUNCTION(BlueprintCallable, Category = "Editor Scripting | SCT", meta = (DevelopmentOnly))
	static void ImportSpatialSkeleton();

private:
	struct FSpatialHeader
	{
		int32 Version;
		int32 FrameCount;
		int32 DeviceOrientation;
		float HorizontalFOV;
		float VerticalFOV;
		float FocalLengthX;
		float FocalLengthY;
		int CaptureType;
	};

	static void ReadHeaderFromBuffer(FMRSerializeFromBuffer& FromBuffer, FSpatialHeader& Header);
	static void ReadUserAnchorsFromBuffer(FMRSerializeFromBuffer& FromBuffer, TArray<FVector>& UserAnchors);
	static void ReadSkeletonDefinitionFromBuffer(FMRSerializeFromBuffer& FromBuffer, FSCTSkeletonDefinition& SkeletonDefinition);

	static void PopulateSpatialCameraAsset(USCTSpatialCameraAsset* Asset, const FSpatialHeader& Header, const TArray<FVector>& UserAnchors, const TArray<uint8>& FrameData);
	
	static void ReadFileWithDialog(const FString& Title, const FString& FileTypes, const FString& DefaultFileName, TArray<uint8>& FileBuffer);
	static bool ChooseSaveLocationWithDialog(const FString& Title, const FString& FileTypes, const FString& DefaultFileName, FString& FileName);
};
