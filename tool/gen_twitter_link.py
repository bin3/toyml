#!/usr/bin/env python

from optparse import OptionParser
import string

def run(opts, args):
    html = '<table border="1">\n'
    with open(opts.inf, 'r') as inf:
        for i, line in enumerate(inf):
            toks = line.split()
            id = toks[0]
            name = toks[1]
            link = 'https://twitter.com/' + name
            a = '<a href=' + link + ' target="_blank">' + name + '</a>'
            html += '<tr><td>' + str(i) + '</td><td>' + a + '</td></tr>\n'
    html += '</table>\n'
    with open(opts.outf, 'w') as outf:
        outf.write(html)

if __name__ == '__main__':
    parser = OptionParser()
    parser.add_option('-i', '--inf', default='cel_id_name.txt', help='the input id-name file')
    parser.add_option('-o', '--outf', default='cel_link.html', help='the output html file of twitter links of celebrities')
    (opts, args) = parser.parse_args()
    print 'options: ', opts
    print 'args: ', args
    run(opts, args)
