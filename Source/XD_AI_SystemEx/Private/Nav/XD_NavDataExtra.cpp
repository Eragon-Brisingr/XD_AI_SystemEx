// Fill out your copyright notice in the Description page of Project Settings.


#include "Nav/XD_NavDataExtra.h"
#include <NavLinkComponent.h>

// Sets default values
AXD_NavDataExtra::AXD_NavDataExtra(const FObjectInitializer& ObjectInitializer)
	:Super()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AXD_NavDataExtra::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AXD_NavDataExtra::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FBox AXD_NavDataExtra::GetComponentsBoundingBox(bool bNonColliding /*= false*/) const
{
	//考虑下PointLinks为空导至LinksBB没更新的问题

	FBox LinksBB(FVector(MAX_flt), FVector(-MAX_flt));

	if (PointLinks.Num() || SegmentLinks.Num())
	{
		for (int32 i = 0; i < PointLinks.Num(); ++i)
		{
			const FNavigationLink& Link = PointLinks[i];
			LinksBB += Link.Left;
			LinksBB += Link.Right;
		}

		for (int32 i = 0; i < SegmentLinks.Num(); ++i)
		{
			const FNavigationSegmentLink& SegmentLink = SegmentLinks[i];
			LinksBB += SegmentLink.LeftStart;
			LinksBB += SegmentLink.LeftEnd;
			LinksBB += SegmentLink.RightStart;
			LinksBB += SegmentLink.RightEnd;
		}
		return LinksBB.ExpandBy(FVector::ZeroVector, FVector(200.f, 200.f, 200.f));
	}

	return LinksBB;
}

FBox AXD_NavDataExtra::GetNavigationBounds() const
{
	return GetComponentsBoundingBox();
}

bool AXD_NavDataExtra::IsNavigationRelevant() const
{
	return (PointLinks.Num() > 0) || (SegmentLinks.Num() > 0);
}

bool AXD_NavDataExtra::GetNavigationLinksClasses(TArray<TSubclassOf<UNavLinkDefinition> >& OutClasses) const
{
	return false;
}

bool AXD_NavDataExtra::GetNavigationLinksArray(TArray<FNavigationLink>& OutLink, TArray<FNavigationSegmentLink>& OutSegments) const
{
	OutLink.Append(PointLinks);
	OutSegments.Append(SegmentLinks);

	return (PointLinks.Num() > 0) || (SegmentLinks.Num() > 0);
}

