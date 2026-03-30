// MageQuest - A JRPG for PocketMage PDA
// Chrono Trigger / Final Fantasy inspired, 1-bit E-Ink optimized
// OTA App by AEB-DevLabs (2026)
// License: Apache-2.0
//
// Controls:
//   Explore: WASD to move, Enter near enemy to engage
//   Battle:  W/S to navigate menu, Enter to confirm action
//   Slider (capacitive touch): Tech special attack (area damage)

#include <globals.h>
#if OTA_APP

// ===================== CONSTANTS =====================
#define MAP_W     20
#define MAP_H     15
#define TILE_S    16
#define MAX_PARTY  2
#define MAX_ENEMIES 3

// Tile types
#define T_GRASS  0
#define T_WALL   1
#define T_TREE   2
#define T_HOUSE  3
#define T_SIGN   4
#define T_PATH   5
#define T_ENEMY  6
#define T_STONE  7

// Game modes
enum GameMode { EXPLORE, BATTLE };
enum TurnPhase { SELECT, ANIMATE, ENEMY_TURN, WIN, GAME_OVER };

// ===================== SPRITES (16x16 1-bit bitmaps) =====================
// Mage sprite - wizard with staff
static const unsigned char spr_mage[] PROGMEM = {
  0x00,0x00, 0x03,0xC0, 0x07,0xE0, 0x0F,0xF0,
  0x07,0xE0, 0x03,0xC0, 0x07,0xE0, 0x0F,0xF0,
  0x0F,0xF0, 0x0F,0xF8, 0x07,0xE0, 0x07,0xE0,
  0x03,0xC0, 0x03,0xC0, 0x06,0x60, 0x06,0x60
};

// Warrior sprite - knight with sword
static const unsigned char spr_warrior[] PROGMEM = {
  0x00,0x00, 0x07,0xC0, 0x0F,0xE0, 0x0F,0xE0,
  0x07,0xC0, 0x07,0xE0, 0x0F,0xF0, 0x1F,0xF0,
  0x1F,0xF8, 0x0F,0xF0, 0x0F,0xF0, 0x07,0xE0,
  0x07,0xE0, 0x03,0xC0, 0x06,0x60, 0x0E,0x70
};

// Slime sprite
static const unsigned char spr_slime[] PROGMEM = {
  0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,
  0x03,0xC0, 0x07,0xE0, 0x0F,0xF0, 0x1F,0xF8,
  0x1F,0xF8, 0x3B,0xDC, 0x3F,0xFC, 0x3F,0xFC,
  0x3F,0xFC, 0x1F,0xF8, 0x0F,0xF0, 0x07,0xE0
};

// Goblin sprite
static const unsigned char spr_goblin[] PROGMEM = {
  0x00,0x00, 0x04,0x20, 0x0E,0x70, 0x0F,0xF0,
  0x0F,0xF0, 0x05,0xA0, 0x07,0xE0, 0x0F,0xF0,
  0x1F,0xF8, 0x1F,0xF8, 0x0F,0xF0, 0x0F,0xF0,
  0x07,0xE0, 0x03,0xC0, 0x06,0x60, 0x06,0x60
};

// ===================== DATA STRUCTURES =====================
struct Character {
  String name;
  int hp, maxHp;
  int mp, maxMp;
  int atk, def, spd;
  int x, y;  // Screen position in battle, tile position in explore
  const unsigned char* sprite;
  String techs[2];
  bool alive;
};

struct Enemy {
  String name;
  int hp, maxHp;
  int atk, def;
  int x, y;
  const unsigned char* sprite;
  bool alive;
};

struct FieldEnemy {
  int x, y;
  bool alive;
  uint8_t type; // 0=slime, 1=goblin, 2=imp
};

// ===================== GAME STATE =====================
static GameMode gameMode;
static TurnPhase turnPhase;
static Character party[MAX_PARTY];
static Enemy enemies[MAX_ENEMIES];
static int numEnemies;
static int currentTurn;
static int battleCursor;
static int battleSubMenu; // -1=main, 0=magic
static String battleLog;
static int playerX, playerY;
static FieldEnemy fieldEnemies[3];
static int damageDisplay;
static int damageTimer;
static int damageX, damageY;

