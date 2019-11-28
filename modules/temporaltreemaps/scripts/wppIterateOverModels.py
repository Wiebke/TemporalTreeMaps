#Inviwo Python script 
import inviwopy
import os

import ivw.utils as inviwo_utils

app = inviwopy.app
network = app.network

dataPath = network.CSVSource2.inputFile_.value
dataDir, _ = os.path.split(dataPath)

for (dirpath, dirnames, filenames) in os.walk(dataDir):
    for filename in filenames:
        if filename.endswith('.csv') and 'dataWPP' in filename: 
           output =  os.sep.join([dirpath, os.path.splitext(os.path.basename(filename))[0]+".png"])
           network.CSVSource2.inputFile_.value = os.sep.join([dirpath, filename])
           network.Canvas.snapshot(output)


