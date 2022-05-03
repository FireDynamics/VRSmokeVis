# VRSmokeVis


## Installation
There are two ways to use the visualization toolkit of this project: Either with or without the [Unreal Engine editor](https://www.unrealengine.com/en-US/download).  
The editor-based version offers everything the editor-free version does, but does also add convenient ways to discover the data of a loaded simulation and to customize the appearance of the visualization, especially of the scenery (e.g. change textures of obstructions or add exit signs).  

### Prerequisites
- Python version 3.6 or above
- [FDSReader](https://pypi.org/project/fdsreader/) 1.6 or above
- [PyYAML](https://pypi.org/project/PyYAML/)
- [Pathos](https://pypi.org/project/pathos/)

_These requirements are only mandatory if not manually preprocessing the fds data as described [here](../../wiki#manually-before-runtime)._

### Without editor
When there is no need for the editor, no further installation will be required.  
Just execute the binary for your system in [the latest release](../../releases/). If there is no binary available for your system, please follow to the _With editor_ instructions below instead.

### With editor
This project is based on Unreal Engine 5 which you have to [download](https://www.unrealengine.com/en-US/download) first.  
While it is not mandatory, using a suitable IDE like [Visual Studio](https://visualstudio.microsoft.com/) or [Rider](https://www.jetbrains.com/lp/rider-unreal/) is recommended.  
...

## Usage
