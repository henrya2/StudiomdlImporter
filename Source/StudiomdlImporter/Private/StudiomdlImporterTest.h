#pragma once

class FStudiomdlImporterTest
{
public:
	FStudiomdlImporterTest();

	void HandleReadSMDModel(const TArray<FString>& Args);

protected:
	FAutoConsoleCommand ReadSMDModelCommand;
};