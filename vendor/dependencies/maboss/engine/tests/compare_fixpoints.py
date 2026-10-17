import numpy, os, sys

def get_raw_data(file):
    
    with open(file, 'r') as f:
    
        raw_lines = f.readlines()[2:]
        raw_data = [line.strip('\n').split("\t")[1:3] for line in raw_lines]
        proba_by_fp = {state:proba for proba,state in raw_data}
        
        return proba_by_fp

def main(args):
    if len(args) >= 3 and os.path.exists(args[1]) and os.path.exists(args[2]) and args[1].endswith(".csv") and args[2].endswith(".csv"):
        table1 = get_raw_data(args[1])
        table2 = get_raw_data(args[2])

        sys.exit(0 if table1 == table2 else 1)
    else:
        print("Wrong argument number, or files do not exist")    
        sys.exit(1)
    
if __name__ == '__main__':
    main(sys.argv)