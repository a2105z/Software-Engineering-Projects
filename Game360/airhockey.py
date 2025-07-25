import pygame
import sys
import random
import math

# === CONSTANTS ===
WIDTH, HEIGHT = 700, 900
BLACK = (0, 0, 0)
WHITE = (255, 255, 255)
RED = (232, 76, 61)
BLUE = (52, 152, 219)
TABLE_COLOR = (30, 120, 30)
GOAL_COLOR = (20, 80, 20)
FPS = 60

GOAL_WIDTH = 300
WALL_THICKNESS = 20
PADDLE_WIDTH = 100
PADDLE_HEIGHT = 20
PUCK_RADIUS = 15

INITIAL_SPEED = 12          # constant puck speed
MAX_BOUNCE_ANGLE = math.radians(75)

pygame.init()
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption('Air Hockey')
clock = pygame.time.Clock()
font = pygame.font.SysFont("Arial", 48, bold=True)
small_font = pygame.font.SysFont("Arial", 24, bold=True)


class Particle:
    """A single particle for collision effects."""
    def __init__(self, x, y, color):
        self.x = x
        self.y = y
        angle = random.uniform(0, 2 * math.pi)
        speed = random.uniform(2, 6)
        self.vx = math.cos(angle) * speed
        self.vy = math.sin(angle) * speed
        self.lifetime = random.randint(20, 40)
        self.radius = random.randint(2, 4)
        self.color = color

    def update(self):
        """Move the particle and decrease its lifetime."""
        self.x += self.vx
        self.y += self.vy
        self.lifetime -= 1

    def draw(self, surface):
        """Draw the particle with fading alpha."""
        if self.lifetime > 0:
            alpha = int(255 * (self.lifetime / 40))
            surf = pygame.Surface((self.radius*2, self.radius*2), pygame.SRCALPHA)
            pygame.draw.circle(
                surf,
                (*self.color, alpha),
                (self.radius, self.radius),
                self.radius
            )
            surface.blit(surf, (self.x - self.radius, self.y - self.radius))


class Paddle:
    """Represents a player paddle."""
    def __init__(self, x, y, color):
        self.color = color
        self.rect = pygame.Rect(0, 0, PADDLE_WIDTH, PADDLE_HEIGHT)
        self.rect.center = (x, y)

    def move(self, dx):
        """Move paddle horizontally, clamped inside the table walls."""
        self.rect.x = max(
            WALL_THICKNESS,
            min(WIDTH - WALL_THICKNESS - self.rect.width, self.rect.x + dx)
        )

    def draw(self, surf):
        """Draw the rounded paddle."""
        pygame.draw.rect(surf, self.color, self.rect, border_radius=10)

    def reset(self, x):
        """Center the paddle over a given x-coordinate."""
        self.rect.centerx = x


class Puck:
    """Represents the puck with constant speed."""
    def __init__(self):
        self.reset()

    def reset(self):
        """Place the puck at center with zero velocity."""
        self.x = WIDTH // 2
        self.y = HEIGHT // 2
        self.dx = 0
        self.dy = 0

    def serve(self, scorer):
        """
        Launch the puck toward the last scorer (1=top, 2=bottom).
        If no scorer yet, pick a random vertical direction.
        """
        angle = random.uniform(-MAX_BOUNCE_ANGLE, MAX_BOUNCE_ANGLE)
        direction = 1 if scorer == 2 else -1
        if scorer is None:
            direction = random.choice([-1, 1])
        # ensure constant speed: dx = v*sin, dy = v*cos
        self.dx = INITIAL_SPEED * math.sin(angle)
        self.dy = direction * INITIAL_SPEED * math.cos(angle)

    def update(self):
        """Move the puck by its velocity (no friction)."""
        self.x += self.dx
        self.y += self.dy

    def draw(self, surf):
        """Draw the puck circle."""
        pygame.draw.circle(surf, WHITE, (int(self.x), int(self.y)), PUCK_RADIUS)

    def rect(self):
        """Return a pygame.Rect covering the puck."""
        return pygame.Rect(
            int(self.x - PUCK_RADIUS),
            int(self.y - PUCK_RADIUS),
            PUCK_RADIUS * 2,
            PUCK_RADIUS * 2
        )


