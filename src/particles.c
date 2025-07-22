#include "headers/drawing.h"
#include "headers/level_loader.h"

#include <windows.h>
#include <stdio.h>

float particleSpeed = 100.0f;
float particleGravity = 120.0f;
int maxParticleCount = 640;
int currentParticleCount = 0;

typedef struct particle {
    Sprite *obj;
    float timeLived;
    float forces[2];
    struct particle *next;
} particle;

typedef struct particleGenerator {
    POINT pos;
    SIZE size;
    float lifespan, particleLifespan;
    int spawnRadius;
    int density, gravityApplied, moveAway; // constant amount of particles, bool, bool
    COLORREF color;
    int smudge;
    float timeLived;
    struct particle *particles;
    struct particleGenerator* next;
} particleGenerator;

particleGenerator *currentParticles = NULL;

void createParticles(POINT pos, SIZE size, float lifespan, float particleLifespan, int spawnRadius, int density, int gravityApplied, int moveAway, COLORREF color, int smudge){
    particleGenerator *newParticleGen = (particleGenerator *)malloc(sizeof(particleGenerator));
    if (newParticleGen == NULL){
        printf("There was an error allocating space for a particle generator!\n");
        return;
    }

    newParticleGen->pos = pos;
    newParticleGen->size = size;
    newParticleGen->lifespan = lifespan;
    newParticleGen->particleLifespan = particleLifespan;
    newParticleGen->spawnRadius = spawnRadius;
    newParticleGen->density = density;
    newParticleGen->gravityApplied = gravityApplied;
    newParticleGen->moveAway = moveAway;
    newParticleGen->color = color;
    newParticleGen->smudge = smudge;
    newParticleGen->timeLived = 0.0f;
    newParticleGen->particles = NULL;
    newParticleGen->next = NULL;

    if (currentParticles == NULL){
        currentParticles = newParticleGen;
    }else{
        particleGenerator *currPar = currentParticles;
        while (currPar->next != NULL) currPar = currPar->next;
        currPar->next = newParticleGen;
    }
}

void endParticles(){
    particleGenerator *curr = currentParticles;
    while (curr != NULL){
        particle *curr_particle = curr->particles;
        while (curr_particle != NULL){
            particle *part_del = curr_particle;
            curr_particle = curr_particle->next;
            EraseSprite(part_del->obj);
            free(part_del);
        }
        particleGenerator *gen_del = curr;
        curr = curr->next;
        free(gen_del);
    }
}

void updateParticles(float dt){
    particleGenerator *curr = currentParticles;
    particleGenerator *prev = NULL;
    while (curr != NULL){
        curr->timeLived += dt;
        int particleCount = 0;
        particle *curr_particle = curr->particles;
        particle *prev_particle = NULL;
        while (curr_particle != NULL){
            curr_particle->timeLived += dt;
            if (curr_particle->timeLived > curr->particleLifespan || currentParticleCount > maxParticleCount){
                if (curr_particle->obj != NULL)EraseSprite(curr_particle->obj);
                particle *tmp_delete = curr_particle;
                if (prev_particle == NULL){
                    curr->particles = curr_particle->next;
                }else{
                    prev_particle->next = curr_particle->next;
                }
                curr_particle = curr_particle->next;
                free(tmp_delete);
                currentParticleCount--;
                continue;
            }
            curr_particle->obj->pos.x += curr_particle->forces[0]*dt;
            curr_particle->obj->pos.y += curr_particle->forces[1]*dt;
            if (curr->gravityApplied){
                curr_particle->forces[1] += particleGravity;
            }
            prev_particle = curr_particle;
            curr_particle = curr_particle->next;
            particleCount++;
        }
        if (particleCount < curr->density && curr->timeLived <= curr->lifespan && currentParticleCount < maxParticleCount){
            currentParticleCount++;
            particle *new_particle = (particle *)malloc(sizeof(particle));
            if (new_particle == NULL){
                printf("There was an error allocating space for a new particle\n");
                return;
            }
            new_particle->next = NULL;
            new_particle->timeLived = 0.0f;

            POINT position;
            position.x = rand()%(curr->spawnRadius * 2) - curr->spawnRadius + curr->pos.x;
            position.y = rand()%(curr->spawnRadius * 2) - curr->spawnRadius + curr->pos.y;
            new_particle->obj = (Sprite *)malloc(sizeof(Sprite));
            if (new_particle->obj == NULL){
                free(new_particle);
                printf("There was an error allocating space for object!\n");
                return;
            }
            
            new_particle->forces[0] = 0.0f;
            new_particle->forces[1] = 0.0f;

            if (curr->moveAway){
                float relativeX = position.x - curr->pos.x;
                float relativeY = position.y - curr->pos.y;
                float slope;
                if (relativeX != 0){
                    slope = abs(relativeY/relativeX);
                }else{
                    slope = 0;
                }
                int localParticleSpeed = curr->moveAway == TRUE ? particleSpeed : curr->moveAway;
                if (abs(relativeX) > abs(relativeY)) {
                    new_particle->forces[0] = relativeX >= 0 ? localParticleSpeed : -localParticleSpeed;
                    new_particle->forces[1] = relativeY >= 0 ?  slope*localParticleSpeed : -slope*localParticleSpeed;
                }else{
                    new_particle->forces[0] = relativeX >= 0 ?  slope*localParticleSpeed : -slope*localParticleSpeed;
                    new_particle->forces[1] = relativeY >= 0 ? localParticleSpeed : -localParticleSpeed;
                }
            }

            int smudgeValue = (rand()%(curr->smudge * 2) - curr->smudge);
            int colorR = (int) GetRValue(curr->color) + smudgeValue;
            int colorG = (int) GetGValue(curr->color) + smudgeValue;
            int colorB = (int) GetBValue(curr->color) + smudgeValue;
            colorR = max(0, min(255, colorR));
            colorG = max(0, min(255, colorG));
            colorB = max(0, min(255, colorB));
            COLORREF particle_color = RGB(colorR, colorG, colorB);
            CreateSprite(new_particle->obj, position.x, position.y, curr->size.cx, curr->size.cy, particle_color);
            if (curr->particles == NULL){
                curr->particles = new_particle;
            }else{
                particle *cc = curr->particles;
                while (cc->next != NULL) cc = cc->next;
                cc->next = new_particle;
            }
        }
        if (curr->timeLived > curr->lifespan && particleCount == 0){
            if (prev == NULL){
                currentParticles = curr->next;
            }else{
                prev->next = curr->next;
            }
            particleGenerator *tmp_delete = curr;
            curr = curr->next;
            free(tmp_delete);
            continue;
        }
        prev = curr;
        curr = curr->next;
    }
}

void drawParticles(HDC hdc){
    particleGenerator *curr = currentParticles;
    while (curr != NULL){
        particle *curr_particle = curr->particles;
        while (curr_particle != NULL){
            if (curr_particle->obj != NULL) PaintSprite(hdc, curr_particle->obj);
            curr_particle = curr_particle->next;
        }
        curr = curr->next;
    }
}