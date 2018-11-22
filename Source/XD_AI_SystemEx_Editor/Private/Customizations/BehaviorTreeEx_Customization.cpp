﻿// Fill out your copyright notice in the Description page of Project Settings.

#include "BehaviorTreeEx_Customization.h"
#include "PropertyHandle.h"
#include "DetailWidgetRow.h"
#include "BehaviorTree/BehaviorTree.h"

#include "BehaviorTreeEx.h"

#define LOCTEXT_NAMESPACE "XD_AI_SystemEx"

UXD_BehaviorTreeFactory::UXD_BehaviorTreeFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UBehaviorTree::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

bool UXD_BehaviorTreeFactory::CanCreateNew() const
{
	return true;
}

UObject* UXD_BehaviorTreeFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UBehaviorTree::StaticClass()));
	return NewObject<UBehaviorTree>(InParent, Class, Name, Flags);;
}

void FBehaviorTreeInstantiatable_Customization::CustomizeHeader(TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	TSharedPtr<IPropertyHandle> BehaviorTree_PropertyHandle = FPropertyCustomizeHelper::GetPropertyHandleByName(StructPropertyHandle, GET_MEMBER_NAME_STRING_CHECKED(FBehaviorTreeInstantiatable, BehaviorTree));

	IPropertyTypeWithInstancedButtonHelper::CustomizeHeader(UBehaviorTree::StaticClass(), UXD_BehaviorTreeFactory::StaticClass(), LOCTEXT("创建行为树", "创建行为树"), StructPropertyHandle, BehaviorTree_PropertyHandle.ToSharedRef(), HeaderRow, StructCustomizationUtils);
}

void FBehaviorTreeWithSubTree_Customization::CustomizeHeader(TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	TSharedPtr<IPropertyHandle> MainBehaviorTree_PropertyHandle = FPropertyCustomizeHelper::GetPropertyHandleByName(StructPropertyHandle, GET_MEMBER_NAME_STRING_CHECKED(FBehaviorTreeWithSubTree, MainBehaviorTree));
	TSharedPtr<IPropertyHandle> BehaviorTree_PropertyHandle = FPropertyCustomizeHelper::GetPropertyHandleByName(MainBehaviorTree_PropertyHandle.ToSharedRef(), GET_MEMBER_NAME_STRING_CHECKED(FBehaviorTreeInstantiatable, BehaviorTree));

	IPropertyTypeWithInstancedButtonHelper::CustomizeHeader(UBehaviorTree::StaticClass(), UXD_BehaviorTreeFactory::StaticClass(), LOCTEXT("创建行为树", "创建行为树"), StructPropertyHandle, BehaviorTree_PropertyHandle.ToSharedRef(), HeaderRow, StructCustomizationUtils);
}

void FBehaviorTreeWithSubTree_Customization::CustomizeChildren(TSharedRef<class IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	FPropertyCustomizeHelper::StructBuilderDrawPropertys(StructBuilder, StructPropertyHandle, { GET_MEMBER_NAME_STRING_CHECKED(FBehaviorTreeWithSubTree, MainBehaviorTree) });
}

#undef LOCTEXT_NAMESPACE