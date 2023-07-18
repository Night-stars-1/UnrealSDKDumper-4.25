#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <set>
#include "wrappers.h"
#include <cassert>

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
  struct Node {
    Node() {
      packageName = "";
      indeg = 0;
      outdeg = 0;
      neighbors = std::vector<Node*>();
    }
    Node(std::string name) {
      packageName = name;
      indeg = 0;
      outdeg = 0;
      neighbors = std::vector<Node*>();
    }
    std::string packageName;
    std::vector<Node*> neighbors;
    int indeg, outdeg;
  };

  // map the TypeName -> PackageName
  static std::unordered_map<std::string, std::string> typeDefMap;

  // map the packageName -> NodePointer
  static std::unordered_map<std::string, Node*> nodesMap;

  // Store all the package nodes for topo sort
  static std::vector<Node*> packageNodes;

  // 硬件自带的类型定义，需要手动获取写上去的
  static std::vector<std::string> BasicTypes;
  static std::vector<std::string> CoreUObject;
  static std::vector<std::string> CppTypes;

  static void PushNode(std::string packageName) {
    auto newNode = new Node(packageName);
    nodesMap[packageName] = newNode;
    packageNodes.push_back(newNode);
  }

  static void LoadUnrealPackageDef() {
    // 加载虚幻引擎自带的一些类的定义，这些定义不是从游戏中dump的，是从引擎代码中抠出来的，需要单独加载
    for (auto ClassName : BasicTypes) {
      typeDefMap[ClassName] = "BasicTypes";
    }
    for (auto ClassName : CoreUObject) {
      typeDefMap[ClassName] = "CoreUObject";
    }
    for (auto ClassName : CppTypes) {
      typeDefMap[ClassName] = "CppTypes";
    }
    PushNode("BasicTypes");
    PushNode("CoreUObject");
    PushNode("CppTypes");
  }



  static std::string GetPureTypeName(std::string fullTypeName) {
    auto removeSubstring = [](std::string& input, const std::string toRemove) {
      size_t pos = input.find(toRemove);
      while (pos != std::string::npos) {
        input.erase(pos, toRemove.length());
        pos = input.find(toRemove, pos);
      }
    };
    auto trim = [](std::string& input) {
      size_t start = 0;
      size_t end = input.length() - 1;

      // Find the first non-space character from the beginning of the string
      while (start <= end && std::isspace(input[start])) {
        start++;
      }

      // Find the last non-space character from the end of the string
      while (end >= start && std::isspace(input[end])) {
        end--;
      }

      // Return the trimmed substring
      input = input.substr(start, end - start + 1);
    };
    std::string res = fullTypeName;
    removeSubstring(res, "enum");
    removeSubstring(res, "class");
    removeSubstring(res, "struct");
    removeSubstring(res, "*");
    trim(res);
    return res;
  }

  static std::vector<std::string> GetGenericTypes(std::string genericTypeName) {
    auto removeSubstring = [](std::string& input, const std::string toRemove) {
      size_t pos = input.find(toRemove);
      while (pos != std::string::npos) {
        input.erase(pos, toRemove.length());
        pos = input.find(toRemove, pos);
      }
    };
    auto splitString = [](const std::string& input, const std::string& delimiters) {
      std::vector<std::string> tokens;
      std::string::size_type startPos = 0;
      std::string::size_type endPos = 0;

      while (endPos != std::string::npos) {
        endPos = input.find_first_of(delimiters, startPos);

        // 如果找到了分隔符
        if (endPos != startPos) {
          std::string token = input.substr(startPos, endPos - startPos);
          tokens.push_back(token);
        }

        // 跳过分隔符，继续查找下一个子串
        if (endPos != std::string::npos) {
          startPos = endPos + 1;
        }
      }
      tokens.pop_back();
      return tokens;
    };
    removeSubstring(genericTypeName, " ");
    const std::string delimiters = "<>,";
    return splitString(genericTypeName, delimiters);
  }

  static void LoadPackageDef(UE_UPackage& package) {
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

    PushNode(packageName);

  }

  static bool AddEdge(std::string packageReferer, std::string packageReferee) {
    // packageReferer -> packageReferee
    if (nodesMap.count(packageReferer) == 0) {
      printf("Could not found Referer class node \"%s\"!", packageReferer.c_str());
      return false;
    }
    if (nodesMap.count(packageReferee) == 0) {
      printf("Could not found Referee class node \"%s\"!", packageReferee.c_str() );
      return false;
    }
    auto pReferer = nodesMap[packageReferer];
    auto pReferee = nodesMap[packageReferee];
    pReferer->outdeg++;
    pReferee->indeg++;
    pReferer->neighbors.push_back(pReferee);
    return true;
  }

  static void BuildRefGraph(UE_UPackage& package) {

    // 存储依赖的类型，去重
    std::set<std::string> refTypes;

    // 存储依赖的package，去重
    std::set <std::string> refPackages;

    auto processStruct = [&refTypes, &refPackages](UE_UPackage::Struct klass) {
      // 处理继承依赖
      auto& superName = klass.SuperName;
      if (superName == "FNone" && verboseDebug) {
        // printf("EmptySuperClass: %s\n", klass.ClassName.c_str());
      }
      assert(superName.find("<") == std::string::npos);  // Should not be possible?
      if (superName != "FNone" && superName != "") refTypes.insert(superName);

      // 处理成员类型依赖
      for (auto& member : klass.Members) {
        auto purename = GetPureTypeName(member.Type);
        if (purename.find("<") != std::string::npos) {
          // generic type
          auto genericTypes = GetGenericTypes(purename);
          for (auto& tname : genericTypes) {
            assert(tname != "");
            refTypes.insert(tname);
          }
        }
        else {
          assert(purename != "");
          refTypes.insert(purename);
        }
      }

      // 处理函数类型依赖
      for (auto& function : klass.Functions) {
        // 处理返回值
        auto purename = GetPureTypeName(function.RetType);
        if (purename.find("<") != std::string::npos) {
          // generic type
          auto genericTypes = GetGenericTypes(purename);
          for (auto& tname : genericTypes) {
            assert(tname != "");
            refTypes.insert(tname);
          }
        }
        else {
          assert(purename != "");
          refTypes.insert(purename);
        }
        // 处理参数
        for (auto paramtype : function.ParamTypes) {
          auto purename = GetPureTypeName(paramtype);
          if (purename.find("<") != std::string::npos) {
            // generic type
            auto genericTypes = GetGenericTypes(purename);
            for (auto& tname : genericTypes) {
              assert(tname != "");
              refTypes.insert(tname);
            }
          }
          else {
            assert(purename != "");
            refTypes.insert(purename);
          }
        }
      }
    };

    // 找出所有类型引用
    for (auto& klass : package.Classes) {
      processStruct(klass);
    }
    for (auto& klass : package.Structures) {
      processStruct(klass);
    }
    
    // 找出所有Package引用
    for (auto& refType : refTypes) {
      if (typeDefMap.count(refType) == 0) {
        printf("[Warning] Cannot find type \"%s\" in any package!\n", refType.c_str());
        continue;
      }
      refPackages.insert(typeDefMap[refType]);
    }

  }
public:
  static void Process(std::vector<UE_UPackage>& packages) {
    LoadUnrealPackageDef();
    for (UE_UPackage& package : packages) {
      LoadPackageDef(package);
    }

    if (verboseDebug) {
      printf("Loaded all packages defs! \n");
    }

    for (UE_UPackage& package : packages) {
      BuildRefGraph(package);
    }
  }

};

#endif