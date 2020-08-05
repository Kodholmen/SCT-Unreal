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
#include "SCTLiveLinkSource.h"
#include "ILiveLinkClient.h"

#include "Misc/FrameRate.h"
#include "Roles/LiveLinkTransformRole.h"
#include "Roles/LiveLinkTransformTypes.h"
#include "Roles/LiveLinkAnimationTypes.h"
#include "Roles/LiveLinkAnimationRole.h"

#define LOCTEXT_NAMESPACE "FSCTLiveLinkModule"
DEFINE_LOG_CATEGORY_STATIC(LogSCTLiveLinkSource, Log, All);

// TEMP
static bool bIsBodyTracking = false;
TArray<uint8> FileBuffer;

FSCTLiveLinkSource::FSCTLiveLinkSource()
{
	// Live link params
	SourceStatus = LOCTEXT("SourceStatus_Active", "Inactive");
	SourceType = LOCTEXT("SCTLiveLinkSourceType", "SCT LiveLink");
	SourceMachineName = LOCTEXT("SCTLiveLinkSourceMachineName", "localhost");

	// TEMP LOAD CAPTURE FROM FILE
	FString FileName = FString("E:\\projects\\private\\MovieReality\\arkitcapture.dat");
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	IFileHandle* File = PlatformFile.OpenRead(*FileName);
	if (File)
	{
		UE_LOG(LogSCTLiveLinkSource, Display, TEXT("[SCT LIVELINK] Opened Replay File with size: %d"), File->Size());

		FileBuffer.AddZeroed(File->Size());
		File->Read(FileBuffer.GetData(), File->Size());
		SpatialData.InitWithBuffer(FileBuffer);
		SpatialData.DeserializeHeader();
	}

	// Tick delegate
	TickDelegate = FTickerDelegate::CreateRaw(this, &FSCTLiveLinkSource::Tick);
	TickDelegateHandle = FTicker::GetCoreTicker().AddTicker(TickDelegate, 1.0f / 60.0f);
}

FSCTLiveLinkSource::~FSCTLiveLinkSource()
{
	FTicker::GetCoreTicker().RemoveTicker(TickDelegateHandle);
}

void FSCTLiveLinkSource::ReceiveClient(ILiveLinkClient* InClient, FGuid InSourceGuid)
{
	UE_LOG(LogSCTLiveLinkSource, Display, TEXT("[MR LIVELINK] Received Client"));

	LiveLinkClient = InClient;
	LiveLinkSourceGuid = InSourceGuid;
}

bool FSCTLiveLinkSource::IsSourceStillValid() const
{
	return LiveLinkClient != nullptr;
}

bool FSCTLiveLinkSource::RequestSourceShutdown()
{
	LiveLinkClient = nullptr;
	LiveLinkSourceGuid.Invalidate();
	return true;
}

FText FSCTLiveLinkSource::GetSourceType() const
{ 
	return SourceType; 
};

FText FSCTLiveLinkSource::GetSourceMachineName() const
{ 
	return SourceMachineName; 
}

FText FSCTLiveLinkSource::GetSourceStatus() const
{ 
	return SourceStatus;
}

void FSCTLiveLinkSource::PopulateBasePropertyNames(TArray<FName>& Names)
{
	Names.Empty();
	//Names.Add(FName("SomeProperty"));
}

void FSCTLiveLinkSource::UpdateBaseStaticData(FLiveLinkBaseStaticData& InOutData)
{
	PopulateBasePropertyNames(BasePropertyNames); //Populate BASE property names array
	InOutData.PropertyNames = BasePropertyNames;
}

void FSCTLiveLinkSource::UpdateSkeletonStaticData(FLiveLinkSkeletonStaticData& InOutData)
{
	SpatialData.DeserializeSkeletonDefinition();
	const kh::FSkeletonDefinition& Def = SpatialData.GetSkeletonDefinition();
	InOutData.SetBoneNames(Def.JointNames);
	InOutData.SetBoneParents(Def.ParentIndices);
}

void FSCTLiveLinkSource::UpdateBaseFrameData(FLiveLinkBaseFrameData& InOutData, float DeltaTime)
{
	InOutData.WorldTime = FPlatformTime::Seconds();
	//InOutData.MetaData.SceneTime = FQualifiedFrameTime(1212, FFrameRate(60, 1));
	InOutData.PropertyValues.Reserve(BasePropertyNames.Num());
	//InOutData.PropertyValues.Add(334433.0f);
}

void FSCTLiveLinkSource::UpdateTransformFrameData(FLiveLinkTransformFrameData& InOutData, float DeltaTime)
{
	UpdateBaseFrameData(InOutData, DeltaTime);
	SpatialData.DeserialiseCamera();
	InOutData.Transform = SpatialData.GetCameraTransform();
}

