#||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
# Simulation Version v2.1  
# 
# ----- ADDITIONS -----
#       * Integration of links between the SUPER and the WALL
#       * Integration of links between the SUPER and the SUPER
#           *       
#       
#         
#       * Development of new capabilities  
#          *
#    
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

# Static Parameters for Simulation Initiation
    current_script_directory    = current_script_directory

#   CELLS PARMETERS
#---STEM---
    InitSTEM_LambdaSurface      = InitSTEM_LambdaSurface
    InitSTEM_TargetSurface      = InitSTEM_TargetSurface # 25=pi*r^2 | 5x4 sides
    InitSTEM_LambdaVolume       = InitSTEM_LambdaVolume
    InitSTEM_TargetVolume       = InitSTEM_TargetVolume 
    DensitySTEM_HalfMaxValue    = DensitySTEM_HalfMaxValue
    EGF_STEM_HalfMaxValue       = EGF_STEM_HalfMaxValue
    STEM_beta_EGF               = STEM_beta_EGF
    InitSTEM_LambdaChemo        = InitSTEM_LambdaChemo  #10000
    # Growth Scalars    
    EGF_GrowthScalar_STEM       = EGF_GrowthScalar_STEM #  
    DensityGrowthScalar_STEM    = DensityGrowthScalar_STEM #
    SLS_STEMDiffCoef            = SLS_STEMDiffCoef

#---BASAL---
    InitBASAL_LambdaSurface     = InitBASAL_LambdaSurface
    InitBASAL_TargetSurface     = InitBASAL_TargetSurface # 25=pi*r^2 = 18 | 5x4 sides =20
    InitBASAL_LambdaVolume      = InitBASAL_LambdaVolume
    InitBASAL_TargetVolume      = InitBASAL_TargetVolume
    InitBASAL_LambdaChemo       = InitBASAL_LambdaChemo #125
    InitBASAL_Division          = InitBASAL_Division
    DensityBASAL_HalfMaxValue   = DensityBASAL_HalfMaxValue # 15.3 is very close to the correct value
    EGF_BASAL_HalfMaxValue      = EGF_BASAL_HalfMaxValue
    BASAL_beta_EGF              = BASAL_beta_EGF
    # Growth Scalars 
    EGF_GrowthScalar_BASAL      = EGF_GrowthScalar_BASAL  #  
    DensityGrowthScalar_BASAL   = DensityGrowthScalar_BASAL   # 
    SLS_BASALDiffCoef           = SLS_BASALDiffCoef
       
#---WING---    
    InitWING_LambdaSurface      = InitWING_LambdaSurface  
    InitWING_TargetSurface      = InitWING_TargetSurface
    InitWING_LambdaVolume       = InitWING_LambdaVolume
    InitWING_TargetVolume       = InitWING_TargetVolume
    InitWING_EGFLambdaChemo     = InitWING_EGFLambdaChemo
    SLS_WINGDiffCoef            = SLS_WINGDiffCoef

#---SUPERFICIAL---
    InitSUPER_LambdaSurface     = InitSUPER_LambdaSurface
    InitSUPER_TargetSurface     = InitSUPER_TargetSurface
    InitSUPER_LambdaVolume      = InitSUPER_LambdaVolume
    InitSUPER_TargetVolume      = InitSUPER_TargetVolume
    EGF_SUPERDiffCoef           = EGF_SUPERDiffCoef 
    SLS_SUPERDiffCoef           = SLS_SUPERDiffCoef
    SloughProbability           = SloughProbability

    # Death Scalars
    DeathTimeScalar             = DeathTimeScalar
    DeathVolumeScalar           = DeathVolumeScalar
    SloughScalar                = SloughScalar 

#---MEMBRANE; LIMBAL MEMBRANE & TEAR---
    SLS_MEMBDiffCoef            = SLS_MEMBDiffCoef
    SLS_LIMBDiffCoef            = SLS_LIMBDiffCoef
    SLS_TEARDiffCoef            = SLS_TEARDiffCoef

# FIELDS
    MovementBias                = object()
    MovementBiasScreteAmount    = MovementBiasScreteAmount    
    MovementBiasUptake          = MovementBiasUptake

    EGF_Field                   = object()
    EGF_ScreteAmount            = EGF_ScreteAmount
    EGF_FieldUptakeBASAL        = EGF_FieldUptakeBASAL
    EGF_FieldUptakeSTEM         = EGF_FieldUptakeSTEM
    EGF_FieldUptakeSuper        = EGF_FieldUptakeSuper
    EGF_FieldUptakeWing         = EGF_FieldUptakeWing
    EGF_GlobalDecay             = EGF_GlobalDecay

    SLS_Field                   = object()
    SLS_X_Center                = SLS_X_Center
    SLS_Y_Center                = SLS_Y_Center
    SLS_Concentration           = SLS_Concentration
    SLS_Gaussian_pulse          = SLS_Gaussian_pulse

# LINKS
    #---SUPER-WALL---
    LINKWALL_lambda_distance    = LINKWALL_lambda_distance
    LINKWALL_target_distance    = 8
    LINKWALL_max_distance       = LINKWALL_max_distance
    #---SUPER-SUPER---
    LINKSUPER_lambda_distance   = LINKSUPER_lambda_distance
    LINKSUPER_target_distance   = LINKSUPER_target_distance
    LINKSUPER_max_distance      = LINKSUPER_max_distance  
    L_max_links_SS              = L_max_links_SS 
    F_max_links_SS              = F_max_links_SS
    AutoAdjustLinks             = AutoAdjustLinks
    Lambda_link_adjustment      = True # If True, the lambda will be adjusted, if False, the target distance will be adjusted    
    Tension_link_SS             = 50

# WOUND
    InjuryType                  = InjuryType
    IsInjury                    = IsInjury
    InjuryTime                  = InjuryTime
    SLS_Injury                  = SLS_Injury
    SLS_Threshold_Method        = SLS_Threshold_Method    
    #---INJURY AREA---
    InjuryX_Center              = InjuryX_Center 
    InjuryY_Center              = InjuryY_Center
    InjuryRadius                = InjuryRadius

# DEBUGGING
    GrowthControl               = GrowthControl
    MitosisControl              = MitosisControl
    DeathControl                = DeathControl
    DifferentiationControl      = DifferentiationControl

# PLOTS
    CC3D_PLOT                   = False
    CellCount                   = CellCount
    XHypTracker                 = XHypTracker 
    SloughTracker               = SloughTracker
    PressureTracker             = PressureTracker
    PressurePlot                = PressurePlot
    VolumeTracker               = VolumeTracker
    EGF_SeenByCell              = EGF_SeenByCell
    SLS_SeenByCell              = SLS_SeenByCell
    CenterBiasPlot              = CenterBiasPlot
    CenterBias                  = CenterBias 
    DivisionTracker             = DivisionTracker
    GrowthPlot                  = GrowthPlot
    ThicknessPlot               = ThicknessPlot
    VolumeSurfaceDetail         = VolumeSurfaceDetail
    MitosisPlot                 = MitosisPlot
    SingleCellPresEGFPlot       = SingleCellPresEGFPlot
    MassConservationPlot        = MassConservationPlot
    SurfactantTracking          = SurfactantTracking
    
    SnapShot                    = SnapShot

