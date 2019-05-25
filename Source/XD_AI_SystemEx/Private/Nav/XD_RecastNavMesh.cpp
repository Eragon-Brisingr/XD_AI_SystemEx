// Fill out your copyright notice in the Description page of Project Settings.


#include "XD_RecastNavMesh.h"
#include "NavigationSystem.h"
#include "DrawDebugHelpers.h"
#include "ParallelFor.h"
#include "XD_NavDataExtra.h"
#include "NavLinkComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "RecastFilter_UseDefaultArea.h"
#include "Utils/XD_AI_Log.h"

AXD_RecastNavMesh::AXD_RecastNavMesh(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	JumpPointConfigs.Add(FJumpPointConfig());
}

void AXD_RecastNavMesh::OnNavMeshTilesUpdated(const TArray<uint32>& ChangedTiles)
{
	Super::OnNavMeshTilesUpdated(ChangedTiles);

	for (uint32 TileIdx : ChangedTiles)
	{
		AXD_NavDataExtra*& NavDataExtra = NavDataExtras.FindOrAdd(TileIdx);
		if (NavDataExtra == nullptr)
		{
			FActorSpawnParameters ActorSpawnParameters;
			ActorSpawnParameters.OverrideLevel = GetLevel();
			ActorSpawnParameters.Name = *FString::Printf(TEXT("%s_[%d]"), *GetName(), TileIdx);
			NavDataExtra = GetWorld()->SpawnActor<AXD_NavDataExtra>(ActorSpawnParameters);
#if WITH_EDITOR
			NavDataExtra->SetActorLabel(ActorSpawnParameters.Name.ToString());
#endif
			NavDataExtra->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
		}
	}

	UWorld* World = GetWorld();

 #if WITH_EDITOR
 	if (bShowDebugInfo)
 	{
 		for (auto& Data : NavDataExtras)
 		{
 			if (Data.Value && Data.Value->bIsNavLinkUpdated == false)
 			{
 				FlushPersistentDebugLines(World);
 				break;
 			}
 		}
 	}
 #endif

	for (FJumpPointConfig& JumpPointConfig : JumpPointConfigs)
	{
		JumpPointConfig.InitializeAreaClass(true);
	}
	ParallelFor(ChangedTiles.Num(), [&](int32 Idx)
		{
			uint32 TileIdx = ChangedTiles[Idx];
			AXD_NavDataExtra* NavDataExtra = *NavDataExtras.Find(TileIdx);
			if (NavDataExtra->bIsNavLinkUpdated)
			{
				return;
			}
			NavDataExtra->PointLinks.Empty();

			const ARecastNavMesh* navdata = Cast<ARecastNavMesh>(UNavigationSystemV1::GetCurrent(World)->MainNavData);
			FRecastDebugGeometry navGeo;
			navGeo.bGatherNavMeshEdges = true;

			// get the navigation vertices from recast via a batch query
			navdata->BeginBatchQuery();
			navdata->GetDebugGeometry(navGeo, TileIdx);
			navdata->FinishBatchQuery();

			// process the navmesh vertices (called nav mesh edges for some occult reason)
			TArray<FVector>& vertices = navGeo.NavMeshEdges;
			const int32 nVertices = vertices.Num();

			if (nVertices > 1)
			{
				for (int32 iVertex = 0; iVertex < nVertices; iVertex += 2)
				{
					const FVector edgeStartVertex = vertices[iVertex];
					const FVector edgeEndVertex = vertices[iVertex + 1];
					const FVector edge = edgeEndVertex - edgeStartVertex;
					const FVector edgeDir = edge.GetUnsafeNormal();

					EDrawDebugTrace::Type DrawDebugTrace = EDrawDebugTrace::None;
#if WITH_EDITOR
					if (bShowDebugInfo)
					{
						DrawDebugTrace = EDrawDebugTrace::ForDuration;
					}
#endif

					for (const auto& JumpPointConfig : JumpPointConfigs)
					{
						const float JumpMaxHeight = JumpPointConfig.JumpMaxHeight;
						const float JumpMaxDistance = JumpPointConfig.JumpMaxDistance;
						const float JumpPointMinDistance = JumpPointConfig.JumpPointMinDistance;
						const float JumpDistanceStepLength = JumpPointConfig.JumpDistanceStepLength;
						const float FallMaxHeight = JumpPointConfig.FallMaxHeight;
						const ETraceTypeQuery JumpPointTraceChannel = JumpPointConfig.JumpPointTraceChannel;
						const FNavigationLinkBase& JumpLinkConfig = JumpPointConfig;

						auto ProcessEdgeStep = [&](const FVector & EdgeStepVertex, const FVector & EdgeDir)
						{
#if WITH_EDITOR
							if (bShowDebugInfo)
							{
								DrawDebugLine(World, EdgeStepVertex, EdgeStepVertex + EdgeDir * JumpMaxDistance, FColor::Purple, true, -1.f, SDPG_World, 1.f);
							}
#endif

							//						不知道怎么用Navigation的RayCast，先不用了
							// 						TArray<FNavigationRaycastWork> RaycastWorks;
							// 						FSharedConstNavQueryFilter NavQueryFilter = UNavigationQueryFilter::GetQueryFilter(*this, this, URecastFilter_UseDefaultArea::StaticClass());
							// 
							// 						for (float StepDistance = JumpDistanceStepLength; StepDistance < JumpMaxDistance; StepDistance += JumpDistanceStepLength)
							// 						{
							// 							FVector StartTraceLocation = EdgeStepVertex + EdgeDir.GetUnsafeNormal2D() * StepDistance + FVector(0.f, 0.f, JumpMaxHeight);
							//  							FVector EndTraceLocation = EdgeStepVertex + EdgeDir.GetUnsafeNormal2D() * StepDistance - FVector(0.f, 0.f, FallMaxHeight);
							// 							RaycastWorks.Add(FNavigationRaycastWork(StartTraceLocation, EndTraceLocation));
							// 
							// #if WITH_EDITOR
							// 							if (bShowDebugInfo)
							// 							{
							// 								DrawDebugLine(World, StartTraceLocation, EndTraceLocation, FColor::Red, true);
							// 							}
							// #endif
							// 						}
							// 						BatchRaycast(RaycastWorks, NavQueryFilter, this);
							// 
							// 						for (auto& RaycastWork : RaycastWorks)
							// 						{
							// 							if (RaycastWork.bDidHit)
							// 							{
							// 								FNavigationLink NavigationLink;
							// 								NavigationLink.Direction = ENavLinkDirection::BothWays;
							// 								const FVector& TargetLocation = RaycastWork.HitLocation;
							// 								FVector StartLocation = EdgeStepVertex + (JumpMaxDistance - (EdgeStepVertex - TargetLocation).Size2D()) * EdgeDir;
							// 								NavigationLink.Left = EdgeStepVertex;
							// 								NavigationLink.Right = TargetLocation;
							// 								NavigationLink.SnapRadius = JumpPointMinDistance / 2.f;
							// 								NavigationLink.SetAreaClass(JumpNavArea);
							// 								NavDataExtra->PointLinks.Add(NavigationLink);
							// 							}
							// 						}

							FHitResult StartHitResult;
							UKismetSystemLibrary::LineTraceSingle(World, EdgeStepVertex, EdgeStepVertex - FVector(0.f, 0.f, 40.f), JumpPointTraceChannel, false, {}, EDrawDebugTrace::None, StartHitResult, false);
							TArray<FHitResult> HitResults;
							TArray<AActor*> IgnoreActors = { StartHitResult.GetActor() };
							for (float StepDistance = JumpMaxDistance; StepDistance > JumpDistanceStepLength; StepDistance -= JumpDistanceStepLength)
							{
								FVector StartTraceLocation = EdgeStepVertex + EdgeDir.GetUnsafeNormal2D() * StepDistance + FVector(0.f, 0.f, 10.f);
								FVector EndTraceLocation = EdgeStepVertex + EdgeDir.GetUnsafeNormal2D() * StepDistance - FVector(0.f, 0.f, FallMaxHeight);

								UKismetSystemLibrary::SphereTraceMulti(World, StartTraceLocation, EndTraceLocation, JumpDistanceStepLength / 2.f, JumpPointTraceChannel, false, IgnoreActors, EDrawDebugTrace::None, HitResults, false);

								for (FHitResult& HitResult : HitResults)
								{
									if (HitResult.GetComponent()->IsNavigationRelevant() && IgnoreActors.Contains(HitResult.GetActor()) == false)
									{
										IgnoreActors.Add(HitResult.GetActor());
										FNavigationLink NavigationLink;
										static_cast<FNavigationLinkBase&>(NavigationLink) = JumpLinkConfig;
										NavigationLink.SetAreaClass(JumpLinkConfig.GetAreaClass());
										const FVector& TargetLocation = HitResult.ImpactPoint;
										FVector StartLocation = EdgeStepVertex + (JumpMaxDistance - (EdgeStepVertex - TargetLocation).Size2D()) * EdgeDir;

										TArray<FHitResult> JumpCollisionResults;
										const float FloatOffset = 20.f;
										float HalfAgentHeight = AgentHeight / 2.f;
										float OffsetZ = HalfAgentHeight + JumpDistanceStepLength + FloatOffset;
										float CollisionTraceZ = StartLocation.Z + OffsetZ;
										UKismetSystemLibrary::CapsuleTraceMulti(World, FVector(StartLocation.X, StartLocation.Y, CollisionTraceZ), FVector(TargetLocation.X, TargetLocation.Y, CollisionTraceZ), JumpDistanceStepLength, AgentHeight - JumpDistanceStepLength - FloatOffset / 2.f, JumpPointTraceChannel, false, {}, DrawDebugTrace, JumpCollisionResults, false);
										bool HasJumpCollision = JumpCollisionResults.ContainsByPredicate([](const auto & HitResult) {return HitResult.GetComponent()->IsNavigationRelevant(); });

										if (!HasJumpCollision)
										{
											bool CanJumpUp = StartLocation.Z - TargetLocation.Z < JumpMaxHeight;

											NavigationLink.Left = StartLocation;
											NavigationLink.Right = TargetLocation;
											NavigationLink.Direction = CanJumpUp ? ENavLinkDirection::BothWays : ENavLinkDirection::LeftToRight;
											NavigationLink.SnapRadius = JumpPointMinDistance / 2.f;
											NavDataExtra->PointLinks.Add(NavigationLink);

											DrawTracedJumpLink(World, StartLocation, TargetLocation);
										}
									}
								}
							}
						};

						const int nEdgeSteps = edge.Size() / JumpPointMinDistance;
						FVector StepDir = FVector(FVector2D(edgeDir).GetRotated(90.f), 0.f).GetUnsafeNormal();

						if (nEdgeSteps == 0)
						{
							ProcessEdgeStep(edgeStartVertex, edgeDir);
						}
						else
						{
							for (int iEdgeStep = 0; iEdgeStep < nEdgeSteps; ++iEdgeStep)
							{
								ProcessEdgeStep(edgeStartVertex + (iEdgeStep * JumpPointMinDistance * edgeDir), StepDir);
							}
						}
						ProcessEdgeStep(edgeEndVertex, StepDir);

						int32 NextIdx = (iVertex + 2) % nVertices;
						const FVector NextEdgeDir = GetEdgeDir(vertices[NextIdx], vertices[NextIdx + 1]);
						FVector NextDir = FVector(FVector2D(NextEdgeDir).GetRotated(90.f), 0.f).GetUnsafeNormal();

						if (FMath::Abs(FVector2D::DotProduct(FVector2D(StepDir), FVector2D(NextDir))) < 0.8f)
						{
							ProcessEdgeStep(edgeEndVertex, FQuat::Slerp(StepDir.ToOrientationQuat(), NextDir.ToOrientationQuat(), 0.5f).Vector());
						}
					}
				}
			}
		}
#if WITH_EDITOR
		, bShowDebugInfo
#endif
	);

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
	for (uint32 TileIdx : ChangedTiles)
	{
		AXD_NavDataExtra* NavDataExtra = *NavDataExtras.Find(TileIdx);
		if (NavDataExtra->bIsNavLinkUpdated)
		{
			continue;
		}
		check(!NavSys->IsNavigationOctreeLocked());
 		INavRelevantInterface* NavElement = Cast<INavRelevantInterface>(NavDataExtra);
 		NavSys->UpdateNavOctreeElement(NavDataExtra, NavElement, UNavigationSystemV1::EOctreeUpdateMode::OctreeUpdate_Default);
 		NavDataExtra->bIsNavLinkUpdated = true;

		XD_AIDisplay_LOG("开始构建[%s]的跳跃信息", *NavDataExtra->GetName());
	}
}

