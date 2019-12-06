// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "XD_BehaviorTreeNodeEditMode.h"
#include "XD_BehaviorTreeNodeEditMode_Example.generated.h"

/**
 * 
 */
UCLASS()
class XD_AI_SYSTEMEX_API UXD_BehaviorTreeNodeEditMode_Example : public UBTDecorator,
	public IXD_BehaviorTreeNodeEditMode
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "Test")
	FTransform RelativeTransform = FTransform(FQuat::Identity, FVector::ZeroVector, FVector::OneVector * 100.f);

	FTransform GetRelativeTransform() const override { return FTransform(RelativeTransform); }
	void SetRelativeTransform(const FTransform& Transform) override { RelativeTransform = Transform; }

	void Render(const FTransform& WorldTransform, const FColor Color, FPrimitiveDrawInterface* PDI) const override;
};
