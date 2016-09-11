#include "StudiomdlImporterPrivatePCH.h"
#include "SMDModel.h"

#define TOKEN_VERSION TEXT("version")
#define TOKEN_NODES TEXT("nodes")
#define TOKEN_SKELETON TEXT("skeleton")
#define TOKEN_TRIANGLES TEXT("triangles")
#define TOKEN_TIME TEXT("time")
#define TOKEN_END TEXT("end")

#define CALC_TOKEN_SIZE(TOKEN) (sizeof(TOKEN) / sizeof(TCHAR) - 1)

#define TOKEN_VERSION_SIZE CALC_TOKEN_SIZE(TOKEN_VERSION)
#define TOKEN_NODES_SIZE CALC_TOKEN_SIZE(TOKEN_NODES)
#define TOKEN_SKELETON_SIZE CALC_TOKEN_SIZE(TOKEN_SKELETON)
#define TOKEN_TIME_SIZE CALC_TOKEN_SIZE(TOKEN_TIME)
#define TOKEN_END_SIZE CALC_TOKEN_SIZE(TOKEN_END)

FSMDModel::FSMDModel()
{

}

bool FSMDModel::LoadModel(const FString& Filename)
{
	SMDContentString.Empty();

	Bones.Empty();
	MaterialNames.Empty();
	Faces.Empty();

	bool bResult = FFileHelper::LoadFileToString(SMDContentString, *Filename);

	if (!bResult)
		return false;

	ParseFile();

	return true;
}

inline bool IsLineChar(const TCHAR Ch)
{
	return Ch == TEXT('\n') || Ch == TEXT('\r');
}

inline bool IsSpaceChar(const TCHAR Ch)
{
	return Ch == TEXT('\t') || Ch == TEXT(' ');
}

inline bool IsSpaceOrLineChar(const TCHAR Ch)
{
	return IsLineChar(Ch) || IsSpaceChar(Ch);
}

void SkipToNextValidLine(const TCHAR*& In)
{
	while (!IsLineChar(*In))
	{
		In++;
	}
}

bool SkipSpaces(const TCHAR*& InOut)
{
	while (true)
	{
		if (*InOut == TEXT('\0'))
			return false;

		if (!IsSpaceChar(*InOut))
			return true;

		InOut++;
	}

	return true;
}

void SkipSpacesAndLine(const TCHAR*& InOut)
{
	while (IsSpaceOrLineChar(*InOut))
	{
		InOut++;
	}
}

bool IsCommentLine(const TCHAR* In)
{
	if (*In == TEXT('/') && *(In + 1) == TEXT('/'))
		return true;

	return false;
}

bool IsComment(const TCHAR* In)
{
	if (*In == TEXT('#') || *In == TEXT(';'))
		return true;

	return false;
}

bool IsEndOfContent(const TCHAR* In)
{
	if (*In == TEXT('\0'))
		return true;

	return false;
}

bool MatchToken(const TCHAR* In, const TCHAR* Token)
{
	for (; *In && *Token; In++, Token++)
	{
		if (*In != *Token)
			return false;
	}

	if (IsSpaceOrLineChar(*In))
		return true;

	return false;
}

bool Advance(const TCHAR*& InOut, int32 Size)
{
	while (*InOut != TEXT('\0') && Size > 0)
	{
		InOut++;
		Size--;
	}

	if (*InOut == TEXT('\0') && Size != 0)
		return false;

	return true;
}

bool AdvanceToNextSpaceOrLine(const TCHAR*& InOut)
{
	while (true)
	{
		if (*InOut == '\0')
			return false;

		if (IsSpaceOrLineChar(*InOut))
		{
			return true;
		}

		InOut++;
	}

	return false;
}

bool AdvanceToNextSpace(const TCHAR*& InOut)
{
	while (true)
	{
		if (*InOut == '\0')
			return false;

		if (IsSpaceChar(*InOut))
		{
			return true;
		}

		InOut++;
	}

	return false;
}

bool ParseSignedInteger(const TCHAR*& InOut, int32& OutVal)
{
	OutVal = FCString::Atoi(InOut);

	SkipSpaces(InOut);

	return true;
}

bool ParseFloat(const TCHAR*& InOut, float& OutVal)
{
	OutVal = FCString::Atof(InOut);

	SkipSpaces(InOut);

	return true;
}

bool ParseVector(const TCHAR*& InOut, FVector& OutVal)
{
	ParseFloat(InOut, OutVal.X);
	ParseFloat(InOut, OutVal.Y);
	ParseFloat(InOut, OutVal.Z);

	return true;
}

