// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
XD_AI_SYSTEMEX_API DECLARE_LOG_CATEGORY_EXTERN(XD_AI_Log, Log, All);

#define XD_AI_Display_LOG(FMT, ...) UE_LOG(XD_AI_Log, Display, TEXT(FMT), ##__VA_ARGS__)
#define XD_AI_Warning_LOG(FMT, ...) UE_LOG(XD_AI_Log, Warning, TEXT(FMT), ##__VA_ARGS__)
#define XD_AI_Error_Log(FMT, ...) UE_LOG(XD_AI_Log, Error, TEXT(FMT), ##__VA_ARGS__)

#define XD_AI_Display_VLog(LogOwner, FMT, ...) UE_VLOG(LogOwner, XD_AI_Log, Display, TEXT(FMT), ##__VA_ARGS__)
#define XD_AI_Warning_VLog(LogOwner, FMT, ...) UE_VLOG(LogOwner, XD_AI_Log, Warning, TEXT(FMT), ##__VA_ARGS__)
#define XD_AI_Error_VLog(LogOwner, FMT, ...) UE_VLOG(LogOwner, XD_AI_Log, Error, TEXT(FMT), ##__VA_ARGS__)