void AXD_RecastNavMesh::OnNavMeshGenerationFinished()
{
	Super::OnNavMeshGenerationFinished();

	for (auto& Data : NavDataExtras)
	{
		if (auto NavDataExtra = Data.Value)
		{
			if (NavDataExtra->bIsNavLinkUpdated)
			{
				//延后1s允许跳跃点再次更新
				FTimerHandle TimeHandle;
				GetWorld()->GetTimerManager().SetTimer(TimeHandle, FTimerDelegate::CreateWeakLambda(NavDataExtra, [=]()
					{
						NavDataExtra->bIsNavLinkUpdated = false;
						XD_AIDisplay_LOG("结束构建[%s]的跳跃信息", *NavDataExtra->GetName());
					}), 1.f, false);
			}
		}
	}
}

FRecastNavMeshGenerator* AXD_RecastNavMesh::CreateGeneratorInstance()
{
	return new FXD_RecastNavMeshGenerator(*this);
}

const FVector AXD_RecastNavMesh::GetEdgeDir(const FVector& EdgeStartVertex, const FVector& EdgeEndVertex) const
{
	return (EdgeEndVertex - EdgeStartVertex).GetUnsafeNormal();
}

void AXD_RecastNavMesh::Destroyed()
{
	CleanExtraData();

	Super::Destroyed();
}