// Map data (20x15 tiles)
static const uint8_t mapData[MAP_H][MAP_W] PROGMEM = {
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,1,0,0,0,0,0,0,0,2,2,2,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,2,2,0,1},
  {1,0,0,3,0,0,0,0,0,4,0,0,2,2,2,2,2,2,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,2,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,0,0,0,1},
  {1,0,0,0,0,0,5,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,6,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,1,1,0,0,1,1,1,0,0,0,0,0,0,7,7,7,7,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,7,7,0,0,7,0,1},
  {1,0,3,0,0,0,3,0,0,0,0,0,0,7,0,0,0,7,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,7,7,7,7,7,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

// ===================== INITIALIZATION =====================
static void initParty() {
  party[0] = {"Arion", 45, 45, 12, 12, 14, 8, 10, 0, 0, spr_mage, {"Fire", "Heal"}, true};
  party[1] = {"Kael",  55, 55,  6,  6, 18, 12, 8, 0, 0, spr_warrior, {"Slash", "Guard"}, true};
}

static void initFieldEnemies() {
  fieldEnemies[0] = {9, 3, true, 0};  // Slime
  fieldEnemies[1] = {11, 7, true, 1}; // Goblin
  fieldEnemies[2] = {5, 6, true, 1};  // Goblin
}

static void startBattle(uint8_t enemyType) {
  gameMode = BATTLE;
  turnPhase = SELECT;
  currentTurn = 0;
  battleCursor = 0;
  battleSubMenu = -1;
  damageTimer = 0;

  numEnemies = 0;
  if (enemyType == 0) { // Slimes
    enemies[0] = {"Slime", 18, 18, 8, 4, 180, 50, spr_slime, true};
    enemies[1] = {"Slime", 18, 18, 8, 4, 230, 80, spr_slime, true};
    numEnemies = 2;
  } else if (enemyType == 1) { // Goblins
    enemies[0] = {"Goblin", 22, 22, 10, 3, 170, 45, spr_goblin, true};
    enemies[1] = {"Goblin", 22, 22, 10, 3, 240, 75, spr_goblin, true};
    numEnemies = 2;
  }

  // Battle positions for party
  party[0].x = 60; party[0].y = 90;
  party[1].x = 40; party[1].y = 120;

  battleLog = "Enemies appear!";
  newState = true;
}

// ===================== BATTLE LOGIC =====================
static int findAliveEnemy() {
  for (int i = 0; i < numEnemies; i++)
    if (enemies[i].alive) return i;
  return -1;
}

static void doBattleAction(const String& action, int target) {
  Character& attacker = party[currentTurn];
  if (target < 0 || !enemies[target].alive) {
    target = findAliveEnemy();
    if (target < 0) return;
  }
  Enemy& t = enemies[target];
  int dmg = 0;

  if (action == "Attack") {
    dmg = max(1, attacker.atk - t.def + (millis() % 4));
    t.hp -= dmg;
    battleLog = attacker.name + " attacks! " + String(dmg) + " dmg";
  }
  else if (action == "Fire" && attacker.mp >= 4) {
    attacker.mp -= 4;
    dmg = max(1, attacker.atk + 6 - t.def);
    t.hp -= dmg;
    battleLog = "Fire! " + String(dmg) + " dmg!";
  }
  else if (action == "Heal" && attacker.mp >= 3) {
    attacker.mp -= 3;
    int heal = 10 + (millis() % 5);
    attacker.hp = min(attacker.maxHp, attacker.hp + heal);
    battleLog = "Heal! +" + String(heal) + " HP";
  }
  else if (action == "Slash") {
    dmg = max(1, (attacker.atk * 3 / 2) - t.def);
    t.hp -= dmg;
    battleLog = "Slash! " + String(dmg) + " dmg!";
  }
  else if (action == "Slider") {
    // Tech special: area damage via capacitive touch slider
    dmg = max(1, attacker.atk * 2 - t.def + (millis() % 8));
    t.hp -= dmg;
    // Splash damage to all enemies
    for (int i = 0; i < numEnemies; i++) {
      if (enemies[i].alive && i != target) {
        int splash = dmg / 3;
        enemies[i].hp -= splash;
        if (enemies[i].hp <= 0) { enemies[i].hp = 0; enemies[i].alive = false; }
      }
    }
    battleLog = "TECH! " + String(dmg) + " massive dmg!!";
  }
  else if (action == "Defend") {
    battleLog = attacker.name + " defends.";
  }

  // Show damage number
  if (dmg > 0) {
    damageDisplay = dmg;
    damageX = t.x + 8;
    damageY = t.y;
    damageTimer = 1; // Will show on next draw
  }

  // Check enemy death
  if (t.hp <= 0) {
    t.hp = 0;
    t.alive = false;
    battleLog += " " + t.name + " defeated!";
  }

  // Check win
  if (findAliveEnemy() < 0) {
    turnPhase = WIN;
    battleLog = "Victory!";
    newState = true;
    return;
  }

  // Advance turn
  currentTurn++;
  if (currentTurn >= MAX_PARTY) {
    // Enemy turn
    for (int i = 0; i < numEnemies; i++) {
      if (!enemies[i].alive) continue;
      int tgt = millis() % MAX_PARTY;
      int edm = max(1, enemies[i].atk - party[tgt].def + (millis() % 3));
      party[tgt].hp = max(0, party[tgt].hp - edm);
      battleLog = enemies[i].name + " attacks " + party[tgt].name + "! " + String(edm) + " dmg";
      if (party[tgt].hp <= 0) {
        party[tgt].alive = false;
        if (!party[0].alive && !party[1].alive) {
          turnPhase = GAME_OVER;
          battleLog = "Game Over... BKSP to restart";
          newState = true;
          return;
        }
      }
    }
    currentTurn = 0;
  }

  battleCursor = 0;
  battleSubMenu = -1;
  newState = true;
}

// ===================== DRAWING: EXPLORE =====================
static void drawExplore() {
  EINK().resetDisplay();

  int camX = max(0, min(playerX - 9, MAP_W - 20));
  int camY = max(0, min(playerY - 7, MAP_H - 15));

  for (int ty = 0; ty < 15; ty++) {
    for (int tx = 0; tx < 20; tx++) {
      int mx = tx + camX, my = ty + camY;
      int sx = tx * TILE_S, sy = ty * TILE_S;
      uint8_t tile = pgm_read_byte(&mapData[my][mx]);

      if (tile == T_WALL) {
        display.fillRect(sx, sy, TILE_S, TILE_S, GxEPD_BLACK);
        // Brick pattern
        for (int i = 0; i < TILE_S; i += 4) {
          display.drawPixel(sx + i, sy + 2, GxEPD_WHITE);
          display.drawPixel(sx + i + 2, sy + 6, GxEPD_WHITE);
          display.drawPixel(sx + i, sy + 10, GxEPD_WHITE);
          display.drawPixel(sx + i + 2, sy + 14, GxEPD_WHITE);
        }
      }
      else if (tile == T_GRASS) {
        // Grass dots
        display.drawPixel(sx + 3, sy + 4, GxEPD_BLACK);
        display.drawPixel(sx + 8, sy + 7, GxEPD_BLACK);
        display.drawPixel(sx + 1, sy + 12, GxEPD_BLACK);
      }
      else if (tile == T_TREE) {
        // Tree: dithered canopy + trunk
        for (int dy = 0; dy < TILE_S; dy++)
          for (int dx = 0; dx < TILE_S; dx++)
            if ((dx + dy) % 3 == 0) display.drawPixel(sx + dx, sy + dy, GxEPD_BLACK);
        display.fillRect(sx + 6, sy + 10, 4, 6, GxEPD_BLACK);
      }
      else if (tile == T_HOUSE) {
        display.fillRect(sx + 2, sy + 4, 12, 10, GxEPD_BLACK);
        display.fillRect(sx + 4, sy + 2, 8, 2, GxEPD_BLACK);
        display.fillRect(sx + 4, sy + 6, 8, 6, GxEPD_WHITE);
      }
      else if (tile == T_SIGN) {
        display.fillRect(sx + 6, sy + 8, 4, 6, GxEPD_BLACK);
        display.fillRect(sx + 2, sy + 2, 12, 8, GxEPD_BLACK);
        display.fillRect(sx + 4, sy + 4, 8, 4, GxEPD_WHITE);
      }
      else if (tile == T_STONE) {
        display.fillRect(sx, sy + 12, TILE_S, 4, GxEPD_BLACK);
      }
      // T_PATH and T_ENEMY: just grass with markers
    }
  }

  // Draw field enemies
  for (int i = 0; i < 3; i++) {
    if (!fieldEnemies[i].alive) continue;
    int sx = (fieldEnemies[i].x - camX) * TILE_S;
    int sy = (fieldEnemies[i].y - camY) * TILE_S;
    const unsigned char* spr = (fieldEnemies[i].type == 0) ? spr_slime : spr_goblin;
    display.drawBitmap(sx, sy, spr, 16, 16, GxEPD_BLACK);
  }

  // Draw player
  display.drawBitmap((playerX - camX) * TILE_S, (playerY - camY) * TILE_S, spr_mage, 16, 16, GxEPD_BLACK);

  // Bottom bar
  display.fillRect(0, 216, 320, 24, GxEPD_BLACK);
  display.setFont(&Font5x7Fixed);
  display.setTextColor(GxEPD_WHITE);
  display.setCursor(6, 232);
  display.print("Verdant Fields    WASD:Move  Enter:Battle");

  EINK().forceSlowFullUpdate(true);
  EINK().refresh();
}

// ===================== DRAWING: BATTLE =====================
static void drawBattle() {
  EINK().resetDisplay();

  // Background: field with dithered grass
  for (int y = 20; y < 155; y += 2)
    for (int x = 0; x < 320; x += 3)
      if ((x + y) % 6 < 2) display.drawPixel(x, y, GxEPD_BLACK);

  // Top trees
  for (int x = 0; x < 320; x += 8) {
    int h = 4 + (x % 16 < 8 ? 3 : -1);
    display.fillRect(x, 0, 6, 12 + h, GxEPD_BLACK);
  }

  // Ground line
  display.drawFastHLine(0, 155, 320, GxEPD_BLACK);

  // Draw enemies
  for (int i = 0; i < numEnemies; i++) {
    if (!enemies[i].alive) continue;
    display.drawBitmap(enemies[i].x, enemies[i].y, enemies[i].sprite, 16, 16, GxEPD_BLACK);
  }

  // Draw party
  display.drawBitmap(party[0].x, party[0].y, party[0].sprite, 16, 16, GxEPD_BLACK);
  display.drawBitmap(party[1].x, party[1].y, party[1].sprite, 16, 16, GxEPD_BLACK);

  // Damage number
  if (damageTimer > 0) {
    display.setFont(&FreeSerif9pt7b);
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(damageX, damageY - 4);
    display.print(String(damageDisplay));
    damageTimer = 0;
  }

  // Battle UI box
  display.fillRect(0, 162, 320, 78, GxEPD_BLACK);
  display.fillRect(2, 164, 316, 74, GxEPD_WHITE);

  // Current turn indicator
  display.setFont(&Font5x7Fixed);
  display.setTextColor(GxEPD_BLACK);
  String who = (currentTurn < MAX_PARTY) ? party[currentTurn].name : "Enemy";
  display.setCursor(8, 174);
  display.print(who + "'s turn");

  // Menu
  if (turnPhase == SELECT) {
    const char* mainMenu[] = {"Attack", "Magic", "Item", "Defend"};
    const char* magicMenu[] = {"Fire", "Heal"};
    const char** items = (battleSubMenu == 0) ? magicMenu : mainMenu;
    int count = (battleSubMenu == 0) ? 2 : 4;

    for (int i = 0; i < count; i++) {
      int by = 180 + i * 14;
      if (i == battleCursor) {
        display.fillRect(8, by - 2, 4, 8, GxEPD_BLACK); // Cursor arrow
      }
      display.setCursor(16, by + 6);
      display.print(items[i]);
    }
  }

  // Party HP display (right side)
  for (int i = 0; i < MAX_PARTY; i++) {
    int by = 170 + i * 16;
    display.setCursor(220, by + 8);
    display.print(party[i].name);
    display.setCursor(270, by + 8);
    display.print(String(party[i].hp) + "/" + String(party[i].maxHp));
    // HP bar
    int barW = (party[i].hp * 40) / party[i].maxHp;
    display.drawRect(220, by + 10, 42, 3, GxEPD_BLACK);
    display.fillRect(220, by + 10, barW, 3, GxEPD_BLACK);
  }

  // Battle log
  display.setCursor(8, 236);
  display.print(battleLog);

  EINK().forceSlowFullUpdate(true);
  EINK().refresh();
}

// ===================== OLED STATUS =====================
static void drawOLED() {
  if (gameMode == EXPLORE) {
    String l1 = "Arion HP:" + String(party[0].hp) + " MP:" + String(party[0].mp);
    String l2 = "Kael  HP:" + String(party[1].hp) + " MP:" + String(party[1].mp);
    OLED().oledLine(l1, 0, false, l2);
  } else {
    String who = (currentTurn < MAX_PARTY) ? party[currentTurn].name : "Enemy";
    const char* menuNames[] = {"Attack", "Magic", "Item", "Defend"};
    String l1 = who + " > " + menuNames[battleCursor] + " | Slider=Tech!";
    String l2 = "HP:" + String(party[0].hp) + " MP:" + String(party[0].mp) +
                "  HP:" + String(party[1].hp) + " MP:" + String(party[1].mp);
    OLED().oledLine(l1, 0, false, l2);
  }
}

// ===================== OTA APP ENTRY POINTS =====================
void APP_INIT() {
  gameMode = EXPLORE;
  playerX = 7;
  playerY = 5;
  initParty();
  initFieldEnemies();
  KB().setKeyboardState(NORMAL);
  newState = true;
}

void processKB_APP() {
  int currentMillis = millis();
  if (currentMillis - KBBounceMillis < KB_COOLDOWN) return;

  char inchar = KB().updateKeypress();
  if (inchar == 0) return;
  KBBounceMillis = currentMillis;

  if (gameMode == EXPLORE) {
    int nx = playerX, ny = playerY;

    if (inchar == 'w' || inchar == 'W') ny--;
    else if (inchar == 's' || inchar == 'S') ny++;
    else if (inchar == 'a' || inchar == 'A') nx--;
    else if (inchar == 'd' || inchar == 'D') nx++;
    else if (inchar == 13) { // Enter: interact/battle
      for (int i = 0; i < 3; i++) {
        if (fieldEnemies[i].alive &&
            abs(fieldEnemies[i].x - playerX) <= 1 &&
            abs(fieldEnemies[i].y - playerY) <= 1) {
          startBattle(fieldEnemies[i].type);
          return;
        }
      }
    }
    else if (inchar == 8) return; // BKSP: future home exit

    // Validate movement
    if (nx >= 0 && nx < MAP_W && ny >= 0 && ny < MAP_H) {
      uint8_t tile = pgm_read_byte(&mapData[ny][nx]);
      if (tile != T_WALL && tile != T_STONE) {
        playerX = nx;
        playerY = ny;
        // Auto-battle on contact
        for (int i = 0; i < 3; i++) {
          if (fieldEnemies[i].alive && fieldEnemies[i].x == playerX && fieldEnemies[i].y == playerY) {
            startBattle(fieldEnemies[i].type);
            return;
          }
        }
        newState = true;
      }
    }
  }
  else if (gameMode == BATTLE) {
    if (turnPhase == GAME_OVER && inchar == 8) {
      // Restart
      initParty();
      gameMode = EXPLORE;
      newState = true;
      return;
    }
    if (turnPhase == WIN && (inchar == 13 || inchar == 8)) {
      // Return to explore
      gameMode = EXPLORE;
      for (int i = 0; i < 3; i++) {
        if (fieldEnemies[i].alive &&
            abs(fieldEnemies[i].x - playerX) <= 1 &&
            abs(fieldEnemies[i].y - playerY) <= 1) {
          fieldEnemies[i].alive = false;
        }
      }
      // Restore party def
      party[0].def = 8;
      party[1].def = 12;
      newState = true;
      return;
    }

    if (turnPhase != SELECT) return;

    int menuSize = (battleSubMenu == 0) ? 2 : 4;

    if (inchar == 'w' || inchar == 'W') {
      battleCursor = (battleCursor - 1 + menuSize) % menuSize;
    }
    else if (inchar == 's' || inchar == 'S') {
      battleCursor = (battleCursor + 1) % menuSize;
    }
    else if (inchar == 'a' || inchar == 'A') {
      if (battleSubMenu >= 0) { battleSubMenu = -1; battleCursor = 0; }
    }
    else if (inchar == 13) { // Enter: confirm
      if (battleSubMenu == -1) {
        switch (battleCursor) {
          case 0: doBattleAction("Attack", 0); break;
          case 1: battleSubMenu = 0; battleCursor = 0; break;
          case 2: { // Item: use potion
            int heal = 15;
            party[currentTurn].hp = min(party[currentTurn].maxHp, party[currentTurn].hp + heal);
            battleLog = "Used Potion! +" + String(heal) + " HP";
            currentTurn++;
            if (currentTurn >= MAX_PARTY) { currentTurn = 0; /* enemy turn handled in doBattleAction */ }
            battleCursor = 0;
            newState = true;
            break;
          }
          case 3: doBattleAction("Defend", 0); break;
        }
      } else {
        // Magic submenu
        if (battleCursor == 0) doBattleAction("Fire", 0);
        else if (battleCursor == 1) doBattleAction("Heal", 0);
        battleSubMenu = -1;
      }
    }
    // Touch slider: Tech special attack
    // Note: On real hardware, detect slider touch via the capacitive sensor
    // For keyboard fallback, use 'e' key
    else if (inchar == 'e' || inchar == 'E') {
      doBattleAction("Slider", 0);
    }

    newState = true;
  }

  // Update OLED
  currentMillis = millis();
  if (currentMillis - OLEDFPSMillis >= (1000 / OLED_MAX_FPS)) {
    OLEDFPSMillis = currentMillis;
    drawOLED();
  }
}

void einkHandler_APP() {
  if (newState) {
    newState = false;
    if (gameMode == EXPLORE) drawExplore();
    else drawBattle();
  }
}

#endif