# TIME OF SIMULATION
    SimTime                     = SimTime

    def __init__(self,frequency=1):

        SteppableBasePy.__init__(self,frequency)

        #  Checking for which if anny plot should be displayed
        if self.PressureTracker:
            self.track_cell_level_scalar_attribute(field_name='Pressure', attribute_name='Pressure')
        if self.VolumeTracker:
            self.track_cell_level_scalar_attribute(field_name='Volume', attribute_name='Volume')
        if self.EGF_SeenByCell:
            self.track_cell_level_scalar_attribute(field_name='EGF_Seen', attribute_name='EGF')
        if self.SLS_SeenByCell:
            self.track_cell_level_scalar_attribute(field_name='SLS_Seen', attribute_name='SLS')
        if self.CenterBias:
            self.track_cell_level_scalar_attribute(field_name='CenterBias', attribute_name='CenterBias')
        if self.DivisionTracker:
            # self.track_cell_level_scalar_attribute(field_name='DivisionCount', attribute_name='DivisionCount')
            self.track_cell_level_scalar_attribute(field_name='CellLineage', attribute_name='CellLineage')
         

    def start(self):         
        """ Inintializing agents and fields at MCS 0"""  
        
        # FIELDS
        self.MovementBias = self.get_field_secretor("BASALMVBIAS")
        self.EGF_Field    = self.get_field_secretor("EGF")
        self.SLS_Field    = self.get_field_secretor("SLS")        
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
        self.lineage = np.arange(10, (len(self.cell_list_by_type(self.STEM)) + len(self.cell_list_by_type(self.BASAL)))+10).tolist()
        # print(self.STEMlineage)
          
        for cell in self.cell_list_by_type(self.STEM):
            cell.targetVolume  = self.InitSTEM_TargetVolume
            cell.lambdaVolume  = self.InitSTEM_LambdaVolume
            cell.lambdaSurface = self.InitSTEM_LambdaSurface
            cell.targetSurface = self.InitSTEM_TargetSurface
            cell.dict["LambdaChemo"] = self.InitSTEM_LambdaChemo
            ChemotaxisData = self.chemotaxisPlugin.addChemotaxisData(cell, "BASALMVBIAS")
            ChemotaxisData.setLambda(cell.dict["LambdaChemo"]) 
            # Get a random index
            random_index = random.randint(0, len(self.lineage) - 1)            
            # Pop the element at the random index
            element = self.lineage.pop(random_index)            
            cell.dict['CellLineage'] = element
        # BASAL          
        for cell in self.cell_list_by_type(self.BASAL):
            cell.targetVolume  = self.InitBASAL_TargetVolume
            cell.lambdaVolume  = self.InitBASAL_LambdaVolume
            cell.lambdaSurface = self.InitBASAL_LambdaSurface
            cell.targetSurface = self.InitBASAL_TargetSurface
            cell.dict['DivisionCount'] = self.InitBASAL_Division
            cell.dict["LambdaChemo"]   = self.InitBASAL_LambdaChemo
            ChemotaxisData = self.chemotaxisPlugin.addChemotaxisData(cell, "BASALMVBIAS")            
            ChemotaxisData.setLambda(cell.dict["LambdaChemo"])
            # Get a random index
            random_index = random.randint(0, len(self.lineage) - 1)            
            # Pop the element at the random index
            element = self.lineage.pop(random_index)            
            cell.dict['CellLineage'] = element
        # WING  
        for cell in self.cell_list_by_type(self.WING):
            cell.targetVolume  = self.InitWING_TargetVolume
            cell.lambdaVolume  = self.InitWING_LambdaVolume
            cell.lambdaSurface = self.InitWING_LambdaSurface
            cell.targetSurface = self.InitWING_TargetSurface
            cell.dict['EGF_LambdaChemo'] = self.InitWING_EGFLambdaChemo
            ChemotaxisData = self.chemotaxisPlugin.addChemotaxisData(cell, "EGF")            
            ChemotaxisData.setLambda(cell.dict["EGF_LambdaChemo"])
        # SUPERFICIAL
        for cell in self.cell_list_by_type(self.SUPER):
            cell.targetVolume   = self.InitSUPER_TargetVolume
            cell.lambdaVolume   = self.InitSUPER_LambdaVolume
            cell.lambdaSurface  = self.InitSUPER_LambdaSurface
            cell.targetSurface  = self.InitSUPER_TargetSurface
            cell.dict['Slough'] = self.SloughProbability
            self.create_links(cell) 
        # TEAR
        for cell in self.cell_list_by_type(self.TEAR):            
            cell.targetVolume  = 50
            cell.lambdaVolume  = 1             

    def step(self, mcs):
        """ Update function for the simulation called every Monte Carlo Step"""

        # ---- CELL PARAMETERS UPDATE ----        
        for cell in self.cell_list_by_type(self.BASAL, self.STEM, self.WING, self.SUPER, self.MEMB, self.LIMB, self.TEAR):
            # MEMBRANE, LIMBAL MEMBRANE, TEAR           
            if cell.type == self.MEMB or cell.type == self.LIMB or cell.type == self.TEAR:
                cell.dict['SLS'] = self.SLS_Field.amountSeenByCell(cell)
            else:
                cell.dict['Pressure']   = abs(cell.pressure)
                cell.dict['Volume']     = cell.volume
                cell.dict['EGF']        = self.EGF_Field.amountSeenByCell(cell)
                cell.dict['SLS']        = self.SLS_Field.amountSeenByCell(cell)
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

                # [x] Better rules for new links when cells have more than 2 neighbors
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
        # return 0 # TODO check this out contituuento and define the formula
                
class GrowthSteppable(SteppableBasePy):
   
    event_data = []

    def __init__(self,frequency=1):

        SteppableBasePy.__init__(self, frequency)

        self.GrowthControl = ConstraintInitializerSteppable.GrowthControl        

        self.DensityBASAL_HalfMaxValue = ConstraintInitializerSteppable.DensityBASAL_HalfMaxValue
        self.DensitySTEM_HalfMaxValue = ConstraintInitializerSteppable.DensitySTEM_HalfMaxValue

        self.EGF_BASAL_HalfMaxValue = ConstraintInitializerSteppable.EGF_BASAL_HalfMaxValue
        self.EGF_STEM_HalfMaxValue = ConstraintInitializerSteppable.EGF_STEM_HalfMaxValue
        
        self.EGF_GrowthScalar_BASAL = ConstraintInitializerSteppable.EGF_GrowthScalar_BASAL
        self.EGF_GrowthScalar_STEM = ConstraintInitializerSteppable.EGF_GrowthScalar_STEM

        self.DensityGrowthScalar_BASAL = ConstraintInitializerSteppable.DensityGrowthScalar_BASAL
        self.DensityGrowthScalar_STEM = ConstraintInitializerSteppable.DensityGrowthScalar_STEM

        self.BASAL_initial_volume = ConstraintInitializerSteppable.InitBASAL_TargetVolume
        self.STEM_initial_volume = ConstraintInitializerSteppable.InitSTEM_TargetVolume

        self.BASAL_doubling = ((1/(HOURtoMCS*8))*self.BASAL_initial_volume) # (vMax for Basal cell growth per mcs) This is the minimum of 8 hours for doubling time 
        
        # self.STEM_doubling = ((1/(HOURtoMCS*8))*self.STEM_initial_volume)   # (vMax for STEM cell growth per mcs) This is the minimum of 8 hours for doubling time 
        self.STEM_doubling = ((1/(HOURtoMCS*8))*self.STEM_initial_volume)*2   # (vMax for STEM cell growth per mcs) This is the minimum of 8 hours for doubling time 

        self.BASAL_beta_EGF = ConstraintInitializerSteppable.BASAL_beta_EGF
        self.STEM_beta_EGF = ConstraintInitializerSteppable.STEM_beta_EGF
  
    def step(self, mcs): 
        
        if self.GrowthControl:
            # ---- BASAL ----
            for cell in self.cell_list_by_type(self.BASAL):
                # GROWTH THROUGH EGF
                cell.dict['EGF_Growth'] = (self.BASAL_beta_EGF * 
                                           (cell.dict['EGF']**4/(self.EGF_BASAL_HalfMaxValue**4 + cell.dict['EGF']**4)) + 
                                           (1 - self.BASAL_beta_EGF)) # Hill Promoter EGF
                # GROWTH CONTACT INHIBITION  
                cell.dict['DensityGrowth'] = ((self.DensityBASAL_HalfMaxValue**4/(self.DensityBASAL_HalfMaxValue**4 + cell.dict['Pressure']**4))) # Hill Inhibitor Pressure        
                # TOTAL GROWTH
                cell.dict['TotalGrowth'] = (self.BASAL_doubling * (cell.dict['DensityGrowth'] * cell.dict['EGF_Growth']))
                cell.targetVolume += cell.dict['TotalGrowth']
                self.event_data.append({'Event Type': 'Basal Growth Vol','Cell Type': cell.type, 'Cell ID': cell.id, 'Volume':cell.volume, 'Time': mcs})
            # ---- STEM ----
            for cell in self.cell_list_by_type(self.STEM):
                # GROWTH THROUGH TEAR 
                cell.dict['EGF_Growth'] = (self.STEM_beta_EGF *
                                           (cell.dict['EGF']**4/(self.EGF_STEM_HalfMaxValue**4 + cell.dict['EGF']**4)) +
                                           (1 - self.STEM_beta_EGF)) # Hill Promoter EGF 
                # GROWTH CONTACT INHIBITION 
                cell.dict['DensityGrowth'] = ((self.DensitySTEM_HalfMaxValue**4/(self.DensitySTEM_HalfMaxValue**4 + cell.dict['Pressure']**4))) # Hill Inhibitor Pressure                        
                # TOTAL GROWTH
                cell.dict['TotalGrowth'] = (self.STEM_doubling * (cell.dict['DensityGrowth'] * cell.dict['EGF_Growth']))
                cell.targetVolume += cell.dict['TotalGrowth']
                self.event_data.append({'Event Type': 'Stem Growth Vol','Cell Type': cell.type, 'Cell ID': cell.id, 'Volume':cell.volume, 'Time': mcs})
            # New automatic surface auto update
            # TODO: Check if this is working correctly
            for cell in self.cell_list_by_type(self.SUPER,self.WING,self.BASAL,self.STEM):                
                if cell.lambdaSurface == 0: # avoid division by 0
                    continue
                else:    
                    cell.targetSurface =  -(1.5/cell.lambdaSurface)+cell.surface
      
    def get_cell_data_mass(self):        
        return self.event_data
    
    def clear_cell_data_mass(self):
        self.event_data = []

