# Temporal Treemaps

This is an Inviwo implementation of Temporal Treemaps for visualizing trees whose topology and data change over time, as described in our paper:

Wiebke Köpp, Tino Weinkauf, ["Temporal Treemaps: Static Visualization of Evolving Trees"](https://www.csc.kth.se/~weinkauf/publications/abskoepp19a.html) (IEEE VIS 2018).

## Setup

Temporal Treemaps have been implemented in [Inviwo](https://inviwo.org/). The setup instructions for Inviwo are briefly summarized below and can also be found in the [Inviwo Github Wiki](https://github.com/inviwo/inviwo/wiki).

| Dependency | Windows (64 bit) | Linux | OSX|
| --- | --- | --- | --- |
| Git | [.exe](https://github.com/git-for-windows/git/releases/download/v2.15.1.windows.2/Git-2.15.1.2-64-bit.exe)  |  `sudo apt-get install git ` | [.dmg](https://sourceforge.net/projects/git-osx-installer/files/git-2.15.1-intel-universal-mavericks.dmg/download?use_mirror=autoselect)
| CMake  | [.msi](https://cmake.org/files/v3.10/cmake-3.10.1-win64-x64.msi)  |  `sudo apt-get install cmake cmake-qt-gui ` | [.dmg](https://cmake.org/files/v3.10/cmake-3.10.2-Darwin-x86_64.dmg)
| Qt  | [.exe](http://download.qt.io/official_releases/qt/5.9/5.9.3/qt-opensource-windows-x86-5.9.3.exe)<sup>1</sup> | [Qt 5 Install instructions](https://wiki.qt.io/Install_Qt_5_on_Ubuntu)<sup>2</sup> | [.dmg](http://download.qt.io/official_releases/qt/5.9/5.9.3/qt-opensource-mac-x64-5.9.3.dmg)<sup>1</sup>
| C++ | [Visual Studio Website](https://visualstudio.microsoft.com/) | `sudo apt-get install build-essential` | [XCode Website](https://developer.apple.com/xcode/)

<sup>1</sup> Select the version for the correct C++ compiler.  
<sup>2</sup> The given download path is outdated. Use a newer version number, e.g. 5.9.3.

* Install the dependencies above.
* Clone this repository and the [Inviwo repository](https://github.com/inviwo/inviwo).
* Run `git submodule sync --recursive` and `git submodule update --init --recursive` within the local Inviwo copy.
* Generate project files using CMake:
  * Set source and binary destination path.
  * Set all Qt variables.
  * Set TemporalTreeMaps as an external module (see below).
* Open the resulting project in Visual Studio/XCode/Shell and build it.

### External Module

The contents of this repository need to be integrated in Inviwo as an external module.
* Add the path to the modules folder of this repository (*TemporalTreeMaps/modules*) to **IVW_EXTERNAL_MODULES** in CMake.
* Select the module **IVW_MODULE_TEMPORALTREEMAPS**.

This will enable some other modules our module depends on as well.

## Examples

Some temporal trees and their respective raw data can be found under *modules/temporaltreemaps/data/*.

Workspaces to compute trees from raw data and to visualize them are located under *modules/temporaltreemaps/data/workspaces* and are additionally accessible through the Inviwo user interface under *File → Example Workspaces → TemporalTreeMaps*.

### Tracking of Sub- or Superlevel Sets

From a given time-varying scalar field, temporal trees are computed from tracking sub- or superlevel sets for selected iso values over time. The workspaces for this kind of data are:

* **SimpleBubbles**: Computation of the temporal tree and its visualization [Figures 4 and 6 in the paper].
* **ViscousFingers**: Visualization of the [data set](https://www.jluk.de/blog/NestedTrackingGraphs/) from the *Nested Tracking Graphs* paper<sup>3</sup> [Figure 8 in the paper].
* **Cylinder**: Visualization of tracked sublevel sets for the vortex activity behind a square cylinder [Figure 9 in the paper].

<sup>3</sup> Jonas Lukasczyk, Gunther Weber, Ross Maciejewski, Christoph Garth, and Heike Leitte, ["Nested Tracking Graphs"](https://www.jluk.de/resources/papers/NestedTrackingGraphs2017.pdf) (EuroVis 2017).

### Git Repository

Extracting data from a Git repositories is done by checking out a given repository several times for a given time span and sampling type (e.g. every tag, every n'th commit). The workspaces for this kind of data are:

* **TreeFromGitRepo**: Temporal tree computation for a given repository.
* **PythonRepo**: Visualization of every 1000th commit of the [CPython repository](https://github.com/python/cpython) between August 1992 and March 2018 [Figure 14 in the paper].

### Demographic data

A hierarchy for countries can be derived by grouping them into continents, regions and subregions. Any associated numerical data can be visualized. The workspaces for this kind of data are:

* **WorldPopulation**: Visualization of population estimates and projections from the [World Population Prospects](https://population.un.org/wpp/) [Figure 12 in the paper]
* **EuropePopulation**: Visualization of the latest available data values from the [UN Demographic Yearbooks](https://unstats.un.org/unsd/demographic-social/products/dyb/) between 1984 to 2018 (leading to values for 1975 to 2016) accounting for historical events such as the dissolution of the Soviet Union and Germany's reunification [Figure 13 in the paper]

### Other

Some simple examples such as full binary trees, temporal trees with a single merge or split and temporal trees for which no order that fulfills all constraints [Figure 5 in the paper] are included as well.

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
