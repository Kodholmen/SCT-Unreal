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
#include "SCTEditorBlueprintLibrary.h"

#include "SCTSerializeFromBuffer.h"
#include "SCTSpatialCameraAsset.h"
#include "SCTSpatialSkeletonAsset.h"
#include "EditorLevelLibrary.h"
#include "Engine/BoxReflectionCapture.h"

#include "DesktopPlatform/Public/IDesktopPlatform.h"
#include "DesktopPlatform/Public/DesktopPlatformModule.h"

#include "AssetRegistryModule.h"
#include "Misc/PackageName.h"

DEFINE_LOG_CATEGORY_STATIC(SCTEditorBlueprintLibrary, Log, All);

void USCTEditorBlueprintLibrary::ImportEnvironmentProbes()
{
	const FString Title = TEXT("Import Environment Anchors");
	const FString FileTypes = TEXT("SCT Data (*.dat)|*.dat");
	TArray<uint8> FileBuffer;

	ReadFileWithDialog(Title, FileTypes, "environmentprobes.dat", FileBuffer);

	if (FileBuffer.Num())
	{
		FMRSerializeFromBuffer FromBuffer;
		FromBuffer.Init(FileBuffer.GetData(), FileBuffer.Num());

		int32 AnchorCount;
		FromBuffer >> AnchorCount;

		for (int i = 0; i < AnchorCount; ++i)
		{
			FVector Extent;
			FromBuffer >> Extent;

			Extent *= 100.0f;
			if (FMath::IsFinite(Extent.X) == false)
				Extent.X = 100000.0f;
			if (FMath::IsFinite(Extent.Y) == false)
				Extent.Y = 100000.0f;
			if (FMath::IsFinite(Extent.Z) == false)
				Extent.Z = 100000.0f;

			FPlane C0;
			FPlane C1;
			FPlane C2;
			FPlane C3;
			FromBuffer >> C0;
			FromBuffer >> C1;
			FromBuffer >> C2;
			FromBuffer >> C3;

			FMatrix RawYUpFMatrix(C0, C1, C2, C3);
			// Extract & convert rotation
			FQuat RawRotation(RawYUpFMatrix);
			FQuat Rotation(-RawRotation.Z, RawRotation.X, RawRotation.Y, -RawRotation.W);

			FTransform Transform;
			Transform.SetLocation(FVector(-RawYUpFMatrix.M[3][2], RawYUpFMatrix.M[3][0], RawYUpFMatrix.M[3][1]) * 100.0f);
			Transform.SetRotation(Rotation);

			FVector Pos = Transform.GetLocation();
			UE_LOG(SCTEditorBlueprintLibrary, Display, TEXT("[SCT Editor Blueprint] Read anchor: pos %f/%f/%f, extent: pos %f/%f/%f"), Pos.X, Pos.Y, Pos.Z, Extent.X, Extent.Y, Extent.Z);

			AActor* cube = UEditorLevelLibrary::SpawnActorFromClass(ABoxReflectionCapture::StaticClass(), Pos);
			cube->SetActorScale3D(Extent);
		}
	}

}

