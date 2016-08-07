#include "StudiomdlImporterPrivatePCH.h"
#include "StudiomdlImporterTest.h"
#include "SMDModel.h"

FStudiomdlImporterTest::FStudiomdlImporterTest()
	: ReadSMDModelCommand(
		TEXT("SMD.ReadSMD"), 
		TEXT("Read SMD for testing only..."), 
		FConsoleCommandWithArgsDelegate::CreateRaw(this, &FStudiomdlImporterTest::HandleReadSMDModel))
{

}

void FStudiomdlImporterTest::HandleReadSMDModel(const TArray<FString>& Args)
{
	if (Args.Num() != 1)
		return;

	FSMDModel SMDModle;
	SMDModle.LoadModel(Args[0]);
}
