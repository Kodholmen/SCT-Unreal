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
#include "ILiveLinkSource.h"
#include "Containers/Ticker.h"

#include "SpatialDataDeserializer.h"

class ILiveLinkClient;
struct FLiveLinkSkeletonStaticData;
struct FLiveLinkTransformFrameData;
struct FLiveLinkAnimationFrameData;

class SCT_API FSCTLiveLinkSource : public ILiveLinkSource
{
public:
	FSCTLiveLinkSource();
	virtual ~FSCTLiveLinkSource();

	// Begin ILiveLinkSource Interface
	virtual void ReceiveClient(ILiveLinkClient* InClient, FGuid InSourceGuid) override;
	virtual bool IsSourceStillValid() const override;
	virtual bool RequestSourceShutdown() override;
	virtual FText GetSourceType() const override;
	virtual FText GetSourceMachineName() const override;
	virtual FText GetSourceStatus() const override;
	// End ILiveLinkSource Interface

private:
	// Add a Tick Function to this livelink source
	bool Tick(float DeltaTime);
	// Update LiveLinkData
	void UpdateLiveLink(float DeltaTime);

	// Role population
	void PopulateBasePropertyNames(TArray<FName>& PropertyNames);
	void UpdateBaseStaticData(FLiveLinkBaseStaticData& InOutData);
	void UpdateSkeletonStaticData(FLiveLinkSkeletonStaticData& InOutData);
	void UpdateBaseFrameData(FLiveLinkBaseFrameData& InOutData, float DeltaTime);
	void UpdateTransformFrameData(FLiveLinkTransformFrameData& InOutData, float DeltaTime);
	void UpdateSkeletonFrameData(FLiveLinkAnimationFrameData& InOutData, float DeltaTime);

	// Property Names
	TArray<FName> BasePropertyNames;

	// Tick Delegate Handles
	FTickerDelegate TickDelegate;
	FDelegateHandle TickDelegateHandle;

	// Livelink Identifiers
	bool bIsInitialized = false;
	ILiveLinkClient* LiveLinkClient;
	FGuid LiveLinkSourceGuid;

	// Livelink Source Parameters
	FText SourceType;
	FText SourceMachineName;
	FText SourceStatus;

	// Spatial Data
	kh::FSpatialDataDeserializer SpatialData;
};
