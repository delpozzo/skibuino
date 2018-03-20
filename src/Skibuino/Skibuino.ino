// ###########################################################################
//          Title: Skibuino
//         Author: Mike Del Pozzo
//    Description: A skiing game for Gamebuino and MAKERbuino.
//        Version: 1.0.0
//           Date: 19 Mar 2018
//        License: GPLv3 (see LICENSE)
//
//    Skibuino Copyright (C) 2018 Mike Del Pozzo
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    any later version.
// ###########################################################################

#include <SPI.h>
#include <Gamebuino.h>
#include <EEPROM.h>
#include "Sprite.h"
#include "Sound.h"

// ######################################################
//    Defines, Enums, Structs, and Globals
// ######################################################

#define MAXENTS 32 // maximum number of entities that can exist at one time
#define MAXFRAMES 4 // maximum number of sprites per entity
#define MAXFLAGS 4 // maximum number of flags per entity
#define MAPWIDTH 128 // map width
#define MAPHEIGHT 256 // map height
#define CAMERAXOFFSET LCDWIDTH/2 // camera X offset
#define CAMERAYOFFSET (LCDHEIGHT/2) + 10 // camera Y offset
#define XSTARTPOS (LCDWIDTH/2) - 4 // starting x coordinate for player
#define YSTARTPOS (LCDHEIGHT/2) - 5 // starting y coordinate for player
#define SCROLLPOS MAPHEIGHT - LCDHEIGHT + 8 // y coordinate at which obstacles refresh
#define XLIMITR MAPWIDTH - 8 // right-most x coordinate the player can travel
#define XLIMITL 0 // left-most x coordinate the player can travel
#define JUMPDURATION 10 // how long the player can jump for
#define JUMPCOOLDOWN 5 // cooldown period between jumps
#define GAMEOVERTIMER 16 // duration before restarting the game

enum TYPES {PLAYER=0, OBSTACLE=1}; // types of entities
enum PLAYERFLAGS {JUMP=0, JUMPCTR=1, GAMEOVER=2, PLAYERTXT=3}; // flags for player

// Entity structure
typedef struct ENTITY_S
{
  byte frame; // sprite frame that will be rendered in the next drawAll() iteration
  const byte *sprite[MAXFRAMES]; // array of sprites for entity of size MAXFRAMES
  byte rotation; // sprite rotation: NOROT [default], ROTCW, ROTCCW, or ROT180
  byte flip; // sprite flip: NOFLIP [default], FLIPH, FLIPV, or FLIPVH
  int x, y; // x and y coordinates of entity
  byte type; // entity type (PLAYER or OBSTACLE)
  byte xspeed; // x movement speed
  byte yspeed; // y movement speed
  byte flag[MAXFLAGS]; // array of flags for entity of size MAXFLAGS
  boolean used; // whether this particular entity in the EntityList[] is in use or not
  void(*think)(struct ENTITY_S *self); // think function pointer that will be called in the next thinkAll() iteration
} Entity;

Entity EntityList[MAXENTS]; // global array of entities

Entity *player; // global pointer to the player's entity
Gamebuino gb; //  gamebuino object
int cameraX, cameraY;// coordinates of the camera
int metersTraveled; // number of meters player has traveled (score)
int highScore; // most meters traveled without crashing
boolean highScoreResetFlag; // flag used for confirming high score reset

// ######################################################
//    Game Setup and Main Logic
// ######################################################

// Entry point
void setup()
{
  gb.begin();
  gb.titleScreen(gameLogo);
  gb.pickRandomSeed();

  initEntities();
  initCamera();

  startGame();
}

// Main game loop
void loop()
{
  if (gb.update())
  {
    thinkAll(); // call each entity's think function
    updateCamera(); // center camera on player
    drawAll(); // draw each entity's current frame
  }
}

// Start game
void startGame()
{
  highScore = getHighScore();
  metersTraveled = 0;
  player = spawnPlayer(XSTARTPOS, YSTARTPOS);
  spawnObstacles();
}

// Check if highscore, restart game
void restartGame()
{
  if(metersTraveled > highScore)
  {
    saveHighScore(metersTraveled);
  }
  
  clearEntities();
  startGame();
}

// ######################################################
//    Pause Functions
// ######################################################

