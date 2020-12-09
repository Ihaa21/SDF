@echo off

set CodeDir=..\code
set DataDir=..\data
set LibsDir=C:\Code\Libs
set OutputDir=..\build_win32
set VulkanIncludeDir="C:\VulkanSDK\1.2.135.0\Include\vulkan"
set VulkanBinDir="C:\VulkanSDK\1.2.135.0\Bin"
set AssimpIncludeDir="%LibsDir%\assimp-5.0.1\include"
set AssimpLibDir=%LibsDir%\assimp-5.0.1\lib\RelWithDebInfo

set CommonCompilerFlags=-Od -MTd -nologo -fp:fast -fp:except- -EHsc -Gm- -GR- -EHa- -Zo -Oi -WX -W4 -wd4127 -wd4201 -wd4100 -wd4189 -wd4505 -Z7 -FC
set CommonCompilerFlags=-I %VulkanIncludeDir% %CommonCompilerFlags%
set CommonCompilerFlags=-I %LibsDir% -I %AssimpIncludeDir% %CommonCompilerFlags%
REM Check the DLLs here
set CommonLinkerFlags=-incremental:no -opt:ref user32.lib gdi32.lib Winmm.lib opengl32.lib DbgHelp.lib d3d12.lib dxgi.lib d3dcompiler.lib %AssimpLibDir%\assimp-vc142-mt.lib

IF NOT EXIST %OutputDir% mkdir %OutputDir%

pushd %OutputDir%

del *.pdb > NUL 2> NUL

REM USING GLSL IN VK USING GLSLANGVALIDATOR
call glslangValidator -S frag -e main -g -V -o %DataDir%\sdf_shader.spv %CodeDir%\shader_sdf.cpp

REM USING HLSL IN VK USING DXC
REM set DxcDir=C:\Tools\DirectXShaderCompiler\build\Debug\bin
REM %DxcDir%\dxc.exe -spirv -T cs_6_0 -E main -fspv-target-env=vulkan1.1 -Fo ..\data\write_cs.o -Fh ..\data\write_cs.o.txt ..\code\bw_write_shader.cpp

REM 64-bit build
echo WAITING FOR PDB > lock.tmp
cl %CommonCompilerFlags% %CodeDir%\sdf_demo.cpp -Fmsdf_demo.map -LD /link %CommonLinkerFlags% -incremental:no -opt:ref -PDB:sdf_demo_%random%.pdb -EXPORT:Init -EXPORT:CodeReload -EXPORT:MainLoop
del lock.tmp
call cl %CommonCompilerFlags% -DDLL_NAME=sdf_demo -Fesdf_demo.exe %LibsDir%\framework_vulkan\win32_main.cpp -Fmsdf_demo.map /link %CommonLinkerFlags%

popd
