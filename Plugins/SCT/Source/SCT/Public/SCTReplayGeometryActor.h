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
#include "ProceduralMeshComponent.h"
#include "GameFramework/Actor.h"

#include "SCTSerializeFromBuffer.h"
#include "SCTReplayGeometryActor.generated.h"

UCLASS()
class SCT_API ASCTReplayGeometryActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ASCTReplayGeometryActor();
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(Config, EditAnywhere, Category = "Spatial Settings", meta = (FilePathFilter = "dat", AbsolutePath))
	FFilePath FileNamePath;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, Category = SCT)
	UProceduralMeshComponent* Mesh;

	void ReadMeshPartFromStream(int Section);

	int CurrentTick;
	int NextActionTick;
	TArray<uint8> FileBuffer;
	FMRSerializeFromBuffer FromBuffer;
};