bool ParseVector2D(const TCHAR*& InOut, FVector2D& OutVal)
{
	ParseFloat(InOut, OutVal.X);
	ParseFloat(InOut, OutVal.Y);

	return true;
}

bool AdvanceUntilChar(const TCHAR*& InOut, TCHAR Ch)
{
	while (true)
	{
		if (*InOut == TEXT('\0'))
			return false;

		if (*InOut == Ch)
			return true;

		InOut++;
	}

	return false;
}

bool SkipToken(const TCHAR*& InOut)
{
	while (true)
	{
		if (*InOut == TEXT('\0'))
			return false;

		if (IsSpaceOrLineChar(*InOut))
			return true;

		InOut++;
	}

	return false;
}

void FSMDModel::ParseFile()
{
	const TCHAR* CurrentBuffer = *SMDContentString;

	bool bValid = true;

	// Read version
	while (true)
	{
		SkipSpacesAndLine(CurrentBuffer);
		if (IsCommentLine(CurrentBuffer))
		{
			SkipToNextValidLine(CurrentBuffer);
			continue;
		}

		if (IsEndOfContent(CurrentBuffer))
		{
			bValid = false;
			break;
		}

		if (MatchToken(CurrentBuffer, TOKEN_VERSION))
		{
			if (!Advance(CurrentBuffer, FCString::Strlen(TOKEN_VERSION)))
			{
				bValid = false;
				break;
			}

			SkipSpaces(CurrentBuffer);
			int32 VersionNum = 1;
			if (!ParseSignedInteger(CurrentBuffer, VersionNum))
			{
				bValid = false;
				break;
			}

			if (VersionNum != 1)
			{
				bValid = false;
				break;
			}

			break;
		}
	}

	if (!bValid)
		return;

	while (true)
	{
		SkipSpacesAndLine(CurrentBuffer);

		if (IsEndOfContent(CurrentBuffer))
			break;

		if (IsCommentLine(CurrentBuffer))
		{
			SkipToNextValidLine(CurrentBuffer);
			continue;
		}

		if (MatchToken(CurrentBuffer, TOKEN_NODES))
		{
			AdvanceToNextSpaceOrLine(CurrentBuffer);
			ParseNodes(CurrentBuffer);
		}
		else if (MatchToken(CurrentBuffer, TOKEN_SKELETON))
		{
			AdvanceToNextSpaceOrLine(CurrentBuffer);
			ParseSkeleton(CurrentBuffer);
		}
		else if (MatchToken(CurrentBuffer, TOKEN_TRIANGLES))
		{
			AdvanceToNextSpaceOrLine(CurrentBuffer);
			ParseTriangles(CurrentBuffer);
		}
		else
		{
			SkipToNextValidLine(CurrentBuffer);
		}
	}
}

void FSMDModel::ParseNodes(const TCHAR*& CurrentBuffer)
{
	while (true)
	{
		SkipSpacesAndLine(CurrentBuffer);
		if (IsCommentLine(CurrentBuffer))
		{
			SkipToNextValidLine(CurrentBuffer);
			continue;
		}

		if (IsEndOfContent(CurrentBuffer))
		{
			UE_LOG(LogSMDImporter, Error, TEXT("Unexpected end of file"));
			break;
		}

		if (MatchToken(CurrentBuffer, TOKEN_END))
		{
			Advance(CurrentBuffer, TOKEN_END_SIZE);
			break;
		}

		if (IsComment(CurrentBuffer))
		{
			AdvanceUntilChar(CurrentBuffer, TEXT('\n'));
			continue;
		}

		int32 NodeIndex = 0;
		if (ParseSignedInteger(CurrentBuffer, NodeIndex))
		{
			if (!SkipSpaces(CurrentBuffer))
				break;

			if (*CurrentBuffer == TEXT('\"'))
			{
				CurrentBuffer++;
				const TCHAR* OldBuffer = CurrentBuffer;
				if (!AdvanceUntilChar(CurrentBuffer, TEXT('\"')))
				{
					UE_LOG(LogSMDImporter, Error, TEXT("Mismatch \""));
					break;
				}

				FSMDBone Bone;
				Bone.Name.AppendChars(OldBuffer, (CurrentBuffer - OldBuffer));

				AdvanceToNextSpaceOrLine(CurrentBuffer);

				if (!SkipSpaces(CurrentBuffer))
				{
					UE_LOG(LogSMDImporter, Error, TEXT("Unexpected end of file"));
					break;
				}

				int32 ParentIndex;
				if (!ParseSignedInteger(CurrentBuffer, ParentIndex))
					break;

				Bone.Parent = ParentIndex;

				Bones.Add(Bone);
			}
		}
	}
}

