@echo off

if not defined DevEnvDir (
	call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
)

if not defined DevEnvDir (
	call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
)

set cl_flags=/Gm- /W1 /MP /MTd /Z7 /Od /EHsc /Zc:offsetof- /nologo /D_WINDOWS
set link_flags=/out:candleknight.exe /incremental:no /debug:full /nologo /subsystem:console
set src_files="%cd%"\all_files.cpp
set lib_files=
python build_pre_process.py
pushd build
cl %cl_flags% %src_files% %lib_files% /link %link_flags%
popd

echo build completed.