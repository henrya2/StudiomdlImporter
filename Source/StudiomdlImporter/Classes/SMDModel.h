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

	uint32 ParentNode;

	TArray<TPair<uint32, float>> BoneLinks;
};

struct FSMDFace
{
	FSMDFace()
		: Material(0)
	{

	}

	uint32 Material;

	FSMDVertex Vertices[3];
};

struct FSMDAnimation
{
	FSMDAnimation()
		: FirstTimeKey(0)
	{
		Keys.Reserve(20);
	}

	struct MatrixKey
	{
		FMatrix Matrix;

		FMatrix Absolute;

		FVector Position;

		FVector Rot;
		//FRotator Rotation;

		double dTime;
	};

	uint32 FirstTimeKey;

	TArray<MatrixKey> Keys;
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

protected:
	FString SMDContentString;

};