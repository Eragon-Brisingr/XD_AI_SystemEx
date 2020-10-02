// Fill out your copyright notice in the Description page of Project Settings.

#include "Customizations/BehaviorTreeEx_Customization.h"
#include <PropertyHandle.h>
#include <DetailWidgetRow.h>
#include <BehaviorTree/BehaviorTree.h>
#include <IDetailChildrenBuilder.h>
#include <Dialogs/Dialogs.h>
#include <PackageTools.h>
#include <Misc/PackageName.h>
#include <Dialogs/DlgPickAssetPath.h>
#include <Serialization/ObjectWriter.h>
#include <Serialization/ObjectReader.h>
#include <AssetRegistryModule.h>
#include <Engine/Selection.h>

#include "Types/BehaviorTreeEx.h"

#define LOCTEXT_NAMESPACE "XD_AI_SystemEx"

namespace BehaviorTreePropertyCustomize
{
	UObject* GetOuter(const TSharedRef<IPropertyHandle>& PropertyHandle)
	{
		TArray<UObject*> Outers;
		PropertyHandle->GetOuterObjects(Outers);
		return Outers.Num() > 0 ? Outers[0] : nullptr;
	}

	template<typename ValueType>
	ValueType* Value(const TSharedPtr<IPropertyHandle>& PropertyHandle)
	{
		if (UObject* Outer = GetOuter(PropertyHandle.ToSharedRef()))
		{
			return reinterpret_cast<ValueType*>(PropertyHandle->GetValueBaseAddress(reinterpret_cast<uint8*>(Outer)));
		}
		return nullptr;
	}

	void StructBuilderDrawPropertys(class IDetailChildrenBuilder& StructBuilder, const TSharedRef<IPropertyHandle>& PropertyHandle, const TArray<FName>& ExcludePropertyNames, UStruct* StopShowType = nullptr)
	{
		uint32 NumChildren = 0;
		PropertyHandle->GetNumChildren(NumChildren);
		for (uint32 ChildIndex = 0; ChildIndex < NumChildren; ++ChildIndex)
		{
			const TSharedRef<IPropertyHandle> ChildHandle = PropertyHandle->GetChildHandle(ChildIndex).ToSharedRef();

			FProperty* ChildProperty = ChildHandle->GetProperty();
			UStruct* OwnerStrcut = ChildProperty->GetOwnerStruct();
			if (StopShowType == nullptr || !OwnerStrcut->IsChildOf(StopShowType))
			{
				if (!ExcludePropertyNames.Contains(*ChildProperty->GetNameCPP()))
				{
					StructBuilder.AddProperty(ChildHandle);
				}
			}
		}
	}

	template<typename ValueType>
	ValueType GetValue(const TSharedPtr<IPropertyHandle>& PropertyHandle)
	{
		if (UObject* Outer = GetOuter(PropertyHandle.ToSharedRef()))
		{
			if (ValueType* Res = reinterpret_cast<ValueType*>(PropertyHandle->GetValueBaseAddress(reinterpret_cast<uint8*>(Outer))))
			{
				return *Res;
			}
		}
		return {};
	}

	template<typename Type>
	void SetValue(const TSharedPtr<IPropertyHandle>& PropertyHandle, const Type& Value, bool NotifyChange = true)
	{
		if (NotifyChange)
		{
			PropertyHandle->NotifyPreChange();
		}
		if (Type* Target = reinterpret_cast<Type*>(PropertyHandle->GetValueBaseAddress(reinterpret_cast<uint8*>(GetOuter(PropertyHandle.ToSharedRef())))))
		{
			*Target = Value;
			if (NotifyChange)
			{
				PropertyHandle->NotifyPostChange(EPropertyValueSetFlags::DefaultFlags);
			}
		}
	}
}

