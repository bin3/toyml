#!/usr/bin/env python

from optparse import OptionParser
import string
import math

def run(opts, args):
    with open(opts.inf, 'r') as inf, open(opts.outf, 'w') as outf:
        first = True
        for line in inf:
            tokens = line.split()
            if first == True:
                first = False
                rows = int(tokens[0])
                cols = int(tokens[1])
                print ('rows=%d, cols=%d' % (rows, cols))
                continue
            entropy = 0
            for token in tokens:
                prob = float(token)
                if prob > 0:
                    entropy += - prob * math.log(prob, 2)
            outf.write('%f\n' % (entropy))

if __name__ == '__main__':
    parser = OptionParser()
    parser.add_option('-i', '--inf', default='topic-cel-prob.dat.final', help='the input probability file')
    parser.add_option('-o', '--outf', default='topic-cel-entropy.dat', help='the output entropy file')
    (opts, args) = parser.parse_args()
    print 'options: ', opts
    print 'args: ', args
    run(opts, args)