class MitosisSteppable(MitosisSteppableBase):
    
    STEM_to_divide = 0
    BASAL_to_divide = 0
    cells_divided_count_history = {'Basal' : [],
                                   'Stem' : [] }  # to store the counts at each MCS    
    event_data = []

    def __init__(self,frequency=1):

        MitosisSteppableBase.__init__(self,frequency)
        
        self.MitosisControl = ConstraintInitializerSteppable.MitosisControl
        
    def start(self):
        self.intialTEARcount = len(self.cell_list_by_type(self.TEAR))

    def step(self, mcs):

        if self.MitosisControl:

            self.STEM_to_divide = 0
            self.BASAL_to_divide = 0
            cells_to_divide=[]

            #---BASAL & STEM---            
            for cell in self.cell_list_by_type(self.BASAL, self.STEM):
                if cell.volume>50:
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
                    self.event_data.append({'Event Type': 'Stem Before Mitosis Vol', 'Cell Type': cell.type, 'Cell ID': cell.id, 'Volume':cell.volume, 'Time': mcs})
                    self.set_parent_child_position_flag(1)                    
                    if (self.LIMB in NEIGHBOR_DICT.keys()):
                        self.divide_cell_orientation_vector_based(cell,1,0,0) # Orientation for Stem division (Vertical)                        
                        self.event_data.append({'Event Type': 'Stem After Mitosis Vol', 'Cell Type': cell.type, 'Cell ID': cell.id, 'Volume':cell.volume, 'Time': mcs})
                    else:
                        self.divide_cell_random_orientation(cell)                       
                        self.event_data.append({'Event Type': 'Stem After Mitosis Vol', 'Cell Type': cell.type, 'Cell ID': cell.id, 'Volume':cell.volume, 'Time': mcs})
               
                # BASAL cell constraints in division    
                elif cell.type == self.BASAL:
                    self.BASAL_to_divide += 1                    
                    self.event_data.append({'Event Type': 'Basal Before Mitosis Vol', 'Cell Type': cell.type, 'Cell ID': cell.id, 'Volume':cell.volume, 'Time': mcs})
                    if cell.dict['DivisionCount'] > 0:
                        cell.dict['DivisionCount'] -= 1                
                        self.divide_cell_random_orientation(cell)
                        # self.divide_cell_orientation_vector_based(cell,1,0,0)                        
                        self.event_data.append({'Event Type': 'Basal After Mitosis Vol', 'Cell Type': cell.type, 'Cell ID': cell.id, 'Volume':cell.volume, 'Time': mcs})
                        
                    else:
                        cell.type = self.WING # Differentiation Due to Last Division
                        cell.lambdaSurface = ConstraintInitializerSteppable.InitWING_LambdaSurface
                        cell.targetSurface = ConstraintInitializerSteppable.InitWING_TargetSurface                     
                        # self.divide_cell_random_orientation(cell)  
                
                elif cell.type == self.TEAR :
                    self.divide_cell_orientation_vector_based(cell,1,0,0)

            # Append counts to history list
            self.cells_divided_count_history['Basal'].append(self.BASAL_to_divide)
            self.cells_divided_count_history['Stem'].append(self.STEM_to_divide)

    def get_divided_cells_counts(self):
        return self.cells_divided_count_history
    
    def get_cell_data_mass(self):        
        return self.event_data

    def clear_divided_cells_counts(self):       
        for key in self.cells_divided_count_history:
           self.cells_divided_count_history[key] = []
    
    def clear_cell_data_mass(self):
        self.event_data = []   

    def update_attributes(self):        
        self.parent_cell.targetVolume /= 2.0 
        self.clone_parent_2_child()
        
class DeathSteppable(SteppableBasePy):

    event_data = []
    
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
        self.deathVolumeScalar  = ConstraintInitializerSteppable.DeathVolumeScalar                
        self.sloughScalar       = ConstraintInitializerSteppable.SloughScalar

        self.SUPER_TargetVolume = ConstraintInitializerSteppable.InitSUPER_TargetVolume
        self.SUPER_TargetSurface = ConstraintInitializerSteppable.InitSUPER_TargetSurface

        # SLS
        self.SLS_Injury = ConstraintInitializerSteppable.SLS_Injury
        self.SLS_Threshold_Method = ConstraintInitializerSteppable.SLS_Threshold_Method
        self.SLS_X_Center = ConstraintInitializerSteppable.SLS_X_Center
        self.SLS_Y_Center = ConstraintInitializerSteppable.SLS_Y_Center
        self.SLS_Concentration = ConstraintInitializerSteppable.SLS_Concentration 
        self.SLS_Gaussian_pulse = ConstraintInitializerSteppable.SLS_Gaussian_pulse       
    
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
                            self.event_data.append({'Event Type': 'Death Vol', 'Cell Type': cell.type, 'Cell ID': cell.id, 'Volume':cell.volume, 'Time': mcs}) 
                            # self.delete_cell(cell)
                            cell.type = self.TEAR
                            cell.targetSurface = 0
                            cell.lambdaSurface = 0
                            if len(self.get_fpp_links_by_cell(cell)) > 0:
                                for link in self.get_fpp_links_by_cell(cell):
                                    self.delete_fpp_link(link) 

                else:
                    # --- CHEMICAL ---
                    if (mcs == self.injuryTime):
                        if self.SLS_Gaussian_pulse:
                            self.SLS_initialField[self.SLS_X_Center, self.SLS_Y_Center, 0] = self.SLS_Concentration
                        else:
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
                                    if (cell.dict['SLS'] > 2):
                                        cell.dict['DEATH_MARK'] = True  
                                        cell.targetVolume = 0
                                        cell.lambdaVolume = 20
                                        cell.dict['SLS_Uptake'] = abs(self.SLS_Field.uptakeInsideCellTotalCount(cell, 100000.0, (cell.dict['SLS']/cell.volume)).tot_amount)
                                        self.event_data.append({'Event Type': 'Threshold_SLS','Cell Type': cell.type, 'Cell ID': cell.id, 'Volume':cell.volume, 'Time': mcs})
                            
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
                        # TODO markov draw for cell death instead of continous volume loss random draw between 0and1 if it is lower than the probability value then cell dies
                        # average of cell death is 3 days
                        
                        if random.random() <= ((1/3.0)/DAYtoMCS):
                            cell.targetVolume = 0
                            cell.lambdaVolume = 1000
                       
                        self.event_data.append({'Event Type': 'Superficial Loss Vol', 'Cell Type': cell.type, 'Cell ID': cell.id, 'Volume':cell.volume, 'Time': mcs})
                        
                        if cell.volume < 15 or self.SUPER not in NEIGHBOR_DICT.keys(): # Minimum Cell Size Before Slough | if no slogh cell will disapear in 672 MCS ~3 days(2.8)
                            cell.dict['Slough'] = (1 - np.exp(1/(-(HOURtoMCS*self.deathTimeScalar)*(cell.volume*self.deathVolumeScalar))))
                        
                            if (random.random() < cell.dict['Slough']):
                                deathsum += 1   
                                cell.targetVolume = 0
                                cell.lambdaVolume = 1000                             
                                self.event_data.append({'Event Type': 'Superficial Slough Vol', 'Cell Type': cell.type, 'Cell ID': cell.id, 'Volume':cell.volume, 'Time': mcs}) 
                                
                DEATHCOUNT = deathsum

            for cell in self.cell_list_by_type(self.SUPER, self.WING, self.BASAL, self.STEM):
                if (cell.volume < 3):                    
                    self.event_data.append({'Event Type': 'Death Vol','Cell Type': cell.type, 'Cell ID': cell.id, 'Volume':cell.volume, 'Time': mcs})                    
                    self.delete_cell(cell)            

    def get_cell_data_mass(self):        
        return self.event_data
    
    def clear_cell_data_mass(self):
        self.event_data = []   
 
