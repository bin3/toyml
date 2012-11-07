#!/usr/bin/env python

from optparse import OptionParser
import string

def run(opts, args):
    header = ''
    with open(opts.header, 'r') as inf:
        header = inf.read()
    footer = "<div id='log'></div></html>"

    body = '<body>\n<h2>Topic-Word</h2>\n'
    with open(opts.inf, 'r') as inf:
        body += "<table id='topics' border='1'>\n"
        wordtr = ''
        probtr = ''
        tid = 0
        for line in inf:
            if line.startswith('Topic'):
                tid += 1
                if tid > 1:
                    wordtr += "</tr>\n"
                    probtr += "</tr>\n"
                    body += wordtr + probtr
                wordtr = "<tr class='word'><td>topic#" + str(tid) + "</td>"
                probtr = "<tr class='prob'><td>topic#" + str(tid) + "</td>"
            else:
                toks = line.strip().split()
                word = toks[0]
                prob = toks[1]
                wordtr += "<td>" + word + "</td>"
                probtr += "<td>" + prob + "</td>"
        wordtr += "</tr>\n"
        probtr += "</tr>\n"
        body += wordtr + probtr
        body += "</table>\n"
    body += '</body>\n'

    html = header + "\n" + body + "\n" + footer
    with open(opts.outf, 'w') as outf:
        outf.write(html)

if __name__ == '__main__':
    parser = OptionParser()
    parser.add_option('-i', '--inf', default='../plsa/topics.dat.final', help='the input topics file')
    parser.add_option('-o', '--outf', default='tm.html', help='the output topics html file')
    parser.add_option('-b', '--header', default='header.html', help='the intput header html file')
    (opts, args) = parser.parse_args()
    print 'options: ', opts
    print 'args: ', args
    run(opts, args)
