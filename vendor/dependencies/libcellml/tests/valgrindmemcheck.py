#!/usr/bin/env python
# run valgrind's memory error checker on all tests.
# filter uninteresting errors and known false positives
#

#
# This file is based on kdevplatform/veritas/tests/runMemcheck.py
# by the KDevelop project, www.kdevelop.org
#
from multiprocessing import Pool, cpu_count
from os import system, remove, environ, defpath, path, pathsep, X_OK, access, sep
from sys import exit, stdout
from subprocess import Popen, PIPE
from xml.dom.minidom import parse, parseString


def garbage(line):
    ''' filter for valgridn output'''
    return not line.startswith('<unknown program name>') and \
           not line.startswith('profiling:')


def memcheck(test):
    ''' run valgrind-memcheck on test in testdir. return xml output as string '''
    test_name = path.basename(test)
    v = find_valgrind()
    cmd = v+" --tool=memcheck --child-silent-after-fork=yes --leak-check=full --xml=yes --xml-fd=3 --num-callers=50 " + test + " 1>{0}.txt 2>{0}.err 3>{0}.xml".format(test_name)
    #print cmd
    system(cmd)
    out = open("{0}.xml".format(test_name)).readlines()
    #remove("/tmp/.memcheck.tmp")
    out = list(filter(garbage, out))
    return ''.join(out)


def xml_child_data(dom,tag):
    ''' extract child data for tag. return None if not found'''
    elem = dom.getElementsByTagName(tag)
    val = None
    if len(elem) != 0:
        val = elem[0].firstChild.data
    return val


class Frame:
    ''' single entry in a memory error backtrace '''
    def __init__(self, dom_frame):
        '''<frame>
        <ip>0x62ACDBF</ip>
        <obj>/home/nix/KdeDev/kde4/lib/libkdevplatformlanguage.so.1.0.0</obj>
        <fn>KDevelop::ParamIterator::ParamIterator(QString, QString, int)</fn>
        <dir>/home/nix/KdeDev/kdevplatform/language/duchain</dir>
        <file>stringhelpers.cpp</file>
        <line>292</line>
        </frame>'''
        self.obj   = xml_child_data(dom_frame, 'obj')
        self.func  = xml_child_data(dom_frame, 'fn')
        self.sfile = xml_child_data(dom_frame, 'file')
        self.sline = xml_child_data(dom_frame, 'line')

    def __str__(self):
        out = ""
        if self.func:
            out += "\t" + self.func
        if self.sfile and self.sline:
            out += " (" + self.sfile + ":" + self.sline + ")"
        #if self.obj:
            #out += "\t" + self.obj + "\n"
        out += "\n"
        return out


class BackTrace:
    ''' valgrind memcheck stack trace '''
    def __init__(self, errordom):
        self.dom = errordom
        self.kind = self.dom.getElementsByTagName('kind')[0].firstChild.data
        stack = self.dom.getElementsByTagName('frame')
        self.stack = []
        for frame in stack:
            if xml_child_data(frame, 'fn'): # filter anonymous frames out
                self.stack.append(Frame(frame))
        self.what = xml_child_data(self.dom, 'what')
        if self.what is None:
            self.what = 'None'

    def is_definitely_lost(self):
        return self.kind == 'Leak_DefinitelyLost'

    def is_qtest(self):
        is_interesting = False
        for frame in self.stack:
            if frame.func:
                if "libcellml" in frame.func:
                    is_interesting = True
                elif "::TestBody" in frame.func:
                    is_interesting = True
            if frame.sfile:
                if frame.sfile.find("-test.cpp") != -1:
                    is_interesting = True
        return is_interesting

    def __str__(self):
        out = self.what + "\n"
        for frame in self.stack:
            out += str(frame)
        return out


def parse_errors(out):
    ''' extract the interesting memcheck errors from the xml-string input 'out'.
    return these as a list '''
    xmldoc = parseString(out)
    errors = xmldoc.getElementsByTagName('error')
    errors_ = []
    for error in errors:
        bt = BackTrace(error)
        if bt.is_definitely_lost() and bt.is_qtest():
            errors_.append(bt)
    return errors_


#from: http://mail.python.org/pipermail/python-list/2002-August/157829.html
def which (filename):
    if 'PATH' not in environ or environ['PATH'] == '':
        p = defpath
    else:
        p = environ['PATH']

    pathlist = p.split (pathsep)

    for thepath in pathlist:
        f = thepath+sep+filename
        if access(f, X_OK):
            return f
    return None


def find_valgrind():
    valgrind = which('valgrind')
    if valgrind != None:
        return valgrind
    else:
        print("valgrind NOT FOUND")
        exit(3)


def run_single_test(exe_name):
    success = False
    if not access(exe_name, X_OK):
        print("executable "+exe_name+" NOT FOUND")
        return success

    sys.stdout.write(">> running valgrind memcheck on: '{0}'".format(path.basename(exe_name)))
    out = memcheck(exe_name)
    errors = parse_errors(out)
    if len(errors) == 0:
        success = True
        sys.stdout.write(" - PASS\n")
    else:
        sys.stdout.write(" - FAIL\n")
        for trace in errors:
            sys.stderr.write(str(trace))
            sys.stderr.write("---------------------------------------------------\n")

    return success

################### ENTRY ####################################################

if __name__ == '__main__':
    import sys
    if len(sys.argv) > 2:
        exit_code = 0
        test_dir = sys.argv[1]
        single_tests = sys.argv[2:]
        successes = []
        failures = []

        with Pool(cpu_count()) as p:
            results = p.map(run_single_test, [path.join(test_dir, single_test) for single_test in single_tests])

        for index, result in enumerate(results):
            if result:
                successes.append(single_tests[index])
            else:
                failures.append(single_tests[index])
                exit_code = 2

        sys.stdout.write(">> Summary\n")
        tot = len(successes) + len(failures)
        per = 100.0*len(successes) / tot
        sys.stdout.write("   {0}% tests passed, {1} tests failed out of {2}\n".format(per, len(failures), tot))
        sys.stdout.write("\n")
        if len(failures):
            sys.stdout.write("   The failed tests were:\n")
            for f in failures:
                sys.stdout.write("    - {0}\n".format(f))

            sys.stdout.write("\n")
        exit(exit_code)
    else:
        print("usage: ./valgrindmemcheck.py test_exectable_dir test_executable_1 [test_exectuable_2 ...]")
        exit(1)