class DifferentiationSteppable(SteppableBasePy):
    
    event_data = []

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
        # For Time based Differentiation
        # for cell in self.cell_list_by_type(self.WING):
        #     cell.dict['Differentiantion'] = False
        #     cell.dict['DifferentiantionTime'] = 0
        pass
       
    def step(self, mcs): 
      
        if self.DifferentiationControl:
            if mcs > HOURtoMCS: #[ ] Need to check for relaxation time                            
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
                        self.event_data.append({'Event Type': 'Wing Before Differentiation Vol', 'Cell Type': cell.type, 'Cell ID': cell.id, 'Volume':cell.volume, 'Time': mcs})
                        cell.type = self.SUPER
                        self.initializeDifferentiatedCell(cell)
                        self.event_data.append({'Event Type': 'Superficial After Differentiation Vol', 'Cell Type': cell.type, 'Cell ID': cell.id, 'Volume':cell.volume, 'Time': mcs})

                        # Time based Differention                                    
                    # if ((self.TEAR in NEIGHBOR_DICT.keys()) and 
                    #     (self.WING in NEIGHBOR_DICT.keys()) and 
                    #     not (self.BASAL in NEIGHBOR_DICT.keys()) and 
                    #     not (self.MEMB in NEIGHBOR_DICT.keys()) and
                    #     cell.dict['Differentiantion'] == False): 
                    #     cell.dict['Differentiantion'] = True
                    #     cell.dict['DifferentiantionTime'] = mcs
                    # if cell.dict['Differentiantion'] and (mcs - cell.dict['DifferentiantionTime'] > HOURtoMCS * 2):
                    #     self.event_data.append({'Event Type': 'Wing Before Differentiation Vol', 'Cell Type': cell.type, 'Cell ID': cell.id, 'Volume':cell.volume, 'Time': mcs})
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
                            # if neighbor_pixel.type == self.BM:
                            if neighbor_pixel.type == self.MEMB:
                                memb_contact_area += 1  # Increment the counter for every MEMB neighbor pixel
                    # Differentiation Rules: The cell will differentiate if its contact area with MEMB is 2 or less pixels
                    if memb_contact_area <= 5:                        
                        self.event_data.append({'Event Type': 'Basal Before Differentiation Vol', 'Cell Type': cell.type, 'Cell ID': cell.id, 'Volume':cell.volume, 'Time': mcs})
                        cell.type = self.WING
                        self.initializeDifferentiatedCell(cell)                        
                        self.event_data.append({'Event Type': 'Wing After Differentiation Vol', 'Cell Type': cell.type, 'Cell ID': cell.id, 'Volume':cell.volume, 'Time': mcs})
                # STEM CELL
                for cell in self.cell_list_by_type(self.STEM):
                    NEIGHBOR_DICT = self.get_cell_neighbor_data_list(cell).neighbor_count_by_type()
                    # Fetch the boundary pixels of the current Basal cell
                    boundary_pixel_list = self.get_cell_boundary_pixel_list(cell)                    
                    memb_contact_area = 0  # Counter for the area of contact with MEMB cells                    
                    # Iterate through each boundary pixel to check for MEMB neighbors
                    # for boundary_pixel_tracker_data in boundary_pixel_list:
                    #     boundary_pixel = boundary_pixel_tracker_data.pixel
                    #     for neighbor_pixel in self.get_pixel_neighbors(boundary_pixel):
                    #         if neighbor_pixel.type == self.LIMB:
                    #             memb_contact_area += 1  # Increment the counter for every MEMB neighbor pixel
                    # Differentiation Rules: The cell will differentiate if its contact area with MEMB is 2 or less pixels
                    if not self.LIMB in NEIGHBOR_DICT.keys():                        
                        self.event_data.append({'Event Type': 'Stem Before Differentiation Vol', 'Cell Type': cell.type, 'Cell ID': cell.id, 'Volume':cell.volume, 'Time': mcs})
                        cell.type = self.BASAL
                        self.initializeDifferentiatedCell(cell)                        
                        self.event_data.append({'Event Type': 'Basal After Differentiation Vol', 'Cell Type': cell.type, 'Cell ID': cell.id, 'Volume':cell.volume, 'Time': mcs})
        
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
    
    def get_cell_data_mass(self):        
        return self.event_data
    
    def clear_cell_data_mass(self):        
        self.event_data = []

    def initializeDifferentiatedCell(self, cell):
        # --- BASAL ---
        if cell.type == self.BASAL:
            cell.lambdaSurface = ConstraintInitializerSteppable.InitBASAL_LambdaSurface
            cell.targetSurface = ConstraintInitializerSteppable.InitBASAL_TargetSurface
            cell.dict["LambdaChemo"] = ConstraintInitializerSteppable.InitBASAL_LambdaChemo
            cell.dict['DivisionCount'] = ConstraintInitializerSteppable.InitBASAL_Division
            ChemotaxisData = self.chemotaxisPlugin.addChemotaxisData(cell, "BASALMVBIAS")            
            ChemotaxisData.setLambda(cell.dict["LambdaChemo"])
        # --- STEM ---
        elif cell.type == self.STEM:
            cell.lambdaSurface = ConstraintInitializerSteppable.InitSTEM_LambdaSurface
            cell.targetSurface = ConstraintInitializerSteppable.InitSTEM_TargetSurface
            cell.dict["LambdaChemo"] = ConstraintInitializerSteppable.InitSTEM_LambdaChemo
            ChemotaxisData = self.chemotaxisPlugin.addChemotaxisData(cell, "BASALMVBIAS")            
            ChemotaxisData.setLambda(cell.dict["LambdaChemo"])
        # --- WING ---
        elif cell.type == self.WING:
            cell.lambdaSurface = ConstraintInitializerSteppable.InitWING_LambdaSurface
            cell.targetSurface = ConstraintInitializerSteppable.InitWING_TargetSurface
            cell.lamdaVolume = ConstraintInitializerSteppable.InitWING_LambdaVolume
            cell.targetVolume = ConstraintInitializerSteppable.InitWING_TargetVolume
            cell.dict["LambdaChemo"] = 0
            # For Time based Differentiation       
            #     cell.dict['Differentiantion'] = False
            #     cell.dict['DifferentiantionTime'] = 0
        # --- SUPER ---
        elif cell.type == self.SUPER:
            cell.lambdaSurface = ConstraintInitializerSteppable.InitSUPER_LambdaSurface
            cell.targetSurface = ConstraintInitializerSteppable.InitSUPER_TargetSurface
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

        for cell in self.cell_list_by_type(self.MEMB, self.BM):
            self.MovementBias.secreteOutsideCellAtBoundaryOnContactWith(cell, self.MovementBiasScreteAmount, [self.MEDIUM, self.WING, self.SUPER, self.TEAR])
        
        for cell in self.cell_list_by_type(self.TEAR):
            (self.SLS_Field.uptakeInsideCellTotalCount(cell, 0.002, (cell.dict['SLS']/cell.volume)).tot_amount)

class TEARSteppable(SteppableBasePy):

    def __init__(self, frequency=1):

        SteppableBasePy.__init__(self, frequency)

    def start(self):

        self.initialTEARcount = len(self.cell_list_by_type(self.TEAR))
        self.Force_TEAR = 30
     
    def step(self, mcs):
        flag = False
        if len(self.cell_list_by_type(self.TEAR)) < self.initialTEARcount:
            flag = True
            
        for cell in self.cell_list_by_type(self.TEAR):
            NEIGHBOR_DICT = self.get_cell_neighbor_data_list(cell).neighbor_count_by_type()            
            is_trapped = (self.MEDIUM not in NEIGHBOR_DICT and self.TEAR not in NEIGHBOR_DICT) or (self.SUPER in NEIGHBOR_DICT and self.STROMA in NEIGHBOR_DICT)

       
            if is_trapped:
                # If the tear cell is trapped, it should die
                cell.targetVolume = 0
            
            elif (not any(neigh_type in NEIGHBOR_DICT for neigh_type in 
                [self.SUPER, self.WING, self.BASAL, self.STEM, self.MEMB, self.LIMB, self.BM, self.STROMA])):
                
                # Move the tear cell downward until it touches another cell
                cell.lambdaVecX = 0
                cell.lambdaVecY = self.Force_TEAR 

            else:
                cell.lambdaVecX = 0
                cell.lambdaVecY = 0

            if flag:
                # random choice on a list (self.cell_list_by_type(self.TEAR))

                cell  = np.random.choice(list(self.cell_list_by_type(self.TEAR)))
                 
                cell.targetVolume += 10 # Add volume to the tear cell