void USCTEditorBlueprintLibrary::ImportSpatialCamera()
{
	TArray<uint8> FileBuffer;
	{
		const FString Title = TEXT("Import Spatial Camera");
		const FString FileTypes = TEXT("SCT Data (*.dat)|*.dat");
		ReadFileWithDialog(Title, FileTypes, "capture.dat", FileBuffer);
	}

	if (FileBuffer.Num() == 0)
		return;

	FMRSerializeFromBuffer FromBuffer;
	FromBuffer.Init(FileBuffer.GetData(), FileBuffer.Num());

	// Header
	FSpatialHeader Header;
	ReadHeaderFromBuffer(FromBuffer, Header);

	// User Anchors
	TArray<FVector> UserAnchors;
	ReadUserAnchorsFromBuffer(FromBuffer, UserAnchors);

	// Frame Data
	TArray<uint8> FrameData;
	FromBuffer >> FrameData;
	UE_LOG(SCTEditorBlueprintLibrary, Display, TEXT("[SCT Editor Blueprint] Read frame data: %d"), FrameData.Num());

	FString AssetFileName = "";
	{
		const FString Title = TEXT("Save Spatial Camera Asset");
		const FString FileTypes = TEXT("Data Asset (*.uasset)|*.uasset");
		if (ChooseSaveLocationWithDialog(Title, FileTypes, "Capture.uasset", AssetFileName) == false)
			return;
	}

	// Create Data Asset
	FString PackageName  = FPackageName::FilenameToLongPackageName(AssetFileName);
	FString ShortName = FPackageName::GetShortName(PackageName);
	UPackage* Package = CreatePackage(nullptr, *PackageName);
	USCTSpatialCameraAsset* Asset = NewObject<USCTSpatialCameraAsset>(Package, USCTSpatialCameraAsset::StaticClass(), *ShortName, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone);

	PopulateSpatialCameraAsset(Asset, Header, UserAnchors, FrameData);

	FAssetRegistryModule::AssetCreated(Asset);
	Asset->MarkPackageDirty();
	UPackage::SavePackage(Package, Asset, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *AssetFileName);
}

void USCTEditorBlueprintLibrary::ImportSpatialSkeleton()
{
	TArray<uint8> FileBuffer;
	{
		const FString Title = TEXT("Import Spatial Skeleton");
		const FString FileTypes = TEXT("SCT Data (*.dat)|*.dat");
		ReadFileWithDialog(Title, FileTypes, "capture.dat", FileBuffer);
	}

	if (FileBuffer.Num() == 0)
		return;

	FMRSerializeFromBuffer FromBuffer;
	FromBuffer.Init(FileBuffer.GetData(), FileBuffer.Num());

	// Header
	FSpatialHeader Header;
	ReadHeaderFromBuffer(FromBuffer, Header);

	// User Anchors
	TArray<FVector> UserAnchors;
	ReadUserAnchorsFromBuffer(FromBuffer, UserAnchors);

	// Skeleton Definition
	FSCTSkeletonDefinition SkeletonDefinition;
	ReadSkeletonDefinitionFromBuffer(FromBuffer, SkeletonDefinition);

	// Frame Data
	TArray<uint8> FrameData;
	FromBuffer >> FrameData;
	UE_LOG(SCTEditorBlueprintLibrary, Display, TEXT("[SCT Editor Blueprint] Read frame data: %d"), FrameData.Num());

	FString AssetFileName = "";
	{
		const FString Title = TEXT("Save Spatial Skeleton Asset");
		const FString FileTypes = TEXT("Data Asset (*.uasset)|*.uasset");
		if (ChooseSaveLocationWithDialog(Title, FileTypes, "SkeletonCapture.uasset", AssetFileName) == false)
			return;
	}

	// Create Data Asset
	FString PackageName = FPackageName::FilenameToLongPackageName(AssetFileName);
	FString ShortName = FPackageName::GetShortName(PackageName);
	UPackage* Package = CreatePackage(nullptr, *PackageName);
	USCTSpatialSkeletonAsset* Asset = NewObject<USCTSpatialSkeletonAsset>(Package, USCTSpatialSkeletonAsset::StaticClass(), *ShortName, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone);

	PopulateSpatialCameraAsset(Asset, Header, UserAnchors, FrameData);
	Asset->SkeletonDefinition = SkeletonDefinition;

	FAssetRegistryModule::AssetCreated(Asset);
	Asset->MarkPackageDirty();
	UPackage::SavePackage(Package, Asset, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *AssetFileName);
}

void USCTEditorBlueprintLibrary::ReadHeaderFromBuffer(FMRSerializeFromBuffer& FromBuffer, FSpatialHeader& Header)
{
	FromBuffer >> Header.Version;
	FromBuffer >> Header.FrameCount;
	FromBuffer >> Header.DeviceOrientation;
	FromBuffer >> Header.HorizontalFOV;
	FromBuffer >> Header.VerticalFOV;
	FromBuffer >> Header.FocalLengthX;
	FromBuffer >> Header.FocalLengthY;
	FromBuffer >> Header.CaptureType;

	check(Header.Version == 202005 && "Version Mismatch. Make sure your plugin and App versions match");
}

