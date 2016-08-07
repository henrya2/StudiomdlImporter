// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "StudiomdlImporterPrivatePCH.h"
#include "StudiomdlImporterTest.h"

#define LOCTEXT_NAMESPACE "FStudiomdlImporterModule"

static FStudiomdlImporterTest* g_StudiomdlImporterTest = nullptr;

void FStudiomdlImporterModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	g_StudiomdlImporterTest = new FStudiomdlImporterTest();
}

void FStudiomdlImporterModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	delete g_StudiomdlImporterTest;
	g_StudiomdlImporterTest = nullptr;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FStudiomdlImporterModule, StudiomdlImporter)

DEFINE_LOG_CATEGORY(LogSMDImporter)