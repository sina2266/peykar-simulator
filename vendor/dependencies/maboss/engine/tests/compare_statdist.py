import numpy, os, sys

def get_raw_data(file):
    with open(file, 'r') as f:
        raw_lines = f.readlines()[1:]
        i=0
        traj_states = []
        traj_probas = []
        all_states = set()

        while(raw_lines[i].startswith('#')):
            
            states = raw_lines[i].split("\t")[1:][::2]
            probas = raw_lines[i].split("\t")[1:][1::2]
            traj_states.append(states)
            traj_probas.append(probas)
            for state in states:
                all_states.add(state)
            i += 1

        states = sorted(list(all_states))
        states_indexes = {state:index for index, state in enumerate(states)}

        new_data = numpy.zeros((len(traj_probas), len(states)))

        for i, t_proba in enumerate(traj_probas):
            for j, proba in enumerate(t_proba):
                new_data[i, states_indexes[traj_states[i][j]]] = float(proba)

        return new_data
    
def main(args):
    if len(args) >= 3 and os.path.exists(args[1]) and os.path.exists(args[2]) and args[1].endswith(".csv") and args[2].endswith(".csv"):
        table1 = get_raw_data(args[1])
        table2 = get_raw_data(args[2])

        if len(args) > 3 and args[3] == "--exact":
            sys.exit(0 if numpy.all(table1 == table2) else 1)
        else:
            sys.exit(0 if numpy.all(numpy.isclose(table1, table2, rtol=1e-4, atol=1e-8)) else 1)
    else:
        print("Wrong argument number, or files do not exist")    
        sys.exit(1)
    
if __name__ == '__main__':
    main(sys.argv)