void USCTEditorBlueprintLibrary::ReadUserAnchorsFromBuffer(FMRSerializeFromBuffer& FromBuffer, TArray<FVector>& UserAnchors)
{
	int32 AnchorCount = 0;
	FromBuffer >> AnchorCount;

	for (int32 i = 0; i < AnchorCount; ++i)
	{
		FVector Pos = FVector::ZeroVector;
		FromBuffer >> Pos;

		UserAnchors.Add(Pos);
	}
}

void USCTEditorBlueprintLibrary::ReadSkeletonDefinitionFromBuffer(FMRSerializeFromBuffer& FromBuffer, FSCTSkeletonDefinition& SkeletonDefinition)
{
	SkeletonDefinition.JointNames.Empty();
	SkeletonDefinition.ParentIndices.Empty();
	SkeletonDefinition.NeutralTransforms.Empty();

	int JointCount = 0;
	FromBuffer >> JointCount;

	for (int i = 0; i < JointCount; ++i)
	{
		FString JointName;
		FromBuffer >> JointName;

		SkeletonDefinition.JointNames.Add(FName(JointName));
	}

	int ParentCount = 0;
	FromBuffer >> ParentCount;

	for (int i = 0; i < ParentCount; ++i)
	{
		int32 ParentIdx = -1;
		FromBuffer >> ParentIdx;

		SkeletonDefinition.ParentIndices.Add(ParentIdx);
	}

	for (int i = 0; i < JointCount; ++i)
	{
		FTransform Trans;
		FromBuffer >> Trans;
		SkeletonDefinition.NeutralTransforms.Add(Trans);
	}
}

void USCTEditorBlueprintLibrary::PopulateSpatialCameraAsset(USCTSpatialCameraAsset* Asset, const FSpatialHeader& Header, const TArray<FVector>& UserAnchors, const TArray<uint8>& FrameData)
{
	Asset->Version = Header.Version;
	Asset->FrameCount = Header.FrameCount;
	Asset->DeviceOrientation = Header.DeviceOrientation;
	Asset->HorizontalFOV = Header.HorizontalFOV;
	Asset->VerticalFOV = Header.VerticalFOV;
	Asset->FocalLengthX = Header.FocalLengthX;
	Asset->FocalLengthY = Header.FocalLengthY;
	Asset->CaptureType = Header.CaptureType;

	Asset->UserAnchors = UserAnchors;
	Asset->FrameData = FrameData;
}

void USCTEditorBlueprintLibrary::ReadFileWithDialog(const FString& Title, const FString& FileTypes, const FString& DefaultFileName, TArray<uint8>& FileBuffer)
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	const void* ParentWindowWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);

	TArray<FString> OutFilenames;
	DesktopPlatform->OpenFileDialog(
		ParentWindowWindowHandle,
		Title,
		TEXT(""),
		*DefaultFileName,
		FileTypes,
		EFileDialogFlags::None,
		OutFilenames
	);

	if (OutFilenames.Num() == 0)
	{
		return;
	}

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	IFileHandle* File = PlatformFile.OpenRead(*OutFilenames[0]);
	if (File)
	{
		UE_LOG(SCTEditorBlueprintLibrary, Display, TEXT("[SCT Editor Blueprint] Opened File with size: %d"), File->Size());

		FileBuffer.AddZeroed(File->Size());
		File->Read(FileBuffer.GetData(), File->Size());
	}
}

bool USCTEditorBlueprintLibrary::ChooseSaveLocationWithDialog(const FString& Title, const FString& FileTypes, const FString& DefaultFileName, FString& FileName)
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	const void* ParentWindowWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);

	TArray<FString> OutFilenames;
	DesktopPlatform->SaveFileDialog(
		ParentWindowWindowHandle,
		Title,
		TEXT(""),
		*DefaultFileName,
		FileTypes,
		EFileDialogFlags::None,
		OutFilenames
	);

	if (OutFilenames.Num() == 0)
	{
		return false;
	}

	FileName = OutFilenames[0];
	return true;
}
