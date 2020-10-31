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
#include "SCTSerializeFromBuffer.h"

namespace kh
{
	struct FSkeletonTransforms
	{
		TArray<FTransform>Transforms;
	};

	struct FSkeletonDefinition
	{
		TArray<FName> JointNames;
		TArray<int32> ParentIndices;
		TArray<FTransform> NeutralTransforms;
	};

	struct FCameraFrameMetaData
	{
		double Timestamp;
		float ExposureOffset;
		double ExposureDuration;
	};

	struct FUserAnchors
	{
		TArray<FVector> Anchors;
	};

	/**
	 *
	 */
	class FSpatialDataDeserializer
	{
	public:
		FSpatialDataDeserializer();
		~FSpatialDataDeserializer();

		void InitWithBuffer(TArray<uint8>& RecvBuffer);
		void DeserializeHeader();
		void DeserialiseCamera();
		void DeserializeUserAnchors();
		void DeserializeSkeletonDefinition();
		void DeserialiseSkeleton();

		void DeserialiseCamera(TArray<uint8>& RecvBuffer);
		void DeserialiseSkeleton(TArray<uint8>& RecvBuffer);

		bool StepFrame(bool bLoop = true);

		const FTransform& GetCameraTransform() const;
		const FCameraFrameMetaData& GetCameraFrameMetaData() const;
		const FSkeletonDefinition& GetSkeletonDefinition() const;
		const FSkeletonTransforms& GetSkeletonTransforms() const;
		const int32 GetDeviceOrientation() const;

	private:
		FTransform DeserializeTransform();

		bool bShouldDeserialize;
		int32 CurrFrame;

		// Header
		int32 Version;
		int32 FrameCount;
		int32 DeviceOrientation; //UIDeviceOrientation
		float HorizontalFOV;
		float VerticalFOV;
		float FocalLengthX;
		float FocalLengthY;
		int CaptureType;

		FMRSerializeFromBuffer FromBuffer;

		FTransform CameraTransform;
		FCameraFrameMetaData CameraMetaData;
		FUserAnchors UserAnchors;

		FSkeletonDefinition SkeletonDefinition;
		FSkeletonTransforms SkeletonTransforms;
	};
}