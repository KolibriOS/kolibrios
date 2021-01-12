# center: 10043
import pygame, sys
if not "tinypy" in sys.version:
    import pygame.locals

SW,SH = 120,120

def julia(s,ca,cb):
    pal = [((min(255,v)),(min(255,v*3/2)),(min(255,v*2))) for v in range(0,256)]
    for y in range(0,SH):
        for x in range(0,SW):
            i=0
            a=((float(x)/SW) * 4.0 - 2.0)
            b=((float(y)/SH) * 4.0 - 2.0)
            while i < 15 and (a*a)+(b*b)<4.0:
                na=(a*a)-(b*b)+ca
                nb=(2.0*a*b)+cb
                a=na
                b=nb
                i = i +1
            s.set_at((x,y),pal[i*16])

def main():
    pygame.init()
    s = pygame.display.set_mode((SW,SH),0,32)
    _quit = False
    while not _quit:
        for e in pygame.event.get():
            if e.type in (pygame.locals.QUIT,pygame.locals.KEYDOWN):
                _quit = True
        
        x,y = pygame.mouse.get_pos()
        ca=((float(x)/SW) * 2.0 - 1.0)
        cb=((float(y)/SH) * 2.0 - 1.0)
        ticks = pygame.time.get_ticks()
        julia(s,ca,cb)
        print(pygame.time.get_ticks()-ticks)
        pygame.display.flip()
        
    
if __name__ == '__main__':
    main()
