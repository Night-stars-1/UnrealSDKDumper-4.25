#pragma once
#include <unordered_map>
#include "wrappers.h"
class ClassSizeFixer
{

  inline static void LoadStruct(UE_UStruct& to_fix) {
    void* addr = to_fix.GetAddress();
    sizeMp[addr] = to_fix.GetSize();
  }
  
  inline static int FixStruct(UE_UStruct& to_fix) {
    auto super = to_fix.GetSuper();
    if (!super) {
      return 0;
    }
    auto superAddr = super.GetAddress();
    if (sizeMp.count(superAddr) == 0) {
      printf("Unknow super for struct: %s\n", to_fix.GetFullName().c_str());
      return 0;
    }
    bool fixed = false;
    for (auto prop = to_fix.GetChildProperties().Cast<UE_FProperty>(); prop; prop = prop.GetNext().Cast<UE_FProperty>()) {
      auto propInterface = prop.GetInterface();
      auto member_offset = propInterface.GetOffset();
      if (sizeMp[superAddr] > member_offset) {
        fixed = true;
        sizeMp[superAddr] = member_offset;
      }
    }
    if (fixed) {
      printf("Fixed class [%s] from 0x%X -> 0x%X \n", super.GetName().c_str(), super.GetSize(), sizeMp[superAddr]);
    }
    to_fix.FixedSize = sizeMp[superAddr];
    return fixed;
  }

public:
  // map struct/class -> it's size
  static std::unordered_map<void*, int> sizeMp;

  inline static void LoadPackage(UE_UPackage& package) {
    auto& objects = package.Package->second;
    for (auto& object : objects) {
      if (object.IsA<UE_UClass>()) {
        LoadStruct(object.Cast<UE_UStruct>());
      }
      else if (object.IsA<UE_UScriptStruct>()) {
        LoadStruct(object.Cast<UE_UStruct>());
      }
    }
  }

  inline static int FixAllPackage(std::vector<UE_UPackage>& packages) {
    int totFixed = 0;
    for (auto& package : packages) {
      auto& objects = package.Package->second;
      for (auto& object : objects) {
        if (object.IsA<UE_UClass>()) {
          totFixed += FixStruct(object.Cast<UE_UStruct>());
        }
        else if (object.IsA<UE_UScriptStruct>()) {
          totFixed += FixStruct(object.Cast<UE_UStruct>());
        }
      }
    }
    return totFixed;
  }
};
