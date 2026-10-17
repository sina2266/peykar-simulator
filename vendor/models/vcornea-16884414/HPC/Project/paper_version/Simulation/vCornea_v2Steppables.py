#||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
# Title:        Towards a computational model of corneal homeostasis and injury 
#  
# Authors:      Joel Vanin1, James A. Glazier1, Michael Getz1, Thomas B. Knudsen12 & Catherine Mahony3
#
# Filiation:    1-Department of Intelligent Systems Engineering and Biocomplexity Institute, Indiana University, Bloomington, IN, USA;
#               2-Center for Computational Toxicology and Exposure, Office of Research and Development, United States Environmental Protection Agency, Research Triangle Park, NC; (Retiered)
#               3-Procter & Gamble Technical Centre, Reading, United Kingdom; mahony.c@pg.com
#
# Abstract:     The cornea is a transparent tissue that plays a crucial role in vision. It is composed of several layers of cells, each with distinct functions...      
#               TODO: Add abstract   
#       
#         
#        
#         
#   Ontology:   - Cell types: Stem, Basal, Wing, Superficial, Membrane, Limbal, Tear
#               - Chemicals: EGF, SLS 
# 
# 
# TODO: Update the ontology
#    Taxonomy 
#    <bqbiol:hasTaxon>
#       <rdf:Bag>
#          <rdf:li rdf:resource="http://identifiers.org/taxonomy/9606">
#       </rdf:Bag>
#    </bqbiol:hasTaxon>

#    Tissue 
#    <bqbiol:occursIn>
#       <rdf:Bag>
#          <rdf:li rdf:resource="http://identifiers.org/bto/BTO:0000286">
#       </rdf:Bag>
#    </bqbiol:occursIn> 

#     Cell Type
#    <bqbiol:hasPart>
#       <rdf:Bag>
#          <rdf:li rdf:resource="http://identifiers.org/cl/CL:0000575">
#          <rdf:li rdf:resource="https://zfin.org/ZFA:0009264#summary">
#       </rdf:Bag>
#    
#    Modeling Approach
#    <bqbiol:isPropertyOf>
#       <rdf:Bag>
#          <rdf:li rdf:resource="http://identifiers.org/mamo/MAMO_0000024">
#       </rdf:Bag>
#    </bqbiol:isPropertyOf>

#    Biological Process
#    <bqbiol:hasProperty>
#       <rdf:Bag>           
#             <rdf:li rdf:resource="http://purl.jp/bio/4/id/201306016465003467">
#       </rdf:Bag>
#    </bqbiol:hasProperty> -->

#    Model Relevance to a Particular Area
#    <bqbiol:hasProperty>
#       <rdf:Bag>
#          <rdf:li rdf:resource="http://identifiers.org/ncit/C17206">
#       </rdf:Bag>
#    </bqbiol:hasProperty>    
   
#    <!-- Created -->
#      <!-- <dc:created>
#         <rdf:Description>
#           <dc:date>2024-08-19T12:00:00Z</dc:date>
#         </rdf:Description>
#       </dc:created> -->
   

#    <!-- Creators -->
#    <!-- <bqbiol:isDescribedBy>
#       <rdf:Bag>
#          <rdf:li rdf:resource="https://orcid.org/0000-0001-9227-9667?lang=en">         
#       </rdf:Bag>   
#    </bqbiol:isDescribedBy> -->
#    <!-- Notes -->
# <!-- </Annotation> -->
#                  
#||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

from cc3d.cpp.PlayerPython import * 
from cc3d import CompuCellSetup
from cc3d.CompuCellSetup import persistent_globals as pg
from cc3d.core.PySteppables import *

import numpy as np
import pandas as pd
import random
import csv
from pathlib import Path
import random as rd
from Parameters import *
import time

current_script_directory = Path(__file__).parent

# Join with a time stamp to avoid overwriting
output_directory = current_script_directory.joinpath("Output",time.strftime("%m%d%Y_%H%M%S"))
pg.set_output_dir(str(output_directory))

# GLOBAL PARAMETERS
# RANDOM_SEED = random.seed(12)

#---- Time Scales ---
# Time Scales in Monte Carlo Steps (MCS) 
# Conversion factors: 1 Hour = 10 MCS, 1 Day = 240 MCS, etc.
HOURtoMCS   = 10 
DAYtoMCS    = 240
WEEKtoMCS   = 1680
MONTHtoMCS  = 7300
YEARtoMCS   = 87600

#---- Spatial Scales ---
UMtoVOXEL   = 2

#---- Plots Variables ---
DEATHCOUNT  = 0

