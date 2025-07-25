import pygame
import sys
import random
import math
import time

# === Constants ===
WIDTH, HEIGHT = 800, 600
FPS = 60

# Paddle
PADDLE_WIDTH, PADDLE_HEIGHT = 100, 15
PADDLE_COLOR = (255, 255, 255)

# Ball
BALL_RADIUS = 8
BALL_COLOR = (255, 200, 0)
BALL_SPEED = 5

# Brick
BRICK_ROWS = {1:3, 2:4, 3:5, 4:6}
BRICK_COLS = 10
BRICK_GAP = 2
BRICK_HEIGHT = 20
BRICK_COLORS = {
    1: ((200,50,50), (255,150,150)),
    2: ((200,100,50), (255,200,150)),
    3: ((200,200,50), (255,255,150)),
}
STEEL_COLOR = (100,100,100)

# Power-ups
PU_SIZE = 16
PU_TYPES = ['expand', 'sticky', 'multi', 'slow', 'life']
PU_COLOR = {
    'expand': (0,255,0),
    'sticky': (255,0,255),
    'multi': (0,255,255),
    'slow': (255,255,0),
    'life': (255,0,0)
}
PU_DURATION = 10  # seconds

# Game states
STATE_START = 'START'
STATE_PLAY = 'PLAYING'
STATE_PAUSE = 'PAUSED'
STATE_LEVEL_COMPLETE = 'LEVEL_COMPLETE'
STATE_GAME_OVER = 'GAME_OVER'

pygame.init()
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("Brick Breaker")
clock = pygame.time.Clock()
font_large = pygame.font.SysFont("Arial", 48, bold=True)
font_small = pygame.font.SysFont("Arial", 24)

# === Classes ===
class Paddle:
    """Player paddle, can be expanded or sticky."""
    def __init__(self):
        self.width = PADDLE_WIDTH
        self.height = PADDLE_HEIGHT
        self.x = WIDTH//2 - self.width//2
        self.y = HEIGHT - self.height - 10
        self.color = PADDLE_COLOR
        self.sticky = False
        self.expand_end = 0

    def update(self):
        """Clamp position and handle power-up expiration."""
        keys = pygame.key.get_pressed()
        if keys[pygame.K_LEFT]:
            self.x = max(0, self.x - 7)
        if keys[pygame.K_RIGHT]:
            self.x = min(WIDTH - self.width, self.x + 7)
        # Check expand expiry
        if self.expand_end and time.time() > self.expand_end:
            self.width = PADDLE_WIDTH
            self.expand_end = 0

    def draw(self):
        pygame.draw.rect(screen, self.color, (self.x, self.y, self.width, self.height), border_radius=5)

    def expand(self):
        """Double paddle width for duration."""
        self.width = PADDLE_WIDTH * 2
        self.expand_end = time.time() + PU_DURATION

class Ball:
    """Ball with constant speed."""
    def __init__(self):
        self.reset()

    def reset(self):
        self.x = WIDTH//2
        self.y = HEIGHT//2
        angle = random.uniform(-math.pi/4, math.pi/4)
        self.vx = BALL_SPEED * math.cos(angle)
        self.vy = -BALL_SPEED * math.sin(angle)

    def update(self):
        self.x += self.vx
        self.y += self.vy
        # Wall bounce
        if self.x <= BALL_RADIUS or self.x >= WIDTH - BALL_RADIUS:
            self.vx *= -1
            spawn_particles(self.x, self.y, count=10, color=BALL_COLOR)
        if self.y <= BALL_RADIUS:
            self.vy *= -1
            spawn_particles(self.x, self.y, count=10, color=BALL_COLOR)

    def draw(self):
        pygame.draw.circle(screen, BALL_COLOR, (int(self.x), int(self.y)), BALL_RADIUS)

    def collide_paddle(self, paddle):
        """Bounce with angle based on hit position."""
        if (paddle.y <= self.y+BALL_RADIUS <= paddle.y+paddle.height 
            and paddle.x <= self.x <= paddle.x+paddle.width):
            rel = (self.x - (paddle.x + paddle.width/2)) / (paddle.width/2)
            angle = rel * (math.pi/3)  # max 60deg
            speed = math.hypot(self.vx, self.vy)
            self.vx = speed * math.sin(angle)
            self.vy = -abs(speed * math.cos(angle))
            spawn_particles(self.x, self.y, count=20, color=paddle.color)
            return True
        return False

