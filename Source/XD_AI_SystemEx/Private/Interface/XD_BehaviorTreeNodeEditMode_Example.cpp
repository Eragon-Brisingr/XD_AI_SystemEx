// Fill out your copyright notice in the Description page of Project Settings.


#include "XD_BehaviorTreeNodeEditMode_Example.h"
#include "SceneManagement.h"

void UXD_BehaviorTreeNodeEditMode_Example::Render(const FTransform& WorldTransform, const FColor Color, FPrimitiveDrawInterface* PDI) const
{
// 	FSphere Sphere;
// 	Sphere.Center = WorldTransform.TransformPosition(Center);
// 	Sphere.W = 100.f;
// 	DrawSphere(PDI, Sphere.Center, FRotator::ZeroRotator, FVector(Sphere.W), 24, 6,
// 		GEngine->ConstraintLimitMaterialPrismatic->GetRenderProxy(), SDPG_World);
// 	DrawWireSphere(PDI, Sphere.Center, Color, Sphere.W, 24, SDPG_Foreground);

	FMatrix WorldPos = (RelativeTransform * WorldTransform).ToMatrixWithScale();
	DrawBox(PDI, WorldPos, FVector::OneVector * 0.5f, GEngine->ConstraintLimitMaterialPrismatic->GetRenderProxy(), SDPG_World);
	DrawWireBox(PDI, WorldPos, FBox(-FVector::OneVector * 0.5f, FVector::OneVector * 0.5f), FLinearColor(Color), SDPG_Foreground);
}
