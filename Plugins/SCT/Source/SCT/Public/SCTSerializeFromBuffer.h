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

MSVC_PRAGMA(warning(push))
// Disable used without initialization warning because the reads are initializing
MSVC_PRAGMA(warning(disable : 4700))

/**
 * Class used to read data from a NBO data buffer
 */
class FMRSerializeFromBuffer
{
protected:
	/** Pointer to the data this reader is attached to */
	uint8* Data;
	/** The size of the data in bytes */
	int32 NumBytes;
	/** The current location in the byte stream for reading */
	int32 CurrentOffset;
	/** Indicates whether reading from the buffer caused an overflow or not */
	bool bHasOverflowed;


public:
	FMRSerializeFromBuffer(void)
		: Data(nullptr)
		, NumBytes(0)
		, CurrentOffset(0)
		, bHasOverflowed(false)
	{
	}

	void Init(uint8* InData, int32 Length)
	{
		Data = InData;
		NumBytes = Length;
	}

	void Reset()
	{
		CurrentOffset = 0;
		bHasOverflowed = false;
	}

	/**
	 * Initializes the buffer, size, and zeros the read offset
	 *
	 * @param InData the buffer to attach to
	 * @param Length the size of the buffer we are attaching to
	 */
	FMRSerializeFromBuffer(uint8* InData, int32 Length) 
		: Data(InData)
		, NumBytes(Length)
		, CurrentOffset(0)
		, bHasOverflowed(false)
	{
	}

	/**
	 * Reads a char from the buffer
	 */
	friend inline FMRSerializeFromBuffer& operator>>(FMRSerializeFromBuffer& Ar, char& Ch)
	{
		if (!Ar.HasOverflow() && Ar.CurrentOffset + 1 <= Ar.NumBytes)
		{
			Ch = Ar.Data[Ar.CurrentOffset++];
		}
		else
		{
			Ar.bHasOverflowed = true;
		}
		return Ar;
	}

	/**
	 * Reads a byte from the buffer
	 */
	friend inline FMRSerializeFromBuffer& operator>>(FMRSerializeFromBuffer& Ar, uint8& B)
	{
		if (!Ar.HasOverflow() && Ar.CurrentOffset + 1 <= Ar.NumBytes)
		{
			B = Ar.Data[Ar.CurrentOffset++];
		}
		else
		{
			Ar.bHasOverflowed = true;
		}
		return Ar;
	}

	/**
	 * Reads an int32 from the buffer
	 */
	friend inline FMRSerializeFromBuffer& operator>>(FMRSerializeFromBuffer& Ar, int32& I)
	{
		return Ar >> *(uint32*)&I;
	}

	template<typename T>
	friend inline void readPod(FMRSerializeFromBuffer& Ar, T& val)
	{
		uint8_t* data = (uint8_t*)&val;
		int64 size = sizeof(T);
		FMemory::Memcpy(data, &Ar.Data[Ar.CurrentOffset], size);
	}

	/**
	 * Reads a uint32 from the buffer
	 */
	friend inline FMRSerializeFromBuffer& operator>>(FMRSerializeFromBuffer& Ar, uint32& D)
	{
		if (!Ar.HasOverflow() && Ar.CurrentOffset + 4 <= Ar.NumBytes)
		{
			readPod<uint32>(Ar, D);
			Ar.CurrentOffset += 4;
		}
		else
		{
			Ar.bHasOverflowed = true;
		}
		return Ar;
	}

	/**
	 * Reads an int64 from the buffer
	 */
	friend inline FMRSerializeFromBuffer& operator>>(FMRSerializeFromBuffer& Ar, int64& I)
	{
		if (!Ar.HasOverflow() && Ar.CurrentOffset + 8 <= Ar.NumBytes)
		{
			readPod<int64>(Ar, I);
			Ar.CurrentOffset += 8;
		}
		else
		{
			Ar.bHasOverflowed = true;
		}
		return Ar;
	}

	/**
	 * Adds a uint64 to the buffer
	 */
	friend inline FMRSerializeFromBuffer& operator>>(FMRSerializeFromBuffer& Ar, uint64& Q)
	{
		if (!Ar.HasOverflow() && Ar.CurrentOffset + 8 <= Ar.NumBytes)
		{
			Q = ((uint64)Ar.Data[Ar.CurrentOffset + 0] << 56) |
				((uint64)Ar.Data[Ar.CurrentOffset + 1] << 48) |
				((uint64)Ar.Data[Ar.CurrentOffset + 2] << 40) |
				((uint64)Ar.Data[Ar.CurrentOffset + 3] << 32) |
				((uint64)Ar.Data[Ar.CurrentOffset + 4] << 24) |
				((uint64)Ar.Data[Ar.CurrentOffset + 5] << 16) |
				((uint64)Ar.Data[Ar.CurrentOffset + 6] << 8) |
				(uint64)Ar.Data[Ar.CurrentOffset + 7];
			Ar.CurrentOffset += 8;
		}
		else
		{
			Ar.bHasOverflowed = true;
		}
		return Ar;
	}

	/**
	 * Reads a float from the buffer
	 */
	friend inline FMRSerializeFromBuffer& operator>>(FMRSerializeFromBuffer& Ar, float& F)
	{
		return Ar >> *(uint32*)&F;
	}