namespace FPropertyTypeWithInstancedButtonHelper
{
	UObject* ConvertObjectToAsset(UObject* ObjectToConvert, UObject* Owner, const TSubclassOf<UFactory>& Factory)
	{
		FString DefaultAsset = FPackageName::GetLongPackagePath(Owner->GetOutermost()->GetName()) + TEXT("/") + ObjectToConvert->GetName();

		TSharedRef<SDlgPickAssetPath> NewCurveDlg =
			SNew(SDlgPickAssetPath)
			.Title(LOCTEXT("ConvertObjectToAsset", "选择一个文件夹创建资源"))
			.DefaultAssetPath(FText::FromString(DefaultAsset));

		if (NewCurveDlg->ShowModal() != EAppReturnType::Cancel)
		{
			FString Package(NewCurveDlg->GetFullAssetPath().ToString());
			FString Name(NewCurveDlg->GetAssetName().ToString());
			FString Group(TEXT(""));

			// Find (or create!) the desired package for this object
			UPackage* Pkg = CreatePackage(NULL, *Package);
			UPackage* OutermostPkg = Pkg->GetOutermost();

			TArray<UPackage*> TopLevelPackages;
			TopLevelPackages.Add(OutermostPkg);
			if (!UPackageTools::HandleFullyLoadingPackages(TopLevelPackages, LOCTEXT("CreateANewObject", "Create a new object")))
			{
				// User aborted.
				return nullptr;
			}

			if (!PromptUserIfExistingObject(Name, Package, Group, Pkg))
			{
				return nullptr;
			}

			// PromptUserIfExistingObject may have GCed and recreated our outermost package - re-acquire it here.
			OutermostPkg = Pkg->GetOutermost();

			// Create a new asset and set it as the external curve
			FName AssetName = *Name;
			UObject* NewObject = Factory.GetDefaultObject()->FactoryCreateNew(ObjectToConvert->GetClass(), Pkg, AssetName, RF_Public | RF_Standalone, NULL, GWarn);
			if (NewObject)
			{
				UObject* ChangeOuterObject = DuplicateObject<UObject>(ObjectToConvert, OutermostPkg);
				// serialize out the original object
				TArray<uint8> Bytes;
				FObjectWriter(ChangeOuterObject, Bytes);
				// serialize old objects on top of the new object
				FObjectReader Reader(NewObject, Bytes);

				// Set the new object as the sole selection.
				USelection* SelectionSet = GEditor->GetSelectedObjects();
				SelectionSet->DeselectAll();
				SelectionSet->Select(NewObject);

				// Notify the asset registry
				FAssetRegistryModule::AssetCreated(NewObject);

				// Mark the package dirty...
				OutermostPkg->MarkPackageDirty();

				return NewObject;
			}
		}
		return nullptr;
	}
	
	void CustomizeHeader(UClass* Type, const TSubclassOf<class UFactory>& Factory, const FText& ButtonName, TSharedRef<class IPropertyHandle> StructPropertyHandle, TSharedRef<class IPropertyHandle> InstancePropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
	{
		HeaderRow.NameContent()
			[
				StructPropertyHandle->CreatePropertyNameWidget()
			]
		.ValueContent()
			.MinDesiredWidth(300.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
			.AutoHeight()
			[
				InstancePropertyHandle->CreatePropertyValueWidget()
			]
		+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Right)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
					.AutoWidth()
				[
					SNew(SButton)
					.OnClicked_Lambda([=]() {
						if (UObject* Instance = BehaviorTreePropertyCustomize::GetValue<UObject*>(InstancePropertyHandle))
						{
							GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(Instance);
						}
						return FReply::Handled();
					})
					.IsEnabled_Lambda([=]() {
						return BehaviorTreePropertyCustomize::GetValue<UObject*>(InstancePropertyHandle) ? true : false;
					})
					[
						SNew(STextBlock)
						.Text(LOCTEXT("打开", "打开"))
					]
				]
				+SHorizontalBox::Slot()
					.AutoWidth()
				[
					SNew(SButton)
					.OnClicked_Lambda([=]() {
						BehaviorTreePropertyCustomize::SetValue(InstancePropertyHandle, NewObject<UObject>(BehaviorTreePropertyCustomize::GetOuter(InstancePropertyHandle), Type, NAME_None, RF_ArchetypeObject | RF_DefaultSubObject));
						return FReply::Handled();
					})
					.IsEnabled_Lambda([=]() {
						if (UObject* Outer = BehaviorTreePropertyCustomize::GetOuter(InstancePropertyHandle))
						{
							if (Outer->HasAnyFlags(RF_ClassDefaultObject))
							{
								return false;
							}
						}
						if (UObject* Instance = BehaviorTreePropertyCustomize::GetValue<UObject*>(InstancePropertyHandle))
						{
							return Instance->IsAsset();
						}
						return true;
					})
					[
						SNew(STextBlock)
						.Text(ButtonName)
					]
				]
				+ SHorizontalBox::Slot()
					.AutoWidth()
				[
					SNew(SButton)
					.OnClicked_Lambda([=]() {
						if (UObject* Instance = BehaviorTreePropertyCustomize::GetValue<UObject*>(InstancePropertyHandle))
						{
							BehaviorTreePropertyCustomize::SetValue(InstancePropertyHandle, ConvertObjectToAsset(Instance, Instance->GetOuter(), Factory));
						}
						return FReply::Handled();
					})
					.IsEnabled_Lambda([=]() {
						if (Factory)
						{
							if (UObject* Instance = BehaviorTreePropertyCustomize::GetValue<UObject*>(InstancePropertyHandle))
							{
								return !Instance->IsAsset();
							}
						}
						return false;
					})
					[
						SNew(STextBlock)
						.Text(LOCTEXT("创建为资源", "创建为资源"))
					]
				]
			]
		];
	}
}

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

