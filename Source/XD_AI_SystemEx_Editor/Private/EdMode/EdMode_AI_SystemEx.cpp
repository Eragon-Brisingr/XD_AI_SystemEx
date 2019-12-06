// Fill out your copyright notice in the Description page of Project Settings.


#include "EdMode_AI_SystemEx.h"
#include "AssetEditorManager.h"
#include "AIGraphEditor.h"
#include "WorkflowCentricApplication.h"
#include "AIGraphNode.h"
#include "BehaviorTree/BehaviorTree.h"
#include "EditorModeManager.h"
#include "SceneManagement.h"
#include "XD_BehaviorTreeNodeEditMode.h"
#include "EngineUtils.h"

#define LOCTEXT_NAMESPACE "XD_AI_SystemEx"

FName EdMode_AI_SystemEx::ID = TEXT("EdMode_AI_SystemEx");

EdMode_AI_SystemEx::EdMode_AI_SystemEx()
{
}

struct HBehaivorTreeNodePointProxy : public HHitProxy
{
	DECLARE_HIT_PROXY();

	HBehaivorTreeNodePointProxy(UAIGraphNode* AIGraphNode, int32 InIndex)
		: HHitProxy(HPP_World), AIGraphNode(AIGraphNode), Index(InIndex)
	{}

	UAIGraphNode* AIGraphNode;
	int32 Index;
};
IMPLEMENT_HIT_PROXY(HBehaivorTreeNodePointProxy, HHitProxy);

void EdMode_AI_SystemEx::Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI)
{
	const FColor NormalColor(200, 200, 200);
	const FColor SelectedColor(255, 128, 0);

	UWorld* World = GetWorld();
	if (SelectedActor)
	{
		for (int32 Idx = 0; Idx < ValidNodeList.Num(); ++Idx)
		{
			UAIGraphNode* Node = ValidNodeList[Idx];
			bool bSelected = (Node == SelectedNode.Get() && Idx == CurrentSelectedIndex);
			const FColor& Color = bSelected ? SelectedColor : NormalColor;

			if (IXD_BehaviorTreeNodeEditMode* BehaviorTreeNodeEditMode = Cast<IXD_BehaviorTreeNodeEditMode>(Node->NodeInstance))
			{
				PDI->SetHitProxy(new HBehaivorTreeNodePointProxy(Node, Idx));
				BehaviorTreeNodeEditMode->Render(SelectedActor->GetActorTransform(), Color, PDI);
				PDI->SetHitProxy(NULL);
			}
		}
	}

	Super::Render(View, Viewport, PDI);
}

void EdMode_AI_SystemEx::DrawHUD(FEditorViewportClient* ViewportClient, FViewport* Viewport, const FSceneView* View, FCanvas* Canvas)
{
	FString Desc = FText::Format(LOCTEXT("BehaviorTree Edit Mode Desc", "BehaviorTree Edit Mode\nCurrent Edit Node : {0}"), SelectedNode.IsValid() ? SelectedNode->GetNodeTitle(ENodeTitleType::FullTitle) : LOCTEXT("None", "None")).ToString();

	const FIntRect CanvasRect = Canvas->GetViewRect();
	int32 XL;
	int32 YL;
	StringSize(GEngine->GetLargeFont(), XL, YL, *Desc);

	const float DrawX = FMath::FloorToFloat(CanvasRect.Min.X + (CanvasRect.Width() - XL) * 0.5f);
	const float DrawY = 50.0f;
	Canvas->DrawShadowedString(DrawX, DrawY, *Desc, GEngine->GetLargeFont(), FLinearColor::Yellow);
}

bool EdMode_AI_SystemEx::HandleClick(FEditorViewportClient* InViewportClient, HHitProxy *HitProxy, const FViewportClick &Click)
{
	if (HitProxy)
	{
		if (HitProxy->IsA(HBehaivorTreeNodePointProxy::StaticGetType()))
		{
			HBehaivorTreeNodePointProxy* BehaivorTreeNodePointProxy = (HBehaivorTreeNodePointProxy*)HitProxy;
			SelectedNode = BehaivorTreeNodePointProxy->AIGraphNode;
			CurrentSelectedIndex = BehaivorTreeNodePointProxy->Index;

			return true;
		}
		else if (HitProxy->IsA(HActor::StaticGetType()))
		{
			SelectedNode.Reset();
		}
	}
	return false;
}

