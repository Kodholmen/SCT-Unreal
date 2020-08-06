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
		, Version(0)
		, FrameCount(0)
		, DeviceOrientation(0)
		, HorizontalFOV(0.0f)
		, VerticalFOV(0.0f)
		, FocalLengthX(0.0f)
		, FocalLengthY(0.0f)
		, CameraTransform()
	{
		CameraTransform.SetLocation(FVector::ZeroVector);
		CameraTransform.SetRotation(FQuat::Identity);
		CameraTransform.SetScale3D(FVector::OneVector);
	}

	FSpatialDataDeserializer::~FSpatialDataDeserializer()
	{
	}

	void FSpatialDataDeserializer::InitWithBuffer(TArray<uint8>& RecvBuffer)
	{
		FromBuffer.Init(RecvBuffer.GetData(), RecvBuffer.Num());
	}

	void FSpatialDataDeserializer::DeserializeHeader()
	{
		Version = 0;
		FrameCount = 0;
		DeviceOrientation = 0;

		FromBuffer >> Version;
		FromBuffer >> FrameCount;
		FromBuffer >> DeviceOrientation;
		FromBuffer >> HorizontalFOV;
		FromBuffer >> VerticalFOV;
		FromBuffer >> FocalLengthX;
		FromBuffer >> FocalLengthY;
		FromBuffer >> CaptureType;

		check(Version == 202004 && "Version Mismatch. Make sure your plugin and App versions match");

		UE_LOG(LogSpatialDataDeserializer, Display, TEXT("[SCT] Version: %d, Frames: %d, Device Orientation: %d, Horizontal FOV: %f, Vertical FOV: %f, Focal Length X: %f, Focal Length Y: %f"), Version, FrameCount, DeviceOrientation, HorizontalFOV, VerticalFOV, FocalLengthX, FocalLengthY);
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

	void FSpatialDataDeserializer::DeserializeSkeletonDefinition()
	{
		if (bShouldDeserialize == false)
			return;

		SkeletonDefinition.JointNames.Empty();
		SkeletonDefinition.ParentIndices.Empty();
		SkeletonDefinition.NeutralTransforms.Empty();
		SkeletonTransforms.Transforms.Empty();

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
			FTransform Trans = DeserializeTransform();
			SkeletonDefinition.NeutralTransforms.Add(Trans);
		}

		SkeletonTransforms.Transforms.AddDefaulted(JointCount);
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
				SkeletonTransforms.Transforms[i] = DeserializeTransform();
			}
		}
	}

	void FSpatialDataDeserializer::DeserialiseCamera(TArray<uint8>& RecvBuffer)
	{
		InitWithBuffer(RecvBuffer);
		DeserialiseCamera();
	}

	void FSpatialDataDeserializer::DeserialiseSkeleton(TArray<uint8>& RecvBuffer)
	{
		InitWithBuffer(RecvBuffer);
		DeserialiseSkeleton();
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

	const FSkeletonDefinition& FSpatialDataDeserializer::GetSkeletonDefinition() const
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

	FTransform FSpatialDataDeserializer::DeserializeTransform()
	{
		//UE_LOG(LogSpatialDataDeserializer, Display, TEXT("[MR LIVELINK] Deserializing frame %d"), CurrFrame);

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
		
		check(Transform.IsValid());

		return Transform;
	}
}