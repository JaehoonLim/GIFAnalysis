# GIFAnalysis
GIF++ Analysis Code

# 1. Make MAP file - MAP* 
```
  DetectorName'\t'     StripPerPartition'\t'   PartitionCut'\n'  
  TDC_start'\t'        TDC_end'\t'     Strip_start'\t' Strip_end'\n'  
  ...  
  TDC_start'\t'        TDC_end'\t'     Strip_start'\t' Strip_end'\n'  
  '\n' (when detector setting is end)  
  
  ex)  
  T3S5 (KODEL_DG12)       32      AB  
  2000    2015    1       16  
  2016    2031    17      32  
  2032    2047    64      49  
  2048    2063    48      33  
```
  You can find TDC channel & strip number for each test beam  
  on https://drive.google.com/open?id=1LyIAcLzgN0jdvHnPx1cIScB__B4uFL9U5UqvBO9ENrg  
  
# 2. Make Calibration file - calibrate.C -> CAL*
  
-  InputRootFile : DAQ.root file - NoSource(or at least ABS220) with Beam   
-  MAPConfigFile : Map file(#1)  
-  Cut_PercentageAfterCut : Check entries after cut  
-  Cut_TimeWindow : Check difference between peek after cut and chamber mean  
-  Cut_RMSAfterCut : Check broad peak  
  (GIF++ do not use calibration time)  
  
# 3. Clustering - KODEL.C -> DAQ-KODEL.root 
  
-  InputRootFile : DAQ.root file  
-  MAPConfigFile : Map file(#1)  
-  Delta_Strip : Definition of adjacent strip for clustering  
-  Delta_Partition : Definition of adjacent partition for clustering  
-  Delta_Time : Definition of adjacent time for clustering  
  
# 4. Make trolly SET file - SET*
  Trolly & Beam SETTING config file  
```  
  BEAM'\t' X_mean'\t' X_error'\t' Y_mean'\t' Y_error'\t' Time_mean'\t' Time_error'\n'
  
  ex)  
  BEAM    600     50      275     50      0       30
  
  DetectorName'\t' Order'\t' DetectorWidth(X)'\t' DetectorHeight(Y-Wide)'\t' DetectorHeight(Y-Narrow)'\t' DetectorPosition(X)'\t' DetectorPositioY(Y)'\t' RotationAngle(ClockWise)'\n'    
  
  ex)  
  T3S5 (KODEL_DG12)       4       800     320     250     400     70      0  
  T3S1 (GT_16)    3       800     480     350     0       65      180
```  
  DetectorWidth, Height, PositionX, PositionY on example file is not real value. (just for tracking test)  
  You have to update based on  
  https://drive.google.com/drive/folders/0BwXFTEDFLcaVelc2SEV2UmRfa1U  
  (Source base coordinate : https://drive.google.com/open?id=0B3Uwh2qCP96vM2s2OFhoVkFmSDg)  
  
# 5. Tracking - Tracking.C -> DAQ-Trk.root
  
-  InputRootFile : DAQ-KODEL.root  
-  MAPConfigFile : MAP file(#1)  
-  SETConfigFile : SET file(#4)  
-  Cut_X : Distance in X-axis  
-  Cut_Y : Distance in Y-axis  
-  Cut_Time : Difference in Time  
  
# 6. Ploters
## KODELPlot.C
  ```
  root 'KODELPlot.C("DAQ-KODEL.root","MAPConfigFile","SETConfigFile","DetectorName","Cluster or Hit","EventNumber or All")'  
  ```
  option "Cluster" will show you : Cluster Position, Cluster Multiplicity, Cluster Size, Cluster Fastest Hit Time  
  option "Hit" will show you : Hit Position, Hit Multiplicity, TDC Hit Time, Calibrated Hit Time  
## TrackPlot.C
  ```
  root 'TrackPlot.C("DAQ-Trk.root","MAPConfigFile","SETConfigFile","Muon or All","EventNumber or All")'  
  ```
  option "Muon" will show you only Muon Track  
  option "All" will show you All Track
## TrackEff.C  
  ```
  root 'TrackEff.C("Last HV point DAQ-Trk.root","Additional Trigger or Simple","DetectorName")'  
  ```
  option "Simple" will calculate efficiency by hit  
  "Additional Trigger" option will calculate efficiency with respect to additional trigger by using tracking  
  ex) "", "T1S1", "T1S1&&T1S2", "T1S1||T1S2", etc  
  
# 7. Printers - Need Compile  
## HitRate.C
  ```  
  HitRate Last_HV_point_DAQ.root MAPConfigFile DetectorName Partition PartitionArea  
  ```
  or, for KODEL Chamber,
  ```
  HitRate Last_HV_point_DAQ.root MAPConfigFile Partition  
  ```
  option "Partition" ex) A, B, C, All
## ClusterRate.C
  ```
  ClusterRate Last_HV_point_DAQ-KODEL.root DetectorName Partition PartitionArea  
  ```
  or, for KODEL Chamber, 
  ```
  ClusterRate Last_HV_point_DAQ-KODEL.root Partition  
  ```
  For option "Partition", You have to check __Partition Cut__ on MAPConfigFile(#1) When you run Clustering(#3)  
## ClusterSizeAfterTracking.C
  ```
  ClusterSizeAfterTracking Last_HV_point_DAQ-Trk.root DetectorName   
  ```
  or, for KODEL Chamber, 
  ```
  ClusterSizeAfterTracking Last_HV_point_DAQ-Trk.root  
  ```
  
