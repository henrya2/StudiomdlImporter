#pragma once

struct FSMDVertex
{
	FSMDVertex()
		: ParentNode(-1)
	{

	}

	FVector Position;
	FVector Normal;
	FVector2D UV;

	int32 ParentNode;

	TArray<TPair<int32, float>> BoneLinks;
};

struct FSMDFace
{
	FSMDFace()
		: Material(0)
	{

	}

	int32 Material;

	FSMDVertex Vertices[3];
};

struct FSMDAnimation
{
	FSMDAnimation()
		: FirstTimeKey(0)
	{
		Keys.Reserve(20);
	}

	struct LocationAndRotationKey
	{
		FVector Position;

		FVector Rot;
	};

	int32 FirstTimeKey;

	TArray<LocationAndRotationKey> Keys;
};

struct FSMDBone
{
	FSMDBone()
		: Parent(-1)
		, bIsUsed(false)
	{

	}

	FString Name;

	int32 Parent;

	FSMDAnimation Anim;

	FMatrix OffsetMatrix;

	bool bIsUsed;
};

class FSMDModel
{
public:
	FSMDModel();

	bool LoadModel(const FString& Filename);

	TArray<FSMDBone> Bones;

	TArray<FString> MaterialNames;

	TArray<FSMDFace> Faces;

protected:
	void ParseFile();

	void ParseNodes(const TCHAR*& CurrentBuffer);

	void ParseSkeleton(const TCHAR*& CurrentBuffer);

	void ParseTriangles(const TCHAR*& CurrentBuffer);

protected:
	FString SMDContentString;

};