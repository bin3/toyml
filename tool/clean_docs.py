#!/usr/bin/env python

from optparse import OptionParser
import string

def load_stopwordset(fname):
    st = set()
    with open(fname, 'r') as inf:
        for line in inf:
            words = line.split()
            for w in words:
                st.add(w)
    return st

def isvalid(token):
    for c in token:
        if c not in string.ascii_lowercase:
            return False
    return True

def clean(opts, args):
    stopwordset = load_stopwordset(opts.stop)
    print 'stopwords=', len(stopwordset)
    cfile = opts.file + '.' + opts.suffix
    cnt = 0
    with open(opts.file, 'r') as inf, open(cfile, 'w') as outf:
        for i, line in enumerate(inf):
            if i % opts.nlog == 0:
                print 'line#', i
            line = line.lower()
            tokens = line.split()
            ctokens = []
            for token in tokens:
                if not isvalid(token[:-1]):
                    continue
                if token[-1] not in string.ascii_lowercase:
                    token = token[:-1]
                if len(token) <= 1:
                    continue
                if token in stopwordset:
                    continue
                ctokens.append(token)
            if len(ctokens) == 0:
                continue
            outf.write(' '.join(ctokens))
            outf.write('\n')
            cnt += len(ctokens)
    print 'all_tokens=', cnt

if __name__ == '__main__':
    parser = OptionParser()
    parser.add_option('-f', '--file', default='tweet_docs.dat', help='the input doc file')
    parser.add_option('-s', '--suffix', default='clean', help='the suffic of output cleaned file')
    parser.add_option('-t', '--stop', default='stopwords.dat', help='the stop words file')
    parser.add_option('-n', '--nlog', default=10000, help='log interval')
    (opts, args) = parser.parse_args()
    print 'options: ', opts
    print 'args: ', args
    clean(opts, args)
