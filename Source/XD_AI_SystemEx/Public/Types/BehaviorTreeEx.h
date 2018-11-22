// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
//#include "UObject/NoExportTypes.h"
#include "BehaviorTreeEx.generated.h"

/**
 * 
 */
class UBehaviorTree;

USTRUCT(BlueprintType)
struct XD_AI_SYSTEMEX_API FBehaviorTreeInstantiatable
{
	GENERATED_BODY()
public:
	explicit operator bool() const { return BehaviorTree ? true : false; }
	operator UBehaviorTree*() { return BehaviorTree; }
	operator const UBehaviorTree*() const { return BehaviorTree; }

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", Instanced)
	UBehaviorTree* BehaviorTree;
};

USTRUCT(BlueprintType)
struct XD_AI_SYSTEMEX_API FBehaviorTreeWithSubTree
{
	GENERATED_BODY()
public:
	explicit operator bool() const { return MainBehaviorTree ? true : false; }
	operator UBehaviorTree*() { return MainBehaviorTree; }
	operator const UBehaviorTree*() const { return MainBehaviorTree; }

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	FBehaviorTreeInstantiatable MainBehaviorTree;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	TMap<FGameplayTag, FBehaviorTreeInstantiatable> SubTreeOverride;
};
