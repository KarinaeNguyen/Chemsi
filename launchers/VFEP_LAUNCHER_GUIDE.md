# VFEP Unified Launcher

**Single executable for all build and test operations** 🚀

## Quick Start

### Double-Click Launch (Interactive)
Simply double-click `vfep_launcher.exe` and you'll see a friendly menu to choose your operation.

### Command-Line Usage
```cmd
vfep_launcher.exe build    # Build the project
vfep_launcher.exe test     # Run all tests
vfep_launcher.exe both     # Build + Test (full workflow)
```

---

## Features ✨

### Silent Background Execution
- **No console windows** - PowerShell scripts run silently in the background
- **Clean popup dialogs** - Professional progress and result notifications
- **Non-intrusive** - No terminal clutter, just clear status messages

### All-in-One Operation
- **Build** - Compile the entire VFEP project
- **Test** - Run comprehensive test suite
- **Build + Test** - Complete workflow in one click

### Professional Branding
- **VFEP logo icon** - Embedded icon visible in Windows Explorer
- **Version information** - 10.0.0 with full metadata
- **Progress tracking** - Real-time status updates via dialogs

---

## How It Works

1. **User launches** `vfep_launcher.exe` (double-click or command-line)
2. **Selects operation** (if no arguments provided)
3. **Background execution** - PowerShell scripts run with hidden console
4. **Progress notification** - "Building project..." or "Running tests..." dialog
5. **Results displayed** - Success ✓ or Error ✗ with exit code

### Behind the Scenes
- Executes `fast_build.ps1` for build operations
- Executes `fast_test.ps1` for test operations
- Uses `CREATE_NO_WINDOW` + `SW_HIDE` flags to hide PowerShell console
- Waits for completion and captures exit codes
- Shows MessageBoxA dialogs for user feedback

---

## File Details

| File | Size | Description |
|------|------|-------------|
| `vfep_launcher.exe` | ~706 KB | Unified launcher executable |
| `fast_build.ps1` | ~3 KB | Build automation script (called by launcher) |
| `fast_test.ps1` | ~2 KB | Test automation script (called by launcher) |

**Source Code:**
- `launchers/vfep_launcher.c` - Main launcher implementation (WinMain, CreateProcessA)
- `launchers/vfep_launcher.rc` - Windows resource file (icon, version info)
- `launchers/build_launchers.ps1` - Build script (windres + gcc)

---

## Exit Codes

| Code | Meaning |
|------|---------|
| 0 | Success - operation completed successfully |
| 1 | Error - build/test failed (see logs for details) |
| -1 | Fatal error - failed to launch PowerShell process |

---

## Building from Source

### Prerequisites
- MinGW-w64 with GCC and windres
- Windows SDK (for windows.h)
- VFEP logo at `Image/logo.ico`

### Compile
```powershell
cd launchers
.\build_launchers.ps1
```

Or manually:
```bash
windres vfep_launcher.rc -o vfep_launcher_res.o
gcc -O2 -mwindows vfep_launcher.c vfep_launcher_res.o -o ../vfep_launcher.exe -lcomdlg32 -lgdi32
```

---

## Comparison: Old vs New

### Before (Separate Executables)
- ❌ `fast_build.exe` (704 KB) - Build only
- ❌ `fast_test.exe` (704 KB) - Test only
- ❌ Two separate programs to manage
- ✓ Console output visible

**Total**: 1.4 MB, 2 executables

### After (Unified Launcher)
- ✅ `vfep_launcher.exe` (706 KB) - Build, Test, or Both
- ✅ Single program with command-line options
- ✅ Interactive menu when no arguments
- ✅ Silent background execution with popup notifications

**Total**: 706 KB, 1 executable

---

## Dialog Examples

### Progress Dialog (Info)
```
┌─────────────────────────────────────┐
│ VFEP Launcher                   [i] │
├─────────────────────────────────────┤
│                                     │
│  Building project...                │
│                                     │
│  This may take a moment.            │
│  The process is running in the      │
│  background.                        │
│                                     │
│            [ OK ]                   │
└─────────────────────────────────────┘
```

### Success Dialog (Green Checkmark)
```
┌─────────────────────────────────────┐
│ VFEP Launcher - Success        [✓]  │
├─────────────────────────────────────┤
│                                     │
│  Build completed successfully!      │
│                                     │
│  Exit Code: 0                       │
│                                     │
│            [ OK ]                   │
└─────────────────────────────────────┘
```

### Error Dialog (Red X)
```
┌─────────────────────────────────────┐
│ VFEP Launcher - Error          [✗]  │
├─────────────────────────────────────┤
│                                     │
│  Tests failed.                      │
│                                     │
│  Exit Code: 1                       │
│                                     │
│  Check the logs for details.        │
│                                     │
│            [ OK ]                   │
└─────────────────────────────────────┘
```

---

## Advantages

### For Users
- ✅ **Simpler** - One program instead of two
- ✅ **Cleaner** - No console spam, just dialogs
- ✅ **Professional** - Polished UI with logo icon
- ✅ **Faster** - Fewer clicks to get things done

### For Developers
- ✅ **Maintainable** - Single codebase for all operations
- ✅ **Extensible** - Easy to add new operations
- ✅ **Debuggable** - Exit codes propagated correctly
- ✅ **Documented** - Clear code with comments

---

## Technical Details

### Compilation Flags
- `-O2` - Optimize for speed
- `-mwindows` - Build Windows GUI application (no console)
- `-lcomdlg32 -lgdi32` - Link Windows common dialogs and GDI

### Windows API Usage
- `WinMain()` - Entry point for Windows GUI apps
- `CreateProcessA()` - Launch PowerShell scripts
- `MessageBoxA()` - Show dialogs (progress, success, error)
- `WaitForSingleObject()` - Wait for PowerShell process completion
- `GetExitCodeProcess()` - Retrieve exit code from process

### Security
- `-ExecutionPolicy Bypass` - Required to run .ps1 scripts
- `-NoProfile` - Skip user profile loading (faster startup)
- `-WindowStyle Hidden` - Additional console hiding (defense in depth)

---

## Troubleshooting

### "Cannot find fast_build.ps1"
- Ensure you're running `vfep_launcher.exe` from the root directory
- The launcher looks for scripts in the current working directory

### "Failed to launch PowerShell"
- Check that PowerShell is installed and in PATH
- Verify `powershell.exe` is accessible

### Build/Test failures
- Check `cpp_engine/` for compilation errors
- Verify Python environment is activated
- Review logs in `test_results/` directory

---

## Future Enhancements

### Possible Additions
- [ ] Add "Clean" operation (remove build artifacts)
- [ ] Add "Install" operation (deploy to system)
- [ ] Configuration dialog for build options
- [ ] Progress bar instead of simple "please wait" message
- [ ] Log viewer integrated into launcher
- [ ] Tray icon for background operations

### Ideas Welcome!
If you have suggestions for improving the launcher, please open an issue or submit a PR.

---

**Created**: February 12, 2026  
**Version**: 10.0.0  
**Author**: VFEP Development Team  
**License**: See main repository LICENSE file

---

## Quick Reference

```powershell
# Interactive menu
.\vfep_launcher.exe

# Build only
.\vfep_launcher.exe build

# Test only
.\vfep_launcher.exe test

# Full workflow
.\vfep_launcher.exe both

# Show in Explorer (to see icon)
explorer.exe /select,vfep_launcher.exe
```

🚀 **Enjoy the streamlined VFEP workflow!**
