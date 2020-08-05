#  Protocol Specification

Even though Spatial Camera Tracker (SCT) comes with plugins for Unity and Unreal, sometimes you need to parse the data yourself or when creating a new importer.
To help facilitate this, this specification exist to describe the data layout.

The data recorded from SCT is saved to a binary file to keep the storage size to a minimum and to support live streaming of data (in future versions).

## Version History
```
202003 - SCT 1.0
202004 - SCT 1.01
```

## Structure

The overall structure is quite simple:

```
[Header]  [Frame Data 1] [Frame Data 2] ... [Frame Data n]
```

In the case of camera tracking only the structure is:
```
[Header]  [Camera Frame 1] [Camera Frame 2] ... [Camera Frame n]
```

When capturing a skeleton  the structure is slightly different as it contains both skeleton *and* camera frames.
Also not the added skeleton definition block:
```
[Header][Skeleton Definition]  [Skeleton Frame 1][Camera Frame 1] [Skeleton Frame 2][Camera Frame 2] ... [Skeleton Frame n][Camera Frame n]
```


Parsing the header will give you information about needed to parse the frame specific data

### Header

The header consists of the following fields:

```
Version (int32) - Identifies the protocol version. This is bumped whenever changes to the stream format are made. Make sure to check the version when parsing
Frame Count (int 32 bits) - The number of frame data blocks the stream contains
Device Orientation (int 32 bits) - The orientation the device had when recording. See UIDeviceOrientation enum for possible values
Horizontal FOV (float 32 bits) - Horizontal Field of View in degrees of the camera lens used when recording. This differs from device to device.
Vertical FOV (float 32 bits) - Vertical Field of View in degrees of the camera lens used when recording. This differs from device to device.
Focal Length X (float 32 bits) - Pixel focal length. X and Y are identical for square pixels.
Focal Length Y (float 32 bits) - Pixel focal length. X and Y are identical for square pixels.
Capture Type (int 32 bits) - Specifies the type of capture. 0 - Skeleton, 1 - Camera World Tracking
```

### Camera Frame
A camera frame defines the camera position and rotation in 3D space and contains the following fields:
```
Timestamp (double 64 bits) - Timestamp when the frame was captured. Use this for precise timing of frames
Position (float3 96 bits) - Standard 3 component vector describing the camera position (x, y, z) 
Rotation (float3 96 bits) - Standard 3 component vector describing the camera rotation in radians (roll, pitch, yaw)
Exposure Offset (float 32 bits) - Use this value to light your scene during its post-processed lighting stage
Exposure Duration (double 64 bits) - Use this value to determine how much motion blur to apply to your virtual content
```

### Skeleton Definition

The skeleton definition describes the names of the joints being serialized, the parent structure needed to reconstruct the skeleton, and the transforms of the neutral pose of the skeleton:
```
[Joint Count] [Joint Name 0] ... [Joint Name n] [Parent Count] [Parent Index 0] ... [Parent Index n] [Joint Transform 0] ... [Joint Transform n]
```
The definition consists of the following fields:
```
Joint Count (int 32 bits) - The number of joints in the skeleton. 
Joint Name (int 32 bits) + n *  (char 8 bit) - The name of the joint. First read the number of characters in the string and then the characters themselves 
Parent Indices Count (int 32 bits)  - The number of parent indices
Parent Index - (int 32 bits) - The position of the index defines which joint the index relates to. The value read defines the joint's parent. A value of -1 means a joint has no parent.
Joint Transform (4x4 Matrix 512 bits) - A complete 4x4 matrix (4 columns and four rows) describing the joint position and orientation. See simd_float4x4 for details. Read Joint Count number of transforms. The index of the transform defines the joint it belongs to and follows the order defined in the skeleton definition.
```

### Skeleton Frame

Each skeleton frame contains information about all skeletons being tracked. At this point only one skeleton is supported.
```
[Skeleton Count] [Transforms skeleton 1] ... [Transforms skeleton n]
```
Fields:
```
Skeleton Count (int 32 bits) - The number of skeletons serialized.
Joint Transform (4x4 Matrix 512 bits) - A complete 4x4 matrix (4 columns and four rows) describing the joint position and orientation. See simd_float4x4 for details.
```
Read Joint Count number of transforms. The index of the transform defines the joint it belongs to and follows the order defined in the skeleton definition.