void FSCTLiveLinkSource::UpdateSkeletonFrameData(FLiveLinkAnimationFrameData& InOutData, float DeltaTime)
{
	UpdateBaseFrameData(InOutData, DeltaTime);

	SpatialData.DeserialiseSkeleton();
	TArray<FTransform> BoneTransforms;
	const kh::FSkeletonDefinition& SkeletonDefinition = SpatialData.GetSkeletonDefinition();
	const kh::FSkeletonTransforms& SkeletonTransforms = SpatialData.GetSkeletonTransforms();

	for (int i = 0, e = SkeletonTransforms.Transforms.Num(); i < e; ++i)
	{
		int ParentIdx = SkeletonDefinition.ParentIndices[i];
		check(ParentIdx >= -1 && ParentIdx < 21);

		FTransform Parent = ParentIdx == -1 ? FTransform::Identity : SkeletonTransforms.Transforms[ParentIdx];
		FTransform Child = SkeletonTransforms.Transforms[i];

		check(Child.IsValid());
		check(Parent.IsValid());

		FName JointName = SkeletonDefinition.JointNames[i];
		FTransform RelativeTransform = Child * Parent.Inverse();

		BoneTransforms.Add(RelativeTransform);

		//BoneTransforms.Add(*SkeletonTransforms.Transforms[i]);
	}
	InOutData.Transforms = BoneTransforms;
}

bool FSCTLiveLinkSource::Tick(float DeltaTime)
{
	// Publish any roles and their properties
	if (bIsInitialized == false && LiveLinkClient)
	{
		UE_LOG(LogSCTLiveLinkSource, Display, TEXT("[SCT LIVELINK] Initializing subject"));
		// This code touches UObjects so needs to be run only in the game thread
		check(IsInGameThread());

		if (bIsBodyTracking)
		{
			const FLiveLinkSubjectKey SubjectKey(LiveLinkSourceGuid, FName("Skeleton Transforms"));
			FLiveLinkStaticDataStruct StaticDataStruct(FLiveLinkSkeletonStaticData::StaticStruct()); // Create SKELETON static struct
			UpdateSkeletonStaticData(*StaticDataStruct.Cast<FLiveLinkSkeletonStaticData>()); // Populate the SKELETON bonen names and parents
			LiveLinkClient->PushSubjectStaticData_AnyThread(SubjectKey, ULiveLinkAnimationRole::StaticClass(), MoveTemp(StaticDataStruct)); // Publish SKELETON role
		}

		{
			const FLiveLinkSubjectKey SubjectKey(LiveLinkSourceGuid, FName("Camera Transform"));
			FLiveLinkStaticDataStruct StaticDataStruct(FLiveLinkTransformStaticData::StaticStruct()); // Create TRANSFORM static struct
			UpdateBaseStaticData(*StaticDataStruct.Cast<FLiveLinkTransformStaticData>()); // Populate the BASE Property names and save to struct
			LiveLinkClient->PushSubjectStaticData_AnyThread(SubjectKey, ULiveLinkTransformRole::StaticClass(), MoveTemp(StaticDataStruct)); // Publish BASIC role
		}

		UE_LOG(LogSCTLiveLinkSource, Display, TEXT("[SCT LIVELINK] Pushed Subject"));

		bIsInitialized = true;

		SourceStatus = LOCTEXT("SourceStatus_Active", "Active");

		return true;
	}

	UpdateLiveLink(DeltaTime);
	return true;
}

void FSCTLiveLinkSource::UpdateLiveLink(float DeltaTime)
{
	// Push SKELETON data to the link
	if (bIsBodyTracking)
	{
		const FLiveLinkSubjectKey SubjectKey(LiveLinkSourceGuid, FName("Skeleton Transforms"));
		FLiveLinkFrameDataStruct FrameDataStruct(FLiveLinkAnimationFrameData::StaticStruct());
		UpdateSkeletonFrameData(*FrameDataStruct.Cast<FLiveLinkAnimationFrameData>(), DeltaTime);
		LiveLinkClient->PushSubjectFrameData_AnyThread(SubjectKey, MoveTemp(FrameDataStruct));
	}

	//Push TRANSFORM data to the link
	{
		const FLiveLinkSubjectKey SubjectKey(LiveLinkSourceGuid, FName("Camera Transform"));
		FLiveLinkFrameDataStruct FrameDataStruct(FLiveLinkTransformFrameData::StaticStruct());
		UpdateTransformFrameData(*FrameDataStruct.Cast<FLiveLinkTransformFrameData>(), DeltaTime);
		LiveLinkClient->PushSubjectFrameData_AnyThread(SubjectKey, MoveTemp(FrameDataStruct));
	}

	if (SpatialData.StepFrame())
	{
		SpatialData.DeserializeHeader();

		if (bIsBodyTracking)
		{
			SpatialData.DeserializeSkeletonDefinition();
		}
	}
}

#undef LOCTEXT_NAMESPACE
