from parse import *

def bin2pref(str):
    return parse('{BRANCH}-{L1I_PREFETCHER}-{L1D_PREFETCHER}-{L2C_PREFETCHER}-{LLC_PREFETCHER}-{LLC_REPLACEMENT}-{NUM_CORE}core', str).named['L1I_PREFETCHER']


def stats2trace(str):
    return parse('{BINARY}.{TRACE}.{N_SIM}.{N_WARM}.stats', str).named['TRACE']