// Show pause menu
void pauseGame()
{
  highScoreResetFlag = false;
  
  while(1)
  {
    if(gb.update())
    {
      drawScore();

      // Draw Confirm High Score Reset Menu
      if(highScoreResetFlag)
      {
        gb.display.cursorY = 18;
        gb.display.cursorX = 12;
        gb.display.println(F("RESET HIGH SCORE"));
        gb.display.println(F("   ...are you sure?"));
        
        gb.display.cursorY = 35;
        gb.display.println(F("A+B: Confirm"));
        gb.display.println(F("  C: Cancel"));

        // Buttons A + B - Reset High Score
        if(gb.buttons.repeat(BTN_A, 1) && gb.buttons.repeat(BTN_B, 1))
        {
          highScoreResetFlag = true;
          saveHighScore(0);
          metersTraveled = 0;
          restartGame();
          return;
        }
        
        // Button C - Cancel
        if(gb.buttons.pressed(BTN_C))
        {
          highScoreResetFlag = false;
        }
      }
      else // Draw Regular Pause Menu
      {
        gb.display.cursorY = 18;
        gb.display.cursorX = 20;
        gb.display.println(F("P A U S E D"));

        gb.display.cursorY = 30;
        gb.display.println(F("A: Title Screen"));
        gb.display.println(F("B: Reset High Score"));
        gb.display.println(F("C: Resume Game"));

        // Button A - Return to Title Screen
        if(gb.buttons.pressed(BTN_A))
        {
          clearEntities();
          setup();
          return;
        }
      
        // Button B - Reset High Score
        if(gb.buttons.pressed(BTN_B))
        {
          highScoreResetFlag = true;
        }

        // Button C - Resume Game
        if(gb.buttons.pressed(BTN_C))
        {
          playSound(sndResume, 0);
          return;
        }
      }
    }
  }
}

// ######################################################
//    Score Functions
// ######################################################

int getHighScore()
{
  long two = EEPROM.read(0);
  long one = EEPROM.read(1);
 
  return ((two << 0) & 0xFFFFFF) + ((one << 8) & 0xFFFFFFFF);
}

void saveHighScore(int score)
{
  byte two = (score & 0xFF);
  byte one = ((score >> 8) & 0xFF);
 
  EEPROM.update(0, two);
  EEPROM.update(1, one);
}

void drawScore()
{
  gb.display.print(metersTraveled);
  gb.display.print(F("M"));
  if(metersTraveled > highScore)
  {
    gb.display.print(F("   New Best!"));
  }
  else
  {
    gb.display.print(F("    Best: "));
    gb.display.print(highScore);
    gb.display.print(F("M"));
  }
}

// ######################################################
//    Sound Functions
// ######################################################

void playSound(const int *snd, byte channel)
{
  gb.sound.command(0, snd[0], 0, channel);      // 0: Set note volume
  gb.sound.command(1, snd[1], 0, channel);      // 1: Select instrument
  gb.sound.command(2, snd[2], snd[3], channel); // 2: Volume slide
  gb.sound.command(3, snd[4], snd[5], channel); // 3: Pitch slide
  gb.sound.command(4, snd[6], snd[7], channel); // 4: Tremolo
  
  gb.sound.playNote(snd[8], snd[9], channel);   // Play note (pitch, duration, channel)
}

// ######################################################
//    Entity Management and Draw Functions
// ######################################################

// Initializes EntityList[] with default values, sets pointers to NULL
void initEntities()
{
  for (int i = 0; i < MAXENTS; i++)
  {
    EntityList[i].used = 0;
    EntityList[i].rotation = NOROT;
    EntityList[i].flip = NOFLIP;
    EntityList[i].think = NULL;

    for (int j = 0; j < MAXFRAMES; j++)
    {
      EntityList[i].sprite[j] = NULL;
    }

    for (int k = 0; k < MAXFLAGS; k++)
    {
      EntityList[i].flag[k] = 0;
    }
  }
}

// Finds first available spot in the EntityList[] and returns a pointer to that Entity
// Returns NULL if the EntityList is full, this happens if MAXENTS is reached
Entity* spawnEntity()
{
  for (int i = 0; i < MAXENTS; i++)
  {
    if (EntityList[i].used == false)
    {
      EntityList[i].used = true;
      return &EntityList[i];
    }
  }
  return NULL;
}

// Calls the think function that is set for each active entity in the EntityList[]
void thinkAll()
{
  for (int i = 0; i < MAXENTS; i++)
  {
    if (EntityList[i].used && EntityList[i].think != NULL)
    {
      EntityList[i].think(&EntityList[i]);
    }
  }
}

// Frees (removes) specific entity, sets pointers to NULL
void freeEntity(Entity *ent)
{
  ent->used = false;
  ent->think = NULL;

  for (int i = 0; i < MAXFRAMES; i++)
  {
    ent->sprite[i] = NULL;
  }
}

// Frees all entities in the EntityList[]
void clearEntities()
{
  for (int i = 0; i < MAXENTS; i++)
  {
    freeEntity(&EntityList[i]);
  }
}

