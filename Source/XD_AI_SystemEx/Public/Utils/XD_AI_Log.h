// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
DECLARE_LOG_CATEGORY_EXTERN(XD_AI_Log, Log, All);

#define XD_AIDisplay_LOG(FMT, ...) UE_LOG(XD_AI_Log, Display, TEXT(FMT), ##__VA_ARGS__)
#define XD_AIWarning_LOG(FMT, ...) UE_LOG(XD_AI_Log, Warning, TEXT(FMT), ##__VA_ARGS__)
#define XD_AIError_Log(FMT, ...) UE_LOG(XD_AI_Log, Error, TEXT(FMT), ##__VA_ARGS__)

#define XD_AIDisplay_VLog(LogOwner, FMT, ...) UE_VLOG(LogOwner, XD_AI_Log, Display, TEXT(FMT), ##__VA_ARGS__)
#define XD_AIWarning_VLog(LogOwner, FMT, ...) UE_VLOG(LogOwner, XD_AI_Log, Warning, TEXT(FMT), ##__VA_ARGS__)
#define XD_AIError_VLog(LogOwner, FMT, ...) UE_VLOG(LogOwner, XD_AI_Log, Error, TEXT(FMT), ##__VA_ARGS__)
