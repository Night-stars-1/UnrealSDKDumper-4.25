#include "EngineHeaderExport.h"
#include <Windows.h>
#include <unordered_map>
#include <string>
#include <vector>
#include <filesystem>
#include "dumper.h"
#include "defs.h"


HMODULE GetMainModule() {
  HMODULE hModule = 0;
  GetModuleHandleEx(
    GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
    (LPCTSTR) GetMainModule,
    &hModule
  );
  return hModule;
}

bool EngineHeaderExport::LoadResourceText(std::string& str, int resourceID)
{
  static std::unordered_map<int, std::string> mp;
  if (mp.count(resourceID)) {
    str = mp[resourceID];
    return true;
  }
  HRSRC hRes = FindResource(GetMainModule(), MAKEINTRESOURCE(resourceID), MAKEINTRESOURCE(TEXTFILE));
  if (!hRes) return false;
  HGLOBAL hData = LoadResource(GetMainModule(), hRes);
  if (!hData) return false;
  DWORD hSize = SizeofResource(GetMainModule(), hRes);
  char* hFinal = (char*)LockResource(hData);
  str.assign(hFinal, hSize);
  mp[resourceID] = str;
  return true;
}

void EngineHeaderExport::ReplaceVAR(std::string& str) {
  auto replaceSubstr = [](std::string& originalStr, std::string substring, std::string replacement) {
    size_t pos = 0;
    const size_t substringLength = substring.length();
    const size_t replacementLength = replacement.length();

    while ((pos = originalStr.find(substring, pos)) != std::string::npos) {
      originalStr.replace(pos, substringLength, replacement);
      pos += replacementLength; // Move past the replaced substring
    }
  };
  auto dumper_ins = Dumper::GetInstance();

  // windows \r\n 会识别成两个换行，要避免这种情况
  replaceSubstr(str, "\r\n", "\n");

  replaceSubstr(str, "VAR_GAME_NAME", dumper_ins->gameName);
  replaceSubstr(str, "VAR_GAME_VERSION", "default" /*TODO: recognise the game version*/);
  replaceSubstr(str, "VAR_NAMESPACE", GNameSpace);
}


