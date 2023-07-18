#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include "wrappers.h"

/*
* Author: BobH
* Time: 2023.7.18
* Usage: 类依赖关系构建图, 拓扑排序, 获得正确头文件顺序
*/

const bool verboseDebug = 1;

#ifndef REFGRAPHSOLVER
#define REFGRAPHSOLVER

class RefGraphSolver
{

  // map the TypeName -> PackageName
  static std::unordered_map<std::string, std::string> typeDefMap;

  static void LoadUnrealPackageDef() {
    // 加载虚幻引擎自带的一些类的定义，这些定义不是从游戏中dump的，是从引擎代码中抠出来的，需要单独加载
  }

  static void LoadPackageDef(UE_UPackage& package) {
    LoadUnrealPackageDef();
    std::string packageName = package.GetObject().GetName();
    for (auto& klass : package.Classes) {
      typeDefMap[klass.ClassName] = packageName;
    }
    for (auto& stru : package.Structures) {
      typeDefMap[stru.ClassName] = packageName;
    }
    for (auto& enums : package.Enums) {
      typeDefMap[enums.EnumName] = packageName;
    }
    if (verboseDebug) {
      printf("Loaded all packages defs!");
    }
  }

  static void BuildRefGraph(UE_UPackage& package) {

  }
public:
  static void Process(std::vector<UE_UPackage>& packages) {
    for (UE_UPackage& package : packages) {
      LoadPackageDef(package);
    }
    for (UE_UPackage& package : packages) {
      BuildRefGraph(package);
    }
  }

};

#endif