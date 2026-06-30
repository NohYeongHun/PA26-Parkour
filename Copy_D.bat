xcopy		/y/s	.\Engine\Public\*.*		.\EngineSDK\Inc\

xcopy		/y		.\Engine\Bin\Debug\Engine.dll	.\Client\Bin\Debug\
xcopy		/y		.\Engine\Bin\Debug\Engine.dll	.\Editor\Bin\Debug\
xcopy		/y		.\Engine\Bin\Debug\Engine.lib	.\EngineSDK\Lib\Debug\
xcopy		/y		.\Engine\Bin\ShaderFiles\*.*		.\Client\Bin\ShaderFiles\
xcopy		/y		.\Engine\Bin\ShaderFiles\*.*		.\Editor\Bin\ShaderFiles\