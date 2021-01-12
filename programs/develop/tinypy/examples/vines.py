#!/bin/env python
#
# vines borrowed from xscreensaver
#
"""
/*-
 * Copyright (c) 1997 by Tracy Camp campt@hurrah.com
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation.
 *
 * This file is provided AS IS with no warranties of any kind.  The author
 * shall have no liability with respect to the infringement of copyrights,
 * trade secrets or any patents by this file or any part thereof.  In no
 * event will the author be liable for any lost revenue or profits or
 * other special, indirect and consequential damages.
 *
 * If you make a modification I would of course appreciate a copy.
 *
 * Revision History:
 * 01-Nov-2000: Allocation checks
 * 11-Jul-1997: David Hansen <dhansen@metapath.com>
 *              Changed names to vines and modified draw loop
 *              to honor batchcount so vines can be grown or plotted.
 * 10-May-1997: Compatible with xscreensaver
 * 21-Mar-1997: David Hansen <dhansen@metapath.com>
 *              Updated mode to draw complete patterns on every
 *              iteration instead of growing the vine.  Also made
 *              adjustments to randomization and changed variable
 *              names to make logic easier to follow.
 */

/*-
 * This was modifed from a 'screen saver' that a friend and I
 * wrote on our TI-8x calculators in high school physics one day
 * Basically another geometric pattern generator, this ones claim
 * to fame is a pseudo-fractal looking vine like pattern that creates
 * nifty whorls and loops.
 */
"""

import sys
import math
import random
import pygame
if "tinypy" not in sys.version:         # not tinypy
    import pygame.locals

SCR_WIDTH   = 800
SCR_HEIGHT  = 600

class VineStruct(object):
    a   = 0
    x1  = 0
    y1  = 0
    x2  = 0
    y2  = 0
    i   = 0
    length      = 0
    iterations  = 0
    constant    = 0
    ang         = 0
    centerx     = 0
    centery     = 0

class Vines(object):
    def __init__(self):
        self.fp             = VineStruct()
        self.fp.i           = 0
        self.fp.length      = 0
        self.fp.iterations  = 30 + random.randint(0, 100)
        
        pygame.init()
        self.screen         = pygame.display.set_mode((SCR_WIDTH, SCR_HEIGHT))
        
    def __drawLine__(self, x1, y1, x2, y2, color):
        
        # validate the bounds
        if x1 < 0: x1 = 0
        if x1 > SCR_WIDTH: x1 = SCR_WIDTH
        if x2 < 0: x2 = 0
        if x2 > SCR_WIDTH: x2 = SCR_WIDTH
        if y1 < 0: y1 = 0
        if y1 > SCR_HEIGHT: y1 = SCR_HEIGHT
        if y2 < 0: y2 = 0
        if y2 > SCR_HEIGHT: y2 = SCR_HEIGHT
        
        if x1 <= x2:
            sx, sy = x1, y1
            dx, dy = x2, y2
        else:
            sx, sy = x2, y2
            dx, dy = x1, y1
        
        if (abs(x1 - x2) < 1e-4):
            x = sx
            if sy > dy:
                sy, dy = dy, sy
            y = sy
            while (y < dy):
                self.screen.set_at((x, y), color)
                y += 1
        else:
            k = (dy - sy) / (dx - sx)
            x = sx
            while (x < dx):
                y = sy + k * (x - sx)
                self.screen.set_at((x, y), color)
                x += 1
        
        pygame.display.flip()
        
    def draw(self):
        red     = random.randint(0, 255)
        green   = random.randint(0, 255)
        blue    = random.randint(0, 255)
        if (self.fp.i >= self.fp.length):
            self.fp.iterations -= 1
            if (self.fp.iterations == 0):
                self.__init__(self)
            self.fp.centerx = random.randint(0, SCR_WIDTH);
            self.fp.centery = random.randint(0, SCR_HEIGHT);
            
            self.fp.ang     = 60 + random.randint(0, 720);
            self.fp.length  = 100 + random.randint(0, 3000);
            self.fp.constant= self.fp.length * (10 + random.randint(0, 10))
            
            self.fp.i       = 0;
            self.fp.a       = 0;
            self.fp.x1      = 0;
            self.fp.y1      = 0;
            self.fp.x2      = 1;
            self.fp.y2      = 0;
        
        count = self.fp.i + random.randint(10, 100)
        if (count > self.fp.length):
            count = self.fp.length
        
        while (self.fp.i < count):
            x1 = self.fp.centerx + (self.fp.x1 / self.fp.constant)
            y1 = self.fp.centery - (self.fp.y1 / self.fp.constant)
            x2 = self.fp.centerx + (self.fp.x2 / self.fp.constant)
            y2 = self.fp.centery - (self.fp.y2 / self.fp.constant)
            
            color = (red, green, blue)
            self.__drawLine__(x1, y1, x2, y2, color)
            
            self.fp.a   += (self.fp.ang * self.fp.i)
            self.fp.x1  = self.fp.x2
            self.fp.y1  = self.fp.y2
            
            self.fp.x2  += int((self.fp.i * (math.cos(self.fp.a) * 360.0)) / (2.0 * math.pi))
            self.fp.y2  += int((self.fp.i * (math.sin(self.fp.a) * 360.0)) / (2.0 * math.pi))
            self.fp.i   += 1

def main():
    myVine = Vines()
    _quit = False
    while not _quit:
        for e in pygame.event.get():
            if e.type in (pygame.locals.QUIT,pygame.locals.KEYDOWN):
                _quit = True
        myVine.draw()

if __name__ == '__main__':
    main()
    print("#OK")