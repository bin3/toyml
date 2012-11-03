#!/usr/bin/env python

from optparse import OptionParser
import string

def run(opts, args):
    header = ''
    with open(opts.header, 'r') as inf:
        header = inf.read()
    footer = "<div id='log'></div></html>"

    body = '<body>\n'
    with open(opts.inf, 'r') as inf:
        wtab = "<table id='topic-word' border='1'>\n"
        ctab = "<table id='topic-cel' border='1'>\n"
        wflag = False
        cflag = False
        wordtr = ''
        probtr = ''
        tid = 0
        for line in inf:
            if line.startswith('Topic'):
                tid += 1
            elif line.find('words:') > 0:
                wflag = True
                cflag = False
                if tid > 1:
                    wordtr += "</tr>\n"
                    probtr += "</tr>\n"
                    ctab += wordtr + probtr
                wordtr = "<tr class='word'><td>topic#" + str(tid) + "</td>"
                probtr = "<tr class='prob'><td>topic#" + str(tid) + "</td>"
            elif line.find('celebrities:') > 0:
                cflag = True
                wflag = False
                if tid > 0:
                    wordtr += "</tr>\n"
                    probtr += "</tr>\n"
                    wtab += wordtr + probtr
                wordtr = "<tr class='word'><td>topic#" + str(tid) + "</td>"
                probtr = "<tr class='prob'><td>topic#" + str(tid) + "</td>"
            elif wflag:
                toks = line.strip().split()
                word = toks[0]
                prob = toks[1]
                wordtr += "<td>" + word + "</td>"
                probtr += "<td>" + prob + "</td>"
            elif cflag:
                toks = line.strip().split()
                word = toks[0]
                prob = toks[2]
                wordtr += "<td>" + word + "</td>"
                probtr += "<td>" + prob + "</td>"
        wordtr += "</tr>\n"
        probtr += "</tr>\n"
        ctab += wordtr + probtr
        wtab += "</table>\n"
        ctab += "</table>\n"
    body = '<body>\n' + '<h2>Topic-Word</h2>'  + wtab + '<h2>Topic-Celebrity</h2>' + ctab +  '</body>\n'

    html = header + "\n" + body + "\n" + footer
    with open(opts.outf, 'w') as outf:
        outf.write(html)

if __name__ == '__main__':
    parser = OptionParser()
    parser.add_option('-i', '--inf', default='../explsa/topics.dat', help='the input topics file')
    parser.add_option('-o', '--outf', default='tm.html', help='the output topics html file')
    parser.add_option('-b', '--header', default='header.html', help='the intput header html file')
    (opts, args) = parser.parse_args()
    print 'options: ', opts
    print 'args: ', args
    run(opts, args)
