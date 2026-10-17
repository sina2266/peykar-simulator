import cmaboss
import numpy as np
import pandas as pd
from unittest import TestCase
from sys import platform

class TestCMaBoSS(TestCase):

    def test_load_model(self):
        net = cmaboss.MaBoSSNet(network="../tests/metastasis/metastasis.bnd")
        cfg = cmaboss.MaBoSSCfg(net, "../tests/metastasis/metastasis.cfg")
        sim = cmaboss.MaBoSSSim(net=net, cfg=cfg)
        res = sim.run()

    def test_load_model_cellcycle(self):
        net = cmaboss.MaBoSSNet(network="../tests/cellcycle/cellcycle.bnd")
        cfg = cmaboss.MaBoSSCfg(net, "../tests/cellcycle/cellcycle_runcfg.cfg", "../tests/cellcycle/cellcycle_runcfg-thread_6.cfg")
        sim = cmaboss.MaBoSSSim(net=net, cfg=cfg)
        res = sim.run()

    def test_simulate(self):
        sim = cmaboss.MaBoSSSim(network="../tests/metastasis/metastasis.bnd", config="../tests/metastasis/metastasis.cfg")
        res = sim.run()
        res.get_probtraj()
        res.get_last_probtraj()
        res.get_nodes_probtraj()
        res.get_last_nodes_probtraj()
        res.get_fp_table()
        
    def test_load_model_error(self):
        with self.assertRaises(cmaboss.BNException):
            cmaboss.MaBoSSSim(network="../tests/metastasis/metastasis-error.bnd", config="../tests/metastasis/metastasis.cfg")

    def test_last_states(self):
        expected = {
            '<nil>': 0.206, 'Apoptosis -- CellCycleArrest': 0.426, 'CellCycleArrest': 0.068, 
            'Migration -- Metastasis -- Invasion -- CellCycleArrest': 0.3
        }

        expected_nodes = {
            'Metastasis': 0.3, 'Invasion': 0.3, 'Migration': 0.3, 'Apoptosis': 0.426, 'CellCycleArrest': 0.794
        }

        sim = cmaboss.MaBoSSSim(network="../tests/metastasis/metastasis.bnd", config="../tests/metastasis/metastasis.cfg")
        res = sim.run(only_last_state=False)
        raw_res, _, states = res.get_last_probtraj()
        for i, value in enumerate(np.nditer(raw_res)):
            self.assertAlmostEqual(value, expected[states[i]])
        
        raw_nodes_res, _, nodes = res.get_last_nodes_probtraj()
        for i, value in enumerate(np.nditer(raw_nodes_res)):
            self.assertAlmostEqual(value, expected_nodes[nodes[i]])
        
        simfinal = cmaboss.MaBoSSSim(network="../tests/metastasis/metastasis.bnd", config="../tests/metastasis/metastasis.cfg")
        resfinal = simfinal.run(only_last_state=True)
    
        raw_res, _, states = res.get_last_probtraj()
        for i, value in enumerate(np.nditer(raw_res)):
            self.assertAlmostEqual(value, expected[states[i]])
        
        raw_nodes_res, _, nodes = res.get_last_nodes_probtraj()
        for i, value in enumerate(np.nditer(raw_nodes_res)):
            self.assertAlmostEqual(value, expected_nodes[nodes[i]])
           
    def test_load_model_str(self):
        with open("../tests/metastasis/metastasis.bnd", "r") as bnd, open("../tests/metastasis/metastasis.cfg", "r") as cfg:    
            sim = cmaboss.MaBoSSSim(network_str=bnd.read(),config_str=cfg.read())
            res = sim.run()
            res.get_probtraj()
            res.get_last_probtraj()
            res.get_nodes_probtraj()
            res.get_last_nodes_probtraj()
            res.get_fp_table()

    def test_load_model_str_error(self):
        with open("../tests/metastasis/metastasis-error.bnd", "r") as bnd, open("../tests/metastasis/metastasis.cfg", "r") as cfg:    
            with self.assertRaises(cmaboss.BNException):
                cmaboss.MaBoSSSim(network_str=bnd.read(),config_str=cfg.read())

    def test_load_sbml(self):
        sim = cmaboss.MaBoSSSim("../tests/sbml/cell_fate.sbml", "../tests/sbml/cell_fate.cfg")
        res = sim.run()

    def test_load_sbml_error(self):
        with self.assertRaises(cmaboss.BNException):
            sim = cmaboss.MaBoSSSim("../tests/sbml/BIOMD0000000562_url.xml")
            
    def test_use_sbml_names(self):
        sim = cmaboss.MaBoSSSim("../tests/sbml/Cohen.sbml")
        nodes = ",".join(sorted([s.split(" ")[1] for s in sim.str_bnd().split("\n") if s.lower().startswith("node")]))
        self.assertEqual(nodes, "S_1,S_10,S_11,S_12,S_13,S_14,S_15,S_16,S_17,S_18,S_19,S_2,S_20,S_21,S_22,S_23,S_24,S_25,S_26,S_27,S_28,S_29,S_3,S_30,S_31,S_32,S_4,S_5,S_6,S_7,S_8,S_9")
        
        sim = cmaboss.MaBoSSSim("../tests/sbml/Cohen.sbml", use_sbml_names=True)        
        nodes = ",".join(sorted([s.split(" ")[1] for s in sim.str_bnd().split("\n") if s.lower().startswith("node")]))
        self.assertEqual(nodes, "AKT1,AKT2,Apoptosis,CDH1,CDH2,CTNNB1,CellCycleArrest,DKK1,DNAdamage,ECM,EMT,ERK,GF,Invasion,Metastasis,Migration,NICD,SMAD,SNAI1,SNAI2,TGFbeta,TWIST1,VIM,ZEB1,ZEB2,miR200,miR203,miR34,p21,p53,p63,p73")

