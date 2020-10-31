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
#include "SCTReplaySkeletonPawn.h"
#include "DrawDebugHelpers.h"

#include "GenericPlatform/GenericPlatformFile.h"
#include "HAL/PlatformFilemanager.h"


#define LOCTEXT_NAMESPACE "FSCTLiveLinkModule"
DEFINE_LOG_CATEGORY_STATIC(SCTReplaySkeletonPawn, Log, All);

ASCTReplaySkeletonPawn::ASCTReplaySkeletonPawn()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ASCTReplaySkeletonPawn::Start()
{
	bRunning = true;
}

void ASCTReplaySkeletonPawn::BeginPlay()
{
	Super::BeginPlay();

	PrimaryActorTick.TickInterval = 1.0f / 60.0f;

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	IFileHandle* File = PlatformFile.OpenRead(*FileNamePath.FilePath);
	if (File)
	{
		UE_LOG(SCTReplaySkeletonPawn, Display, TEXT("[SCT ReplaySkeleton] Opened Replay File with size: %d"), File->Size());
 
		FileBuffer.AddZeroed(File->Size());
		File->Read(FileBuffer.GetData(), File->Size());
		SpatialData.InitWithBuffer(FileBuffer);
		SpatialData.DeserializeHeader();
		SpatialData.DeserializeUserAnchors();
		SpatialData.DeserializeSkeletonDefinition();
	}
}

void ASCTReplaySkeletonPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bRunning == false)
		return;

	SpatialData.DeserialiseSkeleton();
	SpatialData.DeserialiseCamera();

	const kh::FSkeletonTransforms& SkeletonTransforms = SpatialData.GetSkeletonTransforms();
	for (int i = 0, e = SkeletonTransforms.Transforms.Num(); i < e; ++i)
	{
		FTransform JointTrans = SkeletonTransforms.Transforms[i];
		DrawDebugSphere(GetWorld(), JointTrans.GetLocation(), 5.0f, 12, FColor::White, true);
	}

	//FTransform CameraTransform = SpatialData.GetCameraTransform();
	//CameraAnchor->SetRelativeTransform(CameraTransform);


	if (SpatialData.StepFrame(bLoop))
	{
		SpatialData.DeserializeHeader();
		SpatialData.DeserializeUserAnchors();
		SpatialData.DeserializeSkeletonDefinition();
	}
}

void ASCTReplaySkeletonPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

FTransform ASCTReplaySkeletonPawn::GetRelativeTransformByIndex(int index)
{
	const kh::FSkeletonTransforms& SkeletonTransforms = SpatialData.GetSkeletonTransforms();

	if (bRunning == false || SkeletonTransforms.Transforms.Num() == 0)
		return FTransform::Identity;

	check(index >= 0 && index < SkeletonTransforms.Transforms.Num());
	FTransform RelativePart = SkeletonTransforms.Transforms[index] * SkeletonTransforms.Transforms[0];
	return RelativePart * GetActorTransform();
}

FTransform ASCTReplaySkeletonPawn::GetCameraTransform()
{
	return SpatialData.GetCameraTransform() * GetActorTransform();
}

#undef LOCTEXT_NAMESPACE
