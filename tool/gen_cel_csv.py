#!/usr/bin/env python

from optparse import OptionParser
import string

def run(opts, args):
    with open(opts.inf, 'r') as inf, open(opts.outf, 'w') as outf:
        for line in inf:
            cols = line.split()
            cols.append('')
            outf.write(';'.join(cols) + '\n')

if __name__ == '__main__':
    parser = OptionParser()
    parser.add_option('-i', '--inf', default='cel_id_name.txt', help='the input id-name file')
    parser.add_option('-o', '--outf', default='cel_id_name.csv', help='the output csv file')
    (opts, args) = parser.parse_args()
    print 'options: ', opts
    print 'args: ', args
    run(opts, args)