def draw_table():
    """Render the air-hockey table, walls, goals, and center markings."""
    screen.fill(TABLE_COLOR)
    # Walls
    pygame.draw.rect(screen, GOAL_COLOR, (0, 0, WIDTH, WALL_THICKNESS))
    pygame.draw.rect(screen, GOAL_COLOR, (0, HEIGHT - WALL_THICKNESS, WIDTH, WALL_THICKNESS))
    pygame.draw.rect(screen, GOAL_COLOR, (0, 0, WALL_THICKNESS, HEIGHT))
    pygame.draw.rect(screen, GOAL_COLOR, (WIDTH - WALL_THICKNESS, 0, WALL_THICKNESS, HEIGHT))
    # Goals
    pygame.draw.rect(
        screen,
        TABLE_COLOR,
        (WIDTH//2 - GOAL_WIDTH//2, 0, GOAL_WIDTH, WALL_THICKNESS)
    )
    pygame.draw.rect(
        screen,
        TABLE_COLOR,
        (WIDTH//2 - GOAL_WIDTH//2, HEIGHT - WALL_THICKNESS, GOAL_WIDTH, WALL_THICKNESS)
    )
    # Center line (dashed)
    for y in range(WALL_THICKNESS + 20, HEIGHT - WALL_THICKNESS, 40):
        pygame.draw.line(screen, WHITE, (WIDTH//2, y), (WIDTH//2, y + 20), 4)
    # Center circle
    pygame.draw.circle(screen, WHITE, (WIDTH//2, HEIGHT//2), 100, 4)


def main():
    """Main game loop for Air Hockey."""
    paddle1 = Paddle(WIDTH//2, WALL_THICKNESS + 60, RED)
    paddle2 = Paddle(WIDTH//2, HEIGHT - WALL_THICKNESS - 60, BLUE)
    puck = Puck()
    particles = []

    score1, score2 = 0, 0
    last_scorer = None
    playing = False
    game_over = False

    while True:
        clock.tick(FPS)
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                sys.exit()

        keys = pygame.key.get_pressed()
        # Paddle controls
        if keys[pygame.K_a]:
            paddle1.move(-12)
        if keys[pygame.K_d]:
            paddle1.move(12)
        if keys[pygame.K_LEFT]:
            paddle2.move(-12)
        if keys[pygame.K_RIGHT]:
            paddle2.move(12)
        # Serve on 'P'
        if keys[pygame.K_p] and not playing and not game_over:
            playing = True
            puck.serve(last_scorer)
        # Reset on 'R'
        if keys[pygame.K_r]:
            score1, score2 = 0, 0
            last_scorer = None
            playing = False
            game_over = False
            paddle1.reset(WIDTH//2)
            paddle2.reset(WIDTH//2)
            puck.reset()
            particles.clear()

        draw_table()
        # Display scores
        screen.blit(
            font.render(str(score1), True, WHITE),
            (WIDTH//4, WALL_THICKNESS//2)
        )
        screen.blit(
            font.render(str(score2), True, WHITE),
            (3*WIDTH//4 - 20, WALL_THICKNESS//2)
        )

        if game_over:
            # Show winner message
            winner = "Red" if score1 >= 10 else "Blue"
            msg = f"{winner} Wins! Press R to Restart"
            screen.blit(small_font.render(msg, True, WHITE), (WIDTH//2 - 200, HEIGHT//2 - 20))
        else:
            if playing:
                # Move puck and handle collisions
                puck.update()
                pr = puck.rect()

                # Wall collisions
                if pr.left <= WALL_THICKNESS or pr.right >= WIDTH - WALL_THICKNESS:
                    puck.dx = -puck.dx
                    for _ in range(20): particles.append(Particle(puck.x, puck.y, WHITE))
                if pr.top <= WALL_THICKNESS:
                    # if in goal zone, score
                    if WIDTH//2 - GOAL_WIDTH//2 < puck.x < WIDTH//2 + GOAL_WIDTH//2:
                        score2 += 1
                        last_scorer = 2
                        playing = False
                        puck.reset()
                    else:
                        puck.dy = -puck.dy
                        for _ in range(20): particles.append(Particle(puck.x, puck.y, WHITE))
                if pr.bottom >= HEIGHT - WALL_THICKNESS:
                    if WIDTH//2 - GOAL_WIDTH//2 < puck.x < WIDTH//2 + GOAL_WIDTH//2:
                        score1 += 1
                        last_scorer = 1
                        playing = False
                        puck.reset()
                    else:
                        puck.dy = -puck.dy
                        for _ in range(20): particles.append(Particle(puck.x, puck.y, WHITE))

                # Paddle collisions with angle-based reflection
                if pr.colliderect(paddle1.rect):
                    # map contact point to bounce angle
                    offset = (puck.x - paddle1.rect.centerx) / (PADDLE_WIDTH / 2)
                    angle = offset * MAX_BOUNCE_ANGLE
                    puck.dx = INITIAL_SPEED * math.sin(angle)
                    puck.dy = INITIAL_SPEED * math.cos(angle)
                    for _ in range(30): particles.append(Particle(puck.x, puck.y, RED))

                if pr.colliderect(paddle2.rect):
                    offset = (puck.x - paddle2.rect.centerx) / (PADDLE_WIDTH / 2)
                    angle = offset * MAX_BOUNCE_ANGLE
                    puck.dx = INITIAL_SPEED * math.sin(angle)
                    puck.dy = -INITIAL_SPEED * math.cos(angle)
                    for _ in range(30): particles.append(Particle(puck.x, puck.y, BLUE))

                # Check for game over (first to 10)
                if score1 >= 10 or score2 >= 10:
                    game_over = True
                    playing = False

                # Draw the puck
                puck.draw(screen)

            # Draw paddles
            paddle1.draw(screen)
            paddle2.draw(screen)

        # Update and draw all particles
        for p in particles[:]:
            p.update()
            p.draw(screen)
            if p.lifetime <= 0:
                particles.remove(p)

        pygame.display.flip()


if __name__ == '__main__':
    main()