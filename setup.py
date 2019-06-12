# cxFreeze skripta za Šahista.
# Vključimo tudi preveden modul Sahsit in Boost.Python DLL.

# Ročno dodaj msvc****.dll za VS s katerim kompajlaš modul sahist!

import sys
from cx_Freeze import setup, Executable

build_exe_options = {
	"packages": ["os", "sahist", "tkinter", "threading", "math", "tkinter.messagebox", "random", "itertools", "enum"],
	"include_files" : ["figure/", "sahist.ico"],
	"include_msvcr" : True,
	"compressed" : True,                 
	"optimize" : 2,
	"append_script_to_exe" : True
}

base = None
if sys.platform == "win32":
    base = "Win32GUI"

setup(  name = "sahist_gui",
        version = "1.0",
        description = "Šahist",
        options = {"build_exe": build_exe_options },
        executables = [Executable("sahist_gui.py", base=base,
        	icon = "sahist.ico")])