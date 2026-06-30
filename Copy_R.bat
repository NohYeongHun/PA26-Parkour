xcopy		/y/s	.\Engine\Public\*.*			.\EngineSDK\Inc\

xcopy		/y		.\Engine\Bin\Release\Engine.dll	.\Client\Bin\Release\
xcopy		/y		.\Engine\Bin\Release\Engine.dll	.\Editor\Bin\Release\
xcopy		/y		.\Engine\Bin\Release\Engine.lib	.\EngineSDK\Lib\Release\
xcopy		/y		.\Engine\Bin\ShaderFiles\*.*		.\Client\Bin\ShaderFiles\
xcopy		/y		.\Engine\Bin\ShaderFiles\*.*		.\Editor\Bin\ShaderFiles\