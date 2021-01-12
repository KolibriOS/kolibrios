"""
a simple diversing asteroids simulation
"""

import sys
import math
import random
import pygame
if "tinypy" not in sys.version:         # not tinypy
    import pygame.locals

SCR_WIDTH   = 600
SCR_HEIGHT  = 600
NASTEROIDS  = 320       # number of asteroids
INIT_NASTEROIDS = 80    # initial number of asteroids
OFFSET      = 20        # max initial offset
DIV_FACTOR  = 1.1       # diverse factor

class Center(object):
    x = SCR_WIDTH / 2
    y = SCR_HEIGHT / 2

class Graphics(object):
    x = 0
    y = 0
    w = SCR_WIDTH
    h = SCR_HEIGHT    
    center = Center()
    BACKGROUND = (0, 0, 0)  # black background
    
    def __init__(self):
        pygame.init()
        self.screen = pygame.display.set_mode((SCR_WIDTH, SCR_HEIGHT))
        self.clearScreen()
    def drawRect(self, sx, sy, w, h, color):
        sx = int(sx)
        sy = int(sy)
        dx = int(sx + w)
        dy = int(sy + h)
        for x in range(sx, dx):
            for y in range(sy, dy):
                self.screen.set_at((x, y), color)
    def clearScreen(self):
        for x in range(SCR_WIDTH):
            for y in range(SCR_HEIGHT):
                self.screen.set_at((x, y), self.BACKGROUND)
    def flipDisplay(self):
        pygame.display.flip()

class Asteroid(object):
    graphics = Graphics()
    def __init__(self):
        self.x          = 0    # x and y, set to invalid position
        self.y          = 0
        self.size       = 1
        self.color      = [0, 0, 0]
    def show(self):
        self.graphics.drawRect(self.x, self.y, self.size, self.size, self.color)
    def hide(self):
        self.graphics.drawRect(self.x, self.y, self.size, self.size, self.graphics.BACKGROUND)
    def update(self):
        # update asteroids[i]'s position
        if (self.x <= self.graphics.x or self.x >= self.graphics.w or
            self.y <= self.graphics.y or self.y >= self.graphics.h):
            self.x = self.graphics.center.x - OFFSET + OFFSET * 2 * random.random()
            self.y = self.graphics.center.y - OFFSET + OFFSET * 2 * random.random()
            self.color[0] = random.randint(20, 255)
            self.color[1] = random.randint(20, 255)
            self.color[2] = random.randint(20, 255)
        else:
            gx = self.graphics.center.x + (self.x - self.graphics.center.x) * DIV_FACTOR
            if (math.fabs(self.x - self.graphics.center.x) < 1e-6):
                gy = self.graphics.center.y + (self.y - self.graphics.center.y) * DIV_FACTOR
            else:
                k  = (gx - self.graphics.center.x) / (self.x - self.graphics.center.x)
                gy = self.graphics.center.y + (self.y - self.graphics.center.y) * k
            self.x = gx
            self.y = gy
            
        # update self's size
        self.size = int(5 * ((math.fabs(self.x - self.graphics.center.x) * 2) / self.graphics.w))
        if self.size <= 1:
            self.size = 1
        
def main():
    asteroids = []
    for i in range(INIT_NASTEROIDS):
        asteroid = Asteroid()
        asteroid.update()
        asteroids.append(asteroid)
    
    _quit = False
    while not _quit:
        for e in pygame.event.get():
            if e.type in (pygame.locals.QUIT, pygame.locals.KEYDOWN):
                _quit = True        
        
        if (len(asteroids) < NASTEROIDS):
            asteroid = Asteroid()
            asteroid.update()
            asteroids.append(asteroid)
        
        for i in range(len(asteroids)):
            # hide asteroids[i]
            asteroids[i].hide()
            
            # update asteroids[i]
            asteroids[i].update()
            
            # show asteroids[i]
            asteroids[i].show()
        
        # swap display content actually
        Asteroid.graphics.flipDisplay()
        
if __name__ == '__main__':
    main()
    print("#OK")
    