void EdMode_AI_SystemEx::Tick(FEditorViewportClient* ViewportClient, float DeltaTime)
{
	SelectedActor = GetFirstSelectedActorInstance();

	IAssetEditorInstance* ActivedBehaviorTreeEditorInstance = nullptr;
	{
		FAssetEditorManager& AssetEditorManager = FAssetEditorManager::Get();
		TArray<UObject*> AllEditedAssets = AssetEditorManager.GetAllEditedAssets();
		double MaxLastActivationTime = TNumericLimits<double>::Lowest();
		for (UObject* EditedAsset : AllEditedAssets)
		{
			if (UBehaviorTree* BehaviorTree = Cast<UBehaviorTree>(EditedAsset))
			{
				IAssetEditorInstance* AssetEditorInstance = AssetEditorManager.FindEditorForAsset(EditedAsset, false);
				FName EditorName = AssetEditorInstance->GetEditorName();
				static FName BehaviorTreeEditorName = TEXT("Behavior Tree");
				if (EditorName == BehaviorTreeEditorName)
				{
					double LastActivationTime = AssetEditorInstance->GetLastActivationTime();
					if (MaxLastActivationTime < LastActivationTime)
					{
						MaxLastActivationTime = LastActivationTime;
						ActivedBehaviorTreeEditorInstance = AssetEditorInstance;
					}
				}
			}
		}
	}

	ValidNodeList.Empty();
	if (ActivedBehaviorTreeEditorInstance)
	{
		// 由于FBehaviorTreeEditor不公开，只能通过这种方式拿到FAIGraphEditor
		FAIGraphEditor* AIGraphEditor = (FAIGraphEditor*)((char*)ActivedBehaviorTreeEditorInstance + sizeof(FWorkflowCentricApplication));
		FGraphPanelSelectionSet CurrentSelection = AIGraphEditor->GetSelectedNodes();
		for (UObject* Node : CurrentSelection)
		{
			if (UAIGraphNode* AIGraphNode = Cast<UAIGraphNode>(Node))
			{
				if (IXD_BehaviorTreeNodeEditMode* BehaviorTreeNodeEditMode = Cast<IXD_BehaviorTreeNodeEditMode>(AIGraphNode->NodeInstance))
				{
					ValidNodeList.Add(AIGraphNode);
				}
			}
		}
	}
	else
	{
		GLevelEditorModeTools().DeactivateMode(EdMode_AI_SystemEx::ID);
	}
}

bool EdMode_AI_SystemEx::InputDelta(FEditorViewportClient* InViewportClient, FViewport* InViewport, FVector& InDrag, FRotator& InRot, FVector& InScale)
{
	if (IXD_BehaviorTreeNodeEditMode* Node = GetSelectedNode())
	{
		const EAxisList::Type CurrentAxis = InViewportClient->GetCurrentWidgetAxis();
		const FWidget::EWidgetMode WidgetMode = InViewportClient->GetWidgetMode();

		if (CurrentAxis != EAxisList::None)
		{
			const bool bDoRotation = WidgetMode == FWidget::WM_Rotate || WidgetMode == FWidget::WM_TranslateRotateZ;
			const bool bDoTranslation = WidgetMode == FWidget::WM_Translate || WidgetMode == FWidget::WM_TranslateRotateZ;
			const bool bDoScale = WidgetMode == FWidget::WM_Scale;

			FTransform WorldTransform = SelectedActor->GetActorTransform();
			FTransform Transform = Node->GetRelativeTransform();
			Transform.AddToTranslation(WorldTransform.InverseTransformVectorNoScale(InDrag));
			Transform.ConcatenateRotation(InRot.Quaternion());
			Transform.SetScale3D(Transform.GetScale3D() + InScale);
			Node->SetRelativeTransform(Transform);
			return true;
		}
	}
	return false;
}

bool EdMode_AI_SystemEx::ShowModeWidgets() const
{
	return false;
}

bool EdMode_AI_SystemEx::ShouldDrawWidget() const
{
	return SelectedActor && GetSelectedNode() ? true : false;
}

bool EdMode_AI_SystemEx::UsesTransformWidget() const
{
	return SelectedActor && GetSelectedNode() ? true : false;
}

FVector EdMode_AI_SystemEx::GetWidgetLocation() const
{
	return SelectedActor->GetActorTransform().TransformPosition(GetSelectedNode()->GetRelativeTransform().GetLocation());
}

// bool EdMode_AI_SystemEx::GetCustomDrawingCoordinateSystem(FMatrix& InMatrix, void* InData)
// {
// 	IXD_BehaviorTreeNodeEditMode* BehaviorTreeNodeEditMode = Cast<IXD_BehaviorTreeNodeEditMode>(GetSelectedNode());
// 	if (SelectedActor && BehaviorTreeNodeEditMode)
// 	{
// 		InMatrix = (BehaviorTreeNodeEditMode->GetRelativeTransform() * SelectedActor->GetActorTransform()).ToMatrixWithScale();
// 		return true;
// 	}
// 	return false;
// }
// 
// bool EdMode_AI_SystemEx::GetCustomInputCoordinateSystem(FMatrix& InMatrix, void* InData)
// {
// 	return GetCustomDrawingCoordinateSystem(InMatrix, InData);
// }

IXD_BehaviorTreeNodeEditMode* EdMode_AI_SystemEx::GetSelectedNode() const
{
	if (CurrentSelectedIndex < ValidNodeList.Num() && ValidNodeList[CurrentSelectedIndex] == SelectedNode.Get())
	{
		return Cast<IXD_BehaviorTreeNodeEditMode>(SelectedNode->NodeInstance);
	}
	return nullptr;
}

#undef LOCTEXT_NAMESPACE
