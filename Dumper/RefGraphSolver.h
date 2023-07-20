#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <set>
#include <cassert>
#include <fmt/core.h>
#include <fstream>
#include <queue>
#include "wrappers.h"

/*
* Author: BobH
* Time: 2023.7.18
* Usage: 类依赖关系构建图, 拓扑排序, 获得正确头文件顺序
*/

const bool verboseDebug = 1;
const bool ignoreTemplateRef = true; // 模板引用，且有class标志，编译器可以自动识别不用考虑顺序问题
const bool ignoreFuncParamRef = true; // 函数参数引用，有class标志，编译器可自动识别不用考虑顺序问题

#define DEBUG

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
      static int id = 0;
      debug_id = ++id;
      packageName = name;
      indeg = 0;
      outdeg = 0;
      neighbors = std::vector<Node*>();
    }
    std::string packageName;
    std::vector<Node*> neighbors;
    int indeg, outdeg;
    int debug_id;
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
    std::string packageName = package.packageName;
    if (packageName == "CoreUObject") return;
    for (auto& klass : package.Classes) {
      typeDefMap[klass.ClassName] = packageName;
    }
    for (auto& stru : package.Structures) {
      typeDefMap[stru.ClassName] = packageName;
    }
    for (auto& enums : package.Enums) {
      typeDefMap[enums.EnumName] = packageName;
    }
    if(package.Classes.size() || package.Structures.size() || package.Enums.size()) PushNode(packageName);
  }

  static bool AddEdge(std::string packageReferer, std::string packageReferee) {
    static std::ofstream output("relation.txt");
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
    pReferer->indeg++;
    pReferee->outdeg++;
    pReferee->neighbors.push_back(pReferer);
    output << pReferer->packageName << " referes " << pReferee->packageName << std::endl;
    return true;
  }

  static void FixUndefinedClassMember(UE_UPackage::Member& member) {
    // 修复结构中一些没出现的结构，直接用char代替
    member.Type = "/*" + member.Type + "*/" + "char";
    member.Name += fmt::format("[{:#0x}]", member.Size);
  }

  static bool CanIgnoreRef(std::string typeName) {
    // suppose that all template can be ignored.
    // printf("Judge ignore: %s\n", typeName.c_str());
    return true;
  }

  static void BuildRefGraph(UE_UPackage& package) {
    std::string packageName = package.packageName;
    if (packageName == "CoreUObject") {
      // 处理特殊的类，不要使用dump数据
      package.Classes.clear();
      package.Enums.clear();
      package.Structures.clear();
      return;
    }

    // 存储依赖的类型，去重
    std::set<std::string> refTypes;

    // 存储依赖的package，去重
    std::set <std::string> refPackages;

    auto processStruct = [&refTypes, &refPackages](UE_UPackage::Struct& klass) {
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
          bool should_fix = false;
          for (auto& tname : genericTypes) {
            assert(tname != "");
            if (typeDefMap.count(tname) == 0) {
              should_fix = true;
              FixUndefinedClassMember(member);
              break;
            }
          }
          if (!should_fix && ignoreTemplateRef && CanIgnoreRef(purename)) {
            continue;
          }
          if(!should_fix)
            for (auto& tname : genericTypes) {
              assert(tname != "");
              refTypes.insert(tname);
            }
        }
        else {
          assert(purename != "");
          if (typeDefMap.count(purename) == 0)
            FixUndefinedClassMember(member);
          else
            refTypes.insert(purename);
        }
      }

      // 处理函数类型依赖
      for (auto& function : klass.Functions) {
        // 处理返回值
        bool deleteFunction = false; // 如果这个函数返回值有问题，那就设置成void了
        auto purename = GetPureTypeName(function.RetType);
        if (purename.find("<") != std::string::npos) {
          // generic type
          auto genericTypes = GetGenericTypes(purename);
          bool should_fix = false;
          for (auto& tname : genericTypes) {
            assert(tname != "");
            if (typeDefMap.count(tname) == 0) {
              should_fix = true;
              function.RetType = "void";
              function.CppName = "void " + function.FuncName;
              break;
            }
          }
          if (!should_fix && ignoreTemplateRef && CanIgnoreRef(purename)) {
            continue;
          }
          if(!should_fix)
            for (auto& tname : genericTypes) {
              assert(tname != "");
              refTypes.insert(tname);
            }
        }
        else {
          assert(purename != "");
          if (typeDefMap.count(purename) == 0) {
            function.RetType = "void";
            function.CppName = "void " + function.FuncName;
          }
          else {
            refTypes.insert(purename);
          }
        }
        // 处理参数
        for (auto paramtype : function.ParamTypes) {
          auto purename = GetPureTypeName(paramtype);
          if (purename.find("<") != std::string::npos) {
            // generic type
            auto genericTypes = GetGenericTypes(purename);
            bool should_fix = false;
            for (auto& tname : genericTypes) {
              assert(tname != "");
              if (typeDefMap.count(tname) == 0) {
                should_fix = true;
                function.Params = "/*" + function.Params + "*/";
                function.ParamTypes.clear();
                goto end;
              }
            }
            if (!should_fix && ignoreTemplateRef && CanIgnoreRef(purename)) {
              continue;
            }
            if(!should_fix)
              for (auto& tname : genericTypes) {
                assert(tname != "");
                refTypes.insert(tname);
              }
          }
          else {
            assert(purename != "");
            if (typeDefMap.count(purename) == 0) {
              function.Params = "/*" + function.Params + "*/";
              function.ParamTypes.clear();
              goto end;
            }
            else {
              if (ignoreFuncParamRef) continue;
              refTypes.insert(purename);
            }
          }
        }
        end:;
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

    // 链接关系
    for(auto& targetPackage : refPackages) {
      if (packageName == targetPackage) continue;
      AddEdge(packageName, targetPackage);
    }
  }

  static void FixPackageTypeOrder(UE_UPackage& package) {
    std::string packageName = package.packageName;
    if (packageName == "CoreUObject") {
      // 处理特殊的类，不要使用dump数据
      package.Classes.clear();
      package.Enums.clear();
      package.Structures.clear();
      return;
    }

    /// DEBUG!!!
    // if (packageName != "Engine") return;

    // inclassTypeName -> Node*
    std::unordered_map<std::string, Node*> classMp, structMp;
    std::unordered_map<std::string, int> typeMap; // 区分到底是struct还是class

    auto isTypeInCurrentPackage = [&packageName](std::string typeName) {
      if (typeDefMap.count(typeName) == 0) return false;
      return typeDefMap[typeName] == packageName;
    };

    auto insertEdge = [&typeMap](std::string referer, std::string referee, std::unordered_map<std::string, Node*>& nodeMp) {
      if (typeMap[referer] != typeMap[referee]) return;  // class -> struct, struct -> class 没必要处理
      if (referer == referee) return; // 消除自环
      Node* per, * pee;
      if (nodeMp.count(referer)) {
        per = nodeMp[referer];
      }
      else {
        per = new Node(referer);
        nodeMp[referer] = per;
      }
      if (nodeMp.count(referee)) {
        pee = nodeMp[referee];
      }
      else {
        pee = new Node(referee);
        nodeMp[referee] = pee;
      }
      pee->outdeg++;
      per->indeg++;
      pee->neighbors.push_back(per);
      // if (verboseDebug) printf("SelfAddEdge %s -> %s\n", referer.c_str(), referee.c_str());
    };

    auto processStruct = [&isTypeInCurrentPackage, &insertEdge](UE_UPackage::Struct& klass, std::unordered_map<std::string, Node*>& mp) {
      if (klass.ClassName == "FALSAnimRotateInPlace") {
        // _CrtDbgBreak();
      }
      // 处理继承依赖
      auto& superName = klass.SuperName;
      assert(superName.find("<") == std::string::npos);  // Should not be possible?
      if (superName != "FNone" && superName != "" && isTypeInCurrentPackage(superName)) {
        insertEdge(klass.ClassName, superName, mp);
      }

      // 处理成员类型依赖
      for (auto& member : klass.Members) {
        if (member.Type.find("*") != std::string::npos) {
          // class pointre, can be ignored
          continue;
        }
        auto purename = GetPureTypeName(member.Type);
        if (purename.find("<") != std::string::npos) {
          // generic type
          auto genericTypes = GetGenericTypes(purename);
          if (ignoreTemplateRef && CanIgnoreRef(purename)) {
            continue;
          }
          for (auto& tname : genericTypes) {
            assert(tname != "");
            if (isTypeInCurrentPackage(tname)) insertEdge(klass.ClassName, tname, mp);
          }
        }
        else {
          assert(purename != "");
          if (isTypeInCurrentPackage(purename)) insertEdge(klass.ClassName, purename, mp);
        }
      }

      // 处理函数类型依赖
      for (auto& function : klass.Functions) {
        // 处理返回值
        if(function.RetType.find("*") == std::string::npos){
          auto purename = GetPureTypeName(function.RetType);
          if (purename.find("<") != std::string::npos) {
            // generic type
            auto genericTypes = GetGenericTypes(purename);
            if (ignoreTemplateRef && CanIgnoreRef(purename)) {
              continue;
            }
            for (auto& tname : genericTypes) {
              assert(tname != "");
              if (isTypeInCurrentPackage(tname)) insertEdge(klass.ClassName, tname, mp);
            }
          }
          else {
            assert(purename != "");
            if (isTypeInCurrentPackage(purename)) insertEdge(klass.ClassName, purename, mp);
          }
        }
        // 处理参数
        for (auto paramtype : function.ParamTypes) {
          auto purename = GetPureTypeName(paramtype);
          if (purename.find("<") != std::string::npos) {
            // generic type
            auto genericTypes = GetGenericTypes(purename);
            if (ignoreTemplateRef && CanIgnoreRef(purename)) {
              continue;
            }
            for (auto& tname : genericTypes) {
              assert(tname != "");
              if (isTypeInCurrentPackage(tname)) insertEdge(klass.ClassName, tname, mp);
            }
          }
          else {
            assert(purename != "");
            if (ignoreFuncParamRef) continue;
            if (isTypeInCurrentPackage(purename)) insertEdge(klass.ClassName, purename, mp);
          }
        }
      }
    };

    auto inclassTopo = [&packageName](std::vector<UE_UPackage::Struct>& classes, std::unordered_map<std::string, Node*>& mp) {
      std::vector<UE_UPackage::Struct> newOrder;
      std::unordered_map<std::string, UE_UPackage::Struct> mp2;
      std::unordered_map<std::string, bool> debug;
      std::queue<Node*> queue;
      for (auto& klass : classes) {
        mp2[klass.ClassName] = klass;
        // 将没有约束关系的类先放进去
        if (mp.count(klass.ClassName) == 0) {
          newOrder.push_back(klass);
          debug[klass.ClassName] = true;
        }
        else {
          if (mp[klass.ClassName]->indeg == 0) {
            queue.push(mp[klass.ClassName]);
          }
        }
      }
      while (!queue.empty()) {
        Node* front = queue.front();
        debug[front->packageName] = true;
        newOrder.push_back(mp2[front->packageName]);
        queue.pop();
        for (auto other : front->neighbors) {
          other->indeg--;
          if (other->indeg == 0) queue.push(other);
        }
      }
      for (auto& ori : classes) {
        if (debug.count(ori.ClassName) == 0) {
          printf("[disappear][%s] %s\n", packageName.c_str(), ori.ClassName.c_str());
        }
      }
      assert(newOrder.size() == classes.size());
      classes = newOrder;
    };
    for (auto& klass : package.Classes) {
      typeMap[klass.ClassName] = 1;
    }
    for (auto& klass : package.Structures) {
      typeMap[klass.ClassName] = 2;
    }

    // 找出所有自类型引用
    for (auto& klass : package.Classes) {
      processStruct(klass, classMp);
    }
    for (auto& klass : package.Structures) {
      processStruct(klass, structMp);
    }
    inclassTopo(package.Classes, classMp);
    inclassTopo(package.Structures, structMp);
  }

  static void TopoSort() {
    std::queue<Node*> queue;
    for (Node* node : packageNodes) {
      if (node->indeg == 0) {
        // 表示这个模块已经不依赖于其他模块了，可以输出
        queue.push(node);
      }
    }
    int cnt = 0;
    while (!queue.empty()) {
      Node* front = queue.front();
      queue.pop();
      cnt++;
      if(verboseDebug) printf("package[%d]: %s\n", cnt, front->packageName.c_str());
      packageHeaderOrder.push_back(front->packageName);
      for (auto other : front->neighbors) {
        other->indeg--;
        if (other->indeg == 0) queue.push(other);
      }
    }
    if (cnt != packageNodes.size()) {
      printf("[Warning] Not all packages are included in the header for the reference problem. \n");
      for (Node* node : packageNodes) {
        if (node->indeg != 0) {
          printf("\tPackageName: %s\n", node->packageName.c_str());
        }
      }
    }
  }

public:
  static void Process(std::vector<UE_UPackage>& packages) {
    LoadUnrealPackageDef();
    std::unordered_map<std::string, int> packageNameMp;
    for (UE_UPackage& package : packages) {
      package.packageName = package.GetObject().GetName();
      if (packageNameMp.count(package.packageName) > 0) {
        package.packageName += fmt::format("_{}", ++packageNameMp[package.packageName]);
      }
      else {
        packageNameMp[package.packageName] = 1;
      }
      LoadPackageDef(package);
    }

    if (verboseDebug) {
      printf("\nLoaded all packages defs! \n");
    }

    for (UE_UPackage& package : packages) {
      BuildRefGraph(package);
    }

    if (verboseDebug) {
      printf("\nReference graph builded! \n");
    }

    // 处理类内的依赖关系

    for (UE_UPackage& package : packages) {
      FixPackageTypeOrder(package);
    }

    TopoSort();
  }

  static std::vector<std::string> packageHeaderOrder;

};

#endif