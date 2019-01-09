// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XD_PropertyCustomizationEx.h"
#include "Factories/Factory.h"
#include "BehaviorTreeEx_Customization.generated.h"

/**
 * 
 */
UCLASS()
class UXD_BehaviorTreeFactory : public UFactory
{
	GENERATED_UCLASS_BODY()
public:
	// UFactory interface
	UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	bool CanCreateNew() const override;
	bool ShouldShowInNewMenu() const override;
	// End of UFactory interface
};

struct XD_AI_SYSTEMEX_EDITOR_API FBehaviorTreeInstantiatable_Customization : public IPropertyTypeCustomizationMakeInstanceable<FBehaviorTreeInstantiatable_Customization>
{
public:
	/** IPropertyTypeCustomization interface */
	virtual void CustomizeHeader(TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<class IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override{}
};

struct XD_AI_SYSTEMEX_EDITOR_API FBehaviorTreeWithSubTree_Customization : public IPropertyTypeCustomizationMakeInstanceable<FBehaviorTreeWithSubTree_Customization>
{
public:
	/** IPropertyTypeCustomization interface */
	virtual void CustomizeHeader(TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<class IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
};

