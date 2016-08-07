#include "StudiomdlImporterPrivatePCH.h"
#include "SMDModel.h"

#define TOKEN_VERSION TEXT("version")
#define TOKEN_NODES TEXT("nodes")
#define TOKEN_END TEXT("end")

#define CALC_TOKEN_SIZE(TOKEN) (sizeof(TOKEN) / sizeof(TCHAR) - 1)

#define TOKEN_VERSION_SIZE CALC_TOKEN_SIZE(TOKEN_VERSION)
#define TOKEN_NODES_SIZE CALC_TOKEN_SIZE(TOKEN_NODES)
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

	return true;
}

void SkipToNextValidLine(const TCHAR*& In)
{
	while (*In != '\n')
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

		if (*InOut != TEXT(' '))
			return true;

		InOut++;
	}

	return true;
}

void SkipSpacesAndLine(const TCHAR*& InOut)
{
	while (*InOut != '\n' && *InOut != ' ')
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

bool IsEndOfContent(const TCHAR* In)
{
	if (*In == TEXT('\0'))
		return true;

	return false;
}

bool MatchToken(const TCHAR* In, const TCHAR* Token)
{
	if (FCString::Strcmp(In, Token) == 0)
		return true;

	return false;
}

bool Advance(const TCHAR*& InOut, int32 Size)
{
	while (*InOut != '\0' && Size > 0)
	{
		InOut++;
		Size--;
	}

	if (*InOut == '\0' && Size != 0)
		return false;

	return true;
}

bool ParseSignedInteger(const TCHAR*& In, int32& OutVal)
{
	const TCHAR* OldPosition = In;
	if (!SkipSpaces(In))
		return false;
	
	OutVal = FCString::Atoi(OldPosition);

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

		if (*InOut == TEXT('\n') && *InOut == TEXT(' '))
			return true;
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
			continue;
		}

		if (MatchToken(CurrentBuffer, TOKEN_VERSION))
		{
			if (Advance(CurrentBuffer, FCString::Strlen(TOKEN_VERSION)))
			{
				bValid = false;
				break;
			}

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
		if (IsCommentLine(CurrentBuffer))
		{
			continue;
		}

		if (IsEndOfContent(CurrentBuffer))
			break;

		if (MatchToken(CurrentBuffer, TOKEN_NODES))
		{
			ParseNodes(CurrentBuffer);
		}
		else
		{
			if (!SkipToken(CurrentBuffer))
				break;
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
			continue;
		}

		if (IsEndOfContent(CurrentBuffer))
			break;

		if (MatchToken(CurrentBuffer, TOKEN_END))
		{
			Advance(CurrentBuffer, TOKEN_END_SIZE);
			break;
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
					break;

				Bones.AddUninitialized(1);
				FSMDBone& Bone =  Bones.Last();
				Bone.Name.AppendChars(OldBuffer, (CurrentBuffer - OldBuffer));
				CurrentBuffer++;

				if (!SkipSpaces(CurrentBuffer))
					break;

				int32 ParentIndex;
				if (!ParseSignedInteger(CurrentBuffer, ParentIndex))
					break;

				Bone.Parent = ParentIndex;
			}
		}
	}
}