class ConstraintInitializerSteppable(SteppableBasePy):
    """
    Initializes cell constraints and fields for the corneal tissue simulation.
    This includes setting target volumes, surface areas, and chemoattractant coefficients.
    """

    # Static Parameters for Simulation Initiation
    current_script_directory = current_script_directory

    # CELLS PARAMETERS
    # ---STEM---
    InitSTEM_LambdaSurface = InitSTEM_LambdaSurface  # Coefficient for surface area regulation in Stem cells
    InitSTEM_TargetSurface = InitSTEM_TargetSurface  # Target surface area for Stem cells
    InitSTEM_LambdaVolume = InitSTEM_LambdaVolume  # Coefficient for volume regulation in Stem cells
    InitSTEM_TargetVolume = InitSTEM_TargetVolume  # Target volume for Stem cells
    DensitySTEM_HalfMaxValue = DensitySTEM_HalfMaxValue  # Density at half-maximum value for Stem cells
    EGF_STEM_HalfMaxValue = EGF_STEM_HalfMaxValue  # EGF concentration at half-maximum value for Stem cells
    STEM_beta_EGF = STEM_beta_EGF  # Sensitivity of Stem cells to EGF
    InitSTEM_LambdaChemo = InitSTEM_LambdaChemo  # Chemoattractant coefficient for Stem cells
    SLS_STEMDiffCoef = SLS_STEMDiffCoef  # Diffusion coefficient for SLS in Stem cells

    # ---BASAL---
    InitBASAL_LambdaSurface = InitBASAL_LambdaSurface  # Coefficient for surface area regulation in Basal cells
    InitBASAL_TargetSurface = InitBASAL_TargetSurface  # Target surface area for Basal cells
    InitBASAL_LambdaVolume = InitBASAL_LambdaVolume  # Coefficient for volume regulation in Basal cells
    InitBASAL_TargetVolume = InitBASAL_TargetVolume  # Target volume for Basal cells
    InitBASAL_LambdaChemo = InitBASAL_LambdaChemo  # Chemoattractant coefficient for Basal cells
    InitBASAL_Division = InitBASAL_Division  # Division coefficient for Basal cells
    DensityBASAL_HalfMaxValue = DensityBASAL_HalfMaxValue  # Density at half-maximum value for Basal cells
    EGF_BASAL_HalfMaxValue = EGF_BASAL_HalfMaxValue  # EGF concentration at half-maximum value for Basal cells
    BASAL_beta_EGF = BASAL_beta_EGF  # Sensitivity of Basal cells to EGF
    SLS_BASALDiffCoef = SLS_BASALDiffCoef  # Diffusion coefficient for SLS in Basal cells

    # ---WING---
    InitWING_LambdaSurface = InitWING_LambdaSurface  # Coefficient for surface area regulation in Wing cells
    InitWING_TargetSurface = InitWING_TargetSurface  # Target surface area for Wing cells
    InitWING_LambdaVolume = InitWING_LambdaVolume  # Coefficient for volume regulation in Wing cells
    InitWING_TargetVolume = InitWING_TargetVolume  # Target volume for Wing cells
    InitWING_EGFLambdaChemo = InitWING_EGFLambdaChemo  # Chemoattractant coefficient for Wing cells
    SLS_WINGDiffCoef = SLS_WINGDiffCoef  # Diffusion coefficient for SLS in Wing cells

    # ---SUPERFICIAL---
    InitSUPER_LambdaSurface = InitSUPER_LambdaSurface  # Coefficient for surface area regulation in Superficial cells
    InitSUPER_TargetSurface = InitSUPER_TargetSurface  # Target surface area for Superficial cells
    InitSUPER_LambdaVolume = InitSUPER_LambdaVolume  # Coefficient for volume regulation in Superficial cells
    InitSUPER_TargetVolume = InitSUPER_TargetVolume  # Target volume for Superficial cells
    EGF_SUPERDiffCoef = EGF_SUPERDiffCoef  # Diffusion coefficient for EGF in Superficial cells
    SLS_SUPERDiffCoef = SLS_SUPERDiffCoef  # Diffusion coefficient for SLS in Superficial cells

    # ---MEMBRANE; LIMBAL MEMBRANE & TEAR---
    SLS_MEMBDiffCoef = SLS_MEMBDiffCoef  # Diffusion coefficient for SLS in Bowman`s membrane (periphery)
    SLS_LIMBDiffCoef = SLS_LIMBDiffCoef  # Diffusion coefficient for SLS in Bowman`s membrane (limbal)
    SLS_TEARDiffCoef = SLS_TEARDiffCoef  # Diffusion coefficient for SLS in Tear

    # FIELDS
    MovementBias = object()
    MovementBiasScreteAmount = MovementBiasScreteAmount  # Constant chemoattractant secreted by Bowman`s membrane (periphery and limbal)
    MovementBiasUptake = MovementBiasUptake  # Uptake of chemoattractant by Basal cells

    EGF_Field = object()
    EGF_ScreteAmount = EGF_ScreteAmount  # Constant EGF amount secreted by Tear
    EGF_FieldUptakeBASAL = EGF_FieldUptakeBASAL  # Uptake of EGF by Basal cells
    EGF_FieldUptakeSTEM = EGF_FieldUptakeSTEM  # Uptake of EGF by Stem cells
    EGF_FieldUptakeSuper = EGF_FieldUptakeSuper  # Uptake of EGF by Superficial cells
    EGF_FieldUptakeWing = EGF_FieldUptakeWing  # Uptake of EGF by Wing cells
    EGF_GlobalDecay = EGF_GlobalDecay  # EGF global decay

    SLS_Field = object()
    SLS_X_Center = SLS_X_Center  # X coordinate for chemical gaussian pulse
    SLS_Y_Center = SLS_Y_Center  # Y coordinate for chemical gaussian pulse
    SLS_Concentration = SLS_Concentration  # Chemical concentration
    SLS_Gaussian_pulse = SLS_Gaussian_pulse  # Control for type of chemical damage gaussian pulse or uniform distributed

    # LINKS
    # ---SUPER-WALL---
    LINKWALL_lambda_distance = LINKWALL_lambda_distance  # Coefficient for link size regulation between cells and simulation wall
    LINKWALL_target_distance = 8  # Target link size between cells and simulation wall
    LINKWALL_max_distance = LINKWALL_max_distance  # Max size for links between cells and simulation wall
    # ---SUPER-SUPER---
    LINKSUPER_lambda_distance = LINKSUPER_lambda_distance  # Coefficient for link size regulation between cells and cells
    LINKSUPER_target_distance = LINKSUPER_target_distance  # Target link size between cells and cells
    LINKSUPER_max_distance = LINKSUPER_max_distance  # Max size for links between cells and cells
    AutoAdjustLinks = AutoAdjustLinks  # Turns the self-adjusting of the links distance values to maintain a constant tension
    Lambda_link_adjustment = True  # If True, the lambda will be adjusted, if False, the target distance will be adjusted
    Tension_link_SS = 50 #Constant tension for the links between Super-Super cells

    # WOUND
    DeathTimeScalar = DeathTimeScalar  # Time scalar for cell death
    InjuryType = InjuryType  # Mode of injury: true for ablation, false for chemical
    IsInjury = IsInjury  # Turns the injury method on/off for debugging
    InjuryTime = InjuryTime  # Time when injury happens
    SLS_Injury = SLS_Injury  # SLS Injury coefficient
    SLS_Threshold_Method = SLS_Threshold_Method  # Define if the method of cell death by SLS is by a threshold or not
    SLS_Threshold = SLS_Threshold  # Threshold value for cell death by SLS
    # ---INJURY AREA---
    InjuryX_Center = InjuryX_Center  # X coordinate for ablation
    InjuryY_Center = InjuryY_Center  # Y coordinate for ablation
    InjuryRadius = InjuryRadius  # Radius for ablation

    # DEBUGGING
    GrowthControl = GrowthControl  # Turns the growth method on/off for debugging
    MitosisControl = MitosisControl  # Turns the mitosis method on/off for debugging
    DeathControl = DeathControl  # Turns the death method on/off for debugging
    DifferentiationControl = DifferentiationControl  # Turns the differentiation method on/off for debugging

    # PLOTS
    CC3D_PLOT = False
    CellCount = CellCount  # Cell count plot/data collection
    PressureTracker = PressureTracker  # Window in player tracking real-time pressure
    EGF_SeenByCell = EGF_SeenByCell  # Window in player tracking real-time EGF concentration in cell
    SLS_SeenByCell = SLS_SeenByCell  # Window in player tracking real-time SLS concentration in cell
    CenterBias = CenterBias  # Window in player tracking real-time Mbias concentration at edge of Bowman's membrane
    ThicknessPlot = ThicknessPlot  # Thickness of tissue plot/data collection
    SurfactantTracking = SurfactantTracking  # Surfactant tracking plot/data collection

    SnapShot = SnapShot  # Requesting of snapshots of model every 10 MCS

    # TIME OF SIMULATION
    SimTime = SimTime  # End of the simulation

    def __init__(self,frequency=1):

        SteppableBasePy.__init__(self,frequency)

        #  Checking for which if anny plot should be displayed
        if self.PressureTracker: # Help to visualize the pressure in each cell
            self.track_cell_level_scalar_attribute(field_name='Pressure', attribute_name='Pressure')      
        if self.EGF_SeenByCell: # Help to visualize the EGF concentration in each cell
            self.track_cell_level_scalar_attribute(field_name='EGF_Seen', attribute_name='EGF')
        if self.SLS_SeenByCell: # Help to visualize the SLS concentration in each cell
            self.track_cell_level_scalar_attribute(field_name='SLS_Seen', attribute_name='SLS')
        if self.CenterBias: # Help to visualize the chemoattractant concentration at the edge of Bowman's membrane
            self.track_cell_level_scalar_attribute(field_name='CenterBias', attribute_name='CenterBias')
       
    def start(self):         
        """ Inintializing agents and fields at MCS 0"""  
        
        # FIELDS
        self.MovementBias = self.get_field_secretor("BASALMVBIAS") # BASALMVBIAS is the field responsible for guiding Basal cells based on the movement bias secreted by Bowman’s membrane.
        self.EGF_Field    = self.get_field_secretor("EGF") # EGF is the field responsible for guiding Wing, and initiate cell growth for Basal, and Stem cells based on the EGF secreted by Tear.
        self.SLS_Field    = self.get_field_secretor("SLS") # SLS is the field chemical responsible for injury       
        self.get_xml_element("EGF_Coef_SUPER").cdata = self.EGF_SUPERDiffCoef
        self.get_xml_element("EGF_GlobalDecay").cdata = self.EGF_GlobalDecay
        self.get_xml_element("SLS_Coef_SUPER").cdata = self.SLS_SUPERDiffCoef
        self.get_xml_element("SLS_Coef_WING").cdata = self.SLS_WINGDiffCoef
        self.get_xml_element("SLS_Coef_BASAL").cdata = self.SLS_BASALDiffCoef
        self.get_xml_element("SLS_Coef_MEMB").cdata = self.SLS_MEMBDiffCoef
        self.get_xml_element("SLS_Coef_LIMB").cdata = self.SLS_LIMBDiffCoef
        self.get_xml_element("SLS_Coef_TEAR").cdata = self.SLS_TEARDiffCoef
        
        # MEMBANE; LIMB    
        for cell in self.cell_list_by_type(self.MEMB, self.LIMB):         
            cell.targetVolume  = 1
            cell.lambdaVolume  = 100000.0
            cell.lambdaSurface = 0.1
            cell.targetSurface = 1000.0

        # STEM        
        for cell in self.cell_list_by_type(self.STEM):
            cell.targetVolume  = self.InitSTEM_TargetVolume
            cell.lambdaVolume  = self.InitSTEM_LambdaVolume
            cell.lambdaSurface = self.InitSTEM_LambdaSurface
            cell.targetSurface = self.InitSTEM_TargetSurface
            cell.dict['Initial_Volume'] = self.InitSTEM_TargetVolume
            cell.dict["LambdaChemo"] = self.InitSTEM_LambdaChemo
            ChemotaxisData = self.chemotaxisPlugin.addChemotaxisData(cell, "BASALMVBIAS")
            ChemotaxisData.setLambda(cell.dict["LambdaChemo"]) 
           
        # BASAL          
        for cell in self.cell_list_by_type(self.BASAL):
            cell.targetVolume  = self.InitBASAL_TargetVolume
            cell.lambdaVolume  = self.InitBASAL_LambdaVolume
            cell.lambdaSurface = self.InitBASAL_LambdaSurface
            cell.targetSurface = self.InitBASAL_TargetSurface
            cell.dict['Initial_Volume'] = self.InitBASAL_TargetVolume           
            cell.dict["LambdaChemo"]   = self.InitBASAL_LambdaChemo
            ChemotaxisData = self.chemotaxisPlugin.addChemotaxisData(cell, "BASALMVBIAS")            
            ChemotaxisData.setLambda(cell.dict["LambdaChemo"])
         
        # WING  
        for cell in self.cell_list_by_type(self.WING):
            cell.targetVolume  = self.InitWING_TargetVolume
            cell.lambdaVolume  = self.InitWING_LambdaVolume
            cell.lambdaSurface = self.InitWING_LambdaSurface
            cell.targetSurface = self.InitWING_TargetSurface
            cell.dict['Initial_Volume'] = self.InitWING_TargetVolume
            cell.dict['EGF_LambdaChemo'] = self.InitWING_EGFLambdaChemo
            ChemotaxisData = self.chemotaxisPlugin.addChemotaxisData(cell, "EGF")            
            ChemotaxisData.setLambda(cell.dict["EGF_LambdaChemo"])

        # SUPERFICIAL
        for cell in self.cell_list_by_type(self.SUPER):
            cell.targetVolume   = self.InitSUPER_TargetVolume
            cell.lambdaVolume   = self.InitSUPER_LambdaVolume
            cell.lambdaSurface  = self.InitSUPER_LambdaSurface
            cell.targetSurface  = self.InitSUPER_TargetSurface
            cell.dict['Initial_Volume'] = self.InitSUPER_TargetVolume           
            self.create_links(cell) 

        # TEAR
        for cell in self.cell_list_by_type(self.TEAR):            
            cell.targetVolume  = 50
            cell.lambdaVolume  = 1 
            cell.dict['Initial_Volume'] = cell.targetVolume

        # STROMA
        for cell in self.cell_list_by_type(self.STROMA):
            cell.targetVolume  = cell.volume
            cell.lambdaVolume  = 3
                        

    def step(self, mcs):
        """ Update function for the simulation called every Monte Carlo Step"""

        # ---- CELL PARAMETERS UPDATE ----        
        for cell in self.cell_list_by_type(self.BASAL, self.STEM, self.WING, self.SUPER, self.MEMB, self.LIMB, self.TEAR):
            # MEMBRANE, LIMBAL MEMBRANE, TEAR           
            if cell.type == self.MEMB or cell.type == self.LIMB or cell.type == self.TEAR:
                cell.dict['SLS'] = self.SLS_Field.amountSeenByCell(cell)

            # STEM, BASAL, WING, SUPERFICIAL
            else:                
                cell.dict['Pressure']   = abs(cell.pressure)                
                cell.dict['Volume']     = cell.volume
                cell.dict['EGF']        = self.EGF_Field.amountSeenByCell(cell)/cell.volume
                cell.dict['SLS']        = self.SLS_Field.amountSeenByCell(cell)/cell.volume
                    # BASAL
                if cell.type == self.BASAL:              
                    cell.dict['Bias_Uptake'] = self.MovementBias.uptakeInsideCellTotalCount(cell, 100000.0, self.MovementBiasUptake).tot_amount            
                    cell.dict['EGF_Uptake']  = abs(self.EGF_Field.uptakeInsideCellTotalCount(cell, 100000.0, self.EGF_FieldUptakeBASAL).tot_amount)
                    # STEM 
                elif cell.type == self.STEM:
                    cell.dict['EGF_Uptake']  = abs(self.EGF_Field.uptakeInsideCellTotalCount(cell, 100000.0, self.EGF_FieldUptakeSTEM).tot_amount)            
                    # WING  
                elif cell.type == self.WING: 
                    cell.dict['EGF_Uptake']  = abs(self.EGF_Field.uptakeInsideCellTotalCount(cell, 100000.0, self.EGF_FieldUptakeWing).tot_amount)
                    # SUPERFICIAL
                elif cell.type == self.SUPER: 
                    cell.dict['EGF_Uptake']  = abs(self.EGF_Field.uptakeInsideCellTotalCount(cell, 100000.0, self.EGF_FieldUptakeSuper).tot_amount)            
                        # LINKS UPDATE
                    NEIGHBOR_DICT = self.get_cell_neighbor_data_list(cell).neighbor_count_by_type()                
                    if self.WALL in NEIGHBOR_DICT.keys():                    
                        self.update_wall_links(cell)
                    else:
                        self.update_super_super_links(cell)

    def create_links(self, cell):
        """
        Creates links for the given cell.

        Args:
            cell: The cell for which links need to be created.

        Returns:
            None
        """        
        if cell.type == self.SUPER:
            NEIGHBOR_DICT = self.get_cell_neighbor_data_list(cell).neighbor_count_by_type()
            if self.WALL in NEIGHBOR_DICT.keys():                
                if cell.xCOM < self.dim.x/2:                                        
                    closest_neighbor_WALL = self.cell_field[int(0.5), int(cell.yCOM+0.5), 0]                    
                else:                    
                    closest_neighbor_WALL = self.cell_field[int((self.dim.x)-0.5), int(cell.yCOM+0.5), 0]
                
                # Need to check if there is another super cell linked to the wall
                for neighbor, _ in self.get_cell_neighbor_data_list(cell):
                    if neighbor and neighbor.type == self.SUPER:
                        NN_DICT = self.get_cell_neighbor_data_list(neighbor).neighbor_count_by_type()
                        if self.WALL in NN_DICT.keys():                      
                            # Visit all links attached to the neighbor cell
                            for link in self.get_fpp_links_by_cell(neighbor):
                                if link.getOtherCell(neighbor).type == self.WALL:
                                    if cell.yCOM > neighbor.yCOM:
                                        self.delete_fpp_link(link)
                                        Nlink = self.new_fpp_link(cell, neighbor, self.LINKSUPER_lambda_distance,
                                            self.LINKSUPER_target_distance, self.LINKSUPER_max_distance)
                                        link = self.new_fpp_link(cell, closest_neighbor_WALL,self.LINKWALL_lambda_distance,
                                                self.LINKWALL_target_distance, self.LINKWALL_max_distance)
                                        
                                        if self.AutoAdjustLinks:
                                            if self.Lambda_link_adjustment:
                                                self.link_lambda_update(link)
                                                self.link_lambda_update(Nlink)
                                            else:
                                                self.link_target_distance_update(link)
                                                self.link_target_distance_update(Nlink)                                                                                
                                    else:
                                        self.new_fpp_link(cell, neighbor, self.LINKSUPER_lambda_distance,
                                            self.LINKSUPER_target_distance, self.LINKSUPER_max_distance)   
                    
                                        if self.AutoAdjustLinks:
                                            if self.Lambda_link_adjustment:
                                                self.link_lambda_update(link)
                                            else:
                                                self.link_target_distance_update(link) 
                        else:                            
                            link = self.new_fpp_link(cell, closest_neighbor_WALL,self.LINKWALL_lambda_distance,
                                    self.LINKWALL_target_distance, self.LINKWALL_max_distance) 
                            
                            if self.AutoAdjustLinks:
                                if self.Lambda_link_adjustment:
                                    self.link_lambda_update(link)
                                else:
                                    self.link_target_distance_update(link) 

            # No WALL neighbor    
            else:
                for neighbor, _ in self.get_cell_neighbor_data_list(cell):                    
                    if neighbor and neighbor.type == self.SUPER:
                        if not self.get_fpp_link_by_cells(cell, neighbor) and NEIGHBOR_DICT[self.SUPER] < 3 :                    
                            link = self.new_fpp_link(cell,neighbor, self.LINKSUPER_lambda_distance,
                                            self.LINKSUPER_target_distance, self.LINKSUPER_max_distance) 
                               
                            if self.AutoAdjustLinks:
                                if self.Lambda_link_adjustment:
                                    self.link_lambda_update(link)
                                else:
                                    self.link_target_distance_update(link)

    def update_wall_links(self, cell):        
        if cell.xCOM < self.dim.x / 2:
            closest_neighbor_WALL = self.cell_field[int(0.5), int(cell.yCOM + 0.5), 0]
        else:
            closest_neighbor_WALL = self.cell_field[int(self.dim.x - 0.5), int(cell.yCOM + 0.5), 0]
        existing_link_with_closestWALL = self.get_fpp_link_by_cells(cell, closest_neighbor_WALL)             
        if existing_link_with_closestWALL:
            if len(self.get_fpp_links_by_cell(cell)) > 2:
                for link in self.get_fpp_links_by_cell(cell):
                    if link.getOtherCell(cell).type == self.WALL:
                        self.delete_fpp_link(link)
            elif len(self.get_fpp_links_by_cell(cell)) == 1:                
                self.delete_fpp_link(existing_link_with_closestWALL) 
        
        # update the forces if so defined            
            if self.AutoAdjustLinks:
                if self.Lambda_link_adjustment:
                    self.link_lambda_update(existing_link_with_closestWALL)
                else:
                    self.link_target_distance_update(existing_link_with_closestWALL)
        else:
            for link in self.get_fpp_links_by_cell(cell):
                if link.getOtherCell(cell).type == self.WALL:
                    # [ ] We could have a threshold for the distance between the SUPER and the WALL
                    # if  link.getTension > SOME_THRESHOLD:
                    self.delete_fpp_link(link)
            self.create_links(cell)
                    
    def update_super_super_links(self, cell):
        NEIGHBOR_DICT = self.get_cell_neighbor_data_list(cell).neighbor_count_by_type()
        if cell.type == self.SUPER:
            if len(self.get_fpp_links_by_cell(cell)) <= 1 and NEIGHBOR_DICT[self.SUPER] < 3:
                for link in self.get_fpp_links_by_cell(cell):
                    self.delete_fpp_link(link)
                self.create_links(cell)

            elif len(self.get_fpp_links_by_cell(cell)) == 2 and NEIGHBOR_DICT[self.SUPER] == 2:
                if self.AutoAdjustLinks:                
                    for link in self.get_fpp_links_by_cell(cell):
                        if self.Lambda_link_adjustment:
                            self.link_lambda_update(link)                                           
                        else:
                            self.link_target_distance_update(link)
                            
            else:   
                for link in self.get_fpp_links_by_cell(cell):
                    self.delete_fpp_link(link)

                # Rules for new links when cells have more than 2 neighbors
                neighbors = self.get_cell_neighbor_data_list(cell)            
                super_neighbors = [neighbor for neighbor, _ in neighbors if neighbor and neighbor.type == self.SUPER]
                left_neighbor, right_neighbor = self.find_closest_neighbors(cell, super_neighbors)  
                
                if  left_neighbor:
                    link = self.new_fpp_link(cell,left_neighbor, self.LINKSUPER_lambda_distance,
                                                    self.LINKSUPER_target_distance, self.LINKSUPER_max_distance)
                    if self.AutoAdjustLinks:
                        if self.Lambda_link_adjustment:
                            self.link_lambda_update(link)
                        else:
                            self.link_target_distance_update(link)

                if right_neighbor:
                    link = self.new_fpp_link(cell,right_neighbor, self.LINKSUPER_lambda_distance,
                                                    self.LINKSUPER_target_distance, self.LINKSUPER_max_distance)
                    if self.AutoAdjustLinks:
                        if self.Lambda_link_adjustment:
                            self.link_lambda_update(link)
                        else:
                            self.link_target_distance_update(link)
        
    def find_closest_neighbors(self, cell, neighbors):        
        distances = [(neighbor, self.calculate_distance(cell, neighbor)) for neighbor in neighbors]
        # Sort neighbors by distance
        sorted_neighbors = sorted(distances, key=lambda x: x[1])        
        
        left_neighbor = None
        right_neighbor = None
        for neighbor, _ in sorted_neighbors:
            if neighbor.xCOM < cell.xCOM and left_neighbor is None:
                left_neighbor = neighbor
            elif neighbor.xCOM > cell.xCOM and right_neighbor is None:
                right_neighbor = neighbor
            if left_neighbor and right_neighbor:
                break        
        return left_neighbor, right_neighbor 
    
    def calculate_distance(self, cell_a, cell_b):        
        # Euclidean distance 
        return ((cell_a.xCOM - cell_b.xCOM) ** 2 + (cell_a.yCOM - cell_b.yCOM) ** 2) ** 0.5
       
    def link_lambda_update(self,link):
        if ((link.length - link.getTargetDistance())) != 0:
            link.setLambdaDistance((self.Tension_link_SS/2) / (link.length - link.getTargetDistance()))        
        else:
            link.setLambdaDistance(0)
        
    def link_target_distance_update(self, link):
        if (link.getLambdaDistance()) != 0:
            link.setTargetDistance(link.length - (self.Tension_link_SS / (2 * link.getLambdaDistance())))
          
