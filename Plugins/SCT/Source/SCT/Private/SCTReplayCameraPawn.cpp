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
#include "SCTReplayCameraPawn.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "HAL/PlatformFilemanager.h"

#define LOCTEXT_NAMESPACE "FSCTLiveLinkModule"
DEFINE_LOG_CATEGORY_STATIC(SCTReplayCameraPawn, Log, All);

ASCTReplayCameraPawn::ASCTReplayCameraPawn()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ASCTReplayCameraPawn::Start()
{
	bRunning = true;
}

void ASCTReplayCameraPawn::BeginPlay()
{
	Super::BeginPlay();

	PrimaryActorTick.TickInterval = 1.0f / 60.0f;

	if (CameraDataAsset)
	{
		SpatialData.InitWithCameraAsset(CameraDataAsset);
	}
}

void ASCTReplayCameraPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bRunning == false)
		return;

	SpatialData.DeserialiseCamera();

	FTransform CameraTransform = SpatialData.GetCameraTransform();
	SetActorRelativeTransform(CameraTransform);

	SpatialData.StepFrame(bLoop);
}

void ASCTReplayCameraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

#undef LOCTEXT_NAMESPACE
