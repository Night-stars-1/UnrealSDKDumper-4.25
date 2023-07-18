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

#ifndef REFGRAPHSOLVER
#define REFGRAPHSOLVER

class RefGraphSolver
{

  // map the TypeName -> PackageName
  static std::vector<std::string, std::string> typeDefMap;

  static void LoadPackageDef(UE_UPackage& package) {
    std::string packageName = package.GetObject().GetName();
    if (packageName == "Engine") {
      for (auto& classes : package.Classes) {
        printf("test: %s\n", classes.CppName.c_str());
      }
    }
  }

  static void BuildRefGraph() {

  }
public:
  static void Process(std::vector<UE_UPackage>& packages) {
    for (UE_UPackage& package : packages) {
      LoadPackageDef(package);
    }
  }

};

#endif