class GrowthSteppable(SteppableBasePy):

    def __init__(self,frequency=1):

        SteppableBasePy.__init__(self, frequency)

        self.GrowthControl = ConstraintInitializerSteppable.GrowthControl        

        self.DensityBASAL_HalfMaxValue = ConstraintInitializerSteppable.DensityBASAL_HalfMaxValue
        self.DensitySTEM_HalfMaxValue = ConstraintInitializerSteppable.DensitySTEM_HalfMaxValue

        self.EGF_BASAL_HalfMaxValue = ConstraintInitializerSteppable.EGF_BASAL_HalfMaxValue
        self.EGF_STEM_HalfMaxValue = ConstraintInitializerSteppable.EGF_STEM_HalfMaxValue
                
        self.BASAL_initial_volume = ConstraintInitializerSteppable.InitBASAL_TargetVolume
        self.STEM_initial_volume = ConstraintInitializerSteppable.InitSTEM_TargetVolume

        self.BASAL_doubling = ((1/(HOURtoMCS*8))*self.BASAL_initial_volume) # (vMax for Basal cell growth per mcs) This is the minimum of 8 hours for doubling time 
        
        self.STEM_doubling = ((1/(HOURtoMCS*8))*self.STEM_initial_volume)   # (vMax for STEM cell growth per mcs) This is the minimum of 8 hours for doubling time 

        self.BASAL_beta_EGF = ConstraintInitializerSteppable.BASAL_beta_EGF
        self.STEM_beta_EGF = ConstraintInitializerSteppable.STEM_beta_EGF
  
    def step(self, mcs): 
        
        if self.GrowthControl:
            # ---- BASAL ----
            for cell in self.cell_list_by_type(self.BASAL):
                # GROWTH THROUGH EGF
                cell.dict['EGF_Growth'] = (self.BASAL_beta_EGF * 
                                           (cell.dict['EGF']**4/((self.EGF_BASAL_HalfMaxValue/cell.dict["Initial_Volume"])**4 + cell.dict['EGF']**4)) + 
                                           (1 - self.BASAL_beta_EGF)) # Hill Promoter EGF
                # GROWTH CONTACT INHIBITION  
                cell.dict['DensityGrowth'] = ((self.DensityBASAL_HalfMaxValue**4/(self.DensityBASAL_HalfMaxValue**4 + cell.dict['Pressure']**4))) # Hill Inhibitor Pressure        
                # TOTAL GROWTH
                cell.dict['TotalGrowth'] = (self.BASAL_doubling * (cell.dict['DensityGrowth'] * cell.dict['EGF_Growth']))
                cell.targetVolume += cell.dict['TotalGrowth']
                
            # ---- STEM ----
            for cell in self.cell_list_by_type(self.STEM):
                # GROWTH THROUGH TEAR 
                cell.dict['EGF_Growth'] = (self.STEM_beta_EGF *
                                           (cell.dict['EGF']**4/((self.EGF_STEM_HalfMaxValue/cell.dict["Initial_Volume"])**4 + cell.dict['EGF']**4)) +
                                           (1 - self.STEM_beta_EGF)) # Hill Promoter EGF 
                # GROWTH CONTACT INHIBITION 
                cell.dict['DensityGrowth'] = ((self.DensitySTEM_HalfMaxValue**4/(self.DensitySTEM_HalfMaxValue**4 + cell.dict['Pressure']**4))) # Hill Inhibitor Pressure                        
                # TOTAL GROWTH
                cell.dict['TotalGrowth'] = (self.STEM_doubling * (cell.dict['DensityGrowth'] * cell.dict['EGF_Growth']))
                cell.targetVolume += cell.dict['TotalGrowth']
                
            # Automatic surface auto update            
            for cell in self.cell_list_by_type(self.SUPER,self.WING,self.BASAL,self.STEM):                
                if cell.lambdaSurface == 0: # avoid division by 0
                    continue
                else:    
                    cell.targetSurface =  -(1.5/cell.lambdaSurface)+cell.surface

