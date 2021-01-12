#!/usr/bin/env python

import random
#from math import log, exp, sqrt, pi

def test_seed_state():
    """test seed() and getstate()/setstate()
    """
    # random ought to be able to deal with seeds in any form, of follows.
    # following code shouldn't cause an exception.
    random.seed()
    random.seed(0)
    random.seed(-1)
    random.seed(0.1)
    random.seed(-0.1)
    random.seed("a")
    random.seed("abc")
    random.seed("abcd")
    random.seed("fasdfasdfasdfadgaldhgldahlgahdlghadlgladh")
    random.seed("lxhlh90yowhldshlgah;")
    
    # state1 and state2 should be different for different seeds
    random.seed(1)
    state1 = random.getstate()
    random.seed(2)
    state2 = random.getstate()
    rep = 0
    for ind in range(len(state1)):
        elem1 = state1[ind]
        elem2 = state2[ind]
        if (elem1 == elem2): rep += 1
    if (rep > len(state1) / 2):
        print("rep = ", rep, "len(state1) = ", len(state1))
        raise "state1 and state2 should be different"
    
    # for the same seeds, state1 and state2 should be the same
    random.seed(100)
    state1 = random.getstate()
    random.seed(100)
    state2 = random.getstate()
    rep = 0
    for ind in range(len(state1)):
        elem1 = state1[ind]
        elem2 = state2[ind]
        if (elem1 == elem2): rep += 1
    if (rep != len(state1)):
        raise "state1 and state2 should be the same"

def test_jumpahead():
    """jumpahead will change the pseudo-number generator's internal state
    """
    random.seed()
    state1 = random.getstate()
    random.jumpahead(20)
    state2 = random.getstate()
    rep = 0
    for ind in range(len(state1)):
        elem1 = state1[ind]
        elem2 = state2[ind]
        if (elem1 == elem2): rep += 1
    if (rep > len(state1) / 2):
        raise "state1 and state2 can't be the same"
        
def test_setstate():
    """
    """
    random.seed()
    oldState = random.getstate()
    oldRandSeq = [random.random() for i in range(10)]
    random.setstate(oldState)
    newRandSeq = [random.random() for i in range(10)]
    rep = 0
    for ind in range(len(oldRandSeq)):
        elem1 = oldRandSeq[ind]
        elem2 = newRandSeq[ind]
        if (elem1 == elem2): rep += 1
    if (rep != len(oldRandSeq)):
        raise "oldRandSeq and newRandSeq should be the same"

def test_random():
    """generate a random number list
    """
    x = [random.random() for i in range(100)]
    
def test_distribution():
    """these lines are borrowed from python, they shouldn't
        cause any exception.
    """
    g = random
    g.uniform(1,10)
    g.paretovariate(1.0)
    g.expovariate(1.0)
    g.weibullvariate(1.0, 1.0)
    g.normalvariate(0.0, 1.0)
    g.lognormvariate(0.0, 1.0)
    g.vonmisesvariate(0.0, 1.0)
    g.gammavariate(0.01, 1.0)
    g.gammavariate(1.0, 1.0)
    g.gammavariate(200.0, 1.0)
    g.betavariate(3.0, 3.0)

def test_randrange():
    """these input to randrange() shouldn't cause any exception.
    """
    random.randrange(100000)
    random.randrange(-100000)
    random.randrange(0)
    random.randrange(-10.2)
    
    random.randrange(-10, 10)
    random.randrange(2, 1000)
    random.randrange(0, 1)
    random.randrange(-1, 0)
    
    random.randrange(10, 2000, 2)
    random.randrange(-2000, 100, 5)
    random.randrange(-1000.3, 1000.7, 2)

def test_randint():
    """for any valid pair (a, b), randint(a, b) should lay between [a, b]
    """
    for i in range(1000):
        r = random.randint(-10000, 10000)
        if (-10000 <= r <= 10000): continue
        else: raise "error: random.randint()"

def test_choice():
    """random.choice() should be able to deal with string, list.
    """
    S = "abcdefg123*@#$%)("
    L = [1, 2, 3, -1, 0.2, -0.1, -10000, "cyc"]
    
    if random.choice(S) not in S:
        raise "error: random.choice(S)"
    
    if random.choice(L) not in L:
        raise "error: random.choice(L)"

def test_shuffle():
    """test random.shuffle() on list. since string is not writable in-place,
        random.shuffle() can not be applied on string.
        Note: to copy items from a list to a new list, must use syntax like:
            newList = oldList[:]
        if use syntax like: newList = oldList, newList is just an alias of oldList.
    """
    oldL = [1, 2, 3, -1, 0.2, -0.1, -10000, "cyc"]
    newL = oldL[:]
    
    random.shuffle(newL)
    
    rep = 0
    for ind in range(len(oldL)):
        elem1 = oldL[ind]
        elem2 = newL[ind]
        if (elem1 == elem2): rep += 1
    if (rep > len(oldL) / 2):
        raise "oldL and newL shouldn't be the same"
        
def test_53_bits_per_float():
    pass
        
def main():
    test_seed_state()
    test_jumpahead()
    test_setstate()
    test_random()
    test_distribution()
    test_randrange()
    test_randint()
    test_choice()
    test_shuffle()
    test_53_bits_per_float()
    print("#OK")

if __name__ == '__main__':
    main()
