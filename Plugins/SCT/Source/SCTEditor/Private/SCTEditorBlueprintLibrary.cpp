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
#include "EditorLevelLibrary.h"
#include "Engine/BoxReflectionCapture.h"

#include "DesktopPlatform/Public/IDesktopPlatform.h"
#include "DesktopPlatform/Public/DesktopPlatformModule.h"


DEFINE_LOG_CATEGORY_STATIC(SCTEditorBlueprintLibrary, Log, All);

void USCTEditorBlueprintLibrary::ImportEnvironmentProbes()
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	const void* ParentWindowWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);

	const FString Title = TEXT("Import Environment Anchors");
	const FString FileTypes = TEXT("SCT Data (*.dat)|*.dat");

	TArray<FString> OutFilenames;
	DesktopPlatform->OpenFileDialog(
		ParentWindowWindowHandle,
		Title,
		TEXT(""),
		TEXT("environment.dat"),
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
		UE_LOG(SCTEditorBlueprintLibrary, Display, TEXT("[SCT Editor Blueprint] Opened Replay File with size: %d"), File->Size());

		TArray<uint8> FileBuffer;
		FMRSerializeFromBuffer FromBuffer;

		FileBuffer.AddZeroed(File->Size());
		File->Read(FileBuffer.GetData(), File->Size());
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