class MitosisSteppable(MitosisSteppableBase):
    
    STEM_to_divide = 0
    BASAL_to_divide = 0

    def __init__(self,frequency=1):

        MitosisSteppableBase.__init__(self,frequency)
        
        self.MitosisControl = ConstraintInitializerSteppable.MitosisControl
        
        self.InitBASAL_TargetVolume = ConstraintInitializerSteppable.InitBASAL_TargetVolume
        self.InitSTEM_TargetVolume = ConstraintInitializerSteppable.InitSTEM_TargetVolume
        
        self.ThresholdBASAL = 2*self.InitBASAL_TargetVolume 
        self.ThresholdSTEM = 2*self.InitSTEM_TargetVolume
        
    def start(self):
        self.intialTEARcount = len(self.cell_list_by_type(self.TEAR))

    def step(self, mcs):

        if self.MitosisControl:

            self.STEM_to_divide = 0
            self.BASAL_to_divide = 0
            cells_to_divide=[]

            #---BASAL & STEM---            
            for cell in self.cell_list_by_type(self.BASAL, self.STEM):
                if(cell.type == self.BASAL and cell.volume>self.ThresholdBASAL) or (cell.type == self.STEM and cell.volume>self.ThresholdSTEM):
                    cells_to_divide.append(cell)    
                        
            #---TEAR---
            for cell in self.cell_list_by_type(self.TEAR):
                if cell.volume>100:
                    cells_to_divide.append(cell)            

            # Division orientation 
            for cell in cells_to_divide:
                NEIGHBOR_DICT = self.get_cell_neighbor_data_list(cell).neighbor_count_by_type()                        
                    
                # STEM 
                if cell.type == self.STEM:
                    self.STEM_to_divide += 1 
                    self.set_parent_child_position_flag(1)                    
                    if (self.LIMB in NEIGHBOR_DICT.keys()):
                        self.divide_cell_orientation_vector_based(cell,1,0,0) # Orientation for Stem division (Vertical) 
                    else:
                        self.divide_cell_random_orientation(cell) 
               
                # BASAL    
                elif cell.type == self.BASAL:
                    self.BASAL_to_divide += 1 
                    self.divide_cell_random_orientation(cell)                    
                
                # TEAR
                elif cell.type == self.TEAR :
                    self.set_parent_child_position_flag(1)
                    self.divide_cell_orientation_vector_based(cell,1,0,0)

    def update_attributes(self):        
        self.parent_cell.targetVolume /= 2.0 
        self.clone_parent_2_child()
        
