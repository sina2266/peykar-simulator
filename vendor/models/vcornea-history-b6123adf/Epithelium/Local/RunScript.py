from os.path import dirname, join, expanduser
from cc3d.CompuCellSetup.CC3DCaller import CC3DCaller
from cc3d.CompuCellSetup.CC3DPy import CC3DPySim

def main():

    number_of_runs = 1

    # You may put a direct path to a simulation of your choice here and comment out simulation_fname line below
    # simulation_fname = <direct path your simulation>"C:\Users\joelv\Documents\GitHub\vCornea\Epithelium\Local\Project\paper_version\vCornea_v2.cc3d"
    simulation_fname = "/u/jvanin/T_vCornea_paper/Epithelium/Local/Project/paper_version_STEM_ONLY/vCornea_v2.cc3d"
    root_output_folder = join(expanduser('~'), 'CC3DCallerOutput')
    print('root_output_folder=', root_output_folder)

    # this creates a list of simulation file names where simulation_fname is repeated number_of_runs times
    # you can create a list of different simulations if you want.
    sim_fnames = [simulation_fname] * number_of_runs

    ret_values = []
    for i, sim_fname in enumerate(sim_fnames):
        cc3d_caller = CC3DCaller(
            cc3d_sim_fname=sim_fname,
            screenshot_output_frequency=10,
            output_dir=join(root_output_folder,f'vCornea_{i}'),
            sim_input=i,
            result_identifier_tag=i
        )

        ret_value = cc3d_caller.run()
        ret_values.append(ret_value)

    print('return values', ret_values)


if __name__ == '__main__':
    main()