class PlotSteppable(SteppableBasePy):

    def __init__(self, frequency=1):      
        SteppableBasePy.__init__(self, frequency)

        self.CC3D_PLOT      = ConstraintInitializerSteppable.CC3D_PLOT 
        self.mitosis        = MitosisSteppable()
        self.death          = DeathSteppable()
        self.differentiation= DifferentiationSteppable()
        self.growth         = GrowthSteppable()
        self.cellCount      = ConstraintInitializerSteppable.CellCount
        self.xhypTracker    = ConstraintInitializerSteppable.XHypTracker
        self.sloughTracker  = ConstraintInitializerSteppable.SloughTracker
        self.centerBiasPlot = ConstraintInitializerSteppable.CenterBiasPlot
        self.pressurePlot   = ConstraintInitializerSteppable.PressurePlot
        self.growthPlot     = ConstraintInitializerSteppable.GrowthPlot
        self.thicknessPlot  = ConstraintInitializerSteppable.ThicknessPlot
        self.VolumeSurfaceDetailPlot = ConstraintInitializerSteppable.VolumeSurfaceDetail
        self.MitosisPlot    = ConstraintInitializerSteppable.MitosisPlot
        self.SingleCellPresEGFPlot = ConstraintInitializerSteppable.SingleCellPresEGFPlot
        self.MassConservationPlot = ConstraintInitializerSteppable.MassConservationPlot
        self.SurfactantTracking = ConstraintInitializerSteppable.SurfactantTracking
        self.SnapShot       = ConstraintInitializerSteppable.SnapShot 
        self.EGF_GrowthScalar_STEM  = ConstraintInitializerSteppable.EGF_GrowthScalar_STEM
        self.EGF_DensityScalar_STEM = ConstraintInitializerSteppable.DensityGrowthScalar_STEM
        self.EGF_GrowthScalar_BASAL  = ConstraintInitializerSteppable.EGF_GrowthScalar_BASAL
        self.EGF_DensityScalar_BASAL = ConstraintInitializerSteppable.DensityGrowthScalar_BASAL
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

            'slough_tracker': {
                "Wing": [],
                "Individual Volume": [],
                "Average Vol": [],
                "Slough Count": [],
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
                # "Basal Individual": [],
                # "Wings": [],
                # "Average Vol": [],
                # "Slough Count": []
            },

            'growth': {
                "Stem Average Total": [],
                "Basal Average Total": [],
                "Stem Average EGF": [],
                "Basal Average EGF": [],
                "Stem Average Density": [],
                "Basal Average Density": [],
                "Stem Average TargetVolume": [],
                "Basal Average TargetVolume": [],
                "Stem Average EGFseen" : [],
                "Basal Average EGFseen" : [],
                "Stem Average Pressure" : [],
                "Basal Average Pressure" : [],
                "Time": []
            },

            'thickness': {
                                
            },
            'thickness_raw': {
                                
            },

            'volume_surface': {
                'Superficial Volume': [],
                'Superficial Surface': [],
                'Wing Volume': [],
                'Wing Surface': [],
                'Basal Volume': [],
                'Basal Surface': [],                
                'Stem Volume': [],
                'Stem Surface': [],
                'Time': []
            },
            
            'mitosis': {
                'BASAL': [],
                'STEM': [],
                'Time': []
            },
            
            'single_cell_pres_EGF': {
                'BASAL': {},
                'STEM': {},
            },

            'surfactant': {
                'Total Amount': [],
                'Time': []                
            },

            'mass_conservation': []

                            }

    def start(self):
        self.bins_number = 5
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
        # ---- Slough Tracker Plot ----
            if self.sloughTracker:
                self.plot_slough_tracker = self.add_new_plot_window(title='Slough Tracker Tracker',
                                                                x_axis_title='Time (Hour)',
                                                                y_axis_title='Volume(Cyan and Red) / # Cells(Green)',
                                                                x_scale_type='linear', y_scale_type='linear',
                                                                grid=True, config_options={'legend':True})

                self.plot_slough_tracker.add_plot("Wing", style='Dots', color='blue')
                self.plot_slough_tracker.add_plot("Individual Volume", style='Dots', color='Cyan')
                self.plot_slough_tracker.add_plot("Average Vol", style='lines', color='red')
                self.plot_slough_tracker.add_plot("Slough Count", style='lines', color='green')
        # ---- Chemochine Center Movement Bias Plot ----
            if self.centerBiasPlot:
                self.plot_center_bias = self.add_new_plot_window(title='Chemochine Center Movement Bias',
                                                                x_axis_title='Time (Hour)',
                                                                y_axis_title='Strenth of the Signal',
                                                                x_scale_type='linear', y_scale_type='linear',
                                                                grid=True, config_options={'legend':True})
                self.plot_center_bias.add_plot("Wing", style='Dots', color='blue')
                self.plot_center_bias.add_plot("Individual Volume", style='Dots', color='Cyan')
                self.plot_center_bias.add_plot("Average Vol", style='lines', color='red')
                self.plot_center_bias.add_plot("Slough Count", style='lines', color='green')

            if self.pressurePlot:
                self.plot_pressure = self.add_new_plot_window(title='System Pressure',
                                                                x_axis_title='Time (Hour)',
                                                                y_axis_title='Pressure',
                                                                x_scale_type='linear', y_scale_type='linear',
                                                                grid=True, config_options={'legend':True})
                self.plot_pressure.add_plot("Stem Average", style='lines', color='red')
                self.plot_pressure.add_plot("Basal Average", style='lines', color='pink')                

            if self.growthPlot:
                self.plot_growth = self.add_new_plot_window(title='System Growth',
                                                                x_axis_title='Time (Hour)',
                                                                y_axis_title='x100',
                                                                x_scale_type='linear', y_scale_type='linear',
                                                                grid=True, config_options={'legend':True})
                self.plot_growth.add_plot("Stem Average Total", style='lines', color='red')
                self.plot_growth.add_plot("Basal Average Total", style='lines', color='pink')
                self.plot_growth.add_plot("Stem Average EGF", style='lines', color='grey')
                self.plot_growth.add_plot("Basal Average EGF", style='lines', color='Orange')
                self.plot_growth.add_plot("Stem Average Density", style='lines', color='Purple')
                self.plot_growth.add_plot("Basal Average Density", style='lines', color='Brown')
                self.plot_growth.add_plot("Stem Average TargetVolume", style='lines', color='Magenta')
                self.plot_growth.add_plot("Basal Average TargetVolume", style='lines', color='cyan')
                self.plot_growth.add_plot("Stem Average EGFseen", style='lines', color='green')
                self.plot_growth.add_plot("Basal Average EGFseen", style='lines', color='teal')
                self.plot_growth.add_plot("Stem Average Pressure", style='lines', color='Blue')
                self.plot_growth.add_plot("Basal Average Pressure", style='lines', color='lightblue')

            if self.thicknessPlot:
                self.plot_thickness = self.add_new_plot_window(title='Tissue Thickness',
                                                                x_axis_title='Time(Hours)',
                                                                y_axis_title='Average Y position',
                                                                x_scale_type='linear', y_scale_type='linear',
                                                                grid=True, config_options={'legend':True})
                
                for bin_index in self.bin_indexes:
                    self.plot_thickness.add_plot(f'X Coord.{bin_index * self.bin_size}-{(bin_index + 1) * self.bin_size}', style='lines')
                
            if self.VolumeSurfaceDetailPlot:
                self.plot_volume_surface = self.add_new_plot_window(title='Volume Surface Detail',
                                                                x_axis_title='Time(Hours)',
                                                                y_axis_title='Volume Surface',
                                                                x_scale_type='linear', y_scale_type='linear',
                                                                grid=True, config_options={'legend':True})
                self.plot_volume_surface.add_plot('Superficial Volume', style='lines', color='cyan')
                self.plot_volume_surface.add_plot('Superficial Surface', style='lines', color='blue')
                self.plot_volume_surface.add_plot('Wing Volume', style='lines', color='pink')
                self.plot_volume_surface.add_plot('Wing Surface', style='lines', color='red')
                self.plot_volume_surface.add_plot('Basal Volume', style='lines', color='green')
                self.plot_volume_surface.add_plot('Basal Surface', style='lines', color='teal')
                self.plot_volume_surface.add_plot('Stem Volume', style='lines', color='purple')
                self.plot_volume_surface.add_plot('Stem Surface', style='lines', color='brown')

            if self.MitosisPlot:
                self.plot_mitosis = self.add_new_plot_window(title='Mitosis',
                                                                x_axis_title='Time(Hours)',
                                                                y_axis_title='Number of Cells',
                                                                x_scale_type='linear', y_scale_type='linear',
                                                                grid=True, config_options={'legend':True})
                self.plot_mitosis.add_plot('BASAL', style='lines', color='cyan')
                self.plot_mitosis.add_plot('STEM', style='lines', color='blue')
            
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
        global output_directory
        HOURtoMCS_factor = mcs / HOURtoMCS
        self.MCS = mcs

        # ---- End of Simulation ----
        if mcs == (self.SimTime + 1) :
            if self.cellCount:                
                self.write_csv_for_category('cell_count', mcs, output_directory)
            if self.sloughTracker:
                self.write_csv_for_category('slough_tracker', mcs, output_directory)
            if self.pressurePlot:
                self.write_csv_for_category('pressure', mcs, output_directory)
            if self.growthPlot:
                self.write_csv_for_category('growth', mcs, output_directory)
            if self.thicknessPlot:                
                self.write_parquet_for_cell_data_with_replicate('thickness', mcs, output_directory)            
                self.write_parquet_for_cell_data_with_replicate('thickness_raw', mcs, output_directory)
            if self.VolumeSurfaceDetailPlot:
                self.write_csv_for_category('volume_surface', mcs, output_directory)
            if self.MitosisPlot:
                self.write_csv_for_category('mitosis', mcs, output_directory)
            if self.SingleCellPresEGFPlot:                
                self.write_parquet_for_cell_data_with_replicate('single_cell_pres_EGF', mcs, output_directory)
            if self.MassConservationPlot:                
                self.write_parquet_for_cell_data_with_replicate('mass_conservation', mcs, output_directory)
            if self.SurfactantTracking:
                self.write_csv_for_category('surfactant', mcs, output_directory)
                # self.write_parquet_for_cell_data_with_replicate('surfactant', mcs)
            if self.SnapShot:
                self.request_screenshot(mcs=mcs, screenshot_label='Cell_Field_CellField_2D_XY_0')
            
            # Save the cell field as an image
            # self.save_visualizations(mcs)  

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
                # for cell_type, count in self.data_points['cell_count'].items():
                #     self.plot_data_point(self.plot_cell_count, cell_type, HOURtoMCS_factor, count)
                self.plot_cell_count.add_data_point("Superficial", HOURtoMCS_factor, len(self.cell_list_by_type(self.SUPER)))
                self.plot_cell_count.add_data_point("Wing", HOURtoMCS_factor, len(self.cell_list_by_type(self.WING)))
                self.plot_cell_count.add_data_point("Basal", HOURtoMCS_factor, len(self.cell_list_by_type(self.BASAL)))
                self.plot_cell_count.add_data_point("Stem", HOURtoMCS_factor, len(self.cell_list_by_type(self.STEM)))
            
        if self.sloughTracker and mcs % HOURtoMCS == 0:
            
            listVol = []
            
            for cell in self.cell_list_by_type(self.SUPER):
                listVol.append(cell.volume)

            SumVol = sum(listVol)

            listMinVol = [i for i in listVol if i <= 15]

            for i in listMinVol:                
                # CSV
                self.data_points['slough_tracker']['Individual Volume'].append(i)                
                
            AvrVol = SumVol/len(self.cell_list_by_type(self.SUPER))           
            # CSV
            self.data_points['slough_tracker']['Average Vol'].append(AvrVol)
            self.data_points['slough_tracker']['Slough Count'].append(DEATHCOUNT)
            self.data_points['slough_tracker']['Wing'].append(len(self.cell_list_by_type(self.WING)))
            self.data_points['slough_tracker']['Individual Volume'].append(HOURtoMCS_factor)
            self.data_points['slough_tracker']['Time'].append(HOURtoMCS_factor)

            if self.CC3D_PLOT:
                # CC3D Plots
                # for cell_type, count in self.data_points['slough_tracker'].items():                    
                #     self.plot_data_point(self.plot_slough_tracker, cell_type, HOURtoMCS_factor, count)
                self.plot_slough_tracker.add_data_point("Wing", HOURtoMCS_factor, len(self.cell_list_by_type(self.WING)))
                self.plot_slough_tracker.add_data_point("Individual Volume", HOURtoMCS_factor, len(listMinVol))
                self.plot_slough_tracker.add_data_point("Average Vol", HOURtoMCS_factor, AvrVol)
                self.plot_slough_tracker.add_data_point("Slough Count", HOURtoMCS_factor, DEATHCOUNT)

        if self.pressurePlot and mcs % HOURtoMCS == 0:
            BASAL_avg_pressure  = []
            STEM_avg_pressure   = []
            for cell in self.cell_list_by_type(self.BASAL,self.STEM):
                if cell.type == self.BASAL:
                    BASAL_avg_pressure.append(abs(cell.pressure))
                    # self.plot_pressure.add_data_point("Basal Individual", mcs/HOURtoMCS, abs(cell.pressure))

                elif cell.type == self.STEM:
                    STEM_avg_pressure.append(abs(cell.pressure))

            # CSV
            self.data_points['pressure']['Stem Average'].append(np.mean(STEM_avg_pressure))
            self.data_points['pressure']['Basal Average'].append(np.mean(BASAL_avg_pressure))
            self.data_points['pressure']['Time'].append(HOURtoMCS_factor)            

            if self.CC3D_PLOT:
                # CC3D Plots
                # for cell_type, count in self.data_points['pressure'].items():
                #     self.plot_data_point(self.plot_pressure, cell_type, HOURtoMCS_factor, count)
                self.plot_pressure.add_data_point("Stem Average", HOURtoMCS_factor, np.mean(STEM_avg_pressure))
                self.plot_pressure.add_data_point("Basal Average", HOURtoMCS_factor, np.mean(BASAL_avg_pressure))

        if self.growthPlot and mcs % HOURtoMCS == 0:            

            BASAL_avg_EGF_growth  = []
            BASAL_avg_density_growth  = []
            BASAL_avg_total_growth  = []
            BASAL_avg_targetVolume  = []
            BASAL_avg_EGFseen  = []
            BASAL_avg_pressure  = []

            STEM_avg_EGF_growth  = []
            STEM_avg_density_growth  = []
            STEM_avg_total_growth  = []
            STEM_avg_targetVolume  = []
            STEM_avg_EGFseen  = []
            STEM_avg_pressure  = []
                                       

            for cell in self.cell_list_by_type(self.BASAL,self.STEM):
                if cell.type == self.BASAL:
                    BASAL_avg_EGF_growth.append(cell.dict['EGF_Growth'])
                    BASAL_avg_density_growth.append(cell.dict['DensityGrowth'])
                    BASAL_avg_total_growth.append(cell.dict['TotalGrowth'])
                    BASAL_avg_targetVolume.append(cell.targetVolume)
                    BASAL_avg_EGFseen.append(cell.dict['EGF'])
                    BASAL_avg_pressure.append(cell.dict['Pressure'])

                elif cell.type == self.STEM:
                    STEM_avg_EGF_growth.append(cell.dict['EGF_Growth'])
                    STEM_avg_density_growth.append(cell.dict['DensityGrowth'])
                    STEM_avg_total_growth.append(cell.dict['TotalGrowth'])
                    STEM_avg_targetVolume.append(cell.targetVolume)
                    STEM_avg_EGFseen.append(cell.dict['EGF'])
                    STEM_avg_pressure.append(cell.dict['Pressure'])

            # CSV
            self.data_points['growth']['Stem Average Total'].append(np.mean(STEM_avg_total_growth))
            self.data_points['growth']['Basal Average Total'].append(np.mean(BASAL_avg_total_growth))
            self.data_points['growth']['Stem Average EGF'].append(np.mean(STEM_avg_EGF_growth))
            self.data_points['growth']['Basal Average EGF'].append(np.mean(BASAL_avg_EGF_growth))
            self.data_points['growth']['Stem Average Density'].append(np.mean(STEM_avg_density_growth))
            self.data_points['growth']['Basal Average Density'].append(np.mean(BASAL_avg_density_growth))
            self.data_points['growth']['Stem Average TargetVolume'].append(np.mean(STEM_avg_targetVolume))
            self.data_points['growth']['Basal Average TargetVolume'].append(np.mean(BASAL_avg_targetVolume))
            self.data_points['growth']['Stem Average EGFseen'].append(np.mean(STEM_avg_EGFseen))
            self.data_points['growth']['Basal Average EGFseen'].append(np.mean(BASAL_avg_EGFseen))
            self.data_points['growth']['Stem Average Pressure'].append(np.mean(STEM_avg_pressure))
            self.data_points['growth']['Basal Average Pressure'].append(np.mean(BASAL_avg_pressure))
            self.data_points['growth']['Time'].append(HOURtoMCS_factor)

            if self.CC3D_PLOT:
                # for cell_type, count in self.data_points['growth'].items():
                #     self.plot_data_point(self.plot_growth, cell_type, HOURtoMCS_factor, count) 
                self.plot_growth.add_data_point("Stem Average Total", HOURtoMCS_factor, np.mean(STEM_avg_total_growth))
                self.plot_growth.add_data_point("Basal Average Total", HOURtoMCS_factor, np.mean(BASAL_avg_total_growth))
                self.plot_growth.add_data_point("Stem Average EGF", HOURtoMCS_factor, np.mean(STEM_avg_EGF_growth))
                self.plot_growth.add_data_point("Basal Average EGF", HOURtoMCS_factor, np.mean(BASAL_avg_EGF_growth))
                self.plot_growth.add_data_point("Stem Average Density", HOURtoMCS_factor, np.mean(STEM_avg_density_growth))
                self.plot_growth.add_data_point("Basal Average Density", HOURtoMCS_factor, np.mean(BASAL_avg_density_growth))
                self.plot_growth.add_data_point("Stem Average TargetVolume", HOURtoMCS_factor, np.mean(STEM_avg_targetVolume))
                self.plot_growth.add_data_point("Basal Average TargetVolume", HOURtoMCS_factor, np.mean(BASAL_avg_targetVolume))
                self.plot_growth.add_data_point("Stem Average EGFseen", HOURtoMCS_factor, np.mean(STEM_avg_EGFseen))
                self.plot_growth.add_data_point("Basal Average EGFseen", HOURtoMCS_factor, np.mean(BASAL_avg_EGFseen))
                self.plot_growth.add_data_point("Stem Average Pressure", HOURtoMCS_factor, np.mean(STEM_avg_pressure))
                self.plot_growth.add_data_point("Basal Average Pressure", HOURtoMCS_factor, np.mean(BASAL_avg_pressure))
  
        if self.thicknessPlot and mcs % HOURtoMCS == 0:
            self.data_points['thickness'][HOURtoMCS_factor] = {'Bins': {}}
            self.data_points['thickness_raw'][HOURtoMCS_factor] = {'Cell Data': []}
            bins = {index: [] for index in self.bin_indexes}            
           
            for cell in self.cell_list_by_type(self.SUPER):
                # Binning the data
                bin_index = cell.xCOM // self.bin_size               
                bins[bin_index].append(cell.yCOM)

                self.data_points['thickness_raw'][HOURtoMCS_factor]['Cell Data'].append({
                    'CellID': cell.id,
                    'xCOM': cell.xCOM,
                    'yCOM': cell.yCOM,                    
                    'Type': cell.type
                })

            # Check for interface cells with the tear or medium
            for cell in self.cell_list_by_type(self.WING, self.BASAL, self.STEM):
                NEIGHBOR_DICT = self.get_cell_neighbor_data_list(cell).neighbor_count_by_type()
                if self.TEAR in NEIGHBOR_DICT.keys() or self.MEDIUM in NEIGHBOR_DICT.keys():
                    bin_index = cell.xCOM // self.bin_size
                    if bin_index in bins:
                        bins[bin_index].append(cell.yCOM)
                        self.data_points['thickness_raw'][HOURtoMCS_factor]['Cell Data'].append({
                            'CellID': cell.id,
                            'xCOM': cell.xCOM,
                            'yCOM': cell.yCOM,                    
                            'Type': cell.type
                        })              

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
        
        if self.VolumeSurfaceDetailPlot and mcs % HOURtoMCS == 0:

            SUPER_vol = []
            SUPER_surf = []
            WING_vol = []
            WING_surf = []
            BASAL_vol = []
            BASAL_surf = []
            STEM_vol = []
            STEM_surf = []

            for cell in self.cell_list_by_type(self.SUPER,self.WING,self.BASAL,self.STEM):
                if cell.type == self.SUPER:
                    SUPER_vol.append(cell.volume)
                    SUPER_surf.append(cell.surface)
                elif cell.type == self.WING:
                    WING_vol.append(cell.volume)
                    WING_surf.append(cell.surface)
                elif cell.type == self.BASAL:
                    BASAL_vol.append(cell.volume)
                    BASAL_surf.append(cell.surface)
                elif cell.type == self.STEM:
                    STEM_vol.append(cell.volume)
                    STEM_surf.append(cell.surface)
            # CSV
            self.data_points['volume_surface']['Superficial Volume'].append(np.mean(SUPER_vol))
            self.data_points['volume_surface']['Superficial Surface'].append(np.mean(SUPER_surf))
            self.data_points['volume_surface']['Wing Volume'].append(np.mean(WING_vol))
            self.data_points['volume_surface']['Wing Surface'].append(np.mean(WING_surf))
            self.data_points['volume_surface']['Basal Volume'].append(np.mean(BASAL_vol))
            self.data_points['volume_surface']['Basal Surface'].append(np.mean(BASAL_surf))
            self.data_points['volume_surface']['Stem Volume'].append(np.mean(STEM_vol))
            self.data_points['volume_surface']['Stem Surface'].append(np.mean(STEM_surf))
            self.data_points['volume_surface']['Time'].append(HOURtoMCS_factor)

            if self.CC3D_PLOT:
                # for cell_type, count in self.data_points['volume_surface'].items():
                #     self.plot_data_point(self.plot_thickness, cell_type, HOURtoMCS_factor, count)
                self.plot_volume_surface.add_data_point('Superficial Volume', HOURtoMCS_factor, np.mean(SUPER_vol))
                self.plot_volume_surface.add_data_point('Superficial Surface', HOURtoMCS_factor, np.mean(SUPER_surf))
                self.plot_volume_surface.add_data_point('Wing Volume', HOURtoMCS_factor, np.mean(WING_vol))
                self.plot_volume_surface.add_data_point('Wing Surface', HOURtoMCS_factor, np.mean(WING_surf))
                self.plot_volume_surface.add_data_point('Basal Volume', HOURtoMCS_factor, np.mean(BASAL_vol))
                self.plot_volume_surface.add_data_point('Basal Surface', HOURtoMCS_factor, np.mean(BASAL_surf))
                self.plot_volume_surface.add_data_point('Stem Volume', HOURtoMCS_factor, np.mean(STEM_vol))
                self.plot_volume_surface.add_data_point('Stem Surface', HOURtoMCS_factor, np.mean(STEM_surf))
            
        if self.MitosisPlot:
                
            history_count = self.mitosis.get_divided_cells_counts()
            
            # CSV
            self.data_points['mitosis']['BASAL'].append(sum(history_count['Basal']))
            self.data_points['mitosis']['STEM'].append(sum(history_count['Stem']))
            self.data_points['mitosis']['Time'].append(HOURtoMCS_factor)

            if self.CC3D_PLOT:
                # for cell_type, count in self.data_points['mitosis'].items():
                #     self.plot_data_point(self.plot_mitosis, cell_type, HOURtoMCS_factor, count)
                self.plot_mitosis.add_data_point("BASAL", HOURtoMCS_factor, sum(history_count['Basal']))
                self.plot_mitosis.add_data_point("STEM", HOURtoMCS_factor, sum(history_count['Stem']))
            
            self.mitosis.clear_divided_cells_counts()

        if self.SingleCellPresEGFPlot:
                        
            for cell in self.cell_list_by_type(self.BASAL, self.STEM):
                cell_data = {
                    'pressure': cell.dict['Pressure'],
                    'EGF': cell.dict['EGF'],
                    'xCOM': cell.xCOM,
                    'yCOM': cell.yCOM,
                    'Time': HOURtoMCS_factor
                }
                
                if cell.type == self.BASAL:
                    if cell.id not in self.data_points['single_cell_pres_EGF']['BASAL']:
                        self.data_points['single_cell_pres_EGF']['BASAL'][cell.id] = []
                    self.data_points['single_cell_pres_EGF']['BASAL'][cell.id].append(cell_data)
                
                elif cell.type == self.STEM:
                    if cell.id not in self.data_points['single_cell_pres_EGF']['STEM']:
                        self.data_points['single_cell_pres_EGF']['STEM'][cell.id] = []
                    self.data_points['single_cell_pres_EGF']['STEM'][cell.id].append(cell_data)            

        if self.MassConservationPlot:            

            param_death = self.death.get_cell_data_mass()
            param_differentiation = self.differentiation.get_cell_data_mass()
            param_mitosis = self.mitosis.get_cell_data_mass()
            param_growth = self.growth.get_cell_data_mass()  
            
            # Gathering data from each steppable and extending the 'events' list in data_points
            if param_death != []:                
                self.data_points['mass_conservation'].append(param_death)
                self.death.clear_cell_data_mass()
            if param_differentiation != []:                
                self.data_points['mass_conservation'].append(param_differentiation)
                self.differentiation.clear_cell_data_mass()
            if param_mitosis != []:                
                self.data_points['mass_conservation'].append(param_mitosis)
                self.mitosis.clear_cell_data_mass()
            if param_growth != []:               
                self.data_points['mass_conservation'].append(param_growth)
                self.growth.clear_cell_data_mass()           

        if self.SurfactantTracking:
            if mcs == 0: # Start of simulation
                self.SLS_Field = self.get_field_secretor("SLS")

            self.data_points['surfactant']['Total Amount'].append(self.SLS_Field.totalFieldIntegral())
            self.data_points['surfactant']['Time'].append(HOURtoMCS_factor)

            if self.CC3D_PLOT: 
                self.SLS_plot.add_data_point("Total Amount", HOURtoMCS_factor, self.SLS_Field.totalFieldIntegral())
                #self.SLS_plot.add_data_point("Tear cells count", HOURtoMCS_factor, len(self.cell_list_by_type(self.TEAR)))   

    def on_stop(self):  
        print(f'Called on_stop function in PlotSteppable {self.__class__.__name__}') 
        mcs = self.MCS     
        if self.cellCount:                
            self.write_csv_for_category('cell_count', mcs, output_directory)
        if self.sloughTracker:
            self.write_csv_for_category('slough_tracker', mcs, output_directory)
        if self.pressurePlot:
            self.write_csv_for_category('pressure', mcs, output_directory)
        if self.growthPlot:
            self.write_csv_for_category('growth', mcs, output_directory)
        if self.thicknessPlot:
            self.write_parquet_for_cell_data_with_replicate('thickness', mcs, output_directory)           
            self.write_parquet_for_cell_data_with_replicate('thickness_raw', mcs, output_directory)
        if self.VolumeSurfaceDetailPlot:
            self.write_csv_for_category('volume_surface', mcs, output_directory)
        if self.MitosisPlot:
            self.write_csv_for_category('mitosis', mcs, output_directory)
        if self.SingleCellPresEGFPlot:            
            self.write_parquet_for_cell_data_with_replicate('single_cell_pres_EGF', mcs, output_directory)
        if self.MassConservationPlot:
            self.write_parquet_for_cell_data_with_replicate('mass_conservation', mcs, output_directory)            
        if self.SurfactantTracking:
            self.write_csv_for_category('surfactant', mcs, output_directory)
   
    def write_csv_for_category(self, category, mcs, directory="."):
        """
        Writes a CSV file for the given category from the self.data_points dictionary.
        The file will be named after the category and saved in the current_script_directory.
        """        
        data = self.data_points[category]
        # Extract the replicate number from the directory path
        replicate = Path(directory).parts[-2] if len(Path(directory).parts) > 1 else ""
        
        file_name = f"{category}_rep{replicate}_{mcs}.csv"
        if directory == ".":
            file_path = self.current_script_directory.joinpath(file_name)
        else:
            file_path = Path(directory).joinpath(file_name)        

        # Assuming all sub-dictionaries have the same length as 'Time'
        num_entries = len(data['Time'])
        # Ensure all lists have the same length as 'Time'
        for key in data:
            if len(data[key]) < num_entries:
                data[key].extend([0] * (num_entries - len(data[key])))
        
        with open(file_path, 'w', newline='') as csvfile:
            csvwriter = csv.writer(csvfile)
            
            # Write the header (column names)
            headers = list(data.keys())
            csvwriter.writerow(headers)
            
            # Write the rows of data
            for i in range(num_entries):
                row = [data[header][i] for header in headers]
                csvwriter.writerow(row)

    def write_csv_for_cell_data_with_replicate(self, category, mcs, directory="."):
        """
        Writes a CSV file for cell data.
        The file will be named after the category and saved in the given directory,
        with the replicate number extracted from the directory path.
        """
        
        # Extract the replicate number from the directory path
        replicate = Path(directory).parts[-2] if len(Path(directory).parts) > 1 else ""
        
        file_name = f"{category}_rep{replicate}_{mcs}.csv"
        if directory == ".":
            file_path = self.current_script_directory.joinpath(file_name)
        else:
            file_path = Path(directory).joinpath(file_name)        
        # Prepare data for CSV
        rows = []
        for cell_type in ['BASAL', 'STEM']:
            for cell_id, cell_data_list in self.data_points[category][cell_type].items():
                for cell_data in cell_data_list:
                    row = [cell_type, cell_id]
                    row.extend([cell_data[key] for key in ['pressure', 'EGF', 'xCOM', 'yCOM', 'Time']])
                    rows.append(row)
        
        # Write to CSV
        with open(file_path, 'w', newline='') as csvfile:
            csvwriter = csv.writer(csvfile)
            
            # Write the header (column names)
            headers = ['CellType', 'CellID', 'pressure', 'EGF', 'xCOM', 'yCOM', 'Time']
            csvwriter.writerow(headers)
            
            # Write the rows of data
            for row in rows:
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
        if directory == ".":
            file_path = self.current_script_directory.joinpath(file_name)
        else:
            file_path = Path(directory).joinpath(file_name)        
        # Prepare data for Parquet
        
        if category == 'single_cell_pres_EGF':
            rows = []
            for cell_type in ['BASAL', 'STEM']:
                for cell_id, cell_data_list in self.data_points[category][cell_type].items():
                    for cell_data in cell_data_list:
                        row = {
                            'CellType': cell_type,
                            'CellID': cell_id,
                            'pressure': cell_data['pressure'],
                            'EGF': cell_data['EGF'],
                            'xCOM': cell_data['xCOM'],
                            'yCOM': cell_data['yCOM'],
                            'Time': cell_data['Time']
                        }
                        rows.append(row)

        elif category == 'thickness':
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

        elif category == 'thickness_raw':
            rows = []
            for time_step, data in self.data_points[category].items(): 
                for cell_data in data['Cell Data']:
                    row = {                        
                        'Time': time_step,
                        'CellID': cell_data['CellID'],
                        'xCOM': cell_data['xCOM'],
                        'yCOM': cell_data['yCOM'],                        
                        'CellType': cell_data['Type']
                    }
                    rows.append(row)

        elif category == 'mass_conservation':
            rows = [event for sublist in self.data_points['mass_conservation'] for event in sublist]

        # Convert list of dictionaries to DataFrame
        df = pd.DataFrame(rows)        
        # Write to Parquet
        df.to_parquet(file_path, index=False)
    
    def write_to_parquet_for_mass_conservation(self, mcs, directory="."):        
        # Extract the replicate number from the directory path
        replicate = Path(directory).parts[-2] if len(Path(directory).parts) > 1 else ""        
        file_name = f"mass_conservation_rep{replicate}_{mcs}.parquet"
        if directory == ".":
            file_path = self.current_script_directory.joinpath(file_name)
        else:
            file_path = Path(directory).joinpath(file_name)
        # Flatten the list of lists into a single list of dictionaries
        flattened_events = [event for sublist in self.data_points['mass_conservation'] for event in sublist]
        # Convert the list of dictionaries to a DataFrame
        df = pd.DataFrame(flattened_events)
        # Write to Parquet
        df.to_parquet(file_path, index=False)

    def save_visualizations(self, mcs, directory="."):
        # Prepare arrays
        lattice_dims = (self.dim.x, self.dim.y)
        id_array = np.zeros(lattice_dims, dtype=int)
        type_array = np.zeros(lattice_dims, dtype=int)
        # Fill arrays with cell data
        for x, y, z in self.every_pixel():
            cell = self.cell_field[x, y, z]
            if cell:
                id_array[x, y] = cell.id
                type_array[x, y] = cell.type
        unique_ids = np.unique(id_array)
        print("Number of unique IDs:", unique_ids.size)
        # Encode IDs to RGB and rotate image
        rgb_encoded_id_image = self.encode_ids_to_rgb(id_array)
        rgb_encoded_id_image_rotated = rgb_encoded_id_image.rotate(90, expand=True)
        # Normalize and rotate type image
        normalized_type_image = self.normalize_and_create_image(type_array)
        type_image_rotated = normalized_type_image.rotate(90, expand=True)
        # File paths        
        replicate = Path(directory).parts[-2] if len(Path(directory).parts) > 1 else ""
        file_base = f"rep{replicate}_{mcs}"        
        file_path_ids = self.current_script_directory.joinpath(f"{file_base}_cell_ids.png")
        file_path_types = self.current_script_directory.joinpath(f"{file_base}_cell_types.png")
        file_path_ids_csv = self.current_script_directory.joinpath(f"{file_base}_cell_ids.csv")
        file_path_types_csv = self.current_script_directory.joinpath(f"{file_base}_cell_types.csv")        
        # Save images
        rgb_encoded_id_image_rotated.save(file_path_ids)
        print("Saved cell IDs image to", file_path_ids)
        type_image_rotated.save(file_path_types)
        print("Saved cell types image to", file_path_types)
        # Save arrays as CSV
        rotated_id_array = np.rot90(id_array)
        rotated_type_array = np.rot90(type_array)
        np.savetxt(file_path_ids_csv, rotated_id_array, delimiter=',', fmt='%d')
        np.savetxt(file_path_types_csv, rotated_type_array, delimiter=',', fmt='%d')
        print("Saved cell IDs CSV to", file_path_ids_csv)
        print("Saved cell types CSV to", file_path_types_csv)

    def encode_ids_to_rgb(self, id_array):
        """Encode cell IDs into an RGB image."""
        normalized_ids = np.interp(id_array, (id_array.min(), id_array.max()), (0, 2**24 - 1)).astype(int)
        rgb_encoded = np.stack(((normalized_ids >> 16) & 255, (normalized_ids >> 8) & 255, normalized_ids & 255), axis=-1)
        return Image.fromarray(rgb_encoded.astype(np.uint8))   
 
    def normalize_and_create_image(self, array):
        """Normalize an array and create a grayscale image."""
        norm = Normalize(vmin=array.min(), vmax=array.max())
        normalized_array = norm(array) * 255
        # Correctly specify the mode 'L' for grayscale image
        return Image.fromarray(normalized_array.astype(np.uint8), 'L')