class DeathSteppable(SteppableBasePy):

    def __init__(self, frequency=1):

        SteppableBasePy.__init__(self, frequency)

        # CONSTANT CELL DEATH RATE CONTROLLER
        self.DeathControl = ConstraintInitializerSteppable.DeathControl

        # WOUND EVENT 
        self.injuryType = ConstraintInitializerSteppable.InjuryType
        self.injury     = ConstraintInitializerSteppable.IsInjury     
        self.injuryTime = ConstraintInitializerSteppable.InjuryTime

        # INJURY AREA
        self.x_center = ConstraintInitializerSteppable.InjuryX_Center 
        self.y_center = ConstraintInitializerSteppable.InjuryY_Center
        self.radius   = ConstraintInitializerSteppable.InjuryRadius

        # DEATH SCALER 
        self.deathTimeScalar    = ConstraintInitializerSteppable.DeathTimeScalar
        
        # CELL TYPES
        self.SUPER_TargetVolume = ConstraintInitializerSteppable.InitSUPER_TargetVolume
        self.SUPER_TargetSurface = ConstraintInitializerSteppable.InitSUPER_TargetSurface

        # SLS
        self.SLS_Injury = ConstraintInitializerSteppable.SLS_Injury
        self.SLS_Threshold_Method = ConstraintInitializerSteppable.SLS_Threshold_Method
        self.SLS_X_Center = ConstraintInitializerSteppable.SLS_X_Center
        self.SLS_Y_Center = ConstraintInitializerSteppable.SLS_Y_Center
        self.SLS_Concentration = ConstraintInitializerSteppable.SLS_Concentration 
        self.SLS_Gaussian_pulse = ConstraintInitializerSteppable.SLS_Gaussian_pulse
        self.SLS_threshold = ConstraintInitializerSteppable.SLS_Threshold       
    
    def start(self):
        self.SLS_Field = self.get_field_secretor("SLS")        
        self.SLS_initialField = self.field.SLS
        self.SLS = self.field.SLS        
   
    def step(self, mcs):
        global DEATHCOUNT
        # --- INJURY EVENT ---
        if self.injury:
            if (mcs >= self.injuryTime):
                # --- ABRASION ---
                if self.injuryType == 1:
                    cells_to_kill = set()
                    for i in np.arange(-self.radius,  self.radius, 1.0):
                        y_position = self.y_center + i
                        if not (self.y_center + i < 0 or self.y_center + i> self.dim.y):
                            for j in np.arange(-np.sqrt(self.radius**2-i**2), np.sqrt(self.radius**2-i**2),1.0):
                                x_position = self.x_center + j
                                if not (self.x_center + j < 0 or self.x_center + j> self.dim.x):
                                    cell = self.cell_field[x_position, y_position, 0]
                                    if cell:
                                        if (cell.type == self.SUPER or cell.type == self.WING or 
                                            cell.type == self.BASAL or cell.type == self.STEM or 
                                            cell.type == self.MEMB or  cell.type == self.LIMB):
                                            cells_to_kill.add(cell.id)             
                    if (mcs == self.injuryTime):
                        for cellid in cells_to_kill:                        
                            cell = self.fetch_cell_by_id(cellid)
                            cell.type = self.TEAR
                            cell.targetSurface = 0
                            cell.lambdaSurface = 0
                            cell.lambdaVolume = 1
                            if len(self.get_fpp_links_by_cell(cell)) > 0:
                                for link in self.get_fpp_links_by_cell(cell):
                                    self.delete_fpp_link(link) 

                else:
                    # --- CHEMICAL ---
                    if (mcs == self.injuryTime):
                        if self.SLS_Gaussian_pulse:# Gaussian
                            self.SLS_initialField[self.SLS_X_Center, self.SLS_Y_Center, 0] = self.SLS_Concentration
                        else: # Uniform
                            for i in range(self.dim.x):
                                self.SLS_initialField[i, self.SLS_Y_Center, 0] = self.SLS_Concentration/self.dim.x
                    if self.SLS_Injury:
                        for cell in self.cell_list_by_type(self.SUPER, self.WING, self.BASAL, self.STEM, self.MEMB, self.LIMB,):                            
                            if cell.type == self.MEMB or cell.type == self.LIMB:                                
                                pixel_list = self.get_cell_pixel_list(cell)
                                for pixel in pixel_list:
                                    if self.SLS[pixel.pixel.x, pixel.pixel.y, pixel.pixel.z] > 0.1:                            
                                        self.delete_cell(cell)
                            else:
                                if self.SLS_Threshold_Method:
                                    if (cell.dict['SLS'] > self.SLS_threshold/cell.dict['Initial_Volume']):  
                                        cell.dict['DEATH_MARK'] = True  
                                        cell.targetVolume = 0
                                        cell.lambdaVolume = 20
                                        cell.dict['SLS_Uptake'] = abs(self.SLS_Field.uptakeInsideCellTotalCount(cell, 100000.0, (cell.dict['SLS'])).tot_amount)                                        
                            
        # --- Constant Cell Death Rate --- 
        if self.DeathControl: 
            if mcs > HOURtoMCS: # Relaxation Time
                deathsum = 0            
                for cell in self.cell_list_by_type(self.SUPER):
                    NEIGHBOR_DICT = self.get_cell_neighbor_data_list(cell).neighbor_count_by_type()  
                    if (self.TEAR in NEIGHBOR_DICT.keys()) :
                        # On average the whole process for cell turnover takes 7-14 days. 
                        # If we assume that the average layer cells are 4 cells underneath the superficial
                        # at the periphery and  it can take between 1.75 days/layer. If that is maintained
                        # and we consider a 8 layer cells in the limbus we still have the 1.75 rate for the 14 days. 
                        # https://iovs.arvojournals.org/article.aspx?articleid=2123522 ->
                        # Markov draw for cell death instead of continous volume loss random draw between 0and1 if it is lower than the probability value then cell dies
                        # average of cell death is 3 days
                        
                        if random.random() <= ((1/3.0)/DAYtoMCS):
                            cell.targetVolume = 0
                            cell.lambdaVolume = 1000

                        if cell.volume < 15 or self.SUPER not in NEIGHBOR_DICT.keys(): # Minimum Cell Size Before Slough | if no slogh cell will disapear in 672 MCS ~3 days(2.8)
                            deathsum += 1 
                            cell.targetVolume = 0
                            cell.lambdaVolume = 1000 
                                
                DEATHCOUNT = deathsum

            for cell in self.cell_list_by_type(self.SUPER, self.WING, self.BASAL, self.STEM):
                if (cell.volume < 3):
                    self.delete_cell(cell)            