class Brick:
    """Brick which may require multiple hits or be steel."""
    def __init__(self, x, y, health):
        self.x, self.y = x, y
        self.w = WIDTH//BRICK_COLS - BRICK_GAP
        self.h = BRICK_HEIGHT
        self.health = health
        self.max_health = health

    def draw(self):
        if self.health < 0:
            color = STEEL_COLOR
        else:
            c0, c1 = BRICK_COLORS.get(self.health, ((255,255,255),(255,255,255)))
            t = self.health/self.max_health
            color = tuple(int(a + (b-a)*(1-t)) for a,b in zip(c0,c1))
        pygame.draw.rect(screen, color, (self.x, self.y, self.w, self.h), border_radius=3)

    def collide(self, ball):
        """Detect collision and apply damage."""
        rect = pygame.Rect(self.x, self.y, self.w, self.h)
        if rect.collidepoint(ball.x, ball.y - BALL_RADIUS):
            # Bounce
            ball.vy *= -1
            spawn_particles(ball.x, ball.y, count=15, color=(255,255,255))
            # Damage
            if self.health > 0:
                self.health -= 1
                return True
        return False

class PowerUp:
    """Falling power-up that applies effect on catch."""
    def __init__(self, x, y, ptype):
        self.x, self.y = x, y
        self.type = ptype
        self.size = PU_SIZE
        self.speed = 3

    def update(self):
        self.y += self.speed

    def draw(self):
        pygame.draw.rect(screen, PU_COLOR[self.type], (self.x, self.y, self.size, self.size), border_radius=3)

    def collide(self, paddle):
        rect = pygame.Rect(self.x, self.y, self.size, self.size)
        if rect.colliderect(pygame.Rect(paddle.x, paddle.y, paddle.width, paddle.height)):
            return True
        return False

class Particle:
    """Simple particle for effects."""
    def __init__(self, x, y, color):
        self.x, self.y = x, y
        self.vx = random.uniform(-2,2)
        self.vy = random.uniform(-2,2)
        self.life = random.uniform(0.3,0.6)
        self.color = color

    def update(self, dt):
        self.life -= dt
        self.x += self.vx
        self.y += self.vy

    def draw(self):
        if self.life>0:
            alpha = max(0, int(255*(self.life/0.6)))
            surf = pygame.Surface((4,4), pygame.SRCALPHA)
            surf.fill((*self.color, alpha))
            screen.blit(surf, (self.x, self.y))

# === Helper functions ===
particles = []
def spawn_particles(x,y,count,color):
    for _ in range(count):
        particles.append(Particle(x,y,color))