def test_popmaboss(self):
        sim = cmaboss.PopMaBoSSSim("../tests/popmaboss/Fork.bnd", "../tests/popmaboss/Fork.cfg")
        res = sim.run()

        table = pd.DataFrame(*res.get_probtraj())
        table_expected = pd.DataFrame(
            np.array(
                [[1.66255134e-01, 3.40868015e-02, 3.38551043e-02, 8.07009188e-02,
                8.09583182e-02, 1.18692488e-01, 1.18842921e-01, 1.02495633e-01,
                1.02368440e-01, 1.61744241e-01],
                [4.09896097e-04, 1.04891000e-01, 1.04489977e-01, 3.73902360e-02,
                3.75903000e-02, 6.12595657e-03, 6.10061619e-03, 3.14471487e-01,
                3.13558480e-01, 7.49720510e-02],
                [0.00000000e+00, 1.22262280e-01, 1.21886609e-01, 5.70676378e-03,
                5.81079215e-03, 1.22937771e-04, 1.19760919e-04, 3.66654370e-01,
                3.65839924e-01, 1.15965617e-02],
                [0.00000000e+00, 1.24778404e-01, 1.24344195e-01, 7.40079145e-04,
                8.37198592e-04, 4.63800071e-06, 2.43825933e-06, 3.74330132e-01,
                3.73385687e-01, 1.57722799e-03],
                [0.00000000e+00, 1.25099041e-01, 1.24732671e-01, 7.96508015e-05,
                1.32926015e-04, 0.00000000e+00, 0.00000000e+00, 3.75337963e-01,
                3.74386572e-01, 2.31175526e-04],
                [0.00000000e+00, 1.25146088e-01, 1.24776072e-01, 3.91228414e-06,
                3.25290659e-05, 0.00000000e+00, 0.00000000e+00, 3.75510193e-01,
                3.74499678e-01, 3.15281934e-05],
                [0.00000000e+00, 1.25150000e-01, 1.24782886e-01, 0.00000000e+00,
                1.71140772e-05, 0.00000000e+00, 0.00000000e+00, 3.75532358e-01,
                3.74509794e-01, 7.84793118e-06],
                [0.00000000e+00, 1.25150000e-01, 1.24790000e-01, 0.00000000e+00,
                8.54296115e-07, 0.00000000e+00, 0.00000000e+00, 3.75549146e-01,
                3.74510000e-01, 0.00000000e+00],
                [0.00000000e+00, 1.25150000e-01, 1.24790000e-01, 0.00000000e+00,
                0.00000000e+00, 0.00000000e+00, 0.00000000e+00, 3.75550000e-01,
                3.74510000e-01, 0.00000000e+00]]
                , dtype=np.float64
            ),
            np.array([0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0]),
            np.array([
                '[{A:3}]', '[{A -- C:3}]', '[{A -- B:3}]', '[{A:1},{A -- C:2}]', '[{A:1},{A -- B:2}]', '[{A:2},{A -- C:1}]', 
                '[{A:2},{A -- B:1}]', '[{A -- C:1},{A -- B:2}]', '[{A -- C:2},{A -- B:1}]', '[{A:1},{A -- C:1},{A -- B:1}]'
            ])
            
        )
        
        if platform != "darwin":
            pd.testing.assert_frame_equal(table.sort_index(axis=1), table_expected.sort_index(axis=1), check_exact=False)
