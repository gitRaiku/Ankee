#!/bin/python

from lxml import etree
import socket
import os

'''
spath = '/tmp/ankeed.sock'
if os.path.exists(spath):
    os.remove(spath)'''

tree = etree.parse('JMdict_e.xml')
print('Done reading!')

# for x in tree.findall(f".//entry/r_ele[reb='にんげん']"):
es = []

for x in tree.findall(f"//entry/k_ele[keb='人間']"): # TODO: Xpath 'or' what the fuck
    par = x.getparent()
    if par not in es:
        es.append(par)

for x in tree.findall(f"//entry/r_ele[reb='ひどい']"): # TODO: Xpath 'or' what the fuck
    par = x.getparent()
    if par not in es:
        es.append(par)

for s in es:
    # print(s.find('pos').text)
    st = ''
    entry = {'t': '', 'r': '', 'm': '', 'add': ''}
    for y in s.findall('.//'):# gloss'):
        print(f'{y.text} ', end='')
        st += f'{y.text}; '
    print('')
    # st = st[:-2]
    # print(st)

'''
tree = etree.parse('JMdict_e.xml')
print('Done reading!')

for x in tree.findall(".//entry/k_ele[keb='人間']"):
    par = x.getparent()
    senses = par.findall("sense")
    for s in senses:
        print(s.find('pos').text)
        st = ''
        for y in s.findall('gloss'):
            st += f'{y.text}; '
        st = st[:-2]
        print(st)
'''


'''server = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
server.bind(spath)
server.listen()
while True:
    (cs, addr) = server.accept()
    print(f'Got connection fron {addr}')
    l = cs.recv(2)
    l = l[0] << 8 | l[1]
    text = cs.recv(l).decode("utf-8")
    print(f'Got {text}')


    cs.close()'''