// Draws the current sprite frame for each active entity in the EntityList[]
void drawAll()
{
  for (int i = 0; i < MAXENTS; i++)
  {
    if (EntityList[i].used)
    {
      if (EntityList[i].sprite[EntityList[i].frame] != NULL)
      {
        gb.display.drawBitmap(EntityList[i].x - cameraX, EntityList[i].y - cameraY, EntityList[i].sprite[EntityList[i].frame], EntityList[i].rotation, EntityList[i].flip);
      }
    }
  }
}

// Initializes the camera position
void initCamera()
{
  cameraX = 0;
  cameraY = 0;
}

// Centers the camera position on the player
void updateCamera()
{
  if (!player->used) return;

  cameraX = player->x - CAMERAXOFFSET;
  if (cameraX < 0) cameraX = 0;
  if (cameraX > (MAPWIDTH - LCDWIDTH)) cameraX = MAPWIDTH - LCDWIDTH;

  cameraY = player->y - CAMERAYOFFSET;
  if (cameraY < 0) cameraY = 0;
  if (cameraY > (MAPHEIGHT - LCDHEIGHT)) cameraY = MAPHEIGHT - LCDHEIGHT;
}

// ######################################################
//    Player Functions
// ######################################################

Entity* spawnPlayer(int x, int y)
{
  Entity *self = spawnEntity();
  if (self == NULL) return NULL;

  self->think = playerThink;
  self->x = x;
  self->y = y;
  self->type = PLAYER;
  self->xspeed = 0;
  self->yspeed = 2;
  self->sprite[0] = playerNormal;
  self->sprite[1] = playerAngle;
  self->sprite[2] = playerJump;
  self->sprite[3] = playerAngle2;
  self->frame = 0;
  self->flip = NOFLIP;
  self->rotation = NOROT;
  self->flag[JUMP] = 0;
  self->flag[JUMPCTR] = 0;
  self->flag[GAMEOVER] = GAMEOVERTIMER;
  self->flag[PLAYERTXT] = 0;

  return self;
}

void playerThink(Entity *self)
{
  // Always ski downwards at yspeed
  self->y += self->yspeed;

  // By default render frame 0 with no flip
  self->frame = 0;
  self->flip = NOFLIP;

  // D-Pad Right - Turn Right
  if(gb.buttons.repeat(BTN_RIGHT, 1) && !self->flag[JUMP])
  {
    if(self->xspeed == 4)
    {
      self->frame = 3;
    }
    else
    {
      self->frame = 1;
    }
    self->flip = NOFLIP;
    self->x += self->xspeed;
  }

  // D-Pad Left - Turn Left
  if(gb.buttons.repeat(BTN_LEFT, 1) && !self->flag[JUMP])
  {
    if(self->xspeed == 4)
    {
      self->frame = 3;
    }
    else
    {
      self->frame = 1;
    }
    self->flip = FLIPH;
    self->x -= self->xspeed;
  }

  // Button B - Jump
  if(gb.buttons.pressed(BTN_B))
  {
    playSound(sndJump, 0);
  }
  if(gb.buttons.repeat(BTN_B, 1))
  {
    if(!self->flag[JUMPCTR])
    {
      self->flag[JUMPCTR] = JUMPCOOLDOWN;
      self->flag[JUMP] = JUMPDURATION;
    }
  }
  else
  {
    self->flag[JUMP] = 0;
  }

  // Button A - Power Slide
  if(gb.buttons.pressed(BTN_A))
  {
    playSound(sndSlide, 0);
  }
  if(gb.buttons.repeat(BTN_A, 1) && !self->flag[JUMP])
  {
    self->xspeed = 4;
  }
  else
  {
    self->xspeed = 2;
  }

  // Button C - Pause
  if(gb.buttons.pressed(BTN_C))
  {
    playSound(sndPause, 0);
    pauseGame();
  }

  // Jumping logic
  if(self->flag[JUMP])
  {
    self->flag[JUMP]--;
    self->frame = 2;
  }
  else
  {
    self->flag[JUMP] = 0;
  }

  if(self->flag[JUMPCTR])
  {
    self->flag[JUMPCTR]--;
  }

  // Keep player within map bounds
  if(self->x < XLIMITL)
  {
    self->x = XLIMITL;
  }

  if(self->x > XLIMITR)
  {
    self->x = XLIMITR;
  }

  // Refresh obstacles and warp player back to top
  if(self->y > SCROLLPOS)
  {
    clearObstacles();
    self->y = YSTARTPOS;
    spawnObstacles();
  }

  // Update meters traveled and draw score text
  metersTraveled += self->yspeed;
  drawScore();
}

