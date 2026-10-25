# Simple Gain

A JUCE audio effect learning project by YohanSenanayake, based on [Pamplejuce](https://github.com/sudara/pamplejuce).

We are working through the workflow one stage at a time: setup, baseline build, gain implementation and tests, debugging, then release packaging.

## Current stage: project setup

- Internal CMake target: `SimpleGain`; DAW display name: `Simple Gain`.
- Formats: VST3 and Standalone, targeting Windows and Linux.
- IDE: VS Code with the CMake Tools and C/C++ extensions.
- Intel IPP disabled; Windows code signing deferred.
- Audio processing and UI remain the template's pass-through demo.

`CMakeLists.txt` describes what to build. `source/` contains the processor and editor. `tests/` contains the template's Catch2 tests. `.github/workflows/build_and_test.yml` builds and validates on Windows and Linux and prepares installer/ZIP artifacts. Tag-triggered releases are drafts for review.

Dependencies are Git submodules pinned to commits. Initialize them using:

```bash
git submodule update --init --recursive
```

This retrieves JUCE, the template CMake helpers, and bundled modules; it does not compile the plugin. Avoid `--remote` while learning so dependency versions stay reproducible.

## Next stage: baseline build

Check the compiler, CMake, native JUCE development dependencies, and build tool first. Ninja is recommended with VS Code; it is not currently installed on the development machine. We will configure and build the unchanged demo together in the next stage.

Development builds will use **Debug** for breakpoints and diagnostics. Use **Release** later for music sessions and distribution. CMake configure/build/test commands are recorded in `CLAUDE.md` (also linked as `AGENTS.md`).

CI and installer configuration have not been exercised yet. The template's commercial EULA is excluded from the installer pending a license review before distribution.
