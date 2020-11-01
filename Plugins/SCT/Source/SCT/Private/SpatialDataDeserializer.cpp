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
#include "SpatialDataDeserializer.h"

DEFINE_LOG_CATEGORY_STATIC(LogSpatialDataDeserializer, Log, All);

namespace kh
{
	FSpatialDataDeserializer::FSpatialDataDeserializer()
		: bShouldDeserialize(true)
		, CurrFrame(0)
		, FrameCount(0)
		, DeviceOrientation(0)
	{
		CameraTransform.SetLocation(FVector::ZeroVector);
		CameraTransform.SetRotation(FQuat::Identity);
		CameraTransform.SetScale3D(FVector::OneVector);
	}

	FSpatialDataDeserializer::~FSpatialDataDeserializer()
	{
	}

	void FSpatialDataDeserializer::InitWithCameraAsset(class USCTSpatialCameraAsset* Asset)
	{
		FrameCount = Asset->FrameCount;
		DeviceOrientation = Asset->DeviceOrientation;
		FromBuffer.Init(Asset->FrameData.GetData(), Asset->FrameData.Num());
	}

	void FSpatialDataDeserializer::InitWithSkeletonAsset(USCTSpatialSkeletonAsset* Asset)
	{
		InitWithCameraAsset(Asset);
		SkeletonDefinition = Asset->SkeletonDefinition;

		SkeletonTransforms.Transforms.AddDefaulted(SkeletonDefinition.JointNames.Num());
	}

	void FSpatialDataDeserializer::DeserialiseCamera()
	{
		if (bShouldDeserialize == false)
			return;

		FVector Pos = FVector::ZeroVector;
		FVector Rot = FVector::ZeroVector;

		FromBuffer >> CameraMetaData.Timestamp;
		FromBuffer >> Pos;
		FromBuffer >> Rot;
		FromBuffer >> CameraMetaData.ExposureOffset;
		FromBuffer >> CameraMetaData.ExposureDuration;

		CameraTransform.SetLocation(FVector(-Pos.Z, Pos.X, Pos.Y) * 100.0f);
		//PYR from RPY
		CameraTransform.SetRotation(FRotator(FMath::RadiansToDegrees(Rot.X), FMath::RadiansToDegrees(-Rot.Y), FMath::RadiansToDegrees(-Rot.Z)).Quaternion());
	}

	void FSpatialDataDeserializer::DeserialiseSkeleton()
	{
		if (bShouldDeserialize == false)
			return;

		uint32 AnchorCount = 0;
		FromBuffer >> AnchorCount;

		// TODO(kbenjaminsson): Support multiple skeletons
		// for (int i=0; i<AnchorCount; ++i)
		{
			for (int i = 0, e = SkeletonDefinition.ParentIndices.Num(); i<e; ++i)
			{
				FromBuffer >> SkeletonTransforms.Transforms[i];
			}
		}
	}

	bool FSpatialDataDeserializer::StepFrame(bool bLoop)
	{
		if (bLoop == false && CurrFrame >= FrameCount)
		{
			bShouldDeserialize = false;
			return false;
		}

		bool bDidReset = false;

		if (CurrFrame >= FrameCount)
		{
			FromBuffer.Reset();
			CurrFrame = 0;
			bDidReset = true;
		}

		++CurrFrame;

		return bDidReset;
	}

	const FTransform& FSpatialDataDeserializer::GetCameraTransform() const
	{
		return CameraTransform;
	}

	const FCameraFrameMetaData& FSpatialDataDeserializer::GetCameraFrameMetaData() const
	{
		return CameraMetaData;
	}

	const FSCTSkeletonDefinition& FSpatialDataDeserializer::GetSkeletonDefinition() const
	{
		return SkeletonDefinition;
	}

	const FSkeletonTransforms& FSpatialDataDeserializer::GetSkeletonTransforms() const
	{
		return SkeletonTransforms;
	}

	const int32 FSpatialDataDeserializer::GetDeviceOrientation() const
	{
		return DeviceOrientation;
	}
}