void FSMDModel::ParseSkeleton(const TCHAR*& CurrentBuffer)
{
	while (true)
	{
		SkipSpacesAndLine(CurrentBuffer);
		if (IsCommentLine(CurrentBuffer))
		{
			SkipToNextValidLine(CurrentBuffer);
			continue;
		}

		if (IsEndOfContent(CurrentBuffer))
		{
			UE_LOG(LogSMDImporter, Error, TEXT("Unexpected end of file"));
			break;
		}

		if (MatchToken(CurrentBuffer, TOKEN_END))
		{
			Advance(CurrentBuffer, TOKEN_END_SIZE);
			break;
		}

		if (IsComment(CurrentBuffer))
		{
			AdvanceUntilChar(CurrentBuffer, TEXT('\n'));
			continue;
		}

		if (MatchToken(CurrentBuffer, TOKEN_TIME))
		{
			SkipToNextValidLine(CurrentBuffer);

			while (true)
			{
				SkipSpacesAndLine(CurrentBuffer);
				if (isdigit(*CurrentBuffer))
				{
					int32 NodeIndex;
					if (!ParseSignedInteger(CurrentBuffer, NodeIndex))
						break;

					if (NodeIndex >= Bones.Num())
					{
						UE_LOG(LogSMDImporter, Error, TEXT("Time NodeIndex out of range...."));
						break;
					}

					FSMDBone& Bone = Bones[NodeIndex];

					FSMDAnimation::LocationAndRotationKey LocationAndRotation;
					ParseVector(CurrentBuffer, LocationAndRotation.Position);
					ParseVector(CurrentBuffer, LocationAndRotation.Rot);
				}
				else
				{
					break;
				}
			}
		}
	}
}

void FSMDModel::ParseTriangles(const TCHAR*& CurrentBuffer)
{
	while (true)
	{
		SkipSpacesAndLine(CurrentBuffer);
		if (IsCommentLine(CurrentBuffer))
		{
			SkipToNextValidLine(CurrentBuffer);
			continue;
		}

		if (IsEndOfContent(CurrentBuffer))
		{
			UE_LOG(LogSMDImporter, Error, TEXT("Unexpected end of file"));
			break;
		}

		if (MatchToken(CurrentBuffer, TOKEN_END))
		{
			Advance(CurrentBuffer, TOKEN_END_SIZE);
			break;
		}

		if (IsComment(CurrentBuffer))
		{
			AdvanceUntilChar(CurrentBuffer, TEXT('\n'));
			continue;
		}

		{
			const TCHAR* OldBufferPtr = CurrentBuffer;
			AdvanceToNextSpaceOrLine(CurrentBuffer);

			int32 CharsCount = (int32)(CurrentBuffer - OldBufferPtr);
			FString MaterialName;
			MaterialName.AppendChars(OldBufferPtr, CharsCount);

			int32 MaterialIndex = MaterialNames.AddUnique(MaterialName);

			FSMDFace NewFace;
			NewFace.Material = MaterialIndex;

			SkipSpacesAndLine(CurrentBuffer);

			for (int32 i = 0; i < 3; ++i)
			{
				int32 ParentNode = 0;
				ParseSignedInteger(CurrentBuffer, ParentNode);
				ParseVector(CurrentBuffer, NewFace.Vertices[i].Position);
				ParseVector(CurrentBuffer, NewFace.Vertices[i].Normal);
				ParseVector2D(CurrentBuffer, NewFace.Vertices[i].UV);

				int32 BoneWeightCount = 0;
				if (*(CurrentBuffer) != TEXT('\n'))
				{
					ParseSignedInteger(CurrentBuffer, BoneWeightCount);
				}

				for (int32 j = 0; j < BoneWeightCount; ++j)
				{
					int32 NodeIndex = 0;
					float Weight = 0;
					ParseSignedInteger(CurrentBuffer, NodeIndex);
					ParseFloat(CurrentBuffer, Weight);

					NewFace.Vertices[i].BoneLinks.Add(TPairInitializer<int32, float>(NodeIndex, Weight));
				}
			}

			Faces.Add(NewFace);
		}
	}
}
