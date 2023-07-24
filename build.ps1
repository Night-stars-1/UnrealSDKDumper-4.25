param([string]$buildtfm = 'x64')

$build64	 = $buildtfm -eq 'x64'
$build86	 = $buildtfm -eq 'x86'

$msbuildPath = Split-Path (& "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -requires Microsoft.Component.MSBuild -find MSBuild\Current\Bin\amd64\MSBuild.exe | Select-Object -First 1) -Parent
Write-Host "VisualStudioCompiler:" + $msbuildPath
$env:PATH = $msbuildPath + ';' + $env:PATH

if($build86){
    msbuild UnrealDumper.sln -target:Dumper /p:Platform=x86 /p:Configuration=Release
}

if($build64){
    msbuild UnrealDumper.sln -target:Dumper /p:Platform=x64 /p:Configuration=Release
}