// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NavMesh/RecastNavMesh.h"
#include <NavMesh/RecastNavMeshGenerator.h>
#include "XD_RecastNavMesh.generated.h"

class AXD_NavDataExtra;
class UNavArea;

/**
 * 
 */
USTRUCT()
struct FJumpPointConfig : public FNavigationLinkBase
{
	GENERATED_BODY()
public:
	FJumpPointConfig()
	{
		MaxFallDownLength = 0.f;
	}

	UPROPERTY(EditAnywhere, Category = "自定义数据")
	float JumpMaxHeight = 150.f;

	UPROPERTY(EditAnywhere, Category = "自定义数据")
	float FallMaxHeight = 600.f;

	UPROPERTY(EditAnywhere, Category = "自定义数据")
	float JumpMaxDistance = 250.f;

	UPROPERTY(EditAnywhere, Category = "自定义数据")
	float JumpDistanceStepLength = 30.f;

	UPROPERTY(EditAnywhere, Category = "自定义数据")
	float JumpPointMinDistance = 60.f;

	UPROPERTY(EditAnywhere, Category = "自定义数据")
	TEnumAsByte<ETraceTypeQuery> JumpPointTraceChannel = ETraceTypeQuery::TraceTypeQuery1;
};

//自动生成可跳跃区域的连线，现在的实现会导致NavMesh构建两次，想实现完美可从FXD_RecastTileGenerator::GenerateCompressedLayers入手，在网格构建时就插入Link数据
UCLASS()
class XD_AI_SYSTEMEX_API AXD_RecastNavMesh : public ARecastNavMesh
{
	GENERATED_BODY()
public:
	AXD_RecastNavMesh(const FObjectInitializer& ObjectInitializer);

	void OnNavMeshTilesUpdated(const TArray<uint32>& ChangedTiles) override;

	void OnNavMeshGenerationFinished() override;
	FRecastNavMeshGenerator* CreateGeneratorInstance() override;

	const FVector GetEdgeDir(const FVector& EdgeStartVertex, const FVector& EdgeEndVertex) const;

	void Destroyed() override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void CleanExtraData();

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "自定义数据")
	uint8 bShowDebugInfo : 1;
#endif
 	UPROPERTY(EditAnywhere, Category = "自定义数据", Config)
 	TArray<FJumpPointConfig> JumpPointConfigs;

	UPROPERTY(VisibleAnywhere, Category = "数据", Transient)
	TMap<uint32, AXD_NavDataExtra*> NavDataExtras;

protected:
	void DrawTracedJumpLink(UWorld* World, FVector StartLocation, const FVector& TargetLocation);

};

class FXD_RecastNavMeshGenerator : public FRecastNavMeshGenerator
{
public:
	FXD_RecastNavMeshGenerator(AXD_RecastNavMesh& InDestNavMesh)
		:FRecastNavMeshGenerator(InDestNavMesh)
	{}

	TSharedRef<FRecastTileGenerator> CreateTileGenerator(const FIntPoint& Coord, const TArray<FBox>& DirtyAreas) override;
};

class FXD_RecastTileGenerator : public FRecastTileGenerator
{
public:
	friend FXD_RecastNavMeshGenerator;
	using Super = FRecastTileGenerator;

	FXD_RecastTileGenerator(FXD_RecastNavMeshGenerator& ParentGenerator, const FIntPoint& Location)
		:Super(ParentGenerator, Location)
	{}
protected:
	bool GenerateCompressedLayers(FNavMeshBuildContext& BuildContext) override;
};
