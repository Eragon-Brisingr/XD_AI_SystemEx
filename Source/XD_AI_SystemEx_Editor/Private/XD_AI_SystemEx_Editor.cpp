﻿// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "XD_AI_SystemEx_Editor.h"
#include "PropertyEditorModule.h"
#include "ISettingsModule.h"
#include "EditorModeRegistry.h"
#include "Editor.h"
#include "EditorModeManager.h"
#include "AssetEditorManager.h"

#include "XD_PropertyCustomizationEx.h"
#include "XD_RecastNavMesh.h"
#include "EdMode_AI_SystemEx.h"
#include "BehaviorTree/BehaviorTree.h"

#define LOCTEXT_NAMESPACE "FXD_AI_SystemEx_EditorModule"

struct FBehaviorTreeInstantiatable;
struct FBehaviorTreeWithSubTree;

void FXD_AI_SystemEx_EditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

		RegisterCustomProperty(FBehaviorTreeInstantiatable, FBehaviorTreeInstantiatable_Customization);
		RegisterCustomProperty(FBehaviorTreeWithSubTree, FBehaviorTreeWithSubTree_Customization);
	}

	{
		ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

		if (SettingsModule != nullptr)
		{
			SettingsModule->RegisterSettings("Project", "Plugins", "XD_RecastNavMesh",
				LOCTEXT("XD_RecastNavMesh", "XD_RecastNavMesh"),
				LOCTEXT("XD_RecastNavMeshDescription", "Configure the XD_RecastNavMesh."),
				GetMutableDefault<AXD_RecastNavMesh>()
			);
		}
	}

	FEditorModeRegistry::Get().RegisterMode<EdMode_AI_SystemEx>(EdMode_AI_SystemEx::ID);

	FAssetEditorManager& AssetEditorManager = FAssetEditorManager::Get();
	OnAssetOpenedInEditorHandle = AssetEditorManager.OnAssetOpenedInEditor().AddLambda([](UObject* Asset, IAssetEditorInstance* AssetEditorInstance)
		{
			if (UBehaviorTree* BehaviorTree = Cast<UBehaviorTree>(Asset))
			{
				GLevelEditorModeTools().ActivateMode(EdMode_AI_SystemEx::ID);
			}
		});
}

void FXD_AI_SystemEx_EditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	
	{
		FPropertyEditorModule* PropertyModule = FModuleManager::GetModulePtr<FPropertyEditorModule>("PropertyEditor");
		PropertyModule->UnregisterCustomPropertyTypeLayout(GET_TYPE_NAME_CHECKED(FBehaviorTreeInstantiatable));
		PropertyModule->UnregisterCustomPropertyTypeLayout(GET_TYPE_NAME_CHECKED(FBehaviorTreeWithSubTree));
	}

	FAssetEditorManager& AssetEditorManager = FAssetEditorManager::Get();
	AssetEditorManager.OnAssetOpenedInEditor().Remove(OnAssetOpenedInEditorHandle);

	GLevelEditorModeTools().DeactivateMode(EdMode_AI_SystemEx::ID);
	FEditorModeRegistry::Get().UnregisterMode(EdMode_AI_SystemEx::ID);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FXD_AI_SystemEx_EditorModule, XD_AI_SystemEx_Editor)