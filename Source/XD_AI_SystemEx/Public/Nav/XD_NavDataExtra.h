// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Navigation/NavLinkProxy.h"
#include "XD_NavDataExtra.generated.h"

class UNavLinkComponent;
class UNavLinkRenderingComponent;

UCLASS()
class XD_AI_SYSTEMEX_API AXD_NavDataExtra : public ANavLinkProxy
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AXD_NavDataExtra();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	FBox GetComponentsBoundingBox(bool bNonColliding = false) const override;

// 	UPROPERTY(VisibleAnywhere, Category = "NavLink")
// 	UNavLinkComponent* JumpNavLinkComponent;

	int32 bIsNavLinkUpdated : 1;
};