void thinkGameOver(Entity *self)
{
  self->flag[GAMEOVER]--;
  self->frame = 2;
  self->rotation = ROT180;

  drawScore();
  
  gb.display.cursorY = self->y - cameraY - 5;
  gb.display.cursorX = self->x - cameraX;

  if(self->flag[PLAYERTXT] == 0)
  {
    playSound(sndCrash, 0);
    self->flag[PLAYERTXT] = random(9) + 1;
  }

  switch(self->flag[PLAYERTXT])
  {
    case 1: gb.display.print(F("Ooof!"));
      break;
    case 2: gb.display.print(F("Medic!"));
      break;
    case 3: gb.display.print(F("Ouch."));
      break;
    case 4: gb.display.print(F("Doh!"));
      break;
    case 5: gb.display.print(F("My legs!"));
      break;
    case 6: gb.display.print(F("My back!"));
      break;
    case 7: gb.display.print(F("...help"));
      break;
    case 8: gb.display.print(F("Why me?"));
      break;
    case 9: gb.display.print(F("My neck!"));
      break;
  }
  
  if(self->flag[GAMEOVER] <= 0)
  {
    restartGame();
  }
}

// ######################################################
//    Obstacle Functions
// ######################################################

// Spawns random obstacles
void spawnObstacles()
{
  for(int i = 0; i < 10; i++)
  {
    spawnLargeTree(random(MAPWIDTH), random(LCDHEIGHT, MAPHEIGHT - LCDHEIGHT - 16));
    spawnSmallTree(random(MAPWIDTH), random(LCDHEIGHT, MAPHEIGHT - LCDHEIGHT - 16));
  }

  for(int j = 0; j < 10; j++)
  {
    spawnLog(random(MAPWIDTH), random(LCDHEIGHT, MAPHEIGHT - LCDHEIGHT - 16));
  }
}

// Frees all obstacles in the EntityList[]
void clearObstacles()
{
  for(int i = 0; i < MAXENTS; i++)
  {
    if(EntityList[i].type == OBSTACLE)
    {
      freeEntity(&EntityList[i]);
    }
  }
}

// Trees
Entity* spawnLargeTree(int x, int y)
{
  Entity *self = spawnEntity();
  if (self == NULL) return NULL;

  self->think = treeThink;
  self->x = x;
  self->y = y;
  self->type = OBSTACLE;
  self->frame = 0;
  self->flip = NOFLIP;
  self->rotation = NOROT;
  self->flag[0] = 0;

  switch(random(4))
  {
    case 0: self->sprite[0] = largeTree1;
      break;
    case 1: self->sprite[0] = largeTree2;
      break;
    case 2: self->sprite[0] = largeTree3;
      break;
    case 3: self->sprite[0] = largeTree4;
      break;
  }

  return self;
}

Entity* spawnSmallTree(int x, int y)
{
  Entity *self = spawnEntity();
  if(self == NULL) return NULL;

  self->think = treeThink;
  self->x = x;
  self->y = y;
  self->type = OBSTACLE;
  self->frame = 0;
  self->flip = NOFLIP;
  self->rotation = NOROT;
  self->flag[0] = 0;

  switch(random(4))
  {
    case 0: self->sprite[0] = smallTree1;
      break;
    case 1: self->sprite[0] = smallTree2;
      break;
    case 2: self->sprite[0] = smallTree3;
      break;
    case 3: self->sprite[0] = smallTree4;
      break;
  }

  return self;
}

void treeThink(Entity *self)
{
  if(gb.collideBitmapBitmap(player->x, player->y, player->sprite[player->frame], self->x, self->y, self->sprite[self->frame]))
  {
    player->think = thinkGameOver;
  }
}

// Logs
Entity* spawnLog(int x, int y)
{
  Entity *self = spawnEntity();
  if(self == NULL) return NULL;

  self->think = logThink;
  self->x = x;
  self->y = y;
  self->type = OBSTACLE;
  self->frame = 0;
  self->flip = NOFLIP;
  self->rotation = NOROT;
  self->flag[0] = 0;

  switch(random(4))
  {
    case 0: self->sprite[0] = log1;
      break;
    case 1: self->sprite[0] = log2;
      break;
    case 2: self->sprite[0] = log3;
      break;
    case 3: self->sprite[0] = log4;
      break;
  }

  return self;
}

void logThink(Entity *self)
{
  if(player->flag[JUMP]) return;

  if(gb.collideBitmapBitmap(player->x, player->y, player->sprite[player->frame], self->x, self->y, self->sprite[self->frame]))
  {
    player->think = thinkGameOver;
  }
}