void AXD_RecastNavMesh::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CleanExtraData();

	Super::EndPlay(EndPlayReason);
}

void AXD_RecastNavMesh::CleanExtraData()
{
	for (auto& Data : NavDataExtras)
	{
		if (Data.Value)
		{
			Data.Value->Destroy();
		}
	}
}

void AXD_RecastNavMesh::DrawTracedJumpLink(UWorld* World, FVector StartLocation, const FVector& TargetLocation)
{
#if WITH_EDITOR
	if (bShowDebugInfo)
	{
		DrawDebugDirectionalArrow(World, StartLocation, TargetLocation, 5.f, FColor::Green, true, -1.f, SDPG_World, 1.f);
	}
#endif
}

TSharedRef<FRecastTileGenerator> FXD_RecastNavMeshGenerator::CreateTileGenerator(const FIntPoint& Coord, const TArray<FBox>& DirtyAreas)
{
	TSharedRef<FXD_RecastTileGenerator> TileGenerator = MakeShareable(new FXD_RecastTileGenerator(*this, Coord));
	TileGenerator->Setup(*this, DirtyAreas);
	return TileGenerator;
}

bool FXD_RecastTileGenerator::GenerateCompressedLayers(FNavMeshBuildContext& BuildContext)
{
// 	FCompositeNavModifier CompositeNavModifier;
// 
// 	CompositeNavModifier.Add(FSimpleLinkNavModifier());
// 	AppendModifier(CompositeNavModifier, {});
	return Super::GenerateCompressedLayers(BuildContext);
}
