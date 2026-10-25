# Simple Gain

A JUCE audio effect learning project by YohanSenanayake, based on [Pamplejuce](https://github.com/sudara/pamplejuce).

We are working through the workflow one stage at a time: setup, baseline build, gain implementation and tests, debugging, then release packaging.

## Project setup

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

## Current stage: debug the VST3 on Linux

The baseline Debug VST3 and Standalone have been built, and the Standalone has been tried. Next we will inspect the VST3 inside JUCE AudioPluginHost before adding gain processing.

The checked-in VS Code configuration uses the C/C++ extension and GDB (`/usr/bin/gdb`). It uses workspace-relative paths so the checkout can move between directories.

### First debug session

1. Open **Run and Debug**, choose **VST3: JUCE AudioPluginHost (Linux)**, and press **F5**.
2. The pre-launch tasks configure `Builds` as Debug with `JUCE_BUILD_EXTRAS=ON`, then build only `SimpleGain_VST3` and `AudioPluginHost`. The first host build takes longer; later builds are incremental.
3. In the host, drag this entire VST3 bundle onto the graph (not the inner `.so`):

   ```text
   Builds/SimpleGain_artefacts/Debug/VST3/Simple Gain.vst3
   ```

4. Configure the host's audio device settings, then connect both channels from Audio Input → Simple Gain → Audio Output. Use headphones if the input is a microphone to avoid feedback. Double-click Simple Gain to open its editor.
5. Save the graph as a `.filtergraph` file under `Builds/`. AudioPluginHost reopens its most recently saved graph on the next launch, including the plugin path and connections. Graph files may store absolute paths; recreate or rescan the plugin after moving the checkout.

### Breakpoints and repeated edits

- Set a breakpoint inside `PluginEditor::paint` and reopen the plugin editor to exercise message-thread debugging.
- Set a breakpoint inside `PluginProcessor::processBlock` to inspect an audio block. Audio processing must be running; stopping this thread interrupts playback. Disable this breakpoint before resuming normal playback or it will stop every block.
- A hollow breakpoint before the VST3 loads is expected: plugin code lives in a dynamically loaded library. The launch configuration enables pending breakpoints; symbols should resolve when the host loads the plugin.
- Constructor and `prepareToPlay` breakpoints are useful for initialization. Scanning may use a separate process, so use the plugin instance in the host graph to debug its lifecycle.
- Close the host or stop debugging before editing/rebuilding. Press F5 after each change to rebuild and launch a fresh host process. There is no hot reload of a loaded VST3.

`tasks.json` runs CMake; `launch.json` starts the host under GDB. No global plugin installation is needed. To adapt this workflow to another Pamplejuce project, change the plugin target in `tasks.json` and load that project's Debug VST3 in the host.

The equivalent terminal commands are:

```bash
cmake -B Builds -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DJUCE_BUILD_EXTRAS=ON
cmake --build Builds --config Debug --target SimpleGain_VST3 AudioPluginHost --parallel 2
```

`JUCE_BUILD_EXTRAS` registers other JUCE tools too. These commands and the F5 tasks build only the named targets; an unqualified build now includes the other extras. This option is local to the build cache and does not enable extras in CI.

Use **Release** later for music sessions and distribution. CI and installer configuration have not been exercised locally. The template's commercial EULA is excluded from the installer pending a license review before distribution.
