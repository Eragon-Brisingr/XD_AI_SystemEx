// Fill out your copyright notice in the Description page of Project Settings.


#include "XD_NavDataExtra.h"
#include "NavLinkComponent.h"

// Sets default values
AXD_NavDataExtra::AXD_NavDataExtra()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

// 	JumpNavLinkComponent = CreateDefaultSubobject<UNavLinkComponent>(GET_MEMBER_NAME_CHECKED(AXD_NavDataExtra, JumpNavLinkComponent));
// 	SetRootComponent(JumpNavLinkComponent);
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

	return LinksBB;
}

