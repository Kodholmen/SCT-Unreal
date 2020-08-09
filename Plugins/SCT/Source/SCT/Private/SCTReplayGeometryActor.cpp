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
#include "SCTReplayGeometryActor.h"

#define LOCTEXT_NAMESPACE "FSCTLiveLinkModule"
DEFINE_LOG_CATEGORY_STATIC(SCTReplayGeometryActor, Log, All);

// Sets default values
ASCTReplayGeometryActor::ASCTReplayGeometryActor()
	: CurrentTick(0)
	, NextActionTick(0)
{
	PrimaryActorTick.bCanEverTick = true;
	Mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("SCTSceneMesh"));
	SetRootComponent(Mesh);
}

// Called when the game starts or when spawned
void ASCTReplayGeometryActor::BeginPlay()
{
	Super::BeginPlay();

	PrimaryActorTick.TickInterval = 1.0f / 60.0f;

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	IFileHandle* File = PlatformFile.OpenRead(*FileNamePath.FilePath);
	if (File)
	{
		UE_LOG(SCTReplayGeometryActor, Display, TEXT("[SCT ReplayGeometry] Opened Replay File with size: %d"), File->Size());

		FileBuffer.AddZeroed(File->Size());
		File->Read(FileBuffer.GetData(), File->Size());
		FromBuffer.Init(FileBuffer.GetData(), FileBuffer.Num());
	}
}

// Called every frame
void ASCTReplayGeometryActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (FromBuffer.HasOverflow())
	{
		//Reset?
		return;
	}

	if (CurrentTick == NextActionTick)
	{
		int PartCount = 0;
		FromBuffer >> PartCount;

		if (PartCount > 0)
			Mesh->ClearAllMeshSections();

		for (int p = 0; p < PartCount; ++p)
		{
			ReadMeshPartFromStream(p);
		}

		NextActionTick += 60;
	}

	++CurrentTick;
}

void ASCTReplayGeometryActor::ReadMeshPartFromStream(int Section)
{
	int64 VertCount;
	FromBuffer >> VertCount;
	TArray<FVector> Vertices; 
	Vertices.InsertDefaulted(0, VertCount);
	for (int v = 0; v < VertCount; ++v)
	{
		FVector Vert;
		FromBuffer >> Vert;

		Vert = FVector(-Vert.Z, Vert.X, Vert.Y) * 100.0f;
		Vertices[v]=Vert;
	}

	int64 IndicesCount;
	FromBuffer >> IndicesCount;
	TArray<int32> Indices;
	Indices.InsertDefaulted(0, IndicesCount);
	for (int i = 0; i < IndicesCount; ++i)
	{
		uint32 Index;
		FromBuffer >> Index;
		Indices[i] = (int32)Index;
	}

	TArray<FVector> Normals; Normals.InsertDefaulted(0, VertCount);
	TArray<FVector2D> Uv0; Uv0.InsertDefaulted(0, VertCount);
	TArray<FLinearColor> VertexColors; VertexColors.InsertDefaulted(0, VertCount);
	TArray<FProcMeshTangent> Tangents;// tangents.InsertDefaulted(0, bfmesh.vertnum);

	Mesh->CreateMeshSection_LinearColor(Section, Vertices, Indices, Normals, Uv0, VertexColors, Tangents, false);
}

#undef LOCTEXT_NAMESPACE
