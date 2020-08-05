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
#include "GameFramework/Pawn.h"
#include "SpatialDataDeserializer.h"

#include "SCTReplaySkeletonPawn.generated.h"

UCLASS()
class SCT_API ASCTReplaySkeletonPawn : public APawn
{
	GENERATED_BODY()

public:
	ASCTReplaySkeletonPawn();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(Config, EditAnywhere, Category = "Spatial Settings", meta = (FilePathFilter = "dat", AbsolutePath))
		FFilePath FileNamePath;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Spatial Settings")
		bool bLoop = false;

	UFUNCTION(BlueprintCallable, Category = Logic)
		void Start();

	FTransform GetRelativeTransformByIndex(int index);
	FTransform GetCameraTransform();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	TArray<uint8> FileBuffer;
	kh::FSpatialDataDeserializer SpatialData;
	bool bRunning = false;
};
