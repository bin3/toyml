#!/usr/bin/env python

from optparse import OptionParser
import string

def run(opts, args):
    with open(opts.inf, 'r') as inf, open(opts.outf, 'w') as outf:
        for line in inf:
            cols = line.split()
            outf.write('%d\n' % (len(cols)))

if __name__ == '__main__':
    parser = OptionParser()
    parser.add_option('-i', '--inf', default='followee.dat', help='the input followee file')
    parser.add_option('-o', '--outf', default='num_followee.dat', help='the output num_followee file')
    (opts, args) = parser.parse_args()
    print 'options: ', opts
    print 'args: ', args
    run(opts, args)
