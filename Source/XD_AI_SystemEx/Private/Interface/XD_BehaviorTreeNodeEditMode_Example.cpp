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
// 	DrawWireSphere(PDI, Sphere.Center, FLinearColor::Black, Sphere.W, 24, SDPG_World);

	DrawBox(PDI, (RelativeTransform * WorldTransform).ToMatrixWithScale(), FVector::OneVector * 0.5f, GEngine->ConstraintLimitMaterialPrismatic->GetRenderProxy(), SDPG_World);
}
