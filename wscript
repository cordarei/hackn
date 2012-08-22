#!/usr/bin/env python
# encoding: utf-8


from waflib import Logs

VERSION='0.0.1'
APPNAME='hackn'

top = '.'
out = 'build'

def options(opt):
    opt.load('compiler_cxx')

def configure(conf):
    conf.load('compiler_cxx')

def build(bld):
    bld(
        features='cxx cxxprogram',
        cxxflags='-std=c++0x',
        source='src/main.cc',
        includes='src',
        target='hackn',
        lib='ncurses'
    )