class DifferentiationSteppable(SteppableBasePy):

    def __init__(self, frequency=1):  

        SteppableBasePy.__init__(self, frequency)  

        self.DifferentiationControl = ConstraintInitializerSteppable.DifferentiationControl    
        self.LINKWALL_lambda_distance = ConstraintInitializerSteppable.LINKWALL_lambda_distance
        self.LINKWALL_target_distance = ConstraintInitializerSteppable.LINKWALL_target_distance 
        self.LINKWALL_max_distance = ConstraintInitializerSteppable.LINKWALL_max_distance
        self.LINKSUPER_lambda_distance = ConstraintInitializerSteppable.LINKSUPER_lambda_distance
        self.LINKSUPER_target_distance = ConstraintInitializerSteppable.LINKSUPER_target_distance
        self.LINKSUPER_max_distance = ConstraintInitializerSteppable.LINKSUPER_max_distance        
          
    def start(self):        
        pass
       
    def step(self, mcs): 
      
        if self.DifferentiationControl:
            if mcs > HOURtoMCS:                         
                # WING CELL
                for cell in self.cell_list_by_type(self.WING):                    
                    NEIGHBOR_DICT = self.get_cell_neighbor_data_list(cell).neighbor_count_by_type()                                
                    # DIFFERENTIATION RULES
                    # Possibility for a time dependent differentiation instead of instantanious 6 minutes
                    # [ ] Litereature says that the differentiation of the wing cells to the superficial cells is x(units) after signal y

                        # Imidiate Differentiation
                    if ((self.TEAR in NEIGHBOR_DICT.keys()) and 
                        (self.WING in NEIGHBOR_DICT.keys()) and 
                        not (self.BASAL in NEIGHBOR_DICT.keys()) and 
                        not (self.MEMB in NEIGHBOR_DICT.keys())and 
                        not (self.STEM in NEIGHBOR_DICT.keys())): 
                        cell.type = self.SUPER
                        self.initializeDifferentiatedCell(cell)

                        # Time based Differention                                    
                    # if ((self.TEAR in NEIGHBOR_DICT.keys()) and 
                    #     (self.WING in NEIGHBOR_DICT.keys()) and 
                    #     not (self.BASAL in NEIGHBOR_DICT.keys()) and 
                    #     not (self.MEMB in NEIGHBOR_DICT.keys()) and
                    #     cell.dict['Differentiantion'] == False): 
                    #     cell.dict['Differentiantion'] = True
                    #     cell.dict['DifferentiantionTime'] = mcs
                    # if cell.dict['Differentiantion'] and (mcs - cell.dict['DifferentiantionTime'] > HOURtoMCS * 2):
                    #     cell.type = self.SUPER
                    #     self.initializeDifferentiatedCell(cell)

                # BASAL CELL
                for cell in self.cell_list_by_type(self.BASAL):                    
                    # Fetch the boundary pixels of the current Basal cell
                    boundary_pixel_list = self.get_cell_boundary_pixel_list(cell)                    
                    memb_contact_area = 0  # Counter for the area of contact with MEMB cells                    
                    # Iterate through each boundary pixel to check for MEMB neighbors
                    for boundary_pixel_tracker_data in boundary_pixel_list:
                        boundary_pixel = boundary_pixel_tracker_data.pixel
                        for neighbor_pixel in self.get_pixel_neighbors(boundary_pixel):                            
                            if neighbor_pixel.type == self.MEMB:
                                memb_contact_area += 1  # Increment the counter for every MEMB neighbor pixel
                    # Differentiation Rules: The cell will differentiate if its contact area with MEMB is 2 or less pixels
                    if memb_contact_area <= 5:                
                        cell.type = self.WING
                        self.initializeDifferentiatedCell(cell)                        
                        
                # STEM CELL
                for cell in self.cell_list_by_type(self.STEM):
                    NEIGHBOR_DICT = self.get_cell_neighbor_data_list(cell).neighbor_count_by_type()
                    # Fetch the boundary pixels of the current Basal cell
                    boundary_pixel_list = self.get_cell_boundary_pixel_list(cell)                    
                    memb_contact_area = 0  # Counter for the area of contact with MEMB cells
                    # Differentiation Rules: The cell will differentiate if its contact area with MEMB is 2 or less pixels
                    if not self.LIMB in NEIGHBOR_DICT.keys():
                        cell.type = self.BASAL
                        self.initializeDifferentiatedCell(cell)                        
                       
        
    def get_pixel_neighbors(self, pixel):
        # Extracting x, y, z coordinates of the pixel
        x, y, z = pixel.x, pixel.y, pixel.z
        neighbors = []        
        # Define possible relative coordinates for neighbors in a 2D grid
        # relative_coords = [(-1, -1, 0), (-1, 0, 0), (-1, 1, 0), 
        #                     (0, -1, 0),            (0, 1, 0), 
        #                     (1, -1, 0), (1, 0, 0), (1, 1, 0)]
        # Faster version with less neighbors to check (since it is basally constrained)
        relative_coords = [
                            (0, -1, 0),            (0, 1, 0), 
                            (1, -1, 0), (1, 0, 0), (1, 1, 0)]        
        for dx, dy, dz in relative_coords:
            # Check if the neighboring cell type matches our desired cell type
            neighbor_pixel = self.cellField[x + dx, y + dy, z + dz]
            if neighbor_pixel:
                neighbors.append(neighbor_pixel)        
        return neighbors                  
    
   
    def initializeDifferentiatedCell(self, cell):
        # --- BASAL ---
        if cell.type == self.BASAL:
            cell.lambdaSurface = ConstraintInitializerSteppable.InitBASAL_LambdaSurface
            cell.targetSurface = ConstraintInitializerSteppable.InitBASAL_TargetSurface
            cell.dict["LambdaChemo"] = ConstraintInitializerSteppable.InitBASAL_LambdaChemo
            cell.dict['Initial_Volume'] = ConstraintInitializerSteppable.InitBASAL_TargetVolume
            ChemotaxisData = self.chemotaxisPlugin.addChemotaxisData(cell, "BASALMVBIAS")            
            ChemotaxisData.setLambda(cell.dict["LambdaChemo"])
        # --- STEM ---
        elif cell.type == self.STEM:
            cell.lambdaSurface = ConstraintInitializerSteppable.InitSTEM_LambdaSurface
            cell.targetSurface = ConstraintInitializerSteppable.InitSTEM_TargetSurface
            cell.dict['Initial_Volume'] = ConstraintInitializerSteppable.InitSTEM_TargetVolume
            cell.dict["LambdaChemo"] = ConstraintInitializerSteppable.InitSTEM_LambdaChemo
            ChemotaxisData = self.chemotaxisPlugin.addChemotaxisData(cell, "BASALMVBIAS")            
            ChemotaxisData.setLambda(cell.dict["LambdaChemo"])
        # --- WING ---
        elif cell.type == self.WING:
            cell.lambdaSurface = ConstraintInitializerSteppable.InitWING_LambdaSurface
            cell.targetSurface = ConstraintInitializerSteppable.InitWING_TargetSurface
            cell.lamdaVolume = ConstraintInitializerSteppable.InitWING_LambdaVolume
            cell.targetVolume = ConstraintInitializerSteppable.InitWING_TargetVolume
            cell.dict['Initial_Volume'] = ConstraintInitializerSteppable.InitWING_TargetVolume
            cell.dict["LambdaChemo"] = 0
            
            # For Time based Differentiation       
            #     cell.dict['Differentiantion'] = False
            #     cell.dict['DifferentiantionTime'] = 0

        # --- SUPER ---
        elif cell.type == self.SUPER:
            cell.lambdaSurface = ConstraintInitializerSteppable.InitSUPER_LambdaSurface
            cell.targetSurface = ConstraintInitializerSteppable.InitSUPER_TargetSurface
            cell.dict['Initial_Volume'] = ConstraintInitializerSteppable.InitSUPER_TargetVolume
            cell.dict["LambdaChemo"] = 0

