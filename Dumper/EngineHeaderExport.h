#pragma once
#include <string>
#include <filesystem>
#include <fmt/core.h>
#include <fstream>
#include "resource.h"
/*
* Author: BobH
* Time: 2023.7.19
* Usage: 引擎自带的头文件的导出，可能需要判断游戏的版本？
*/


#ifndef ENGINEHEADEREXPORT
#define ENGINEHEADEREXPORT

class EngineHeaderExport
{
  static bool LoadResourceText(std::string& str, int resourceID);

  static void ReplaceVAR(std::string& str);


public:
  
  static void Process(std::filesystem::path dir) {
    const static std::vector<std::pair<int, std::string>> EngineFiles = {
      std::make_pair(BASICTYPES_PACKAGE, "BasicTypes_Package.h"),
      std::make_pair(BASICTYPES_CLASSES, "BasicTypes_Classes.h"),
      std::make_pair(BASICTYPES_STRUCT, "BasicTypes_Structs.h"),
      std::make_pair(COREUOBJECT_PACKAGE, "CoreUObject_Package.h"),
      std::make_pair(COREUOBJECT_CLASSES, "CoreUObject_Classes.h"),
      std::make_pair(COREUOBJECT_STRUCT, "CoreUObject_Structs.h"),
      std::make_pair(COREUOBJECT_CPP, "CoreUObject_Package.cpp"),
      std::make_pair(BASICTYPES_CPP, "BasicTypes_Package.cpp"),
      std::make_pair(COREUOBJECT_PARAM, "CoreUObject_Params.h"),
    };
    // 导出两个引擎文件
    for (auto& enginefile : EngineFiles) {
      std::ofstream output(dir / enginefile.second);
      std::string content;
      if (!LoadResourceText(content, enginefile.first)) {
        printf("[ERROR] Fail to read engine header file: %s\n", enginefile.second.c_str());
        continue;
      }
      ReplaceVAR(content);
      // fmt::print(file, "{}", content);
      output << content;
    }
  }
};

#endif
