import numpy, os, sys

def get_raw_data(file):
    with open(file, 'r') as f:
        raw_lines = f.readlines()
        
        first_index = next(i for i, col in enumerate(raw_lines[0].strip('\n').split('\t')) if col == "State")
        raw_data = [line.strip('\n').split('\t') for line in raw_lines[1:]]
        
        raw_states = [[s for s in t_data[first_index::3]] for t_data in raw_data]
        raw_probas = [[s for s in t_data[first_index+1::3]] for t_data in raw_data]
        raw_errors = [[s for s in t_data[first_index+2::3]] for t_data in raw_data]
    
    return (raw_states, raw_probas, raw_errors)


def build_states(raw_states1, raw_states2):
    
    states = set()
    for t_states in raw_states1:
        states.update(t_states)
    for t_states in raw_states2:
        states.update(t_states)

    states = sorted(list(states))
    return states

    
def build_table(raw_states, raw_probas, states, states_indexes):
        
    new_data = numpy.zeros((len(raw_probas), len(states)))    
    for i, t_probas in enumerate(raw_probas):
        for j, proba in enumerate(t_probas):
            new_data[i, states_indexes[raw_states[i][j]]] = proba
        
    return new_data

    
def main(args):
    if len(args) >= 3 and os.path.exists(args[1]) and os.path.exists(args[2]) and args[1].endswith(".csv") and args[2].endswith(".csv"):
        raw_states1, raw_probas1, raw_errors1 = get_raw_data(args[1])
        raw_states2, raw_probas2, raw_errors2 = get_raw_data(args[2])

        states = build_states(raw_states1, raw_states2)
        states_indexes = {state:index for index, state in enumerate(states)}

        table1 = build_table(raw_states1, raw_probas1, states, states_indexes)
        table2 = build_table(raw_states2, raw_probas2, states, states_indexes)    
            
        if len(args) > 3 and args[3] == "--exact":
            sys.exit(0 if numpy.all(table1 == table2) else 1)
            
        elif len(args) > 3 and args[3] == "--auto":
            
            table_error1 = build_table(raw_states1, raw_errors1, states, states_indexes)
            table_error2 = build_table(raw_states2, raw_errors2, states, states_indexes)
            
            atol = numpy.max(numpy.max(table_error1)) + numpy.max(numpy.max(table_error2))
            
            table_rtol1 = numpy.divide(table_error1, numpy.clip(table1, 1e-16, 1.0))
            table_rtol2 = numpy.divide(table_error2, numpy.clip(table2, 1e-16, 1.0))
            
            rtol = numpy.max(numpy.max(table_rtol1)) + numpy.max(numpy.max(table_rtol2))
            
            sys.exit(0 if numpy.all(numpy.isclose(table1, table2, rtol=rtol, atol=atol)) else 1)

        else:
            rtol = 1e-4
            atol = 1e-8
            if len(args) > 3:
                rtol = float(args[3])
            if len(args) > 4:
                atol = float(args[4])
            
            sys.exit(0 if numpy.all(numpy.isclose(table1, table2, rtol=rtol, atol=atol)) else 1)
    else:
        print("Wrong argument number, or files do not exist")    
        sys.exit(1)
    
if __name__ == '__main__':
    main(sys.argv)