class SecretionSteppable(SteppableBasePy):

    def __init__(self, frequency=1):
        
        SteppableBasePy.__init__(self, frequency)

        self.EGF_SecreteAmount          = ConstraintInitializerSteppable.EGF_ScreteAmount
        self.MovementBiasScreteAmount   = ConstraintInitializerSteppable.MovementBiasScreteAmount
    
    def start(self):

        self.MovementBias   = self.get_field_secretor("BASALMVBIAS")
        self.EGF_Field      = self.get_field_secretor("EGF")
        self.SLS_Field      = self.get_field_secretor("SLS")
        self.intialTEARcount= len(self.cell_list_by_type(self.TEAR))
     
    def step(self, mcs):

        for cell in self.cell_list_by_type(self.MEMB):
            self.MovementBias.secreteOutsideCellAtBoundaryOnContactWith(cell, self.MovementBiasScreteAmount, [self.MEDIUM, self.WING, self.SUPER, self.TEAR])
        
        for cell in self.cell_list_by_type(self.TEAR):
            (self.SLS_Field.uptakeInsideCellTotalCount(cell, 0.002, (cell.dict['SLS']/cell.volume)).tot_amount)

class TEARSteppable(SteppableBasePy):

    def __init__(self, frequency=1):

        SteppableBasePy.__init__(self, frequency)

    def start(self):

        self.initialTEARcount = len(self.cell_list_by_type(self.TEAR))
        self.Force_TEAR = 30
        self.cell_id = None
        self.chosen_cell = None
     
    def step(self, mcs):
        flag = False
        if len(self.cell_list_by_type(self.TEAR)) < self.initialTEARcount:
            flag = True
        elif len(self.cell_list_by_type(self.TEAR)) > self.initialTEARcount:
            sorted_cells_y = sorted(self.cell_list_by_type(self.TEAR), key=lambda x: x.yCOM)
            cell = sorted_cells_y[0]            
            cell.targetVolume = 0
            cell.lambdaVolume = 1000 
            
        for cell in self.cell_list_by_type(self.TEAR):
            NEIGHBOR_DICT = self.get_cell_neighbor_data_list(cell).neighbor_count_by_type()            
            is_trapped = (self.MEDIUM not in NEIGHBOR_DICT and self.TEAR not in NEIGHBOR_DICT) or (self.SUPER in NEIGHBOR_DICT and self.STROMA in NEIGHBOR_DICT)             
       
            if is_trapped:
                # If the tear cell is trapped, it should die
                cell.targetVolume = 0
            
            elif (not any(neigh_type in NEIGHBOR_DICT for neigh_type in 
                [self.SUPER, self.WING, self.BASAL, self.STEM, self.MEMB, self.LIMB, self.STROMA])):
                
                # Move the tear cell downward until it touches another cell
                cell.lambdaVecX = 0
                cell.lambdaVecY = self.Force_TEAR 

            else:
                cell.lambdaVecX = 0
                cell.lambdaVecY = 0
                # cell.targetVolume = 50

            if flag:
                # random choice on a list (self.cell_list_by_type(self.TEAR))
                # if self.chosen_cell is None:
                sorted_cells_x = sorted(self.cell_list_by_type(self.TEAR), key=lambda x: x.xCOM)
                half_len = len(sorted_cells_x) // 2
                cell = sorted_cells_x[half_len -1]               
                # cell  = np.random.choice(list(self.cell_list_by_type(self.TEAR)))                
                cell.targetVolume = 101 # Add volume to the tear cell
                    # self.chosen_cell.lambdaVolume = 1000               
                # else:
             
