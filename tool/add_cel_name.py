#!/usr/bin/env python

from optparse import OptionParser
import string

def run(opts, args):
    id2name = {}
    with open(opts.id_name_path, 'r') as inf:
        for line in inf:
            toks = line.split()
            id = toks[0]
            name = toks[1]
            id2name[id] = name

    with open(opts.inf, 'r') as inf, open(opts.outf, 'w') as outf:
        work_flag = False
        for line in inf:
            if line.find('celebrities:') > 0:
                work_flag = True
            elif line.startswith('Topic #'):
                work_flag = False
            elif work_flag == True:
                toks = line.split()
                id = toks[0]
                name = id2name[id]
                line = '\t' + name + line 
            outf.write(line)

if __name__ == '__main__':
    parser = OptionParser()
    parser.add_option('-c', '--id_name_path', default='cel_id_name.txt', help='the input id-name file')
    parser.add_option('-i', '--inf', default='topics.dat.final', help='the input topics file')
    parser.add_option('-o', '--outf', default='topics.dat', help='the output topics file with names celebrities')
    (opts, args) = parser.parse_args()
    print 'options: ', opts
    print 'args: ', args
    run(opts, args)
