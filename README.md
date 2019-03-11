# Temporal Treemaps

This is an Inviwo implementation of Temporal Treemaps for visualizing trees whose topology and data change over time, as described in our paper:

Wiebke Köpp, Tino Weinkauf, ["Temporal Treemaps: Static Visualization of Evolving Trees"](https://www.csc.kth.se/~weinkauf/publications/abskoepp19a.html) (IEEE VIS 2018).

## Setup

Temporal Treemaps have been implemented in [Inviwo](https://inviwo.org/). The setup instructions for Inviwo are briefly summarized below and can also be found in the [Inviwo Github Wiki](https://github.com/inviwo/inviwo/wiki).

| Dependency | Windows (64 bit) | Linux | OSX|
| --- | --- | --- | --- |
| Git | [.exe](https://github.com/git-for-windows/git/releases/download/v2.15.1.windows.2/Git-2.15.1.2-64-bit.exe)  |  `sudo apt-get install git ` | [.dmg](https://sourceforge.net/projects/git-osx-installer/files/git-2.15.1-intel-universal-mavericks.dmg/download?use_mirror=autoselect)
| CMake  | [.msi](https://cmake.org/files/v3.10/cmake-3.10.1-win64-x64.msi)  |  `sudo apt-get install cmake cmake-qt-gui ` | [.dmg](https://cmake.org/files/v3.10/cmake-3.10.2-Darwin-x86_64.dmg)
| Qt  | [.exe](http://download.qt.io/official_releases/qt/5.9/5.9.3/qt-opensource-windows-x86-5.9.3.exe)<sup>2</sup> | [Qt 5 Install instructions](https://wiki.qt.io/Install_Qt_5_on_Ubuntu)<sup>2</sup> | [.dmg](http://download.qt.io/official_releases/qt/5.9/5.9.3/qt-opensource-mac-x64-5.9.3.dmg)<sup>2</sup>
| C++ | [Visual Studio Website](https://visualstudio.microsoft.com/) | `sudo apt-get install build-essential ` | [XCode Website](https://developer.apple.com/xcode/)

<sup>1</sup> Select the version for the correct C++ compiler  
<sup>2</sup> The given download path is outdated. Use a newer version number, e.g. 5.9.3

* Install the dependencies above
* Clone this repository and the [Inviwo repository](https://github.com/inviwo/inviwo)
* Run `git submodule sync --recursive` and `git submodule update --init --recursive` within the local Inviwo copy
* Generate project files using CMake
  * Set source and binary destination path
  * Set all Qt variables
  * Set TemporalTreeMaps as an external modules (see below)
* Open the resulting project in Visual Studio/XCode/Shell and build it

### External Modules

The contents of this repository need to be integrated in Inviwo as an external module.
* Add the path to the modules folder of this repository (*TemporalTreeMaps/modules*) to **IVW_EXTERNAL_MODULES** in CMake.
* Select the module **IVW_MODULE_TEMPORALTREEMAPS**

## Reproducing results from the paper

The temporal trees for the data sets used in the paper can be found under *modules/temporaltreemaps/data/*.

The corresponding workspaces are located under *modules/temporaltreemaps/data/workspaces* and are additionally accessible through the user interface under *File → Example Workspaces → TemporalTreeMaps*.

Some data extraction workspaces are given as well.

## Cite

Please cite our paper if you use this code in your own work:

```
@article{koepp19a,
  author    = {Wiebke K\"{o}pp and Tino Weinkauf},
  title     = {Temporal Treemaps: Static Visualization of Evolving Trees},
  journal   = {IEEE Transactions on Visualization and Computer Graphics (Proc. IEEE VIS)},
  year      = {2019},
  volume    = {25},
  number    = {1},
  month     = jan,
}
```