class PlotSteppable(SteppableBasePy):

    def __init__(self, frequency=1):      
        SteppableBasePy.__init__(self, frequency)

        self.CC3D_PLOT      = ConstraintInitializerSteppable.CC3D_PLOT 
        self.mitosis        = MitosisSteppable()
        self.death          = DeathSteppable()
        self.differentiation= DifferentiationSteppable()
        self.growth         = GrowthSteppable()
        self.cellCount      = ConstraintInitializerSteppable.CellCount
        
        self.thicknessPlot  = ConstraintInitializerSteppable.ThicknessPlot
       
        self.SurfactantTracking = ConstraintInitializerSteppable.SurfactantTracking
        self.SnapShot       = ConstraintInitializerSteppable.SnapShot 
        
        self.BASAL_to_divide = MitosisSteppable.BASAL_to_divide
        self.STEM_to_divide  = MitosisSteppable.STEM_to_divide
        self.current_script_directory = Path(__file__).parent
        self.SimTime = ConstraintInitializerSteppable.SimTime
        self.data_points =  {
            'cell_count': {
                "Superficial": [],
                "Wing": [],
                "Basal": [],
                "Stem": [],
                "Time": []
            },

            'center_bias': {
                "Wing": [],
                "Individual Volume": [],
                "Average Vol": [],
                "Slough Count": [],
                "Time": []
            },

            'pressure': {
                "Stem Average": [],
                "Basal Average": [],
                "Time": []               
            },           

            'thickness': {                                
            },
             
            'surfactant': {
                'Total Amount': [],
                'Time': []                
            },
                            }

    def start(self):

        self.bins_number = 10
        self.bin_size = self.dim.x // self.bins_number        
        self.bin_indexes = list(range(self.bins_number))

        if self.CC3D_PLOT:
        # ---- Cell Count Plot ----
            if self.cellCount:
                self.plot_cell_count = self.add_new_plot_window(title='Cell Count by Type',
                                                                x_axis_title='Time (Hours)',
                                                                y_axis_title='Number of Cells',
                                                                x_scale_type='linear', y_scale_type='linear',
                                                                grid=True, config_options={'legend':True})

                self.plot_cell_count.add_plot("Superficial", style='Lines', color='cyan')
                self.plot_cell_count.add_plot("Wing", style='Lines', color='blue')
                self.plot_cell_count.add_plot("Basal", style='Lines', color='pink')
                self.plot_cell_count.add_plot("Stem", style='Lines', color='red')

            if self.thicknessPlot:
                self.plot_thickness = self.add_new_plot_window(title='Tissue Thickness',
                                                                x_axis_title='Time(Hours)',
                                                                y_axis_title='Average Y position',
                                                                x_scale_type='linear', y_scale_type='linear',
                                                                grid=True, config_options={'legend':True})
                
                for bin_index in self.bin_indexes:
                    self.plot_thickness.add_plot(f'X Coord.{bin_index * self.bin_size}-{(bin_index + 1) * self.bin_size}', style='lines')

            if self.SurfactantTracking:
                self.SLS_Field = self.get_field_secretor("SLS")
                self.SLS_plot = self.add_new_plot_window(title='Surfactant available',
                                                        x_axis_title='Time(Hours)',
                                                        y_axis_title='Average Y position',
                                                        x_scale_type='linear', y_scale_type='linear',
                                                        grid=True, config_options={'legend':True})
                self.SLS_plot.add_plot("Total Amount", style='lines', color='Blue')

    def step(self, mcs):
        global DEATHCOUNT
        HOURtoMCS_factor = mcs / HOURtoMCS

        # ---- End of Simulation ----
        if mcs == (self.SimTime + 1) :
            if self.cellCount:                
                self.write_csv_for_category('cell_count', mcs)           
            if self.thicknessPlot:                
                self.write_parquet_for_cell_data_with_replicate('thickness', mcs)
            if self.SurfactantTracking:
                self.write_csv_for_category('surfactant', mcs)               
            if self.SnapShot:                
                self.request_screenshot(mcs=mcs, screenshot_label='Cell_Field_CellField_2D_XY_0')
            CompuCellSetup.stop_simulation()

        # ---- Plots ----
        if self.cellCount and mcs % HOURtoMCS == 0:
            # CSV
            self.data_points['cell_count']['Superficial'].append(len(self.cell_list_by_type(self.SUPER)))
            self.data_points['cell_count']['Wing'].append(len(self.cell_list_by_type(self.WING)))
            self.data_points['cell_count']['Basal'].append(len(self.cell_list_by_type(self.BASAL)))
            self.data_points['cell_count']['Stem'].append(len(self.cell_list_by_type(self.STEM)))            
            self.data_points['cell_count']['Time'].append(HOURtoMCS_factor)

            if self.CC3D_PLOT:
                # CC3D Plots               
                self.plot_cell_count.add_data_point("Superficial", HOURtoMCS_factor, len(self.cell_list_by_type(self.SUPER)))
                self.plot_cell_count.add_data_point("Wing", HOURtoMCS_factor, len(self.cell_list_by_type(self.WING)))
                self.plot_cell_count.add_data_point("Basal", HOURtoMCS_factor, len(self.cell_list_by_type(self.BASAL)))
                self.plot_cell_count.add_data_point("Stem", HOURtoMCS_factor, len(self.cell_list_by_type(self.STEM)))
  
        if self.thicknessPlot and mcs % HOURtoMCS == 0:
            self.data_points['thickness'][HOURtoMCS_factor] = {'Bins': {}}            
            bins = {index: [] for index in self.bin_indexes}            
           
            for cell in self.cell_list_by_type(self.SUPER):
                # Binning the data
                bin_index = cell.xCOM // self.bin_size               
                bins[bin_index].append(cell.yCOM)

            # Check for interface cells with the tear or medium
            for cell in self.cell_list_by_type(self.WING, self.BASAL, self.STEM):
                NEIGHBOR_DICT = self.get_cell_neighbor_data_list(cell).neighbor_count_by_type()
                if self.TEAR in NEIGHBOR_DICT.keys() or self.MEDIUM in NEIGHBOR_DICT.keys():
                    bin_index = cell.xCOM // self.bin_size
                    if bin_index in bins:
                        bins[bin_index].append(cell.yCOM)
                      
            # Calculate mean height of the top cells in each bin
            for bin_index in self.bin_indexes:
                cells = bins[bin_index]
                if not cells:
                    mean_top_height = 0                           
                else:
                    mean_top_height = np.mean(cells)                    

                # Update the 'Bins' dictionary with the mean top height
                if bin_index not in self.data_points['thickness'][HOURtoMCS_factor]['Bins']:
                    self.data_points['thickness'][HOURtoMCS_factor]['Bins'][bin_index] = []
                self.data_points['thickness'][HOURtoMCS_factor]['Bins'][bin_index].append(mean_top_height)

            if self.CC3D_PLOT:
                for bin in self.bin_heights_over_time:
                    self.plot_thickness.add_data_point(f'X Coord.{bin * self.bin_size}-{(bin + 1) * self.bin_size}', HOURtoMCS_factor, self.data_points['thickness'][HOURtoMCS_factor]['Bins'][bin])
        
        if self.SurfactantTracking:
            if mcs == 0: # Start of simulation
                self.SLS_Field = self.get_field_secretor("SLS")

            self.data_points['surfactant']['Total Amount'].append(self.SLS_Field.totalFieldIntegral())
            self.data_points['surfactant']['Time'].append(HOURtoMCS_factor)

            if self.CC3D_PLOT: 
                self.SLS_plot.add_data_point("Total Amount", HOURtoMCS_factor, self.SLS_Field.totalFieldIntegral())                  

        if self.SnapShot and mcs % HOURtoMCS == 0:
            self.request_screenshot(mcs=mcs, screenshot_label='Cell_Field_CellField_2D_XY_0')
            self.request_screenshot(mcs=mcs, screenshot_label='EGF_ConField_2D_XY_0')
            self.request_screenshot(mcs=mcs, screenshot_label='EGF_Seen_ScalarFieldCellLevel_2D_XY_0')
            self.request_screenshot(mcs=mcs, screenshot_label='Pressure_ScalarFieldCellLevel_2D_XY_0')
            self.request_screenshot(mcs=mcs, screenshot_label='SLS_ConField_2D_XY_0')
            self.request_screenshot(mcs=mcs, screenshot_label='SLS_Seen_ScalarFieldCellLevel_2D_XY_0')
               
    def write_csv_for_category(self, category, mcs):
        """
        Writes a CSV file for the given category from the self.data_points dictionary.
        The file will be named after the category and saved in the current_script_directory.
        """        
        data = self.data_points[category]
        file_name = f"{category}_{mcs}.csv"
        file_path = self.current_script_directory.joinpath(file_name)
        
        # Assuming all sub-dictionaries have the same length as 'Time'
        num_entries = len(data['Time'])
        
        with open(file_path, 'w', newline='') as csvfile:
            csvwriter = csv.writer(csvfile)
            
            # Write the header (column names)
            headers = list(data.keys())
            csvwriter.writerow(headers)
            
            # Write the rows of data
            for i in range(num_entries):
                row = [data[header][i] for header in headers]
                csvwriter.writerow(row)


    def write_parquet_for_cell_data_with_replicate(self, category, mcs, directory="."):
        """
        Writes a Parquet file for cell data.
        The file will be named after the category and saved in the given directory,
        with the replicate number extracted from the directory path.
        """        
        # Extract the replicate number from the directory path
        replicate = Path(directory).parts[-2] if len(Path(directory).parts) > 1 else ""        
        file_name = f"{category}_rep{replicate}_{mcs}.parquet"
        file_path = self.current_script_directory.joinpath(file_name)        
        # Prepare data for Parquet

        if category == 'thickness':
            rows = []
            for time_step, data in self.data_points[category].items():
                for bin_key, heights in data['Bins'].items():
                        for height in heights:
                            row = {                            
                                'Time': time_step,
                                'Bin': bin_key,
                                'Height': height                            
                            }
                            rows.append(row)

        # Convert list of dictionaries to DataFrame
        df = pd.DataFrame(rows)        
        # Write to Parquet
        df.to_parquet(file_path, index=False)
   