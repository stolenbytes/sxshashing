# sxshashing

This is sample code for the technique described in the article **SXS, hashing and persistence**.

To build and test code you will need **Visual Studio Build tools** 2013, 2015 or 2017, either standalone or one integrated with Visual Studio, and **cmake**:

* [Visual C++ 2015 Build tools](http://landinghub.visualstudio.com/visual-cpp-build-tools)
* [Build Tools for Visual Studio 2017](https://www.visualstudio.com/downloads/#build-tools-for-visual-studio-2017)
* [cmake](https://cmake.org/)

Navigate to the folder where you have cloned this repository from Build tools command prompt and type (if cmake.exe is in your %PATH%):

```
cmake.bat
```

This will compile **sxshashing.exe**, by running it it will create **sxshashing.exe.local** which will have full winsxs path inside, and relaunch itself. If everything went fine it will display messagebox saying that comctl32.dll is loaded from .local folder.

**sxshashing.py** is python script which can be used to extract the first dependent assembly from the embeded manifest, and will also generate name for faked version. You will need [pefile](https://github.com/erocarrera/pefile) for python.

Example:
```
#sxshashing.py calc.exe
Full WinSXS path:
x86_microsoft.windows.common-controls_6595b64144ccf1df_6.0.0.0_none_346120bcec3f883c
Winners key:
x86_microsoft.windows.common-controls_6595b64144ccf1df_none_aaab8e0a9f4bd480
Full WinSXS path fake version:
x86_microsoft.windows.common-controls_6595b64144ccf1df_6.0.65535.65535_none_02695abd61c0090
```

or to fake with different version:

```
#sxshashng.py calc.exe 6.0.65535.1000
Full WinSXS path:
x86_microsoft.windows.common-controls_6595b64144ccf1df_6.0.0.0_none_346120bcec3f883c
Winners key:
x86_microsoft.windows.common-controls_6595b64144ccf1df_none_aaab8e0a9f4bd480
Full WinSXS path fake version:
x86_microsoft.windows.common-controls_6595b64144ccf1df_6.0.65535.1000_none_972634cc7771ad3d
```