# Level layouts
def load_level(n):
    bricks = []
    if n<5:
        rows = BRICK_ROWS[n]
        for row in range(rows):
            for col in range(BRICK_COLS):
                x = col*(WIDTH//BRICK_COLS)
                y = 50 + row*(BRICK_HEIGHT+BRICK_GAP)
                # random multi-hit
                health = random.choice([1,1,2,3])
                bricks.append(Brick(x,y,health))
    else:
        # Boss level: one big steel brick
        bx = WIDTH//4; by=100
        bw = WIDTH//2; bh=BRICK_HEIGHT*2
        boss = Brick(bx,by,-1)  # health<0 = steel
        boss.w, boss.h = bw, bh
        bricks=[boss]
    return bricks

# === Main Game ===
def main():
    state = STATE_START
    level = 1
    lives = 3
    score = 0

    paddle = Paddle()
    balls = [Ball()]
    bricks = load_level(level)
    powerups = []
    last_time = time.time()

    while True:
        dt = clock.tick(FPS)/1000
        screen.fill((30,30,30))
        for event in pygame.event.get():
            if event.type==pygame.QUIT:
                pygame.quit(); sys.exit()
            if event.type==pygame.KEYDOWN:
                if state==STATE_START and event.key==pygame.K_SPACE:
                    state = STATE_PLAY
                elif state==STATE_LEVEL_COMPLETE and event.key==pygame.K_SPACE:
                    level +=1; balls=[Ball()]; bricks=load_level(level); state=STATE_PLAY
                elif state==STATE_GAME_OVER and event.key==pygame.K_SPACE:
                    level=1; lives=3; score=0; balls=[Ball()]; bricks=load_level(level); state=STATE_PLAY
                elif state==STATE_PLAY and event.key==pygame.K_p:
                    state = STATE_PAUSE
                elif state==STATE_PAUSE and event.key==pygame.K_p:
                    state = STATE_PLAY

        if state==STATE_START:
            screen.blit(font_large.render("Brick Breaker Deluxe",1,(255,255,255)),(120,200))
            screen.blit(font_small.render("Press SPACE to Start",1,(200,200,200)),(280,300))

        elif state in [STATE_PLAY, STATE_PAUSE]:
            # Update objects
            if state==STATE_PLAY:
                paddle.update()
                for ball in balls:
                    ball.update()
                    if ball.collide_paddle(paddle):
                        pass
                    # bottom collision
                    if ball.y>HEIGHT:
                        balls.remove(ball); lives-=1
                        if lives>0: balls.append(Ball())
                # Brick collisions
                removed=[]
                for brick in bricks:
                    for ball in balls:
                        if brick.collide(ball):
                            if brick.health==0: removed.append(brick)
                            # drop powerup
                            if random.random()<0.2:
                                pu = random.choice(PU_TYPES)
                                powerups.append(PowerUp(brick.x+brick.w/2,brick.y,pu))
                for b in removed:
                    bricks.remove(b); score+=10
                # Power-up updates
                for pu in powerups:
                    pu.update()
                    pu.draw()
                    if pu.collide(paddle):
                        # apply effect
                        if pu.type=='expand': paddle.expand()
                        if pu.type=='sticky': paddle.sticky=True
                        if pu.type=='multi': balls.append(Ball())
                        if pu.type=='slow': 
                            for ball in balls: ball.vx*=0.5; ball.vy*=0.5
                        if pu.type=='life': lives+=1
                        powerups.remove(pu)
                # Particles
                for p in particles[:]:
                    p.update(dt); p.draw()
                    if p.life<=0: particles.remove(p)

                # Check transitions
                if lives<=0:
                    state=STATE_GAME_OVER
                elif not bricks:
                    state=STATE_LEVEL_COMPLETE

            # Draw all
            paddle.draw()
            for ball in balls: ball.draw()
            for brick in bricks: brick.draw()
            # UI
            screen.blit(font_small.render(f"Level: {level}",1,(255,255,255)),(10,10))
            screen.blit(font_small.render(f"Lives: {lives}",1,(255,255,255)),(120,10))
            screen.blit(font_small.render(f"Score: {score}",1,(255,255,255)),(220,10))
            if state==STATE_PAUSE:
                screen.blit(font_large.render("Paused",1,(200,200,200)),(330,250))

        elif state==STATE_LEVEL_COMPLETE:
            screen.blit(font_large.render(f"Level {level} Complete!",1,(255,255,255)),(200,200))
            screen.blit(font_small.render("Press SPACE to Continue",1,(200,200,200)),(260,300))

        elif state==STATE_GAME_OVER:
            screen.blit(font_large.render("Game Over",1,(255,100,100)),(300,200))
            screen.blit(font_small.render("Press SPACE to Restart",1,(200,200,200)),(270,300))

        pygame.display.flip()

if __name__=="__main__":
    main()