bool UXD_BehaviorTreeFactory::ShouldShowInNewMenu() const
{
	return false;
}

UObject* UXD_BehaviorTreeFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UBehaviorTree::StaticClass()));
	return NewObject<UBehaviorTree>(InParent, Class, Name, Flags);;
}

void FBehaviorTreeInstantiatable_Customization::CustomizeHeader(TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	TSharedPtr<IPropertyHandle> DisplayBehaviorTree_PropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FBehaviorTreeInstantiatable, DisplayBehaviorTree));

	BehaviorTreePropertyCustomize::SetValue(DisplayBehaviorTree_PropertyHandle, BehaviorTreePropertyCustomize::GetValue<FBehaviorTreeInstantiatable>(StructPropertyHandle).BehaviorTree, false);
	DisplayBehaviorTree_PropertyHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([=]()
		{
			if (FBehaviorTreeInstantiatable* BehaviorTreeInstantiatable = BehaviorTreePropertyCustomize::Value<FBehaviorTreeInstantiatable>(StructPropertyHandle))
			{
				BehaviorTreeInstantiatable->BehaviorTree = BehaviorTreeInstantiatable->DisplayBehaviorTree;
			}
		}));

	FPropertyTypeWithInstancedButtonHelper::CustomizeHeader(UBehaviorTree::StaticClass(), UXD_BehaviorTreeFactory::StaticClass(), LOCTEXT("创建行为树", "创建行为树"), StructPropertyHandle, DisplayBehaviorTree_PropertyHandle.ToSharedRef(), HeaderRow, StructCustomizationUtils);
}

void FBehaviorTreeWithSubTree_Customization::CustomizeHeader(TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	TSharedPtr<IPropertyHandle> MainBehaviorTree_PropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FBehaviorTreeWithSubTree, MainBehaviorTree));
	TSharedPtr<IPropertyHandle> DisplayBehaviorTree_PropertyHandle = MainBehaviorTree_PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FBehaviorTreeInstantiatable, DisplayBehaviorTree));

	BehaviorTreePropertyCustomize::SetValue(DisplayBehaviorTree_PropertyHandle, BehaviorTreePropertyCustomize::GetValue<FBehaviorTreeInstantiatable>(StructPropertyHandle).BehaviorTree, false);
	DisplayBehaviorTree_PropertyHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([=]()
		{
			if (FBehaviorTreeInstantiatable * BehaviorTreeInstantiatable = BehaviorTreePropertyCustomize::Value<FBehaviorTreeInstantiatable>(StructPropertyHandle))
			{
				BehaviorTreeInstantiatable->BehaviorTree = BehaviorTreeInstantiatable->DisplayBehaviorTree;
			}
		}));

	FPropertyTypeWithInstancedButtonHelper::CustomizeHeader(UBehaviorTree::StaticClass(), UXD_BehaviorTreeFactory::StaticClass(), LOCTEXT("创建行为树", "创建行为树"), StructPropertyHandle, DisplayBehaviorTree_PropertyHandle.ToSharedRef(), HeaderRow, StructCustomizationUtils);
}

void FBehaviorTreeWithSubTree_Customization::CustomizeChildren(TSharedRef<class IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	BehaviorTreePropertyCustomize::StructBuilderDrawPropertys(StructBuilder, StructPropertyHandle, { GET_MEMBER_NAME_CHECKED(FBehaviorTreeWithSubTree, MainBehaviorTree) });
}

#undef LOCTEXT_NAMESPACE
