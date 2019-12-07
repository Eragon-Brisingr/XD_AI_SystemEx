// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EdMode.h"

class UAIGraphNode;

/**
 * 
 */
class XD_AI_SYSTEMEX_EDITOR_API FEdMode_AI_SystemEx : public FEdMode
{
	using Super = FEdMode;
public:
	FEdMode_AI_SystemEx();

	bool IsCompatibleWith(FEditorModeID OtherModeID) const override { return true; }
	bool UsesToolkits() const override { return false; }
	void Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI) override;
	void DrawHUD(FEditorViewportClient* ViewportClient, FViewport* Viewport, const FSceneView* View, FCanvas* Canvas) override;
	bool HandleClick(FEditorViewportClient* InViewportClient, HHitProxy *HitProxy, const FViewportClick &Click) override;
	void Tick(FEditorViewportClient* ViewportClient, float DeltaTime) override;

	bool InputDelta(FEditorViewportClient* InViewportClient, FViewport* InViewport, FVector& InDrag, FRotator& InRot, FVector& InScale) override;
	bool ShowModeWidgets() const override;
	bool ShouldDrawWidget() const override;
	bool UsesTransformWidget() const override;
	FVector GetWidgetLocation() const override;
 	bool GetCustomDrawingCoordinateSystem(FMatrix& InMatrix, void* InData) override;
 	bool GetCustomInputCoordinateSystem(FMatrix& InMatrix, void* InData) override;
public:
	static FName ID;

private:
	TArray<UAIGraphNode*> ValidNodeList;
	AActor* SelectedActor;

	int32 CurrentSelectedIndex = 0;
	TWeakObjectPtr<UAIGraphNode> SelectedNode;
	class IXD_BehaviorTreeNodeEditMode* GetSelectedNode() const;
};
