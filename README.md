# UnrealSDKDumper 
![workflow](https://github.com/BobHUnrealTech/UnrealSDKDumper-4.25/actions/workflows/build.yml/badge.svg)

### 支持的游戏
 - Fortnite
 - Rogue Company
 - Dead By Daylight
 - Witch It
 - Brickadia
 - POLYGON
 - SCUM
 - Scavengers
 - The Isle
 - Splitgate
 - Dauntless
 - Boundary
 - The Cycle: Frontier
 - Calabiyau(卡拉彼丘)
 - Seria(晶核：魔导觉醒)
 - Game(尘白禁区)
### 支持引擎版本: 
- UE 4.23-4.27

### 我们做了什么

- 根据不同package包含的类的引用关系，进行拓扑排序，确定头文件正确引用顺序
- 测试大量游戏，找出一些造成sdk编译出错的变量名（例如：+.()<> 中文等），并进行处理
- 生成一个导入即可编译成功的sdk.h文件以及导入即可用的SDK文件夹

### Todo:

- 自动生成VS脚手架项目，尝试做到像CheatGear一样能够打开即用
- 生成cpp代理文件，实现能够通过ProcessEvent调用引擎的函数
- 生成偏移断言测试项目，方便测试各个成员偏移是否符合预期

### Contributor

Modified by BobH