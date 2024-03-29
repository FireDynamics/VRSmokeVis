# VRSmokeVis


## Installation and Usage
There are two ways to use the visualization toolkit of this project: Either with or without the [Unreal Engine editor](https://www.unrealengine.com/en-US/download).  
The editor-based version offers everything the editor-free version does, but does also add convenient ways to discover the data of a loaded simulation and to customize the appearance of the visualization, especially of the scenery (e.g. change textures of obstructions or add exit signs).  

### Prerequisites
- Python version 3.6 or above
- [FDSReader](https://pypi.org/project/fdsreader/) 1.8.4 or above
- [PyYAML](https://pypi.org/project/PyYAML/)
- [Pathos](https://pypi.org/project/pathos/)
- [Unreal Engine setup for C++-based projects](https://www.reddit.com/r/unrealengine/comments/8yhbq3/visual_studio_related_error/)

_These requirements are only mandatory if not manually preprocessing the fds data as described [here](../../wiki#manually-before-runtime)._

### Without editor
When there is no need for the editor, no further installation will be required.  
Just execute the binary for your system in [the latest release](../../releases/). If there is no binary available for your system, please follow to the _With editor_ instructions below instead.  

Now you can simply start the application which will prompt you to enter the location of your FDS simulation (either the _[chid].smv_ file without manual preprocessing or the _[chid]-smv.yaml_ file after preprocessing) and the destination where loaded data should be stored. After loading the simulation, the geometry is automatically generated and placed into the world. By pressing "H" (see [keyboard controls](#keyboard-controls) for more shortcuts), a UI will be shown which lets you select the data you want to load and show in the scene. Pressing "P" will then start/unpause the visualization of the simulation.  

_Be aware: The non-editor version does not yet work with VR as the VR UI is still work in progress and does not yet allow to enter a simulation at runtime._

### With editor
This project is based on Unreal Engine 5 which you have to [download](https://www.unrealengine.com/en-US/download) first.  
While it is not mandatory, using a suitable IDE like [Visual Studio](https://visualstudio.microsoft.com/) or [Rider](https://www.jetbrains.com/lp/rider-unreal/) is recommended.  

After cloning the repository, open the project inside the Unreal Engine. The project will be built automatically and the editor will be opened with the project content loaded in the content browser.  

Now you can simply drag-and-drop the simulation-file from your explorer into the content browser (either the _[chid].smv_ file without manual preprocessing or the _[chid]-smv.yaml_ file after preprocessing), which will load the simulation metadata and save it to disk. Now drag a Simulation-Blueprint (*/[PluginRoot]/Blueprints/BP_Simulation*) into the level and set the _SimulationAsset_ to the newly created one (probably called *SA_[chid]*, was generated by dragging the simulation file into the editor). If you want the geometry to be created automatically, just press the button _Spawn simulation geometry_ in the same properties window below the _SimulationAsset_ property.  

### Configuration
After starting the project once, you will find a configuration file to configure ColorMaps, etc. More information about the configuration file [here](../../wiki/config).

## Keyboard and VR controls
| Keyboard Key | VR Controller Key |  Action  |
|:---:|:---:|----------|
|  H  | B | Show/Hide UI |
|  P  | A | Pause/Unpause simulation |
|  F  | / | Fast-Forward the simulation time by 20 steps |
|  R  | / | Rewind the simulation time by 20 steps |
|  O  | / | Open/Close simulation loading prompt |
| Tab | / | Toggle Spectator On/Off |
| Left Mouse Button | Trigger press | Left click |
