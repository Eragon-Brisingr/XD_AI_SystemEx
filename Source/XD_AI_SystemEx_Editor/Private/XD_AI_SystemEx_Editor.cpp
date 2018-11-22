﻿// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "XD_AI_SystemEx_Editor.h"
#include "PropertyEditorModule.h"
#include "XD_PropertyCustomizationEx.h"

#define LOCTEXT_NAMESPACE "FXD_AI_SystemEx_EditorModule"

void FXD_AI_SystemEx_EditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

		RegisterCustomProperty(struct FBehaviorTreeInstantiatable, FBehaviorTreeInstantiatable_Customization);
		RegisterCustomProperty(struct FBehaviorTreeWithSubTree, FBehaviorTreeWithSubTree_Customization);
	}
}

void FXD_AI_SystemEx_EditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FXD_AI_SystemEx_EditorModule, XD_AI_SystemEx)