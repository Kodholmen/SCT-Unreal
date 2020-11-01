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
#include "SCTSpatialCameraAsset.h"
#include "SCTSpatialSkeletonAsset.h"

namespace kh
{
	struct FSkeletonTransforms
	{
		TArray<FTransform>Transforms;
	};

	struct FCameraFrameMetaData
	{
		double Timestamp;
		float ExposureOffset;
		double ExposureDuration;
	};

	/**
	 *
	 */
	class FSpatialDataDeserializer
	{
	public:
		FSpatialDataDeserializer();
		~FSpatialDataDeserializer();

		void InitWithCameraAsset(USCTSpatialCameraAsset* Asset);
		void InitWithSkeletonAsset(USCTSpatialSkeletonAsset* Asset);
		void DeserialiseCamera();
		void DeserialiseSkeleton();

		bool StepFrame(bool bLoop = true);

		const FTransform& GetCameraTransform() const;
		const FCameraFrameMetaData& GetCameraFrameMetaData() const;
		const FSCTSkeletonDefinition& GetSkeletonDefinition() const;
		const FSkeletonTransforms& GetSkeletonTransforms() const;
		const int32 GetDeviceOrientation() const;

	private:

		bool bShouldDeserialize;
		int32 CurrFrame;
		int32 FrameCount;
		int32 DeviceOrientation;

		FMRSerializeFromBuffer FromBuffer;

		FTransform CameraTransform;
		FCameraFrameMetaData CameraMetaData;

		FSCTSkeletonDefinition SkeletonDefinition;
		FSkeletonTransforms SkeletonTransforms;
	};
}