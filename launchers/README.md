# VFEP Launcher Executables

This directory contains source code for building launcher executables that wrap PowerShell scripts with the VFEP logo icon.

## Purpose

Convert PowerShell scripts (`.ps1`) to native Windows executables (`.exe`) with:
- **VFEP logo icon** embedded
- **Version information** in file properties  
- **Professional appearance** in Windows Explorer
- **Easy launch** - double-click instead of right-click → Run with PowerShell

## Files

### Launcher Source Code
`fast_build_launcher.c` - Launcher for fast_build.ps1 script
- `fast_test_launcher.c` - Launcher for fast_test.ps1 script

### Resource Files (Icons + Version Info)
- `fast_build.rc` - Resource script for fast_build.exe
- `fast_test.rc` - Resource script for fast_test.exe

### Build Scripts
- `build_launchers.ps1` - PowerShell script to compile launchers
- `build_launchers.bat` - Batch script to compile launchers (alternative)

## Building the Launchers

### Requirements
- **MinGW-w64** GCC compiler (must be in PATH)
- **windres** resource compiler (included with MinGW)
- **logo.ico** file in `../Image/` directory

### Quick Build

```powershell
# From launchers directory
.\build_launchers.ps1
```

Or:

```batch
build_launchers.bat
```

### Manual Build (if needed)

```batch
cd launchers

REM Compile resource files
windres fast_build.rc -o fast_build_res.o
windres fast_test.rc -o fast_test_res.o

REM Compile and link executables
gcc -O2 -mwindows fast_build_launcher.c fast_build_res.o -o ..\fast_build.exe -lcomdlg32 -lgdi32
gcc -O2 -mwindows fast_test_launcher.c fast_test_res.o -o ..\fast_test.exe -lcomdlg32 -lgdi32
```

## Output

After building, the following executables are created in the repository root:

- `fast_build.exe` - Launches fast_build.ps1 with VFEP icon
- `fast_test.exe` - Launches fast_test.ps1 with VFEP icon

## Usage

### Instead of:
```powershell
.\fast_build.ps1
.\fast_test.ps1
```

### Simply:
```
Double-click fast_build.exe
Double-click fast_test.exe
```

Or from command line:
```batch
fast_build.exe
fast_test.exe
```

## Technical Details

### How It Works
1. Launcher `.exe` parses command-line arguments
2. Locates corresponding `.ps1` script in current directory
3. Invokes PowerShell with `-ExecutionPolicy Bypass` to run the script
4. Waits for script completion and returns exit code
5. Shows console output in real-time

### Icon Resource
- Icon file: `../Image/logo.ico`
- Embedded in `.exe` at compile time via `.rc` resource script
- Visible in Windows Explorer, taskbar, Alt+Tab, etc.

### Version Information
Each `.exe` includes Windows version information:
- Company: VFEP Development Team
- Product: VFEP Build Tools
- Version: 10.0.0.0
- Copyright: © 2026

View in Windows Explorer: Right-click `.exe` → Properties → Details tab

## Updating the Icon

To change the logo:
1. Replace `../Image/logo.ico` with new icon file
2. Rebuild launchers: `.\build_launchers.ps1`

## Adding More Launchers

To create additional launcher executables:

1. **Create launcher source** (`my_script_launcher.c`):
   ```c
   // Based on fast_build_launcher.c template
   // Update script filename in snprintf to reference your .ps1 file
   ```

2. **Create resource file** (`my_script.rc`):
   ```rc
   // Based on fast_build.rc template
   // Update FileDescription, InternalName, OriginalFilename
   ```

3. **Add to build script**:
   ```powershell
   # In build_launchers.ps1, add compilation steps
   windres my_script.rc -o my_script_res.o
   gcc -O2 -mwindows my_script_launcher.c my_script_res.o -o ..\my_script.exe -lcomdlg32 -lgdi32
   ```

## Troubleshooting

### "gcc not found"
- Install MinGW-w64: https://www.mingw-w64.org/
- Add MinGW `bin` directory to PATH
- Restart terminal/IDE

### "windres not found"
- windres is included with MinGW-w64
- Ensure full MinGW installation (not just GCC)

### "logo.ico not found"
- Verify file exists at `../Image/logo.ico` relative to launchers directory
- Check path in `.rc` files is correct

### Launcher doesn't execute script
- Verify `.ps1` script exists in repository root
- Check script filename matches in launcher `.c` code
- Ensure PowerShell is installed and in PATH

## Benefits

✅ **Professional appearance** - Icon in Windows Explorer
✅ **User-friendly** - Double-click to run (no PowerShell popup)
✅ **Version tracking** - Embedded version info visible in Windows
✅ **Distribution** - Single `.exe` file easier to share than `.ps1` + instructions
✅ **Consistent branding** - VFEP logo on all tools

---

**Last Updated**: February 12, 2026  
**VFEP Version**: Phase 10 Complete