	/**
	 * Reads a double from the buffer
	 */
	friend inline FMRSerializeFromBuffer& operator>>(FMRSerializeFromBuffer& Ar, double& Dbl)
	{
		return Ar >> *(uint64*)&Dbl;
	}

	friend inline FMRSerializeFromBuffer& operator>>(FMRSerializeFromBuffer& Ar, FVector& Vec)
	{
		Ar >> Vec.X;
		Ar >> Vec.Y;
		Ar >> Vec.Z;

		return Ar;
	}

	friend inline FMRSerializeFromBuffer& operator>>(FMRSerializeFromBuffer& Ar, FQuat& Quat)
	{
		Ar >> Quat.X;
		Ar >> Quat.Y;
		Ar >> Quat.Z;
		Ar >> Quat.W;

		return Ar;
	}

	friend inline FMRSerializeFromBuffer& operator>>(FMRSerializeFromBuffer& Ar, FPlane& Plane)
	{
		Ar >> Plane.X;
		Ar >> Plane.Y;
		Ar >> Plane.Z;
		Ar >> Plane.W;

		return Ar;
	}

	friend inline FMRSerializeFromBuffer& operator>>(FMRSerializeFromBuffer& Ar, FTransform& Transform)
	{
		FPlane C0;
		FPlane C1;
		FPlane C2;
		FPlane C3;

		Ar >> C0;
		Ar >> C1;
		Ar >> C2;
		Ar >> C3;

		FMatrix RawYUpFMatrix(C0, C1, C2, C3);

		// Extract & convert rotation
		FQuat RawRotation(RawYUpFMatrix);
		FQuat Rotation(-RawRotation.Z, RawRotation.X, RawRotation.Y, -RawRotation.W);

		Transform.SetLocation(FVector(-RawYUpFMatrix.M[3][2], RawYUpFMatrix.M[3][0], RawYUpFMatrix.M[3][1]) * 100.0f);
		Transform.SetRotation(Rotation);

		return Ar;
	}

	/**
	 * Reads a FString from the buffer
	 */
	friend inline FMRSerializeFromBuffer& operator>>(FMRSerializeFromBuffer& Ar, FString& String)
	{
		// We send strings length prefixed
		int32 Len = 0;
		Ar >> Len;

		// Check this way to trust NumBytes and CurrentOffset to be more accurate than the packet Len value
		const bool bSizeOk = (Len >= 0) && (Len <= (Ar.NumBytes - Ar.CurrentOffset));
		if (!Ar.HasOverflow() && bSizeOk)
		{
			// Handle strings of zero length
			if (Len > 0)
			{
				char* Temp = (char*)FMemory_Alloca(Len + 1);
				// memcpy it in from the buffer
				FMemory::Memcpy(Temp, &Ar.Data[Ar.CurrentOffset], Len);
				Temp[Len] = '\0';

				FUTF8ToTCHAR Converted(Temp);
				TCHAR* Ptr = (TCHAR*)Converted.Get();
				String = Ptr;
				Ar.CurrentOffset += Len;
			}
			else
			{
				String.Empty();
			}
		}
		else
		{
			Ar.bHasOverflowed = true;
		}

		return Ar;
	}

	/**
	 * Reads an FName from the buffer
	 */
	friend inline FMRSerializeFromBuffer& operator>>(FMRSerializeFromBuffer& Ar, FName& Name)
	{
		FString NameString;
		Ar >> NameString;
		Name = FName(*NameString);
		return Ar;
	}

	/**
	 * Reads the rest of the buffer to an array
	 */
	friend inline FMRSerializeFromBuffer& operator>>(FMRSerializeFromBuffer& Ar, TArray<uint8>& Array)
	{
		uint32 NumToRead = Ar.NumBytes - Ar.CurrentOffset;
		Array.AddUninitialized(NumToRead);
		Ar.ReadBinary(Array.GetData(), NumToRead);

		return Ar;
	}

	/**
	 * Reads a blob of data from the buffer
	 *
	 * @param OutBuffer the destination buffer
	 * @param NumToRead the size of the blob to read
	 */
	void ReadBinary(uint8* OutBuffer, uint32 NumToRead)
	{
		if (!HasOverflow() && CurrentOffset + (int32)NumToRead <= NumBytes)
		{
			FMemory::Memcpy(OutBuffer, &Data[CurrentOffset], NumToRead);
			CurrentOffset += NumToRead;
		}
		else
		{
			bHasOverflowed = true;
		}
	}

	/**
	 * Seek to the desired position in the buffer
	 *
	 * @param Pos the offset from the start of the buffer
	 */
	void Seek(int32 Pos)
	{
		checkSlow(Pos >= 0);

		if (!HasOverflow() && Pos < NumBytes)
		{
			CurrentOffset = Pos;
		}
		else
		{
			bHasOverflowed = true;
		}
	}

	/** @return Current position of the buffer being to be read */
	inline int32 Tell(void) const
	{
		return CurrentOffset;
	}

	/** Returns whether the buffer had an overflow when reading from it */
	inline bool HasOverflow(void) const
	{
		return bHasOverflowed;
	}

	/** @return Number of bytes remaining to read from the current offset to the end of the buffer */
	inline int32 AvailableToRead(void) const
	{
		return FMath::Max<int32>(0, NumBytes - CurrentOffset);
	}

	/**
	 * Returns the number of total bytes in the buffer
	 */
	inline int32 GetBufferSize(void) const
	{
		return NumBytes;
	}
};

MSVC_PRAGMA(warning(pop))